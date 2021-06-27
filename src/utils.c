#include <stdio.h>

#include "utils.h"

void puts_no_newline(const char* str) {
    fputs(str, stdout);
    fflush(stdout);
}
