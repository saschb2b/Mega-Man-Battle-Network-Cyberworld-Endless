# Build image for host (x86-64) and ROCKNIX (aarch64) binaries.
# Debian trixie matches the glibc (2.41) and SDL2 (2.32) on current ROCKNIX.
FROM debian:trixie
RUN dpkg --add-architecture arm64 && apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential crossbuild-essential-arm64 make pkg-config ca-certificates file \
    libsdl2-dev libsdl2-dev:arm64 \
 && rm -rf /var/lib/apt/lists/*
