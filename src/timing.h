#ifndef TIMING_H
#define TIMING_H

void printTime(const struct timespec* time, const struct timespec* start_time);
void* timer(void* arg_ptr);

#endif // TIMING_H
