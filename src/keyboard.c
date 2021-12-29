/* Input handling through evdev */
#include <dirent.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#ifdef __linux__
#include <linux/input.h>
#else
#include <sys/dev/evdev/input.h>
#endif // __linux__

#include "compile_settings.h"
#include "keyboard.h"

#define EVIOCGBIT_SET(bit) (eviocgbit_res[(bit) / 8] & (1 << (bit) % 8))

/* Gives to buf the files that support EV_KEY.
 * Uses optarg if it is given, otherwise autodetects the paths.
 * Returns amount found, or 0 on error/none read. */
int getKeyEventFiles(FILE* buf[MAX_DEVICES], char* optarg)
{
    /* Get paths */
    int path_amount = 0;
    char paths[MAX_DEVICES][MAX_DEVICE_PATH_LEN];

    if (optarg != NULL) {
        const char splitter[] = ",";
        char* strtok_res = strtok(optarg, splitter);
        while (strtok_res != NULL && path_amount < MAX_DEVICES) {
            memcpy(paths[path_amount], strtok_res, MAX_DEVICE_PATH_LEN);
            path_amount++;
            strtok_res = strtok(NULL, splitter);
        }
    } else {
        const char input_path[] = "/dev/input/";
        DIR* input_dir = opendir(input_path);
        if (input_dir == NULL) {
            perror("opendir");
            return 0;
        }

        struct dirent* input_dir_entry;
        while ((input_dir_entry = readdir(input_dir)) != NULL
            && path_amount < MAX_DEVICES) {
            if (strncmp(input_dir_entry->d_name, "event", 5) == 0
                && strlen(input_dir_entry->d_name)
                    <= MAX_DEVICE_PATH_LEN - sizeof(input_path)) {
                memcpy(paths[path_amount], input_path, sizeof(input_path));
                strcat(paths[path_amount], input_dir_entry->d_name);
                path_amount++;
            }
        }

        closedir(input_dir);
    }

    /* Open paths */
    FILE** internal_buf = reallocarray(NULL, path_amount, sizeof(FILE*));
    if (internal_buf == NULL) {
        return 0;
    }
    for (int i = 0; i < path_amount; i++) {
        internal_buf[i] = fopen(paths[i], "rb");
        if (internal_buf[i] == NULL) {
            perror(paths[i]);
            return 0;
        }
    }

    /* Discard files that do not support EV_KEY */
    int file_amount = 0;
    for (int i = 0; i < path_amount; i++) {
        uint8_t eviocgbit_res[EV_KEY / 8 + 1] = { 0 };
        if (ioctl(fileno(internal_buf[i]),
                EVIOCGBIT(0, sizeof(eviocgbit_res)),
                &eviocgbit_res)
                < 0
            || !EVIOCGBIT_SET(EV_KEY)) {
            fclose(internal_buf[i]);
            continue;
        }
        buf[file_amount] = internal_buf[i];
        file_amount++;
    }

    free(internal_buf);
    return file_amount;
}

/* Filters buf to devices that have at least 1 of the keys specified.
 * If there is a key in keys that no devices in buf have, it returns 0,
 * otherwise it returns the new amount of files. */
int filterToSupporting(const uint16_t keys[KEY_AMOUNT],
    FILE* buf[MAX_DEVICES],
    int amount)
{
    bool keys_supported[KEY_AMOUNT] = { false };
    int file_amount = 0;
    for (int i = 0; i < amount; i++) {
        uint8_t eviocgbit_res[KEY_MAX / 8 + 1] = { 0 };
        if (ioctl(
                fileno(buf[i]),
                EVIOCGBIT(EV_KEY, sizeof(eviocgbit_res)),
                &eviocgbit_res)
            < 0) {
            goto file_loop_end;
        }

        bool file_has_key = false;
        for (int j = 0; j < KEY_AMOUNT; j++) {
            if (EVIOCGBIT_SET(keys[j])) {
                file_has_key = true;
                keys_supported[j] = true;
            }
        }
        if (file_has_key) {
            buf[file_amount] = buf[i];
            file_amount++;
            continue;
        }
    file_loop_end:
        fclose(buf[i]);
    }

    bool all_keys_supported = true;
    for (int i = 0; i < KEY_AMOUNT; i++) {
        all_keys_supported = all_keys_supported && keys_supported[i];
    }
    return all_keys_supported ? file_amount : 0;
}

/* Sets key variable to value from optarg.
 * Used in argument parsing. */
void set_key(uint16_t* restrict key, char* optarg)
{
    const uint16_t atoi_res = atoi(optarg);
    if (atoi_res == 0) {
        fputs("Invalid key code specified, using default key\n", stderr);
    } else {
        *key = atoi_res;
    }
}

/* Checks if key was pressed until one is, & sets when to the time it was pressed.
 * Must be called with the same values for files & file_amount every time.
 * Returns: 0 on error getting event, otherwise the key that was pressed */
uint16_t keyPressed(FILE** files, int file_amount, struct timeval* restrict when)
{
    struct pollfd file_pollfds[file_amount];
    for (int i = 0; i < file_amount; i++) {
        const int fd = fileno(files[i]);
        if (fd == -1) {
            perror("fileno");
            return 0;
        }
        file_pollfds[i] = (struct pollfd) { fd, POLLIN, 0 };
    }

    while (poll(file_pollfds, file_amount, -1) != -1) {
        for (int i = 0; i < file_amount; i++) {
            if (file_pollfds[i].revents & POLLERR) {
                return 0;
            }

            if (file_pollfds[i].revents & POLLIN) {
                struct input_event event;
                if (fread(&event, sizeof(event), 1, files[i]) != 1) {
                    return 0;
                }

                if (event.type == EV_KEY && event.value) {
                    *when = event.time;
                    return event.code;
                }
            }
        }
    }

    return 0;
}
