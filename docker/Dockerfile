# Build image for the host (x86-64) binary: the tests, captures, tools and
# the code's checks (build.py lint: GCC's analyzer, lizard's complexity).
# (the PortMaster port's aarch64 binary: docker/Dockerfile.portmaster)
FROM debian:trixie
RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential make pkg-config ca-certificates file \
    cmake curl libsdl2-dev python3-pip \
 && pip3 install --break-system-packages --no-cache-dir lizard==1.24.0 \
 && rm -rf /var/lib/apt/lists/*
# The embedded GBA core (docs/EMULATION.md)
COPY mgba.sh /tmp/mgba.sh
RUN sh /tmp/mgba.sh host
