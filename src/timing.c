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

    if (time) {
        const int time_nsec = (time->tv_sec - start.tv_sec) * 100
            + (time->tv_nsec - start.tv_nsec) / 10000000;
        printf("\r%d:%.2d.%.2d",
                time_nsec / 6000,
                time_nsec / 100 % 60,
                time_nsec % 100);
        fflush(stdout);
    } else
        start = *start_time;
}

/* Updates the time every centisecond
 * arg_ptr & return value must be void* for pthread */
void* timer(void* arg_ptr) {
    struct threadInfo* info_ptr = arg_ptr;
    struct timespec* current_time = malloc(sizeof(struct timespec));
    if (!current_time) {
        perror("malloc");
        pthread_exit(NULL);
    }
    pthread_mutex_lock(info_ptr->mtx_ptr);
    while (info_ptr->do_thread) { // Mutex must be locked when checking this
        pthread_mutex_unlock(info_ptr->mtx_ptr);

        usleep(10000);
        /* Get the current time & update the timer */
        if (clock_gettime(CLOCK_TAI, current_time) == -1) {
            perror("clock_gettime");
            goto end_of_thread;
        }
        printTime(current_time, NULL);

        pthread_mutex_lock(info_ptr->mtx_ptr);
    }
    pthread_mutex_unlock(info_ptr->mtx_ptr);
end_of_thread:
    free(current_time);
    pthread_exit(NULL);
}
