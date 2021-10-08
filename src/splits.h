#ifndef SPLITS_H
#define SPLITS_H

struct split {
    /* struct timeval best_time; ** not yet implemented **/
    // Requires compile_settings.h
    char name[MAX_SPLIT_NAME_LEN];
};

int getSplitsFromInput(struct split* restrict buf);
void saveSplits(const struct split* restrict splits,
        FILE* restrict split_file,
        int split_amount);
void startSplit(struct timeval start_time, bool first_split);

#endif // SPLITS_H
