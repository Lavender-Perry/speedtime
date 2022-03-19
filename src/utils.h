#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

struct thread_args {
    bool do_countdown;
    long countdown_time;
    bool parse_mode;
    bool run_thread;
    pthread_mutex_t* mtx_ptr;
};

char* fgetsNew(char* restrict buf, size_t buflen, FILE* file);

#endif // UTILS_H
