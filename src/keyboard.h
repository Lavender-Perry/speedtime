#ifndef KEYBOARD_EVENTS_H
#define KEYBOARD_EVENTS_H

int getKeyEventPaths(char** restrict buf, int max, char* optarg);
// Requires asm/types.h
void set_key(__u16* restrict key, char* optarg);
// Requires asm/types.h & sys/time.h
__u16 keyPressed(FILE** files, int file_amount, struct timeval* restrict when);

#endif // KEYBOARD_EVENTS_H
