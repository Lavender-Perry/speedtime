#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compile_settings.h"
#include "splits.h"
#include "timing.h"
#include "utils.h"

/* Reads from stdin & puts splits in buf, returns amount of splits or -1 on error
 * Stops when empty line read or on EOF */
int getSplitsFromInput(struct split* restrict buf) {
    int i = 0; // Must be in this scope to return it

    while (fgets_no_newline(buf[i].name, sizeof(buf[i].name), stdin)[0] != '\0'
            && i < MAX_SPLITS) {
        if (!buf[i].name)
            return -1;
        // buf[i].best_time = (struct timeval) {0, 0};
        i++;
    }

    return i;
}

/* Saves new splits, prompting for the file to save them in, or updates existing splits
 * if the best times improve. */
void saveSplits(const struct split* restrict splits,
        FILE* restrict split_file,
        int split_amount) {
    if (split_file) {
        // TODO: update best times in split file if they improve
        fclose(split_file);
    } else {
        char splits_name[MAX_SPLIT_NAME_LEN];

        // Discard sent input
        struct pollfd stdin_pollfd = {fd: STDIN_FILENO, events: POLLIN};
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
            // Create the split file, failing if it already exists
            const int fd = open(splits_name, // TODO: add a prefix
                    O_CREAT | O_WRONLY | O_EXCL,
                    S_IRUSR | S_IWUSR);
            if (fd < 0) { // Failure
                perror("open");
                return;
            }
            // Save splits to file
            const ssize_t write_amount = sizeof(struct split) * split_amount;
            if (write(fd, splits, write_amount) == write_amount)
                puts("Splits successfully saved.");
            else
                fputs("Error saving splits.\n", stderr);
            // Close file
            if (close(fd) == -1)
                perror("close");
        }
    }
    return;
input_read_err:
    fputs("Error reading your input.\n", stderr);
}

/* Starts the split, by printing the time for the previous
 * & moving the cursor to where the time should be printed for the next. */
void startSplit(struct timeval start_time, bool first_split, pthread_mutex_t* mtx_ptr) {
    static struct timeval begin_time;
    if (first_split)
        begin_time = start_time;
    else {
        pthread_mutex_lock(mtx_ptr);
        printTime(start_time, begin_time);
        pthread_mutex_unlock(mtx_ptr);
    }
}
