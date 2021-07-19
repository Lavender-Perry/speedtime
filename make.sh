#!/usr/bin/env sh

qwhich() {
    which $1 > /dev/null
}

mkdir -p src/user
[ -f src/user/config.h ] || cp src/config.def.h src/user/config.h

{ qwhich nix-build && { nix-build; exit; } } || {
    qwhich $CC || { qwhich clang && CC=clang; } || { qwhich gcc && CC=gcc; } ||
    { echo No C compiler found, install one or set CC to a C compiler; exit 1; }

    ./build.sh
}
