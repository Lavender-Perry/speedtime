#include <asm/types.h>
#include <errno.h>
#include <fcntl.h>
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
#include "utils.h"

#include "user/config.h"

int main(int argc, char** argv) {
    /* Variables that could be set by argument parsing */
    char* key_event_path = NULL;
    __u16 toggle_key = DEFAULT_TOGGLE_KEY;
    FILE* split_file = NULL;
    int split_amount = 0;
    struct split splits[MAX_SPLITS];
    bool run_with_splits = false;

    /* Argument parsing */
    int opt;
    while ((opt = getopt(argc, argv, "f:k:l:s")) != -1)
        switch (opt) {
            case 'f': // Set path to file for monitoring key events
                key_event_path = optarg;
                break;
            case 'k': // Set key to toggle the timer
                toggle_key = atoi(optarg);
                if (!toggle_key) {
                    fputs("Invalid key code specified, using default key\n", stderr);
                    toggle_key = DEFAULT_TOGGLE_KEY;
                }
                break;
            case 'l': // Load splits from file specified by optarg
                split_file = fopen(optarg, "rw"); // TODO: add a prefix
                if (!split_file)
                    goto fopen_err;
                split_amount = fread(splits,
                        sizeof(struct split), MAX_SPLITS, split_file);
                // Fallthrough
            case 's': // Create new splits
                if (opt != 'l') // Making sure it is not run by 'l' with fallthrough
                    split_amount = getSplitsFromInput(splits);
                if (split_amount > 0)
                    run_with_splits = true;
                else
                    fputs("No splits could be read\n", stderr);
                // Fallthrough
            default:
                break;
        }

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
    struct timespec current_time;

    // Print with no newline
    fputs("0:00", stdout);
    fflush(stdout);

    /* Wait until enter key pressed to start the timer */
    int toggleKeyPressed = false;

    while (!toggleKeyPressed) {
        toggleKeyPressed = keyPressed(toggle_key, key_event_fp, &current_time);
        if (toggleKeyPressed == -1) {
            return_value = errno;
            goto program_end;
        }
    }

    /* Start the timer */
    printTime(NULL, &current_time, false); // Give start time to printTime()

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
        toggleKeyPressed = keyPressed(toggle_key, key_event_fp, &current_time);
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
    printTime(&current_time, NULL, true);
    puts(""); // Print newline
    if (return_value)
        puts("The printed time is most likely NOT accurate!");

program_end:
    fclose(key_event_fp);
    if (run_with_splits) {
        if (split_file) {
            // TODO: update best times in split file if they improve
            fclose(split_file);
        } else {
            char split_name[30];
            puts("Please enter the name you would like to save the splits as.\n"
                    "Or enter \"cancel\" (without quotes) to not save the splits.");
            do if (fgets_no_newline(split_name, sizeof(split_name), stdin) == NULL) {
                fputs("Error reading your input.\n", stderr);
                return ERROR_STATUS;
            } while (split_name[0] == '\0');
            if (strcmp(split_name, "cancel")) { // "cancel" not read
                // Create the split file, failing if it already exists
                const int fd = open(split_name, // TODO: add a prefix
                        O_CREAT | O_WRONLY | O_EXCL,
                        S_IRUSR | S_IWUSR);
                if (fd < 0) { // Failure
                    perror("open");
                    return errno;
                }
                // Save splits to file
                const ssize_t write_amount = sizeof(struct split) * split_amount;
                if (write(fd, splits, write_amount) == write_amount)
                    puts("Splits successfully saved.");
                else {
                    fputs("Error saving splits.\n", stderr);
                    return ERROR_STATUS;
                }
            }
        }
    }
    return return_value;

fopen_err:
    perror("fopen");
    return errno;
}
