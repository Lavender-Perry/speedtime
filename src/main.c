#include "getKeyEventFile.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

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
    // TODO: Actually do stuff with the file
    fclose(key_event_file);
}
