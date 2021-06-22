#include "getKeyEventFile.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

int main(int argc, char** argv) {
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
    while (1) {
        unsigned short event_info[4];
        // Skip past date information
        fseek(key_event_file, sizeof(struct timeval), SEEK_CUR);

        size_t bytes_read = fread(event_info, 2, 4, key_event_file) * 2;
        // Enter key is code 28
        // We need event type 1
        // This does not do what it should yet, it is just for testing
        if (bytes_read == 8) {
            printf(
                    "type: %hu, code: %hu, value: %u\n",
                    event_info[0],
                    event_info[1],
                    event_info[2] + ((unsigned int) event_info[3] << 8));
        } else
            fprintf(
                    stderr,
                    "Error reading event: expected 8 bytes, recieved %lu\n",
                    bytes_read);
    }
    fclose(key_event_file);
}
