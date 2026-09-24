#!/bin/sh
# Builds a minimal static libmgba (GBA core only, no frontends, scripting,
# debugger or external dependencies) for x86-64 and aarch64 into /opt/mgba;
# with arguments, for those targets only (host, aarch64).
# mGBA is MPL-2.0: https://github.com/mgba-emu/mgba
set -e
VER=0.10.5
SHA=91d6fbd32abcbdf030d58d3f562de25ebbc9d56040d513ff8e5c19bee9dacf14
cd /tmp
curl -fsSL -o mgba.tar.gz "https://github.com/mgba-emu/mgba/archive/refs/tags/$VER.tar.gz"
echo "$SHA  mgba.tar.gz" | sha256sum -c -
tar xzf mgba.tar.gz
OPTS="-DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON -DBUILD_SHARED=OFF -DBUILD_QT=OFF -DBUILD_SDL=OFF \
 -DBUILD_GL=OFF -DBUILD_GLES2=OFF -DBUILD_GLES3=OFF -DUSE_EPOXY=OFF -DDISABLE_DEPS=ON -DUSE_DEBUGGERS=OFF \
 -DUSE_GDB_STUB=OFF -DUSE_EDITLINE=OFF -DENABLE_SCRIPTING=OFF -DUSE_LUA=OFF -DM_CORE_GB=OFF -DM_CORE_GBA=ON \
 -DUSE_DISCORD_RPC=OFF -DBUILD_LTO=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON"
build() { # $1 name, $2 extra cmake args
	cmake -S mgba-$VER -B build-$1 $OPTS -DCMAKE_INSTALL_PREFIX=/opt/mgba/$1 $2
	cmake --build build-$1 -j"$(nproc)"
	cmake --install build-$1
}
TARGETS=${*:-host aarch64}
case " $TARGETS " in *" host "*) build host "" ;; esac
case " $TARGETS " in *" aarch64 "*)
cat > aarch64.cmake <<'T'
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
T
build aarch64 "-DCMAKE_TOOLCHAIN_FILE=/tmp/aarch64.cmake" ;;
esac
cp mgba-$VER/LICENSE /opt/mgba/LICENSE
rm -rf /tmp/mgba* /tmp/build-* /tmp/aarch64.cmake
