#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "splits.h"

#include "user/config.h"

/* Reads from stdin & returns an array of splits
 * Stops when "END" read or on EOF */
struct split* getSplitsFromInput(void) {
    struct split* saved_splits = NULL;
    size_t bufsize = MAX_SPLIT_LEN;

    char* line;

    const size_t split_size = sizeof(struct split);
    while (getline(&line, &bufsize, stdin) != EOF && strcmp(line, "END\n")) {
        const size_t saved_splits_size = sizeof(saved_splits);

        saved_splits = realloc(saved_splits, saved_splits_size + split_size);
        if (!saved_splits) {
            perror("realloc");
            break;
        }

        saved_splits[saved_splits_size / split_size] = (struct split) {
            .name = line, .best_time = (struct timespec) {0, 0}
        };
    }

    return saved_splits;
}
