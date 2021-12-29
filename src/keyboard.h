#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

#include "compile_settings.h"

#define KEY_AMOUNT 2 // Amount of keys this program monitors for

int getKeyEventFiles(FILE* buf[MAX_DEVICES], char* optarg);
int filterToSupporting(const uint16_t keys[KEY_AMOUNT],
    FILE* buf[MAX_DEVICES],
    int amount);
void set_key(uint16_t* restrict key, char* optarg);
uint16_t keyPressed(FILE** files, int file_amount, struct timeval* restrict when);

#endif // KEYBOARD_H
