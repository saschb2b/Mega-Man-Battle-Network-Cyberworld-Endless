#!/usr/bin/env python3
"""rom_tables.py ROM: a Battle Network 4-6 ROM's map tables, found by their
structure (docs/MULTIROM.md). Prints the candidate tables of map group
pointers, each to a list of 12-byte MapBGDescriptors (tile set, palette,
tile map; the tile map a (w, h, 0, 0, u32 12, u32 offset) header before
LZ77 data), and the coordinate data tables whose empty groups match the
internet's map table. Reads the ROM; writes nothing.

  python3 tools/rom_tables.py ~/.cache/mmbn-ref/roms/bn6g.gba   # BN6: 0x0329C4, 0x03354C
"""
import struct
import sys


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    d = open(sys.argv[1], 'rb').read()
    n = len(d)

    def u32(o):
        return struct.unpack_from('<I', d, o)[0]

    def ptr(v):
        return 0x08000000 <= v <= 0x08000000 + n - 16

    def tile_map(o):
        return o + 16 <= n and 16 <= d[o] and 16 <= d[o + 1] and d[o + 2] in (0, 1) and d[o + 3] == 0 and u32(o + 4) == 12 and d[o + 12] == 0x10

    def desc(o):
        return o + 12 <= n and ptr(u32(o)) and ptr(u32(o + 4)) and ptr(u32(o + 8)) and tile_map(u32(o + 8) - 0x08000000)

    def maps(o):
        k = 0
        while desc(o + 12 * k):
            k += 1
        return k

    def coord(o):
        return o + 20 <= n and d[o + 16] == 0x10 and all(x < 0x100000 for x in struct.unpack_from('<IIII', d, o))

    # runs of pointers to descriptor lists, empty groups (0) between
    o, tables = 0, []
    while o < n - 16:
        k = 0
        while ptr(u32(o + 4 * k)) and maps(u32(o + 4 * k) - 0x08000000):
            k += 1
        if k >= 3:
            tables.append(o)
            o += 4 * k
        else:
            o += 4
    for t in tables:
        entries = []
        for i in range(-8, 32):
            v = u32(t + 4 * i)
            entries.append('.' if v == 0 else ('m' if ptr(v) and maps(v - 0x08000000) else '?'))
        print(f'map descriptor lists near 0x{t:06X}: 8 before .. 32 after: {"".join(entries)}')
    # coordinate tables: a pointer per group to lists of coordinate data
    for t in tables[:1]:
        empty = [i for i in range(32) if u32(t + 4 * i) == 0]
        for c in range(0, n - 128, 4):
            ok = True
            for i in range(21):
                v = u32(c + 4 * i)
                if (i in empty) != (v == 0) or (v and not (ptr(v) and ptr(u32(v - 0x08000000)) and coord(u32(v - 0x08000000) - 0x08000000))):
                    ok = False
                    break
            if ok:
                print(f'coordinate data lists at 0x{c:06X} (empty groups as the map table from 0x{t:06X})')


if __name__ == '__main__':
    main()
