#ifndef TIMING_H
#define TIMING_H

void printTime(struct timespec* time, struct timespec* start_time);
void* timer(void* arg_ptr);

#endif // TIMING_H
