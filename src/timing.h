#ifndef TIMING_H
#define TIMING_H

void* timer(void* time_ptr);
void printTime(struct timespec* time, struct timespec* start_time);

extern pthread_mutex_t timer_mtx;
extern bool do_timer;

#endif // TIMING_H
