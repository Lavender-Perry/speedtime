#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

/* Prints elapsed time using the current timespec & starting timespec, after start_time
 * is given. Prints out values less than a second if morePrecision is true
 * First call: start_time must be starting time, time must be NULL
 * Other calls: time must be current time */
void printTime(const struct timespec* time,
        const struct timespec* start_time,
        bool morePrecision) {
    static struct timespec start; // Set to *start_time on first call

    if (!time) {
        start = *start_time;
        return;
    }

    const long tv = (time->tv_sec - start.tv_sec) * 100
        + (time->tv_nsec - start.tv_nsec) / 10000000;

    printf("\r%ld:%.2ld", tv / 6000, tv / 100 % 60);
    if (morePrecision)
        printf(".%.2ld", tv % 100);
    fflush(stdout);
}

/* Updates the time every second
 * arg_ptr & return value must be void* for pthread */
void* timer(const void* arg_ptr) {
    const bool* do_thread = arg_ptr;
    struct timespec current_time = {0, 0};
    while (*do_thread) {
        sleep(1);
        /* Get the current time & update the timer */
        if (clock_gettime(CLOCK_TAI, &current_time) == -1) {
            perror("clock_gettime");
            break;
        }
        printTime(&current_time, NULL, false);
    }
    pthread_exit(NULL);
}
