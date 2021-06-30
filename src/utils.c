#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utils.h"

/* puts() without a newline at the end */
void putsNoNewline(const char* str) {
    fputs(str, stdout);
    fflush(stdout);
}
