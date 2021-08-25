#include <stdio.h>
#include <string.h>
#include <time.h>

#include "splits.h"
#include "utils.h"

#include "user/config.h"

/* Reads from stdin & puts splits in buf, returns amount of splits or -1 on error
 * Stops when "END" read or on EOF */
int getSplitsFromInput(struct split* buf) {
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
