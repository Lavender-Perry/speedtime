#include <stdio.h>

/* puts() without a newline at the end */
void putsNoNewline(const char* str) {
    fputs(str, stdout);
    fflush(stdout);
}
