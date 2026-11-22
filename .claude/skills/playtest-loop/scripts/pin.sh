#!/bin/bash
# pin.sh NAME: the playtest NAME runs the current Linux build from now on,
# whatever is rebuilt meanwhile (tools/play.py keeps NAME/bin with bin.pin),
# so its replays stay exact. Run python3 build.py linux first.
set -e
cd "$(git rev-parse --show-toplevel)"
h=.build/play/${1:?usage: pin.sh NAME}
mkdir -p "$h"
rm -rf "$h/bin"
mkdir -p "$h/bin"
cp -p build/linux/cyberworld "$h/bin/"
cp -rp build/linux/lib "$h/bin/lib"
touch "$h/bin.pin"
echo "pinned $h/bin to $(git rev-parse --short HEAD)$(git diff --quiet || echo ' (with uncommitted changes)')"
