#!/bin/sh
# Builds SDL2 and the GBA core for macOS into PREFIX ($1) as static,
# universal libraries (Apple silicon and Intel, macOS 11 on), with their
# licenses in PREFIX/share/licenses. Runs on a Mac (build.py macos; the CI
# job caches PREFIX). SDL2 is zlib-licensed, mGBA MPL-2.0.
set -e
PREFIX=${1:?usage: deps.sh PREFIX}
SDL_VER=2.32.10
SDL_SHA=5f5993c530f084535c65a6879e9b26ad441169b3e25d789d83287040a9ca5165
MGBA_VER=0.10.5
MGBA_SHA=91d6fbd32abcbdf030d58d3f562de25ebbc9d56040d513ff8e5c19bee9dacf14
MAC="-DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64;x86_64 -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0"
JOBS=$(sysctl -n hw.ncpu)
work=$(mktemp -d)
cd "$work"

curl -fsSL -o sdl2.tar.gz "https://github.com/libsdl-org/SDL/releases/download/release-$SDL_VER/SDL2-$SDL_VER.tar.gz"
echo "$SDL_SHA  sdl2.tar.gz" | shasum -a 256 -c -
tar xzf sdl2.tar.gz
cmake -S SDL2-$SDL_VER -B build-sdl2 $MAC -DCMAKE_INSTALL_PREFIX="$PREFIX" \
 -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TEST=OFF -DSDL_TESTS=OFF
cmake --build build-sdl2 -j"$JOBS"
cmake --install build-sdl2

# (docker/mgba.sh's options: the GBA core alone, no frontends or dependencies)
curl -fsSL -o mgba.tar.gz "https://github.com/mgba-emu/mgba/archive/refs/tags/$MGBA_VER.tar.gz"
echo "$MGBA_SHA  mgba.tar.gz" | shasum -a 256 -c -
tar xzf mgba.tar.gz
cmake -S mgba-$MGBA_VER -B build-mgba $MAC -DCMAKE_INSTALL_PREFIX="$PREFIX" \
 -DBUILD_STATIC=ON -DBUILD_SHARED=OFF -DBUILD_QT=OFF -DBUILD_SDL=OFF -DBUILD_GL=OFF -DBUILD_GLES2=OFF \
 -DBUILD_GLES3=OFF -DUSE_EPOXY=OFF -DDISABLE_DEPS=ON -DUSE_DEBUGGERS=OFF -DUSE_GDB_STUB=OFF -DUSE_EDITLINE=OFF \
 -DENABLE_SCRIPTING=OFF -DUSE_LUA=OFF -DM_CORE_GB=OFF -DM_CORE_GBA=ON -DUSE_DISCORD_RPC=OFF -DBUILD_LTO=OFF
cmake --build build-mgba -j"$JOBS"
cmake --install build-mgba

mkdir -p "$PREFIX/share/licenses"
cp SDL2-$SDL_VER/LICENSE.txt "$PREFIX/share/licenses/SDL2.txt"
cp mgba-$MGBA_VER/LICENSE "$PREFIX/share/licenses/mGBA.txt"
cd /
rm -rf "$work"
