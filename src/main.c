#include <asm/types.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "keyboard_events.h"
#include "splits.h"
#include "timing.h"

#include "user/config.h"

int main(int argc, char** argv) {
    /* Variables that could be set by argument parsing */
    char* key_event_path = NULL;
    __u16 toggle_key = DEFAULT_TOGGLE_KEY;
    FILE* split_file;
    int split_amount = 0;
    struct split splits[MAX_SPLITS] = {0};

    /* Argument parsing */
    int opt;
    while ((opt = getopt(argc, argv, ":f:k:s")) != -1)
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
            case 's':
                if (!optarg)
                    split_amount = getSplitsFromInput(splits);
                else {
                    split_file = fopen(optarg, "rw");
                    if (!split_file)
                        goto fopen_err;
                    split_amount = fread(splits, sizeof(struct split), MAX_SPLITS, split_file);
                    if (!split_amount)
                        fputs("No splits could be read from the file\n", stderr);
                }
                break;
            case ':':
                fprintf(stderr, "Option %c requires an argument\n", optopt);
            default:
                break;
        }

    // TEMPORARY
    for (int i = 0; i < split_amount; i++)
        printf("%s\n", splits[i].name);

    if (!key_event_path) { // No custom file path given
        key_event_path = getKeyEventFile();
        if (!key_event_path) {
            fputs("Error finding the keyboard event file.\n"
                    "Please specify the file by adding the arguments "
                    "\"-f /path/to/event_file\"\n", stderr);
            return errno;
        }
    }
    FILE* key_event_fp = fopen(key_event_path, "r");
    if (!key_event_fp)
        goto fopen_err;

    /* Set up for starting the timer */
    int return_value = EXIT_SUCCESS;
    struct timespec* current_time = malloc(sizeof(struct timespec));
    if (!current_time) {
        perror("malloc");
        return_value = errno;
        goto end_no_free;
    }

    // Print with no newline
    fputs("0:00", stdout);
    fflush(stdout);

    /* Wait until enter key pressed to start the timer */
    int toggleKeyPressed = false;

    while (!toggleKeyPressed) {
        toggleKeyPressed = keyPressed(toggle_key, key_event_fp, current_time);
        if (toggleKeyPressed == -1) {
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
            return_value = errno;
            break;
        }
    }
    /* Stop the timer */
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
end_no_free:
    fclose(key_event_fp);
    return return_value;

fopen_err:
    perror("fopen");
    return errno;
}
