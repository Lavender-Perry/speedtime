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
#include "utils.h"

void set_key(__u16* restrict key, char* optarg);

/* Argument parsing, event loop, etc. */
int main(int argc, char** argv) {
    /* Variables that could be set by argument parsing */
    char* key_event_path = NULL;
    __u16 stop_key = DEFAULT_STOP_KEY;
    __u16 timerCtrl_key = DEFAULT_CONTROL_KEY;
    FILE* split_file = NULL;
    int split_amount = 0;
    struct split splits[MAX_SPLITS];
    bool run_with_splits = false;

    /* Argument parsing */
    int opt;
    while ((opt = getopt(argc, argv, "f:k:c:l:s")) != -1)
        switch (opt) {
            case 'f': // Set path to file for monitoring key events
                key_event_path = optarg;
                break;
            case 'k': // Set key to stop the timer
                set_key(&stop_key, optarg);
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
    struct timeval start_time, current_time;

    // Do not echo input
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    fputs("\033[H\033[J", stdout); // Clear console

    if (run_with_splits) {
        // Print split names on seperate lines
        for (int i = 0; i < split_amount; i++)
            puts(splits[i].name);
        // Move cursor
        printf("\033[%dA\033[%dC", split_amount, MAX_SPLIT_NAME_LEN + 2);
    }

    /* Wait until timer control key pressed to start the timer */
    __u16 keyPressedResult;
    do {
        keyPressedResult = keyPressed(key_event_fp, &start_time);
        if (keyPressedResult == 0xffff)
            goto program_end;
    } while (keyPressedResult != timerCtrl_key);

    /* Start the timer */
    startSplit(start_time, true, NULL);

    pthread_t timer_thread_id;
    pthread_mutex_t timer_mtx;
    pthread_mutex_init(&timer_mtx, NULL);
    struct thread_args timer_args = {true, &timer_mtx};
    if (pthread_create(&timer_thread_id, NULL, timer, &timer_args)) {
        fputs("Error creating timer thread\n", stderr);
        goto program_end;
    }

    int current_split = 1;

    do {
        keyPressedResult = keyPressed(key_event_fp, &current_time);
        if (keyPressedResult == 0xffff) {
            perror("Error getting key press");
            break;
        }
        if (keyPressedResult == timerCtrl_key) {
            if (current_split >= split_amount)
                break;
            else {
                current_split++;
                startSplit(current_time, false, timer_args.mtx_ptr);
            }
        }
    } while (keyPressedResult != stop_key);

    /* Stop the timer */
    timer_args.run_thread = false;
    if (pthread_join(timer_thread_id, NULL) == -1)
        perror("pthread_join");
    printTime(current_time, start_time);
    puts(""); // Print newline
    if (errno)
        puts("The printed time is most likely NOT accurate!");
    pthread_mutex_destroy(&timer_mtx);

program_end:
    fclose(key_event_fp);
    // Allow echoing input again
    term.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    if (run_with_splits)
        saveSplits(splits, split_file, split_amount);
    return errno;

fopen_err:
    perror("fopen");
    return errno;
}

/* Sets key variable to value from optarg.
 * Used in argument parsing. */
void set_key(__u16* restrict key, char* optarg) {
    const __u16 atoi_res = atoi(optarg);
    if (!atoi_res)
        fputs("Invalid key code specified, using default key\n", stderr);
    else
        *key = atoi_res;
}
