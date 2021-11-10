#include <asm/types.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>

#include "compile_settings.h"

/* Gives to buf the files that store keyboard events.
 * Uses optarg if it is given, otherwise autodetects the paths.
 * Returns amount found, or 0 on error/none read. */
int getKeyEventFiles(FILE* buf[MAX_KEYBOARDS], char* optarg) {
    int file_amount = 0;
    char paths[MAX_KEYBOARDS][MAX_KEYBOARD_PATH_LEN];

    if (optarg) {
        const char splitter[] = ",";
        strcpy(paths[file_amount], strtok(optarg, splitter));
        while (paths[file_amount] != NULL && file_amount < MAX_KEYBOARDS) {
            file_amount++;
            strcpy(paths[file_amount], strtok(NULL, splitter));
        }
    } else {
        // /proc/bus/input/devices gives a list of devices & info about them
        FILE* devices_list = fopen("/proc/bus/input/devices", "r");
        if (!devices_list) {
            perror("fopen");
            return 0;
        }

        char line[100];
        char* event_handler = NULL;

        while (fgets(line, sizeof(line), devices_list) != NULL
                && file_amount < MAX_KEYBOARDS) {
            if (event_handler) {
                const char strcheck[] = "B: EV=120013";
                const size_t checklen = strlen(strcheck);

                if (strlen(line) >= checklen
                        && !strncmp(line, strcheck, checklen)) { // Device is a keyboard
                    const char* input_dir = "/dev/input/";

                    strcpy(paths[file_amount], input_dir);
                    strcat(paths[file_amount], event_handler);

                    event_handler = NULL;
                    file_amount++;
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
    }

    for (int i = 0; i < file_amount; i++) {
        buf[i] = fopen(paths[i], "rb");
        if (!buf[i]) {
            char err_msg[25 + MAX_KEYBOARD_PATH_LEN] = "fopen on key event file ";
            strcat(err_msg, paths[i]);

            while (--i >= 0)
                fclose(buf[i]);

            perror(err_msg);
            return 0;
        }
    }

    return file_amount;
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
 * Returns: 0 on error getting event, otherwise the key that was pressed */
__u16 keyPressed(FILE** files, int file_amount, struct timeval* restrict when) {
    struct pollfd file_pollfds[file_amount];
    for (int i = 0; i < file_amount; i++)
        file_pollfds[i] = (struct pollfd) { fileno(files[i]), POLLIN, 0 };

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
