#include <stdio.h>
#include <string.h>

/* fgets but with the following behavorial changes:
 * 1 - The newline character is not included
 * 2 - All extra input on the line is discarded */
char* fgets_no_newline(char* restrict buf, size_t buflen, FILE* restrict fp)
{
    if (fgets(buf, buflen, fp) != NULL) {
        char* newline_loc = strchr(buf, '\n');
        if (newline_loc)
            *newline_loc = '\0';
        else {
            int c;
            while ((c = getc(fp)) != '\n' && c != EOF)
                /* Discard input */;
        }
        return buf;
    }
    return NULL;
}
