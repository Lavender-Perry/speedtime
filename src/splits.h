#ifndef SPLITS_H
#define SPLITS_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "compile_settings.h"

struct split {
    long best_time;
    char name[MAX_SPLIT_NAME_LEN];
};

int getSplits(FILE* file, struct split* restrict buf);
void putSplits(const struct split* restrict splits,
        size_t split_amount,
        FILE* restrict split_file);
void printSplits(const struct split* restrict splits, int split_amount);
void splitParseModePrint(const struct split* restrict split);
void startSplit(struct timeval start_time,
        pthread_mutex_t* mtx_ptr,
        bool first_split,
        bool parse_mode,
        long* restrict best_split_time);

#endif // SPLITS_H
