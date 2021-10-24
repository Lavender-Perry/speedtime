#ifndef COMPILE_SETTINGS_H
#define COMPILE_SETTINGS_H

#define MAX_KEYBOARDS 3
// Requires linux/input-event-codes.h
#define DEFAULT_STOP_KEY KEY_ESC // Default key to stop the timer
#define DEFAULT_CONTROL_KEY KEY_ENTER // Default key to go to next split/start/stop

#define MAX_SPLITS 15 // Max amount of splits
#define MAX_SPLIT_NAME_LEN 30 // Max amount of chars in a split name & split file path

#endif // COMPILE_SETTINGS_H
