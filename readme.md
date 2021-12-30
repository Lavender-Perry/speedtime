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

This project uses [Meson](https://mesonbuild.com/) for builds, build it as you would any other Meson project.
\
Builds are confirmed to work on the x86-64 architecture, glibc or BSD libc, & Linux or FreeBSD.
\
Your OS must use evdev for input (Linux & FreeBSD both do).
