#ifndef KEYBOARD_EVENTS_H
#define KEYBOARD_EVENTS_H

char* getKeyEventFile(void);
int enterKeyPressed(FILE* keyboard_event_fp, struct timespec* when);

#endif // KEYBOARD_EVENTS_H
