# Research harness: runs the player's own ROM headlessly through libmgba to
# capture screens and VRAM while locating data. Never part of a release.
FROM debian:trixie
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential libmgba-dev python3 python3-pil python3-numpy \
 && rm -rf /var/lib/apt/lists/*
