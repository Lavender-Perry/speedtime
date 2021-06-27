#include <errno.h>
#include <linux/input-event-codes.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "speedtime.h"
#include "utils.h"

int main(const int argc, char** argv) {
    /* Find & open the key event handler file,
     * storing the file pointer in key_event_fp. */

    char* key_event_path = NULL;
    bool free_key_event_path = false;
    // Set key_event_path to a custom file path if one is given
    for (int i = 2; i < argc; i++)
        if (!strcmp(argv[i - 1], "-k")) {
            key_event_path = argv[i];
            break;
        }
    if (!key_event_path) { // No custom file path given
        key_event_path = getKeyEventFile();
        if (!key_event_path) {
            fputs("Error finding the keyboard event file.\n"
                    "Please specify the file by adding the arguments "
                    "\"-k /path/to/event_file\"\n", stderr);
            return errno ? errno : EXIT_FAILURE;
        }
        free_key_event_path = true;
    }
    FILE* key_event_fp = fopen(key_event_path, "r");
    if (free_key_event_path)
        free(key_event_path);
    if (!key_event_fp) {
        perror("Error opening key event file");
        return errno;
    }

    /* Make a struct pollfd (key_event_pollfd) set up to check if there is input in the
     * file from key_event_fp */

    struct pollfd key_event_pollfd;
    key_event_pollfd.fd = fileno(key_event_fp);
    key_event_pollfd.events = POLLIN;

    /* Set up for starting the event loop */

    int return_value = EXIT_SUCCESS;
    puts_no_newline("0:00.00");

    /* Start the main event loop of the program,
     * which controls the timer & keeps track of time */

    while (true) { // Main event loop
        static clock_t start_clock = 0; // Stores value from clock() when timer started
        static const clock_t clock_err = (clock_t) -1; // Error value from clock()

        /* Check if the key event file has any data to read.
         * If not, update the timer
         * If so, read from key_event_fp & check if the enter key was pressed.
         * If it was, toggle the timer */

        poll(&key_event_pollfd, 1, 0);
        const short returned_events = key_event_pollfd.revents;
        if (returned_events & POLLERR) {
            EVENT_LOOP_ERR_EXIT("Error checking for keyboard events.\n")
        }
        if (returned_events & POLLHUP) {
            EVENT_LOOP_ERR_EXIT("Keyboard event stream hung up.  "
                    "The file is probably incorrect.\n")
        }
        if (key_event_pollfd.revents & POLLIN) { // Input available bit is set 
            // event_info[0]: type, event_info[1]: code,
            // event_info[2]: value (lower byte), event_info[3]: value (upper byte)
            unsigned short event_info[4];

            // Skip past date information
            fseek(key_event_fp, sizeof(struct timeval), SEEK_CUR);

            const size_t bytes_read = fread(event_info, 2, 4, key_event_fp) * 2;
            if (bytes_read != 8) {
                EVENT_LOOP_ERR_EXIT("Error reading event: not enough bytes\n")
            }
            // No need to check event_info[3], it is always 0 for key inputs
            if (event_info[0] == EV_KEY
                    && event_info[1] == KEY_ENTER
                    && event_info[2]) {
                clock_t keypress_clock = clock();
                if (keypress_clock == clock_err) {
                    EVENT_LOOP_ERR_EXIT(CLOCK_ERR_STR)
                }
                if (start_clock) {
                    print_clock_time(keypress_clock - start_clock);
                    puts_no_newline("\n0:00.00");
                    start_clock = 0;
                } else
                    start_clock = keypress_clock;
            }
        } else if (start_clock) { /* Update the time if the timer is going */
            clock_t current_clock = clock();
            if (current_clock == clock_err) {
                EVENT_LOOP_ERR_EXIT(CLOCK_ERR_STR)
            }
            print_clock_time(current_clock - start_clock);
        }
    }
    fclose(key_event_fp);
    return return_value;
}

/* Autodetects & returns the path of the file that stores keyboard events */
char* getKeyEventFile() {
    // /proc/bus/input/devices gives a list of devices & info about them
    FILE* devices_list = fopen("/proc/bus/input/devices", "r");
    if (!devices_list) {
        perror("Error opening input device list");
        return NULL;
    }

    // Value will not change.  It is not const because getline would not accept it.
    size_t bufsize = 100;

    char* line = malloc(bufsize);
    if (!line) {
        fputs("Error allocating memory to read line from list of devices\n", stderr);
        return NULL;
    }

    char* event_handler = NULL;
    char* return_value = NULL;

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
    fclose(devices_list);
    return return_value;
}

void print_clock_time(const clock_t clock_time) {
    const int time_int = clock_time * 100 / CLOCKS_PER_SEC;
    printf("\r%i:%.2i.%.2i", time_int / 6000, time_int / 100 % 60, time_int % 100);
    fflush(stdout);
}
