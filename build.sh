err() {
    echo Errors during build
    rm -r /tmp/speedtime_build
    exit 1
}

# For nix-build
[ -z $stdenv ] || source $stdenv/setup
[ -z $src ] && src="./src"
[ -z $out ] && out="./result"

mkdir -p /tmp/speedtime_build $out/bin

for csrc in $src/*.c; do
    $CC -Wall -Wextra -Werror \
        -c $csrc -o /tmp/speedtime_build/${csrc:$(( ${#src} + 1 ))}.o || err
done
$CC -pthread /tmp/speedtime_build/*.o -o $out/bin/speedtime || err

rm -r /tmp/speedtime_build
