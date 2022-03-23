#include <bits/types/struct_timeval.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "utils.h"

/* Converts a timeval to a long representing amount of centiseconds */
long timevalToLong(struct timeval time) {
    return time.tv_sec * 100 + time.tv_usec / 10000;
}

/* Prints elapsed time */
void printTime(long tv, bool parse_mode)
{
    printf("%.2ld:%.2ld.%.2ld", tv / 6000, tv / 100 % 60, tv % 100);

    if (parse_mode) {
        puts("");
    } else {
        fputs("\033[8D\033[1B", stdout);
        fflush(stdout);
    }
}

/* Updates the time every second
 * arg_ptr & return value must be void* for pthread */
void* timer(void* arg_ptr)
{
    struct thread_args* args = arg_ptr;
    long minutes, seconds;
    if (args->do_countdown) {
        minutes = args->countdown_time / 6000;
        seconds = args->countdown_time % 6000 / 100;
    } else {
        minutes = seconds = 0;
    }

    while (args->run_thread) {
        pthread_mutex_lock(args->mtx_ptr);

        printf("%.2ld:%.2ld", minutes, seconds);
        if (args->parse_mode) {
            puts("");
        } else {
            fputs("\033[5D", stdout);
            fflush(stdout);
        }

        pthread_mutex_unlock(args->mtx_ptr);

        sleep(1);
        if (args->do_countdown) {
            if (seconds == 0) {
                if (minutes == 0) {
                    break;
                }
                seconds = 59;
                minutes--;
            } else {
                seconds--;
            }
            continue;
        }
        if (seconds == 59) {
            seconds = 0;
            minutes++;
        } else {
            seconds++;
        }
    }
    return NULL;
}
