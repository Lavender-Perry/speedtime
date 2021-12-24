# Speedtime

A timer that runs entirely within a terminal.

## How it works

It monitors the keyboard for keypresses, & reacts whenever a specific (configurable)
key is pressed.
\
This means that if you use a terminal emulator, you can interact with the program
without having the window focused.

## Goals

1.  To have as low resource usage as possible.
2.  To have plain output that is easy to parse so it can be used with alternative frontends.

## Building

1.  Install a C compiler & change the Tupfile to build with that C compiler
2.  Install Tup
3.  Run `tup` in the directory of this repository.

Builds are only confirmed to work with Clang & GCC on x86-64 Linux with glibc.
\
Builds do not work on non-Linux systems.
