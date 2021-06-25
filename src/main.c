#include "getKeyEventFile.h"
#include <errno.h>
#include <linux/input-event-codes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

int main(int argc, char** argv) {
    int return_value = 0;

    char* key_event_handler = NULL;
    for (int i = 2; i < argc; i++)
        if (!strcmp(argv[i - 1], "-k")) {
            key_event_handler = argv[i];
            break;
        }
    if (!key_event_handler) {
        key_event_handler = getKeyEventFile();
        if (!key_event_handler) {
            fprintf(stderr, "Error finding the keyboard event file.\n"
                    "Please specify the file by adding the arguments "
                    "\"-k /path/to/event_file\"\n");
            return errno ? errno : 1;
        }
    }

    char key_event_path[20] = "/dev/input/";
    strcat(key_event_path, key_event_handler);
    FILE* key_event_file = fopen(key_event_path, "r");
    if (errno) {
        perror("Error opening key event file");
        return errno;
    }

    while (true) {
        unsigned short event_info[4];
        // Skip past date information
        fseek(key_event_file, sizeof(struct timeval), SEEK_CUR);

        const size_t bytes_read = fread(event_info, 2, 4, key_event_file) * 2;
        // This does not do what it should yet, it just exits the loop instead
        if (bytes_read == 8) {
            // type is event_info[0] & code is event_info[1]
            // event_info[2] stores all binary below 256 for value
            // It is the only part we care about because value from key event is
            // always less than 256 (can only be 0, 1, or 2).  O signals key release,
            // all other values mean the key is pressed
            if (event_info[0] == EV_KEY && event_info[1] == KEY_ENTER && event_info[2])
                break;
            }
        } else {
            fprintf(
                    stderr,
                    "Error reading event: expected 6 bytes, recieved %lu\n",
                    bytes_read);
            return_value = 1;
            break;
        }
    }
    fclose(key_event_file);
    return return_value;
}
