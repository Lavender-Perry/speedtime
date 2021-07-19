#include <asm/types.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "user/config.h"
#include "keyboard_events.h"
#include "timing.h"

int main(int argc, char** argv) {
    /* Variables that could be set by argument parsing */
    __u16 toggle_key = DEFAULT_TOGGLE_KEY;
    char* key_event_path = NULL;

    /* Argument parsing */
    int opt;
    while ((opt = getopt(argc, argv, ":f:k:")) != -1)
        switch (opt) {
            case 'f':
                key_event_path = optarg;
                break;
            case 'k':
                toggle_key = atoi(optarg);
                if (!toggle_key) {
                    fputs("Invalid key code specified, using default key\n", stderr);
                    toggle_key = DEFAULT_TOGGLE_KEY;
                }
                break;
            case ':':
                fprintf(stderr, "Option %c requires an argument\n", optopt);
            default:
                break;
        }

    bool free_key_event_path = false;
    if (!key_event_path) { // No custom file path given
        key_event_path = getKeyEventFile();
        if (!key_event_path) {
            fputs("Error finding the keyboard event file.\n"
                    "Please specify the file by adding the arguments "
                    "\"-f /path/to/event_file\"\n", stderr);
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
    fputs("0:00", stdout);
    fflush(stdout);

    /* Wait until enter key pressed to start the timer */
    const char* key_err_msg = "Error getting key press info";
    int toggleKeyPressed = false;

    while (!toggleKeyPressed) {
        toggleKeyPressed = keyPressed(toggle_key, key_event_fp, current_time);
        if (toggleKeyPressed == -1) {
            perror(key_err_msg);
            return_value = errno;
            goto program_end;
        }
    }

    /* Start the timer */
    printTime(NULL, current_time, false); // Give start time to printTime()

    pthread_t timer_thread_id;
    bool do_thread = true;
    if (pthread_create(&timer_thread_id, NULL, timer, &do_thread)) {
        fputs("Error creating timer thread\n", stderr);
        goto program_end;
    }

    /* Wait until key pressed to stop the timer */
    toggleKeyPressed = false;
    while (!toggleKeyPressed) {
        /* End if any timer thread errors (errors in timer thread will set errno) */
        if (errno) {
            return_value = errno;
            goto program_end;
        }
        /* Check if the key is pressed & handle errors */
        toggleKeyPressed = keyPressed(toggle_key, key_event_fp, current_time);
        if (toggleKeyPressed == -1) {
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
    printTime(current_time, NULL, true);
    puts(""); // Print newline
    if (return_value)
        puts("The printed time is most likely NOT accurate!");

program_end:
    free(current_time);
    fclose(key_event_fp);
    return return_value;
}
