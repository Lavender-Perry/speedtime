#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "splits.h"

#include "user/config.h"

/* Reads from stdin & puts splits in buf, returns amount of splits
 * Stops when "END" read or on EOF */
int getSplitsFromInput(struct split* buf) {
    size_t bufsize = MAX_SPLIT_LEN;

    char* line;

    int i = 0; // Must be in this scope to return it

    while (getline(&line, &bufsize, stdin) != EOF
            && strcmp(line, "END\n") && i < MAX_SPLITS) {
        buf[i] = (struct split) {
            .name = line, .best_time = (struct timespec) {0, 0}
        };
        i++;
    }

    return i;
}
