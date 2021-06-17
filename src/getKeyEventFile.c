#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Returns the basename of the file that stores keyboard events
char* getKeyEventFile() {
    // /proc/bus/input/devices gives a list of devices & info about them
    FILE* devices_list = fopen("/proc/bus/input/devices", "r");
    if (errno) {
        perror("Error opening input device list");
        return NULL;
    }

    size_t bufsize = 100;
    char* line = (char*) malloc(bufsize * sizeof(char));
    char* event_handler = NULL;

    while (getline(&line, &bufsize, devices_list) != -1) {
        if (event_handler) {
            const char strcheck[] = "B: EV=120013";
            const size_t checklen = strlen(strcheck);

            if (strlen(line) >= checklen
                    && !strncmp(line, strcheck, checklen)) {
                // Device is a keyboard, clean up & return
                free(line);
                fclose(devices_list);
                return event_handler;
            }
        }
        if (line[0] == 'H') {
            // Event string has either a space or '=' before it
            const char splitter[] = "= ";

            const char strcheck[] = "event";
            const size_t checklen = strlen(strcheck);

            event_handler = strtok(line, splitter);
            while (strlen(event_handler) < checklen
                    || strncmp(event_handler, strcheck, checklen))
                event_handler = strtok(NULL, splitter);
        }
    }
    // No keyboard event file found, clean up & return
    // Not freeing line, getline sets line to null when there are no lines left
    fclose(devices_list);
    return NULL;
}
