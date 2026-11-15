#!/bin/sh
# Builds libretro gpSP with the game's additions (docker/gpsp-cyberworld.patch:
# the ROM taken from the game's own pages, hooks the translator calls out to)
# as a static library, its header (cw.h) and libretro.h, into PREFIX:
#   gpsp.sh psp  [PREFIX] [PATCH]   the PSP's (pspdev's psp-gcc, its MIPS dynarec)
#   gpsp.sh host [PREFIX] [PATCH]   a computer's (its x86-64 dynarec), to test the
#                                   game on gpSP here (CYBERWORLD_CORE=gpsp, build.py)
# gpSP is GPL-2.0 (https://github.com/libretro/gpsp): a build linking it is
# distributed under the GPL's terms (docs/engine/emulation/psp-and-ds.md).
set -e
TARGET=$1
PREFIX=${2:-/opt/gpsp/$1}
PATCH=${3:-/tmp/gpsp-cyberworld.patch}
COMMIT=5819380c2ffb0900219d700a382ee68c464ebb99   # 19 September 2026
SHA=1dfc919966cd58ce1d8f2580041d9bffe2e313d9442260c713ed0286a048a733
WORK=$(mktemp -d)
cd "$WORK"
curl -fsSL -o gpsp.tar.gz "https://github.com/libretro/gpsp/archive/$COMMIT.tar.gz"
echo "$SHA  gpsp.tar.gz" | sha256sum -c - >/dev/null
tar xzf gpsp.tar.gz
cd "gpsp-$COMMIT"
patch -s -p1 < "$PATCH"
case $TARGET in
psp)
	make platform=psp1 -j"$(nproc)" >/dev/null
	LIB=gpsp_libretro_psp1.a ;;
host)
	# (unix builds a shared library, which needs no game: its objects archived
	# instead, linked into the game's binary as the PSP's are)
	make platform=unix -j"$(nproc)" >/dev/null
	find . -name '*.o' -exec ar rcs libgpsp.a {} +
	LIB=libgpsp.a ;;
*)
	echo "gpsp.sh: psp or host" >&2
	exit 1 ;;
esac
mkdir -p "$PREFIX/lib" "$PREFIX/include"
cp "$LIB" "$PREFIX/lib/libgpsp.a"
cp cw.h libretro/libretro-common/include/libretro.h "$PREFIX/include/"
cp COPYING "$PREFIX/LICENSE"
cd /
rm -rf "$WORK"
