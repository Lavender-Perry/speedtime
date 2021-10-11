#ifndef SPLITS_H
#define SPLITS_H

struct split {
    // Requires compile_settings.h
    char name[MAX_SPLIT_NAME_LEN];
};

int getSplitsFromInput(struct split* restrict buf);
void saveSplits(const struct split* restrict splits,
        FILE* restrict split_file,
        int split_amount);
// Requires stdbool.h & pthread.h
void startSplit(struct timeval start_time, bool first_split, pthread_mutex_t* mtx_ptr);

#endif // SPLITS_H
