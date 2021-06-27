#ifndef SPEEDTIME_H
#define SPEEDTIME_H

/* Function declarations */
char* getKeyEventFile();
void print_clock_time(const clock_t clock_time);

/* Macros */
#define CLOCK_ERR_STR "Error calling clock() to update the timer.\n"
#define EVENT_LOOP_ERR_EXIT(str) \
    fputs(str, stderr); \
    return_value = EXIT_FAILURE; \
    break;

#endif // SPEEDTIME_H
