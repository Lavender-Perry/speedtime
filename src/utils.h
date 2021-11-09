#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

struct thread_args {
    bool parse_mode;
    bool run_thread;
    pthread_mutex_t* mtx_ptr;
};

char* fgets_no_newline(char* restrict buf, size_t buflen, FILE* restrict fp);

#endif // UTILS_H
