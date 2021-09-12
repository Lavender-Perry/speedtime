#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

/* Prints elapsed time using the current timespec & starting timespec */
void printTime(const struct timespec* time, const struct timespec* start_time) {
    const long tv = (time->tv_sec - start_time->tv_sec) * 100
        + (time->tv_nsec - start_time->tv_nsec) / 10000000;
    printf("\r%ld:%.2ld.%.2ld", tv / 6000, tv / 100 % 60, tv % 100);
    fflush(stdout);
}

/* Updates the time every second
 * arg_ptr & return value must be void* for pthread */
void* timer(const void* arg_ptr) {
    const bool* do_thread = arg_ptr;
    int minutes, seconds = 0;
    while (*do_thread) {
        sleep(1);
        if (seconds == 59) {
            seconds = 0;
            minutes++;
        } else
            seconds++;
        printf("\r%d:%.2d", minutes, seconds);
        fflush(stdout);
    }
    pthread_exit(NULL);
}
