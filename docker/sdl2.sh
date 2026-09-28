#!/bin/sh
# Builds SDL2 for the Linux desktop release into /opt/sdl2. Its video and
# audio backends (X11, Wayland, PulseAudio, PipeWire, ALSA...) load at run
# time when the player's system has them, so the library itself needs only
# glibc. `sdl2.sh windows` builds it for 64-bit Windows instead (MinGW-w64,
# /opt/mingw.cmake), a static library the game links in; `sdl2.sh android`
# only unpacks the source into /opt/sdl2-src, which the Android app builds
# with itself (its Java classes and its CMake project).
# SDL2 is zlib-licensed: https://github.com/libsdl-org/SDL
set -e
VER=2.32.10
SHA=5f5993c530f084535c65a6879e9b26ad441169b3e25d789d83287040a9ca5165
cd /tmp
curl -fsSL -o sdl2.tar.gz "https://github.com/libsdl-org/SDL/releases/download/release-$VER/SDL2-$VER.tar.gz"
echo "$SHA  sdl2.tar.gz" | sha256sum -c -
tar xzf sdl2.tar.gz
if [ "$1" = android ]; then
	mv SDL2-$VER /opt/sdl2-src
	rm -f /tmp/sdl2.tar.gz
	exit 0
fi
if [ "$1" = windows ]; then
	cmake -S SDL2-$VER -B build-sdl2 -DCMAKE_TOOLCHAIN_FILE=/opt/mingw.cmake -DCMAKE_BUILD_TYPE=Release \
	 -DCMAKE_INSTALL_PREFIX=/opt/sdl2 -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TEST=OFF -DSDL_TESTS=OFF
else
	cmake -S SDL2-$VER -B build-sdl2 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/sdl2 \
	 -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST=OFF -DSDL_TESTS=OFF -DSDL_RPATH=OFF
fi
cmake --build build-sdl2 -j"$(nproc)"
cmake --install build-sdl2
cp SDL2-$VER/LICENSE.txt /opt/sdl2/LICENSE.txt
rm -rf /tmp/sdl2.tar.gz /tmp/SDL2-$VER /tmp/build-sdl2
