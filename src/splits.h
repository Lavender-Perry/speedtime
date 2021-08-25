#ifndef SPLITS_H
#define SPLITS_H

#include "user/config.h"

struct split {
    char name[MAX_SPLIT_NAME_LEN];
    struct timespec best_time;
};

int getSplitsFromInput(struct split* buf);

#endif // SPLITS_H
