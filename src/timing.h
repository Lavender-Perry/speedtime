#ifndef TIMING_H
#define TIMING_H

void printTime(struct timeval time, struct timeval start_time, bool parse_mode);
void* timer(void* arg_ptr);

#endif // TIMING_H
