#ifndef SPLITS_H
#define SPLITS_H

struct split {
    char* name;
    struct timespec best_time;
};

struct split* getSplitsFromInput(void);

#endif // SPLITS_H
