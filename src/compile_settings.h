#ifndef COMPILE_SETTINGS_H
#define COMPILE_SETTINGS_H

/* Getting key code macros for DEFAULT_*_KEY default values,
 * & PATH_MAX macro for MAX_*_PATH_LEN default value. */
#ifdef __linux__
#include <linux/input-event-codes.h>
#include <linux/limits.h>
#else
#include <dev/evdev/input-event-codes.h>
#include <sys/syslimits.h>
#endif // __linux__

/* Setting these values:
 *     All max values must be integers greater than 0.
 *     If MAX_DEVICE_PATH_LEN is set below 20, device autodetection may not work.
 *     Setting MAX_TIME_DIGITS lower than 20 may cause time to be loaded incorrectly.
 *     Default keys can be non-negative integers or a macro from input-event-codes.h. */

#define MAX_DEVICES 30               // Max devices the program will monitor for input
#define MAX_DEVICE_PATH_LEN PATH_MAX // Max amount of chars in a device path

#define DEFAULT_STOP_KEY KEY_ESC      // Default key to stop the timer
#define DEFAULT_CONTROL_KEY KEY_ENTER // Default key to go to next split/start/stop

#define MAX_SPLITS 30                // Max amount of splits
#define MAX_SPLITS_PATH_LEN PATH_MAX // Max amount of chars in a split file path
#define MAX_SPLIT_NAME_LEN 50        // Max amount of chars in a split name
#define MAX_TIME_DIGITS 20           // Max digits the saved centiseconds can have

#endif // COMPILE_SETTINGS_H
