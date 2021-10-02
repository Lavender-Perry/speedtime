#ifndef SPLITS_H
#define SPLITS_H

struct split {
    char name[MAX_SPLIT_NAME_LEN]; // Requires compile_settings.h
    // struct timeval best_time;
};

int getSplitsFromInput(struct split* restrict buf);
int saveSplits(const struct split* restrict splits,
        FILE* restrict split_file,
        int split_amount,
        int return_value);
void printSplits(const struct split* restrict splits, int split_amount);
void startSplit(struct timeval start_time, int split_num);

#endif // SPLITS_H
