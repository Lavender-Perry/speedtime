#include <errno.h>
#include <linux/input-event-codes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "speedtime.h"

int main(const int argc, const char** argv) {
    /* Find & open the key event handler file,
     * storing the file pointer in key_event_file */

    char* key_event_path = NULL;
    // Set key_event_path to a custom file path if one is given
    for (int i = 2; i < argc; i++)
        if (!strcmp(argv[i - 1], "-k")) {
            key_event_path = argv[i];
            break;
        }
    if (!key_event_path) { // No custom file path given
        key_event_path = getKeyEventFile();
        if (!key_event_path) {
            fputs(stderr, "Error finding the keyboard event file.\n"
                    "Please specify the file by adding the arguments "
                    "\"-k /path/to/event_file\"\n");
            return errno ? errno : EXIT_FAILURE;
        }
    }
    FILE* key_event_file = fopen(key_event_path, "r");
    if (errno) {
        perror("Error opening key event file");
        return errno;
    }

    /* Start the main event loop of the program,
     * which controls the timer & keeps track of time */

    int return_value = EXIT_SUCCESS;
    puts("0:00.00");
    while (true) { // Main event loop
        static clock_t start_clock = 0; // Stores value from clock() when timer started

        /* Read from key_event_file & check if the enter key was pressed.
         * If it was, toggle the timer */

        // event_info[0]: type, event_info[1]: code,
        // event_info[2]: value (lower byte), event_info[3]: value (upper byte)
        unsigned short event_info[4];

        // Skip past date information
        fseek(key_event_file, sizeof(struct timeval), SEEK_CUR);

        const size_t bytes_read = fread(event_info, 2, 4, key_event_file) * 2;
        if (bytes_read != 8) {
            fprintf(
                    stderr,
                    "Error reading event: expected 8 bytes, recieved %lu\n",
                    bytes_read);
            return_value = EXIT_FAILURE;
            break;
        }
        // No need to check event_info[3], it is always 0 for key inputs
        if (event_info[0] == EV_KEY
                && event_info[1] == KEY_ENTER
                && event_info[2]) {
            clock_t keypress_clock = clock();
            if (start_clock) {
                print_clock_time(keypress_clock - start_clock);
                puts("\n0:00.00");
                start_clock = 0;
            } else
                start_clock = keypress_clock;
        }

        /* Update the time if the timer is going */
        if (start_clock)
            print_clock_time(clock() - start_clock);
    }
    fclose(key_event_file);
    return return_value;
}

/* Autodetects & returns the path of the file that stores keyboard events */
char* getKeyEventFile() {
    // /proc/bus/input/devices gives a list of devices & info about them
    FILE* devices_list = fopen("/proc/bus/input/devices", "r");
    if (errno) {
        perror("Error opening input device list");
        return NULL;
    }

    // Value will not change.  It is not const because getline would not accept it.
    size_t bufsize = 100;

    char* line = (char*) malloc(bufsize * sizeof(char));
    char* event_handler = NULL;

    char* return_value = NULL;

    while (getline(&line, &bufsize, devices_list) != EOF) {
        if (event_handler) {
            const char strcheck[] = "B: EV=120013";
            const size_t checklen = strlen(strcheck);

            if (strlen(line) >= checklen
                    && !strncmp(line, strcheck, checklen)) { // Device is a keyboard
                return_value = "/dev/input/";
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
    fclose(devices_list);
    return return_value;
}

void print_clock_time(const clock_t clock_time) {
    const int time_int = clock_time * 100 / CLOCKS_PER_SEC;
    printf("\r%i:%.2i.%.2i", time_int / 6000, time_int / 100 % 60, time_int % 100);
    fflush(stdout);
}
