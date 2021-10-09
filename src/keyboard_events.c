#include <asm/types.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Autodetects & returns the path of the file that stores keyboard events */
char* getKeyEventFile(void) {
    static char return_value[20] = {0};

    // /proc/bus/input/devices gives a list of devices & info about them
    FILE* devices_list = fopen("/proc/bus/input/devices", "r");
    if (!devices_list) {
        perror("fopen");
        return return_value;
    }

    char line[100];
    char* event_handler;

    while (fgets(line, sizeof(line), devices_list) != NULL) {
        if (event_handler) {
            const char strcheck[] = "B: EV=120013";
            const size_t checklen = strlen(strcheck);

            if (strlen(line) >= checklen
                    && !strncmp(line, strcheck, checklen)) { // Device is a keyboard
                const char* input_dir = "/dev/input/";

                strcpy(return_value, input_dir);
                strcat(return_value, event_handler);
                break;
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
    return return_value;
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

/* Checks if key was pressed until one is, & sets when to the time it was pressed
 * Returns: 0xffff on error getting event, or the key that was pressed */
__u16 keyPressed(FILE* keyboard_event_fp, struct timeval* restrict when) {
    struct input_event event;
    while (fread(&event, sizeof(event), 1, keyboard_event_fp) == 1)
        if (event.type == EV_KEY && event.value) {
            *when = event.time;
            return event.code;
        }
    // Error
    return 0xffff;
}
