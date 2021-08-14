#ifndef SPLITS_H
#define SPLITS_H

#include "user/config.h"

struct split {
    char name[MAX_SPLIT_LEN];
    struct timespec best_time;
};

#endif // SPLITS_H
