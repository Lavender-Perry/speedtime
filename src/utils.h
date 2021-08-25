#ifndef UTILS_H
#define UTILS_H

#define ERROR_STATUS errno ? errno : EXIT_FAILURE
char* fgets_no_newline(char* buf, size_t buflen, FILE* fp);

#endif // UTILS_H
