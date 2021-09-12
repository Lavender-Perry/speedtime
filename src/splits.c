#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "compile_settings.h"
#include "splits.h"
#include "utils.h"

/* Reads from stdin & puts splits in buf, returns amount of splits or -1 on error
 * Stops when empty line read or on EOF */
int getSplitsFromInput(struct split* restrict buf) {
    int i = 0; // Must be in this scope to return it

    while (fgets_no_newline(buf[i].name, MAX_SPLIT_NAME_LEN, stdin)[0] != '\0'
            && i < MAX_SPLITS) {
        if (!buf[i].name)
            return -1;
        buf[i].best_time = (struct timespec) {0, 0};
        i++;
    }

    return i;
}

/* Saves new splits, prompting for the file to save them in, or updates existing splits
 * if the best times improve. 
 * Returns return_value on success or errno/EXIT_FAILURE on error */
int saveSplits(const struct split* restrict splits,
        FILE* restrict split_file,
        int split_amount,
        int return_value) {
    if (split_file) {
        // TODO: update best times in split file if they improve
        fclose(split_file);
    } else {
        char split_name[30];

        // Get name to save splits as
        puts("Please enter the name you would like to save the splits as.\n"
                "Or enter \"cancel\" (without quotes) to not save the splits.");
        do if (fgets_no_newline(split_name, sizeof(split_name), stdin) == NULL) {
            fputs("Error reading your input.\n", stderr);
            return errno ? errno : EXIT_FAILURE;
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
                return_value = EXIT_FAILURE;
            }
            // Close file
            if (close(fd) == -1)
                perror("close");
        }
    }
    return errno ? errno : return_value;
}
