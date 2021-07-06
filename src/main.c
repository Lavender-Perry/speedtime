#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "keyboard_events.h"
#include "timing.h"


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

    // Print with no newline
    fputs("0:00.00", stdout);
    fflush(stdout);

    /* Wait until enter key pressed to start the timer */
    const char* key_err_msg = "Error getting key press info";
    // Give key_event_fp to function, enterPressed will be false
    int enterPressed = false;

    while (!enterPressed) {
        enterPressed = enterKeyPressed(key_event_fp, current_time);
        if (enterPressed < 0) {
            perror(key_err_msg);
            return_value = errno;
            goto program_end;
        }
    }

    /* Start the timer */
    printTime(NULL, current_time); // Give start time to printTime()

    pthread_t timer_thread_id;
    bool do_thread = true;
    if (pthread_create(&timer_thread_id, NULL, timer, &do_thread)) {
        fputs("Error creating timer thread\n", stderr);
        goto program_end;
    }

    /* Wait until enter key pressed to stop the timer */
    enterPressed = false;
    while (!enterPressed) {
        /* End if any timer thread errors (errors in timer thread will set errno) */
        if (errno) {
            return_value = errno;
            goto program_end;
        }
        /* Check if the enter key is pressed & handle errors */
        enterPressed = enterKeyPressed(key_event_fp, current_time);
        if (enterPressed < 0) {
            perror(key_err_msg);
            return_value = errno;
            break;
        }
    }
    do_thread = false;
    if (pthread_join(timer_thread_id, NULL) == -1) {
        perror("pthread_join");
        return_value = errno;
    }
    if (enterPressed != -2) // Don't print time if there was a time error
        printTime(current_time, NULL);
    else
        fputs("Error getting the time that the timer was stopped at.\n"
                "The printed time is most likely NOT accurate!", stdout);
    puts(""); // Print newline

program_end:
    free(current_time);
    fclose(key_event_fp);
    return return_value;
}
