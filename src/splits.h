#ifndef SPLITS_H
#define SPLITS_H

struct split {
    char name[MAX_SPLIT_NAME_LEN]; // Requires compile_settings.h
    struct timespec best_time;
};

int getSplitsFromInput(struct split* buf);

#endif // SPLITS_H
