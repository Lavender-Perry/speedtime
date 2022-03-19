#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "compile_settings.h"
#include "splits.h"
#include "timing.h"
#include "utils.h"

/* Reads from file & puts splits in buf, returns amount of splits or -1 on error
 * Stops when empty line read or on EOF */
int getSplits(FILE* file, struct split* restrict buf)
{
    int i = 0; // Must be in this scope to return it
    char time_read_buf[20];

    while (i < MAX_SPLITS
        && fgetsNew(buf[i].name, sizeof(buf[i].name), file) != NULL
        && buf[i].name[0]) {
        if (file == stdin) {
            buf[i].best_time = 0;
        } else if (fgetsNew(time_read_buf, sizeof(time_read_buf), file) == NULL
            || (buf[i].best_time = atol(time_read_buf)) == 0) {
            return -1;
        }

        i++;
    }

    return i;
}

/* Saves new splits, prompting for the file to save them in, or updates existing splits
 * to have the new best times. */
void putSplits(const struct split* restrict splits,
               size_t split_amount,
               FILE* restrict split_file)
{
    if (split_file != NULL) {
        if (fseek(split_file, 0, SEEK_SET)) {
            perror("fseek");
            return;
        }

        goto write_splits;
    }

    char splits_name[MAX_SPLITS_PATH_LEN];

    // Discard sent input
    struct pollfd stdin_pollfd = { STDIN_FILENO, POLLIN, 0 };
    while (poll(&stdin_pollfd, 1, 0)
        && stdin_pollfd.revents & POLLIN
        && getc(stdin) != EOF) { }

    puts("Now saving the new splits. "
         "Please press enter with the program window/console focused.");

    // Wait for send, then discard previously unsent input.
    // This is not the final value for splits_name,
    // it is being used as a buffer for discarded input.
    if (!fgetsNew(splits_name, sizeof(splits_name), stdin)) {
        goto input_read_err;
    }

    puts("Please enter the name you would like to save the splits as.\n"
         "Or enter \"cancel\" (without quotes) to not save the splits.");
    do {
        if (fgetsNew(splits_name, sizeof(splits_name), stdin) == NULL) {
            goto input_read_err;
        }
    } while (splits_name[0] == '\0');

    if (strcmp(splits_name, "cancel")) {                  // "cancel" not read
        if (!(split_file = fopen(splits_name, "w+xe"))) { // Open file/error handling
            perror("fopen");
            return;
        }

        goto write_splits;
    }

    return;

write_splits:
    for (size_t i = 0; i < split_amount; i++) {
        if (fprintf(split_file, "%s\n%li\n", splits[i].name, splits[i].best_time) < 0) {
            fputs("Error saving splits.\n", stderr);
            break;
        }
    }
    fclose(split_file);
    return;

input_read_err:
    fputs("Error reading your input.\n", stderr);
}

/* Prints the split name, space for the time, & the best time for each split */
void printSplits(const struct split* restrict splits, int split_amount)
{
    for (int i = 0; i < split_amount; i++) {
        fputs(splits[i].name, stdout);

        // Move cursor to make room for time
        printf("\033[%dG", MAX_SPLIT_NAME_LEN + 10);

        if (splits[i].best_time == 0) {
            puts("--:--.--");
        } else {
            printTime(splits[i].best_time, true);
        }
    }

    // Move cursor to time area of first split
    printf("\033[0;%dH", MAX_SPLIT_NAME_LEN + 1);
}

/* Prints split name & best time for parse mode */
void splitParseModePrint(const struct split* restrict split)
{
    puts(split->name);
    fputs("best ", stdout);
    printTime(split->best_time, true);
}

/* Starts the split by updating best time if needed & printing the time */
void startSplit(struct timeval start_time,
                pthread_mutex_t* mtx_ptr,
                bool first_split,
                bool parse_mode,
                long* restrict best_split_time)
{

    static struct timeval begin_time;
    if (first_split) {
        begin_time = start_time;
        return;
    }

    const long split_time = timevalToLong(begin_time) - timevalToLong(start_time);

    if (!parse_mode && best_split_time) {
        int color_val = 33; // Yellow
        if (split_time < *best_split_time || *best_split_time == 0) {
            // Time is better than best
            *best_split_time = split_time;
            color_val = 32; // Green
        } else if (split_time > *best_split_time) {
            // Time is worse than best
            color_val = 31; // Red
        }
        printf("\033[%dm", color_val);
    }

    pthread_mutex_lock(mtx_ptr);
    printTime(split_time, parse_mode);
    pthread_mutex_unlock(mtx_ptr);
    fputs("\033[0m", stdout);
}
