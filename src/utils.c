#include <stdio.h>
#include <string.h>

/* fgets, but with no newline character & the rest of the line consumed. */
char* fgetsNew(char* restrict buf, size_t buflen, FILE* file)
{
    if (fgets(buf, buflen, file) != NULL) {
        char* newline_loc = strchr(buf, '\n');
        if (newline_loc != NULL) {
            *newline_loc = '\0';
        } else {
            int c;
            do {
                c = getc(file);
            } while (c != '\n' && c != EOF);
        }
        return buf;
    }
    return NULL;
}
