#!/bin/sh
# Builds SDL2 for the Linux desktop release into /opt/sdl2. Its video and
# audio backends (X11, Wayland, PulseAudio, PipeWire, ALSA...) load at run
# time when the player's system has them, so the library itself needs only
# glibc. SDL2 is zlib-licensed: https://github.com/libsdl-org/SDL
set -e
VER=2.32.10
SHA=5f5993c530f084535c65a6879e9b26ad441169b3e25d789d83287040a9ca5165
cd /tmp
curl -fsSL -o sdl2.tar.gz "https://github.com/libsdl-org/SDL/releases/download/release-$VER/SDL2-$VER.tar.gz"
echo "$SHA  sdl2.tar.gz" | sha256sum -c -
tar xzf sdl2.tar.gz
cmake -S SDL2-$VER -B build-sdl2 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/sdl2 \
 -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST=OFF -DSDL_TESTS=OFF -DSDL_RPATH=OFF
cmake --build build-sdl2 -j"$(nproc)"
cmake --install build-sdl2
cp SDL2-$VER/LICENSE.txt /opt/sdl2/LICENSE.txt
rm -rf /tmp/sdl2.tar.gz /tmp/SDL2-$VER /tmp/build-sdl2
