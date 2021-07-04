# Speedtime
A timer that runs entirely within a terminal.
\
It works by monitoring the keyboard for keypresses, & starting / stopping itself
whenever a specific key is pressed.  This means that if you use a terminal emulator, you
can start / stop the time without having the window focused.
## Building
If you have Nix installed, you can run `nix-build` in the directory of this repo.
\
If you do not want to install Nix, install gcc or another C compiler, & run `build.sh`.
\
If you want to use a compiler that is not gcc, edit build.sh, & if you are using Nix, 
edit default.nix.
## Future Goals
Check open issues labeled with "future goal"
