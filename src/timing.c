#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "structs.h"

/* Prints elapsed time using the current timespec & starting timespec, after start_time
 * is given.
 * First call: start_time must be starting time, time must be NULL
 * Other calls: time must be current time, start_time ignored */
void printTime(struct timespec* time, struct timespec* start_time) {
    static struct timespec start; // Set to *start_time on first call

    if (!time) {
        start = *start_time;
        return;
    }

    const int time_nsec = (time->tv_sec - start.tv_sec) * 100
        + (time->tv_nsec - start.tv_nsec) / 10000000;
    printf("\r%d:%.2d.%.2d",
            time_nsec / 6000,
            time_nsec / 100 % 60,
            time_nsec % 100);
    fflush(stdout);
}

/* Updates the time every centisecond
 * arg_ptr & return value must be void* for pthread */
void* timer(const void* arg_ptr) {
    const bool* do_thread = arg_ptr;
    struct timespec* current_time = malloc(sizeof(struct timespec));
    if (!current_time) {
        perror("malloc");
        pthread_exit(NULL);
    }
    while (*do_thread) {
        usleep(10000);
        /* Get the current time & update the timer */
        if (clock_gettime(CLOCK_TAI, current_time) == -1) {
            perror("clock_gettime");
            break;
        }
        printTime(current_time, NULL);
    }
    free(current_time);
    pthread_exit(NULL);
}
