#! /usr/bin/env bash

for csrc in src/*.c; do
    trimmed_csrc=${csrc:4}
    gcc -Wall -Wextra -Werror -c $csrc -o build/${trimmed_csrc%?}o
done \
    && gcc build/*.o -o build/result \
    && echo Executable is at build/result \
    || echo Errors during build \
