# Build image for the host (x86-64) binary: the tests, captures and tools.
# (the PortMaster port's aarch64 binary: docker/Dockerfile.portmaster)
FROM debian:trixie
RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential make pkg-config ca-certificates file \
    cmake curl libsdl2-dev \
 && rm -rf /var/lib/apt/lists/*
# The embedded GBA core (docs/EMULATION.md)
COPY mgba.sh /tmp/mgba.sh
RUN sh /tmp/mgba.sh host
