#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "timing.h"

/* Updates the time every centisecond
 * time_ptr & return value must be void* for pthread_create() */
void* timer(void* time_ptr) {
    if (!time_ptr) {
        perror("malloc");
        pthread_exit(NULL);
    }
    struct timespec* current_time = time_ptr;
    pthread_mutex_lock(&timer_mtx);
    while (do_timer) { // Mutex must be locked when checking this
        pthread_mutex_unlock(&timer_mtx);

        usleep(10000);
        /* Get the current time & update the timer */
        clock_gettime(CLOCK_TAI, current_time); // No error handling, error is unlikely
        printTime(current_time, NULL);

        pthread_mutex_lock(&timer_mtx);
    }
    pthread_mutex_unlock(&timer_mtx);
    free(time_ptr);
    pthread_exit(NULL);
}

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
