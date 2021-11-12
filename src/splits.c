#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "compile_settings.h"
#include "splits.h"
#include "timing.h"
#include "utils.h"

/* Reads from stdin & puts splits in buf, returns amount of splits or -1 on error
 * Stops when empty line read or on EOF */
int getSplitsFromInput(struct split* restrict buf) {
    int i = 0; // Must be in this scope to return it

    do {
        if (fgets_no_newline(buf[i].name, sizeof(buf[i].name), stdin) == NULL)
            return -1;

        if (buf[i].name[0] == '\0')
            break;

        buf[i].best_time = LONG_MAX;
        i++;
    } while (i < MAX_SPLITS);

    return i;
}

/* Saves new splits, prompting for the file to save them in, or updates existing splits
 * to have the new best times. */
void saveSplits(const struct split* restrict splits,
        size_t split_amount,
        FILE* restrict split_file) {
    if (split_file) {
        if (fseek(split_file, 0, SEEK_SET))
            perror("fseek");
        else
            if (fwrite(splits, sizeof(struct split), split_amount, split_file)
                    != split_amount)
                perror("fwrite");
        fclose(split_file);
        return;
    }

    char splits_name[MAX_SPLIT_NAME_LEN];

    // Discard sent input
    struct pollfd stdin_pollfd = {STDIN_FILENO, POLLIN, 0};
    while (poll(&stdin_pollfd, 1, 0)
            && stdin_pollfd.revents & POLLIN
            && getc(stdin) != EOF)
        /* Discard input */;

    puts("Now saving the new splits. "
            "Please press enter with the program window/console focused.");

    // Wait for send, then discard previously unsent input.
    // This is not the final value for splits_name, it is being used as a buffer for
    // discarded input.
    if (!fgets_no_newline(splits_name, sizeof(splits_name), stdin))
        goto input_read_err;

    // Get name to save splits as
    puts("Please enter the name you would like to save the splits as.\n"
            "Or enter \"cancel\" (without quotes) to not save the splits.");
    do
        if (!fgets_no_newline(splits_name, sizeof(splits_name), stdin))
            goto input_read_err;
    while (splits_name[0] == '\0');

    if (strcmp(splits_name, "cancel")) { // "cancel" not read
        if (!(split_file = fopen(splits_name, "w+bx"))) { // Open file/error handling
            char err_msg[10 + MAX_SPLIT_NAME_LEN] = "fopen on ";
            strcat(err_msg, splits_name);
            perror(err_msg);
            return;
        }

        if (fwrite(splits, sizeof(struct split), split_amount, split_file)
                == split_amount)
            puts("Splits successfully saved.");
        else
            perror("fwrite");

        fclose(split_file);
    }
    return;

input_read_err:
    fputs("Error reading your input.\n", stderr);
}

/* Prints the split name, space for the time, & the best time for each split */
void printSplits(const struct split* restrict splits, int split_amount) {
    for (int i = 0; i < split_amount; i++) {
        fputs(splits[i].name, stdout);

        // Move cursor to make room for time
        printf("\033[%dG", MAX_SPLIT_NAME_LEN + 10);

        if (splits[i].best_time == LONG_MAX)
            puts("--:--.--");
        else
            printTime(splits[i].best_time, true);
    }

    // Move cursor to time area of first split
    printf("\033[0;%dH", MAX_SPLIT_NAME_LEN + 1);
}

/* Prints split name & best time for parse mode */
void splitParseModePrint(const struct split* restrict split) {
    puts(split->name);
    fputs("best ", stdout);
    printTime(split->best_time, true);
}

/* Starts the split by printing the time for the previous, updating best time if needed,
 * & moving the cursor to where the time should be printed for the next. */
void startSplit(struct timeval start_time,
        pthread_mutex_t* mtx_ptr,
        bool first_split,
        bool parse_mode,
        long* restrict best_split_time) {

    static struct timeval begin_time;
    if (first_split) {
        begin_time = start_time;
        return;
    }

    const long split_time = timeDiffToLong(start_time, begin_time);

    if (best_split_time && split_time < *best_split_time)
        *best_split_time = split_time;

    pthread_mutex_lock(mtx_ptr);
    printTime(split_time, parse_mode);
    pthread_mutex_unlock(mtx_ptr);
}
