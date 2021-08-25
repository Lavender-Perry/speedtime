#include <stdio.h>
#include <string.h>

char* fgets_no_newline(char* buf, size_t buflen, FILE* fp) {
    if (fgets(buf, buflen, fp) != NULL) {
        buf[strchr(buf, '\n') - buf] = '\0';
        return buf;
    }
    return NULL;
}
