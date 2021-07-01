#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "keyboard_events.h"
#include "timing.h"
#include "utils.h"

int main(const int argc, char** argv) {
    /* Find & open the key event handler file,
     * storing the file pointer in key_event_fp. */
    char* key_event_path = NULL;
    bool free_key_event_path = false;
    // Set key_event_path to a custom file path if one is given
    for (int i = 2; i < argc; i++)
        if (!strcmp(argv[i - 1], "-k")) {
            key_event_path = argv[i];
            break;
        }
    if (!key_event_path) { // No custom file path given
        key_event_path = getKeyEventFile();
        if (!key_event_path) {
            fputs("Error finding the keyboard event file.\n"
                    "Please specify the file by adding the arguments "
                    "\"-k /path/to/event_file\"\n", stderr);
            return errno ? errno : EXIT_FAILURE;
        }
        free_key_event_path = true;
    }
    FILE* key_event_fp = fopen(key_event_path, "r");
    if (free_key_event_path)
        free(key_event_path);
    if (!key_event_fp) {
        perror("fopen");
        return errno;
    }

    /* Set up for starting the timer */
    int return_value = EXIT_SUCCESS;
    struct timespec* current_time = malloc(sizeof(struct timespec));
    if (!current_time) {
        perror("malloc");
        free(key_event_fp);
        return errno;
    }
    putsNoNewline("0:00.00");

    /* Wait until enter key pressed to start the timer */
    int enterPressed = 0;
    while (!enterPressed) {
        enterPressed = enterKeyPressed(key_event_fp, current_time);
        if (enterPressed == -1) {
event_read_err:
            fputs("Error reading keyboard event\n", stderr);
            return_value = EXIT_FAILURE;
            goto program_end;
        }
    }
    if (current_time->tv_sec == 0) {
time_err:
        perror("clock_gettime");
        return_value = errno;
        goto program_end;
    }

    /* Start the timer */
    printTime(NULL, current_time); // Give start time to printTime()

    pthread_t timer_thread_id;
    pthread_mutex_t thread_mtx = PTHREAD_MUTEX_INITIALIZER;
    struct threadInfo timer_arg = {
        .do_thread = true,
        .mtx_ptr = &thread_mtx,
    };
    if (pthread_create(&timer_thread_id, NULL, timer, &timer_arg)) {
        fputs("Error creating timer thread\n", stderr);
        goto program_end;
    }

    while (true) {
        /* Check if the enter key is pressed & stop the timer if it is */
        enterPressed = enterKeyPressed(key_event_fp, current_time);
        if (enterPressed == -1)
            goto event_read_err;
        if (current_time->tv_sec == 0)
            goto time_err;
        if (enterPressed) {
            pthread_mutex_lock(timer_arg.mtx_ptr);
            timer_arg.do_thread = false;
            pthread_mutex_unlock(timer_arg.mtx_ptr);
            if (pthread_join(timer_thread_id, NULL) == -1) {
                perror("pthread_join");
                return_value = errno;
            }
            printTime(current_time, NULL);
            puts(""); // Print newline
            break;
        }
    }

program_end:
    free(current_time);
    fclose(key_event_fp);
    return return_value;
}
