#ifndef SPLITS_H
#define SPLITS_H

struct split {
    char name[1000];
    struct timespec best_time;
};

int getSplitsFromInput(struct split* buf);

#endif // SPLITS_H
