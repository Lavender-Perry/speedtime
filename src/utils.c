#include <stdio.h>
#include <string.h>

char* fgets_no_newline(char* restrict buf, size_t buflen, FILE* restrict fp) {
    if (fgets(buf, buflen, fp) != NULL) {
        char* newline_loc = strchr(buf, '\n');
        if (newline_loc)
            *newline_loc = '\0';
        return buf;
    }
    return NULL;
}
