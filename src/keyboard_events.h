#ifndef KEYBOARD_EVENTS_H
#define KEYBOARD_EVENTS_H

char* getKeyEventFile(void);
// Requires asm/types.h
__u16 keyPressed(FILE* keyboard_event_fp, struct timeval* restrict when);

#endif // KEYBOARD_EVENTS_H
