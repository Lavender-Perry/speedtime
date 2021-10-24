#include <asm/types.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>

#include "compile_settings.h"

/* Gives to buf the path of the files that store keyboard events
 * Uses optarg if it is given, otherwise autodetects the paths
 * Returns amount found, or 0 on error/none read */
int getKeyEventPaths(char** restrict buf, int max, char* optarg) {
    int path_amount = 0;

    if (optarg) {
        const char splitter[] = ",";
        buf[path_amount] = strtok(optarg, splitter);
        while (buf[path_amount] != NULL && path_amount < max) {
            path_amount++;
            buf[path_amount] = strtok(NULL, splitter);
        }
        return path_amount;
    }

    // No optarg given, autodetect key files

    // /proc/bus/input/devices gives a list of devices & info about them
    FILE* devices_list = fopen("/proc/bus/input/devices", "r");
    if (!devices_list) {
        perror("fopen");
        return 0;
    }

    char line[100];
    char* event_handler;

    while (fgets(line, sizeof(line), devices_list) != NULL && path_amount < max) {
        if (event_handler) {
            const char strcheck[] = "B: EV=120013";
            const size_t checklen = strlen(strcheck);

            if (strlen(line) >= checklen
                    && !strncmp(line, strcheck, checklen)) { // Device is a keyboard
                const char* input_dir = "/dev/input/";

                strcpy(buf[path_amount], input_dir);
                strcat(buf[path_amount], event_handler);

                event_handler = NULL;
                path_amount++;
                continue;
            }
        }
        if (line[0] == 'H') {
            // Event string has either a space or '=' before it
            const char splitter[] = "= ";

            const char strcheck[] = "event";
            const size_t checklen = strlen(strcheck);

            event_handler = strtok(line, splitter);
            while ((strlen(event_handler) < checklen
                    || strncmp(event_handler, strcheck, checklen))
                    && event_handler != NULL)
                event_handler = strtok(NULL, splitter);
        }
    }
    fclose(devices_list);
    return path_amount;
}

/* Sets key variable to value from optarg.
 * Used in argument parsing. */
void set_key(__u16* restrict key, char* optarg) {
    const __u16 atoi_res = atoi(optarg);
    if (!atoi_res)
        fputs("Invalid key code specified, using default key\n", stderr);
    else
        *key = atoi_res;
}

/* Checks if key was pressed until one is, & sets when to the time it was pressed.
 * Must be called with the same values for files & file_amount every time.
 * On first call, when should be NULL.  It sets up the pollfds on first call.
 * Returns: file_amount on first call,
 *          0 on error getting event,
 *          otherwise the key that was pressed */
__u16 keyPressed(FILE** files, int file_amount, struct timeval* restrict when) {
    static struct pollfd file_pollfds[MAX_KEYBOARDS];

    if (!when) {
        for (int i = 0; i < file_amount; i++)
            file_pollfds[i] = (struct pollfd) { fileno(files[i]), POLLIN, 0 };
        return file_amount;
    }

    while (poll(file_pollfds, file_amount, -1) != -1)
        for (int i = 0; i < file_amount; i++) {
            if (file_pollfds[i].revents & POLLERR)
                return 0;

            if (file_pollfds[i].revents & POLLIN) {
                struct input_event event;
                if (fread(&event, sizeof(event), 1, files[i]) != 1)
                    return 0;

                if (event.type == EV_KEY && event.value) {
                    *when = event.time;
                    return event.code;
                }
            }
        }

    return 0;
}
