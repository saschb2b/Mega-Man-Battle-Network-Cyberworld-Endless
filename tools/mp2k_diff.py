#!/usr/bin/env python3
"""mp2k_diff.py ORACLE OURS START [FRAMES]: the engine's MP2K (--mp2k-trace)
laid beside BN6's own (romlab's `sound`) frame by frame, from the oracle's
frame START (the song's first engine frame) and ours' 0: the music player
and its tracks, the DirectSound and PSG channels, and the mix each frame
writes, byte for byte. Prints each part's first difference and how many
frames differ; exits 1 where any does. Numbers only; reads no ROM.
docs/ROM_DATA.md, BN6's sound engine, has the layout; docs/EMULATION.md, The
engine's own sound, how it is run."""
import struct
import sys

SI = 0xFB0
REC = SI + 0x100 + 0x40 + 0x280
PERIOD, PER_FRAME = 9, 176
# (offset, size, name) of what each part's records hold that both sides set
CHAN = [(0, 0x0E, 'head'), (0x10, 5, 'note'), (0x18, 8, 'count/fw'), (0x20, 0x18, 'pitch,wav,cur,track,list')]
CGB = [(0, 0x0E, 'head'), (0x10, 5, 'note'), (0x19, 7, 'psg'), (0x20, 0x18, 'pitch,wav,cur,track,list')]
TRACK = [(0, 0x24, 'fields'), (0x24, 12, 'tone'), (0x40, 0x10, 'cmd,stack')]
PLAYER = [(0, 8, 'song,status'), (9, 1, 'priority'), (0xC, 4, 'clock'), (0x1C, 0xE, 'tempo,fade'), (0x30, 4, 'tone')]


def load(path):
    data = open(path, 'rb').read()
    return [data[i:i + REC] for i in range(0, len(data) - REC + 1, REC)]


def slice_of(rec):
    counter = rec[4]
    s = PERIOD - (counter - 1) if counter > 1 else 0
    r = rec[0x350 + s * PER_FRAME:0x350 + (s + 1) * PER_FRAME]
    l = rec[0x350 + 0x630 + s * PER_FRAME:0x350 + 0x630 + (s + 1) * PER_FRAME]
    return r, l


def parts(rec):
    out = {}
    for i in range(4):
        for off, size, name in CHAN:
            out[f'ds{i} {name}'] = rec[0x50 + 0x40 * i + off:0x50 + 0x40 * i + off + size]
        for off, size, name in CGB:
            out[f'psg{i + 1} {name}'] = rec[SI + 0x40 * i + off:SI + 0x40 * i + off + size]
    for off, size, name in PLAYER:
        out[f'music {name}'] = rec[SI + 0x100 + off:SI + 0x100 + off + size]
    for t in range(8):
        for off, size, name in TRACK:
            base = SI + 0x140 + 0x50 * t
            out[f'track{t} {name}'] = rec[base + off:base + off + size]
    out['reverb'] = rec[5:6]
    r, l = slice_of(rec)
    out['mix right'], out['mix left'] = r, l
    return out


def main():
    if len(sys.argv) < 4:
        sys.exit(__doc__)
    oracle, ours, start = load(sys.argv[1] + '.si'), load(sys.argv[2] + '.si'), int(sys.argv[3])
    frames = min(len(ours), len(oracle) - start, int(sys.argv[4]) if len(sys.argv) > 4 else 1 << 30)
    first, count = {}, {}
    for f in range(frames):
        po, pu = parts(oracle[start + f]), parts(ours[f])
        for k in po:
            if po[k] != pu[k]:
                count[k] = count.get(k, 0) + 1
                if k not in first:
                    first[k] = (f, po[k].hex(), pu[k].hex())
    print(f'{frames} frames from the oracle\'s {start}')
    if not first:
        print('PASS: every part the same, every frame')
        return 0
    for k, (f, o, u) in sorted(first.items(), key=lambda kv: kv[1][0]):
        print(f'FAIL {k}: first at frame {f} ({count[k]} frames)\n  game {o[:96]}\n  ours {u[:96]}')
    return 1


if __name__ == '__main__':
    sys.exit(main())
