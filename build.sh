#! /usr/bin/env bash

err() {
    echo Errors during build
    exit
}

echo Building speedtime...

for csrc in src/*.c; do
    gcc -Wall -Wextra -Werror -c $csrc -o build/${csrc:4}.o || err
done
gcc build/*.o -o build/result && echo Executable is at build/result || err
