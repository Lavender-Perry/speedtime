#!/usr/bin/env sh

qwhich() {
    which "$@" > /dev/null 2>&1
}

clean_exit() {
    rm -r $BUILD
    exit $1
}

[ -z $BUILD ] && BUILD="./build"
[ -z $SRC ] && SRC="./src"
[ -z $OUT ] && OUT="./result"

mkdir -p $BUILD $OUT/bin $SRC/user

if [ ! -f src/user/config.h ]; then
    cp $SRC/config.def.h $SRC/user/config.h
    echo -n "Config header created.  Would you like to edit it before building? (y/n) "
    read answer
    if [ "$answer" != "${answer#[Yy]}" ]; then
        if [ -z $EDITOR ]; then
            echo "\$EDITOR not set.  Please edit $SRC/user/config.h manually.\
                Then run this script again."
            clean_exit 1
        else
            $EDITOR $SRC/user/config.h
        fi
    fi
fi

qwhich $CC || { qwhich clang && CC=clang; } || { qwhich gcc && CC=gcc; } || {
    echo "No C compiler found, install one or set CC to a C compiler"
    clean_exit 1
}

for csrc in $SRC/*.c; do
    $CC -Wall -Wextra -Werror -c $csrc -o $BUILD/${csrc:$(( ${#SRC} + 1 ))}.o ||
        clean_exit 1
done
$CC -pthread $BUILD/*.o -o $OUT/bin/speedtime || clean_exit 1

echo "Build succeeded.  Executable is at $OUT/bin/speedtime"
clean_exit 0
