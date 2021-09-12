#ifndef SPLITS_H
#define SPLITS_H

struct split {
    char name[MAX_SPLIT_NAME_LEN]; // Requires compile_settings.h
    struct timespec best_time;
};

int getSplitsFromInput(struct split* restrict buf);
int saveSplits(const struct split* restrict splits,
        FILE* restrict split_file,
        int split_amount,
        int return_value);

#endif // SPLITS_H
