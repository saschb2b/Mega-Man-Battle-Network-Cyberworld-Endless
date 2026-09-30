#!/bin/sh
# Builds a minimal static libmgba (GBA core only, no frontends, scripting,
# debugger or external dependencies) for x86-64 and aarch64 into /opt/mgba;
# with arguments, for those targets only (host, aarch64, web, windows: the
# MinGW-w64 cross compiler of docker/Dockerfile.windows; android: each
# Android ABI with the NDK of docker/Dockerfile.android; 3ds: the Nintendo
# 3DS, with devkitPro's devkitARM and mGBA's own 3DS toolchain file, the
# library alone). The web build
# (WebAssembly, in the Emscripten image) runs without threads: a page served
# without cross-origin isolation cannot share memory between them.
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
	cmake -S mgba-$VER -B "build-$1" $OPTS -DCMAKE_INSTALL_PREFIX="/opt/mgba/$1" $2
	cmake --build "build-$1" -j"$(nproc)"
	cmake --install "build-$1"
}
TARGETS=${*:-host aarch64}
case " $TARGETS " in *" host "*) build host "" ;; esac
case " $TARGETS " in *" web "*)
	emcmake cmake -S mgba-$VER -B build-web $OPTS -DCMAKE_INSTALL_PREFIX=/opt/mgba/web \
	 -DCMAKE_C_FLAGS="-DDISABLE_THREADING -D_GNU_SOURCE" -DHAVE_PTHREAD_H=OFF
	cmake --build build-web -j"$(nproc)"
	cmake --install build-web ;;
esac
case " $TARGETS " in *" windows "*)
	# (its CMake asks for epoxy on Windows, for the OpenGL of its own
	# frontends, which are not built here)
	sed -i 's/if(WIN32 AND NOT (LIBMGBA_ONLY OR SKIP_LIBRARY OR USE_EPOXY))/if(FALSE)/' mgba-$VER/CMakeLists.txt
	build windows "-DCMAKE_TOOLCHAIN_FILE=/opt/mingw.cmake" ;;
esac
case " $TARGETS " in *" 3ds "*)
	# (the library target alone: mGBA's own 3DS frontend needs the GB core;
	# its headers copied as its install would, the generated ones too. Its
	# 3DS setup, linked into the library, takes a 32 MB buffer that the
	# core copies the ROM into before main: the game hands the core 16 MB,
	# so the buffer is 16 MB. The game splits the app's memory itself,
	# src/core/main.c. Its pictures are 32-bit, as on every other target:
	# the 3DS's 16-bit ones were set for the library alone, not in the
	# headers it installs, and the game read them as 32-bit, two rows side
	# by side)
	sed -i -e 's/romBuffer = malloc(0x02000000);/romBuffer = malloc(0x01000000);/' \
	 -e 's/romBufferSize = 0x02000000;/romBufferSize = 0x01000000;/' mgba-$VER/src/platform/3ds/ctru-heap.c
	if grep -q 0x02000000 mgba-$VER/src/platform/3ds/ctru-heap.c; then exit 1; fi
	sed -i 's/ COLOR_16_BIT COLOR_5_6_5 / /' mgba-$VER/src/platform/3ds/CMakeLists.txt
	if grep -q COLOR_16_BIT mgba-$VER/src/platform/3ds/CMakeLists.txt; then exit 1; fi
	cmake -S mgba-$VER -B build-3ds $OPTS -DCMAKE_INSTALL_PREFIX=/opt/mgba/3ds \
	 -DCMAKE_TOOLCHAIN_FILE="$PWD/mgba-$VER/src/platform/3ds/CMakeToolchain.txt" -DLIBMGBA_ONLY=ON \
	 -DCMAKE_POSITION_INDEPENDENT_CODE=OFF
	cmake --build build-3ds --target mgba -j"$(nproc)"
	mkdir -p /opt/mgba/3ds/lib /opt/mgba/3ds/include
	cp build-3ds/libmgba.a /opt/mgba/3ds/lib/
	cp -r mgba-$VER/include/mgba mgba-$VER/include/mgba-util /opt/mgba/3ds/include/
	cp -r build-3ds/include/mgba /opt/mgba/3ds/include/ 2>/dev/null || true ;;
esac
case " $TARGETS " in *" android "*)
	for abi in arm64-v8a armeabi-v7a x86_64; do
		build android/$abi "-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake -DANDROID_ABI=$abi -DANDROID_PLATFORM=android-21"
	done ;;
esac
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
