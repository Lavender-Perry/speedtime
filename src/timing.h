#ifndef TIMING_H
#define TIMING_H

long timeDiffToLong(struct timeval time, struct timeval start_time);
void printTime(long tv, bool parse_mode);
void* timer(void* arg_ptr);

#endif // TIMING_H
