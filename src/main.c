#include <asm/types.h>
#include <errno.h>
#include <linux/input-event-codes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <sys/time.h>
#include <unistd.h>

#include "compile_settings.h"
#include "keyboard.h"
#include "splits.h"
#include "timing.h"
#include "utils.h"

/* Argument parsing, event loop, etc. */
int main(int argc, char** argv) {
    /* Variables that could be set by argument parsing */
    FILE* key_event_files[MAX_KEYBOARDS];
    int keyboard_amount = 0;
    __u16 stop_key = DEFAULT_STOP_KEY;
    __u16 timerCtrl_key = DEFAULT_CONTROL_KEY;
    FILE* split_file = NULL;
    int split_amount = 1;
    struct split splits[MAX_SPLITS];
    bool run_with_splits = false;
    bool parse_mode = false;

    /* Argument parsing */
    int opt;
    while ((opt = getopt(argc, argv, "f:k:c:l:spie")) != -1)
        switch (opt) {
            case 'f': // Set paths to files for monitoring key events
                keyboard_amount = getKeyEventFiles(key_event_files, optarg);
                break;
            case 'k': // Set key to stop the timer
                set_key(&stop_key, optarg);
                break;
            case 'c': // Set key to control the timer
                set_key(&timerCtrl_key, optarg);
                break;
            case 'l': // Load splits from file specified by optarg
                split_file = fopen(optarg, "r+b");
                if (!split_file) {
                    perror("Invalid split file given");
                    return errno;
                }
                split_amount = fread(splits,
                        sizeof(struct split),
                        MAX_SPLITS,
                        split_file);
                goto split_check;
            case 's': // Create new splits
                split_amount = getSplitsFromInput(splits);
split_check:
                if (split_amount > 0)
                    run_with_splits = true;
                else
                    fputs("No splits could be read\n", stderr);
                break;
            case 'p': // Turn on parse mode
                parse_mode = true;
                break;
            case 'i': // Print some info
                printf("control key: %d\n", timerCtrl_key);
                printf("stop key: %d\n", stop_key);
                printf("max splits: %d\n", MAX_SPLITS);
                printf("max split name length: %d\n", MAX_SPLIT_NAME_LEN);
                printf("max keyboards: %d\n", MAX_KEYBOARDS);
                break;
            case 'e':
                return 0;
        }

    if (!keyboard_amount
            && !(keyboard_amount = getKeyEventFiles(key_event_files, NULL))) {
        fputs("Error finding the keyboard event files.\n"
                "Please specify the files by adding the arguments "
                "\"-f /path/to/event_file,/other/path,etc\"\n", stderr);
        return errno;
    }

    /* Set up for starting the timer */
    struct timeval start_time, current_time;
    struct termios term;

    if (!parse_mode) {
        // Do not echo input
        tcgetattr(STDIN_FILENO, &term);
        term.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &term);

        fputs("\033[H\033[J", stdout); // Clear console

        if (run_with_splits)
            printSplits(splits, split_amount);
    }

    /* Wait until timer control key pressed to start the timer */
    __u16 keyPressedResult;
    do {
        keyPressedResult = keyPressed(key_event_files, keyboard_amount, &start_time);
        if (keyPressedResult == 0)
            goto program_end;
    } while (keyPressedResult != timerCtrl_key);

    /* Start the timer */
    if (parse_mode)
        splitParseModePrint(&splits[0]);

    startSplit(start_time, NULL, true, parse_mode, NULL);

    pthread_t timer_thread_id;
    pthread_mutex_t timer_mtx;
    pthread_mutex_init(&timer_mtx, NULL);
    struct thread_args timer_args = {parse_mode, true, &timer_mtx};
    if (pthread_create(&timer_thread_id, NULL, timer, &timer_args)) {
        fputs("Error creating timer thread\n", stderr);
        goto program_end;
    }

    int current_split = 1;

    do {
        keyPressedResult = keyPressed(key_event_files, keyboard_amount, &current_time);
        if (keyPressedResult == 0) {
            perror("Error getting key press");
            break;
        }
        if (keyPressedResult == timerCtrl_key) {
            startSplit(current_time,
                    timer_args.mtx_ptr,
                    false,
                    parse_mode,
                    run_with_splits ? &splits[current_split - 1].best_time : NULL);

            if (current_split == split_amount)
                break;

            if (parse_mode)
                splitParseModePrint(&splits[current_split]);

            current_split++;
        }
    } while (keyPressedResult != stop_key);

    /* Stop the timer */
    timer_args.run_thread = false;
    if (pthread_join(timer_thread_id, NULL) == -1)
        perror("pthread_join");

    printTime(timeDiffToLong(current_time, start_time), parse_mode);

    if (run_with_splits && !parse_mode)
        printf("\033[%d;0H", split_amount + 1); // Move to after splits

    if (errno)
        fputs("The printed time is most likely NOT accurate!\n", stderr);

    pthread_mutex_destroy(&timer_mtx);

program_end:
    for (int i = 0; i < keyboard_amount; i++)
        fclose(key_event_files[i]);

    if (!parse_mode) {
        // Allow echoing input again
        term.c_lflag |= ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
    }

    if (run_with_splits)
        saveSplits(splits, split_amount, split_file);
    return errno;
}
