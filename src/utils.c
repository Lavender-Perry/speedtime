#include <stdio.h>
#include <string.h>

char* fgets_no_newline(char* restrict buf, size_t buflen, FILE* restrict fp) {
    if (fgets(buf, buflen, fp) != NULL) {
        buf[strchr(buf, '\n') - buf] = '\0';
        return buf;
    }
    return NULL;
}
