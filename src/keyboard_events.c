#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

/* Autodetects & returns the path of the file that stores keyboard events */
char* getKeyEventFile() {
    char* return_value = NULL;

    // /proc/bus/input/devices gives a list of devices & info about them
    FILE* devices_list = fopen("/proc/bus/input/devices", "r");
    if (!devices_list) {
        perror("Error opening input device list");
        return return_value;
    }

    // Value will not change.  It is not const because getline would not accept it.
    size_t bufsize = 100;

    char* line = malloc(bufsize);
    if (!line) {
        perror("Error allocating memory to read line from list of devices");
        goto end_of_loop;
    }

    char* event_handler = NULL;

    while (getline(&line, &bufsize, devices_list) != EOF) {
        if (event_handler) {
            const char strcheck[] = "B: EV=120013";
            const size_t checklen = strlen(strcheck);

            if (strlen(line) >= checklen
                    && !strncmp(line, strcheck, checklen)) { // Device is a keyboard
                const char* input_dir = "/dev/input/";
                return_value = malloc(strlen(input_dir) + strlen(event_handler) + 1);
                if (!return_value) {
                    fputs("Error allocating memory for file path string\n", stderr);
                    break;
                }
                strcpy(return_value, input_dir);
                strcat(return_value, event_handler);
                free(line);
                break;
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
end_of_loop:
    fclose(devices_list);
    return return_value;
}

/* After keyboard_event_fp given: checks when the enter key was pressed, updates when
 * First call: keyboard_event_fp file pointer to read events from, when NULL
 * Other calls: when pointer to put time into, keyboard_event_fp ignored
 * Returns:
 *     0 on first call, or
 *     -2 on error getting time, or
 *     -1 on error getting event, or
 *     if the key was pressed */
int enterKeyPressed(FILE* keyboard_event_fp, struct timespec* when) {
    /* Read & parse data */
    struct timeval event_time;
    // event_info[0]: type, event_info[1]: code,
    // event_info[2]: value (lower byte), event_info[3]: value (upper byte)
    unsigned short event_info[4];

    // Read event time
    if (fread(&event_time, sizeof(event_time), 1, keyboard_event_fp) != 1)
        return -2;
    // Update the time in struct timespec* when
    when->tv_sec = event_time.tv_sec;
    when->tv_nsec = event_time.tv_usec * 1000;
    // Read event data
    if (fread(event_info, sizeof(short), 4, keyboard_event_fp) != 4)
        return -1;

    /* Return if the key was pressed or not */
    // No need to check event_info[3], it is always 0 for key inputs
    return event_info[0] == EV_KEY && event_info[1] == KEY_ENTER && event_info[2];
}
