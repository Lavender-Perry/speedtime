#ifndef KEYBOARD_EVENTS_H
#define KEYBOARD_EVENTS_H

#include <asm/types.h>
#include <stdio.h>
#include <sys/time.h>

int getKeyEventFiles(FILE* buf[MAX_KEYBOARDS], char* optarg);
void set_key(__u16* restrict key, char* optarg);
__u16 keyPressed(FILE** files, int file_amount, struct timeval* restrict when);

#endif // KEYBOARD_EVENTS_H
