# Speedtime
A timer for Linux that runs entirely within a terminal.
\
## How it works
It monitors the keyboard for keypresses, & reacts whenever a specific (configurable)
key is pressed.
\
This means that if you use a terminal emulator, you can interact with the program
without having the window focused.
## Building
Install GCC & Tup & run `tup` in the directory of this repository.
\
Builds that do not use GCC are not supported, but if you want you can edit Tupfile to
use a different compiler.
