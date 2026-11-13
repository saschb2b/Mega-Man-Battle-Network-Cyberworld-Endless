#!/usr/bin/env python3
"""The GFXAnim scripts a net map starts as it is entered, read from the ROM:
which tiles each tile animation writes, how many a frame, how many frames
and how long each stays (docs/rom/graphics.md, Net maps' own tile
animations).

    python3 tools/map_anims.py ROM GROUP [MAP ...]     # GROUP in hex: 95 the Undernet

A map sets those tile numbers where the animation shows; in the tile set as
the ROM stores it they hold numbered placeholders.
"""
import struct
import sys

BUS = 0x08000000
MAP_ANIMS = 0x030998   # RomLayout.map_anims: the groups' LoadGFXAnims routines, one a group from 0x80


def scripts(rom, group, number):
    """The scripts of map (group, number): dicts of at, cmd, source, to, count, frames, delays."""
    u32 = lambda o: struct.unpack_from('<I', rom, o)[0]
    u16 = lambda o: struct.unpack_from('<H', rom, o)[0]
    routine = (u32(MAP_ANIMS + 4 * (group - 0x80)) & ~1) - BUS
    # push {lr}; lsls r1, r1, #2; ldr r0, =lists; ldr r0, [r0, r1]
    if u16(routine) != 0xB500 or u16(routine + 2) != 0x0089 or u16(routine + 4) >> 8 != 0x48:
        raise SystemExit(f'group {group:#x}: not the routine expected at {routine + BUS:#010x}')
    lists = u32(((routine + 8) & ~3) + 4 * (u16(routine + 4) & 0xFF)) - BUS
    at, out = u32(lists + 4 * number) - BUS, []
    while u32(at) != 0xFFFFFFFF:
        s = u32(at) - BUS
        frames, p = [], s + 12
        while u32(p) > 2 and len(frames) < 80:
            frames.append(u32(p + 4))
            p += 8
        out.append(dict(at=s + BUS, cmd=rom[s + 8], source=u32(s), to=u32(s + 4), count=rom[s + 10], frames=len(frames), delays=sorted(set(frames))))
        at += 4
    return out


def main():
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)
    rom = open(sys.argv[1], 'rb').read()
    group = int(sys.argv[2], 16)
    for number in [int(a) for a in sys.argv[3:]] or range(3):
        print(f'map {group:02x}:{number}')
        for s in scripts(rom, group, number):
            if s['cmd'] not in (4, 0x18):
                continue   # (a palette's colours: another header)
            tile = (s['to'] - 0x06000000) // 32
            print(f"  script {s['at']:#010x}: {s['count']} tiles a frame to VRAM {s['to']:#010x} (tile {tile}),"
                  f" {s['frames']} frames, {'/'.join(map(str, s['delays']))} game frames each, its sheet {s['source']:#010x}")


if __name__ == '__main__':
    main()
