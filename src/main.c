#include <asm/types.h>
#include <errno.h>
#include <linux/input-event-codes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/time.h>
#include <unistd.h>

#include "compile_settings.h"
#include "keyboard_events.h"
#include "splits.h"
#include "timing.h"

/* Sets key variable to value from optarg.
 * Used in argument parsing. */
void set_key(__u16* restrict key, char* optarg) {
    const __u16 atoi_res = atoi(optarg);
    if (!atoi_res)
        fputs("Invalid key code specified, using default key\n", stderr);
    else
        *key = atoi_res;
}

/* Argument parsing, event loop, etc. */
int main(int argc, char** argv) {
    /* Variables that could be set by argument parsing */
    char* key_event_path = NULL;
    __u16 reset_key = DEFAULT_RESET_KEY;
    __u16 timerCtrl_key = DEFAULT_CONTROL_KEY;
    FILE* split_file = NULL;
    int split_amount = 0;
    struct split splits[MAX_SPLITS];
    bool run_with_splits = false;

    /* Argument parsing */
    int opt;
    while ((opt = getopt(argc, argv, "f:r:c:l:s")) != -1)
        switch (opt) {
            case 'f': // Set path to file for monitoring key events
                key_event_path = optarg;
                break;
            case 'r': // Set key to reset the timer
                set_key(&reset_key, optarg);
                break;
            case 'c': // Set key to control the timer
                set_key(&timerCtrl_key, optarg);
                break;
            case 'l': // Load splits from file specified by optarg
                split_file = fopen(optarg, "rw"); // TODO: add a prefix
                if (!split_file)
                    goto fopen_err;
                split_amount = fread(splits,
                        sizeof(struct split), MAX_SPLITS, split_file);
                goto split_check;
            case 's': // Create new splits
                split_amount = getSplitsFromInput(splits);
split_check:
                if (split_amount > 0)
                    run_with_splits = true;
                else
                    fputs("No splits could be read\n", stderr);
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
    struct timeval start_time, stop_time;

    // Do not echo input
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    fputs("\033[H\033[J", stdout); // Clear console

    if (run_with_splits)
        // Print split names on seperate lines
        for (int i = 0; i < split_amount; i++)
            puts(splits[i].name);
    else {
        // Output "00:00" with no newline
        fputs("00:00", stdout);
        fflush(stdout);
    }

    /* Wait until enter key pressed to start the timer */
    int ctrlKeyPressed = false;
    while (!ctrlKeyPressed) {
        ctrlKeyPressed = keyPressed(timerCtrl_key, key_event_fp, &start_time);
        if (ctrlKeyPressed == -1) {
            return_value = errno;
            goto program_end;
        }
    }

    /* Start the timer */
    pthread_t timer_thread_id;
    bool do_thread = true;
    if (pthread_create(&timer_thread_id, NULL, timer, &do_thread)) {
        fputs("Error creating timer thread\n", stderr);
        goto program_end;
    }

    /* Wait until key pressed to stop the timer */
    ctrlKeyPressed = false;

    while (!ctrlKeyPressed) {
        ctrlKeyPressed = keyPressed(timerCtrl_key, key_event_fp, &stop_time);
        if (ctrlKeyPressed == -1) {
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
    printTime(stop_time, start_time);
    puts(""); // Print newline
    if (return_value)
        puts("The printed time is most likely NOT accurate!");

program_end:
    fclose(key_event_fp);
    // Allow echoing input again
    term.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    if (run_with_splits)
        return saveSplits(splits, split_file, split_amount, return_value);
    return return_value;

fopen_err:
    perror("fopen");
    return errno;
}
