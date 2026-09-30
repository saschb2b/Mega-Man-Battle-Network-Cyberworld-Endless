#!/bin/sh
# The 3DS's CIA tools (build.py 3ds): makerom (Project_CTR) packs the game
# as a title the HOME Menu installs, bannertool makes the banner it shows
# when the game is chosen. Built from their sources at pinned versions.
set -e
cd /tmp
git clone -q --depth 1 --branch makerom-v0.19.0 https://github.com/3DSGuy/Project_CTR ctr
make -C ctr/makerom deps -j"$(nproc)" >/dev/null
make -C ctr/makerom -j"$(nproc)" >/dev/null
install -m 755 ctr/makerom/bin/makerom /usr/local/bin/makerom
git clone -q --recursive --depth 1 --shallow-submodules --branch 1.2.0 https://github.com/diasurgical/bannertool bt
make -C bt -j"$(nproc)" >/dev/null
install -m 755 bt/output/linux-x86_64/bannertool /usr/local/bin/bannertool
rm -rf ctr bt
makerom -h >/dev/null 2>&1 || true
test -x /usr/local/bin/makerom && test -x /usr/local/bin/bannertool
