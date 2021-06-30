err() {
    echo Errors during build
    exit 1
}

# For nix-build
[ -z $stdenv ] || source $stdenv/setup
[ -z $src ] && src="./src" || cp -r $src ./
[ -z $out ] && out="./result"

mkdir -p build $out/bin

for csrc in $src/*.c; do
    gcc -Wall -Wextra -Werror -c $csrc -o build/${csrc:$(( ${#src} + 1 ))}.o || err
done
gcc -pthread build/*.o -o $out/bin/speedtime || err
