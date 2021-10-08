#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

/* Prints elapsed time using the current timeval & starting timeval; */
void printTime(struct timeval time, struct timeval start_time) {
    const long tv_sec = time.tv_sec - start_time.tv_sec;
    const long tv_usec = time.tv_usec - start_time.tv_usec;
    printf("%.2ld:%.2ld.%.2ld\033[8D", tv_sec / 60, tv_sec % 60, tv_usec);
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
        printf("%.2d:%.2d\033[5D", minutes, seconds);
        fflush(stdout);
    }
    pthread_exit(NULL);
}
