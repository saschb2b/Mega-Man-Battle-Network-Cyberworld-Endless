#!/bin/sh
# Builds mbedTLS, static, for the PS4 into /opt/mbedtls (docker/ps4.cmake):
# the TLS online play runs on there (src/online/online_stream_tls.c), as
# OpenOrbis's SDK has no libcurl. Without a platform entropy source of its
# own (the game hands it the kernel's, getentropy) and without 128-bit
# division.
# mbedTLS is Apache-2.0: https://github.com/Mbed-TLS/mbedtls
set -e
VER=2.28.10
SHA=19e5b81fdac0fe22009b9e2bdcd52d7dcafbf62bc67fc59cf0a76b5b5540d149
cd /tmp
curl -fsSL -o mbedtls.tar.bz2 "https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-$VER/mbedtls-$VER.tar.bz2"
echo "$SHA  mbedtls.tar.bz2" | sha256sum -c -
tar xjf mbedtls.tar.bz2
python3 mbedtls-$VER/scripts/config.py -f mbedtls-$VER/include/mbedtls/config.h set MBEDTLS_NO_PLATFORM_ENTROPY
# (no 128-bit division, whose runtime routine the PS4's link has not)
python3 mbedtls-$VER/scripts/config.py -f mbedtls-$VER/include/mbedtls/config.h set MBEDTLS_NO_UDBL_DIVISION
cmake -S mbedtls-$VER -B build-mbedtls -DCMAKE_TOOLCHAIN_FILE=/opt/ps4.cmake -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_INSTALL_PREFIX=/opt/mbedtls -DENABLE_PROGRAMS=OFF -DENABLE_TESTING=OFF -DUSE_SHARED_MBEDTLS_LIBRARY=OFF \
 -DUSE_STATIC_MBEDTLS_LIBRARY=ON
cmake --build build-mbedtls -j"$(nproc)"
cmake --install build-mbedtls
cp mbedtls-$VER/LICENSE /opt/mbedtls/LICENSE
rm -rf /tmp/mbedtls* /tmp/build-mbedtls
