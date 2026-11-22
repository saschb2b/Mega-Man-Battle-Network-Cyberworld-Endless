#!/usr/bin/env python3
"""mp2k_diff.py ORACLE OURS START [FRAMES] [--map MAP]: the engine's MP2K
(--mp2k-trace) laid beside BN6's own (romlab's `sound`) frame by frame, from
the oracle's frame START (the song's first engine frame) and ours' 0: the
music player and its tracks, the DirectSound and PSG channels, and the mix
each frame writes, byte for byte. Prints each part's first difference and
how many frames differ; exits 1 where any does. Numbers only; reads no ROM.
MAP (a JSON list of [from, length, to]) moves the oracle's pointers first:
another game's song against its copy in BN6 (tools/mp2k_verify.py's xN/),
its RAM's places and the song's pieces where BN6 and the copy have them;
then the PSG's registers and wave RAM (romlab's PREFIX.io, 0x04000060 to
0x0400009F; the FIFOs after them follow the DMA's timing) each frame too,
what the PSG plays.
docs/rom/sound.md, BN6's sound engine, has the layout; docs/engine/emulation/engine-sound.md, The
engine's own sound, how it is run."""
import json
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


def moved(rec, ranges):
    """the record with its pointers where `ranges` moves them: the channels'
    sample, place, track and chain, the player's song and voices, the
    tracks' channel, voice and commands (a voice's +4 where it has a
    pointer, its +8 a key split's)"""
    b = bytearray(rec)

    def move(off):
        v = struct.unpack_from('<I', b, off)[0]
        for lo, n, to in ranges:
            if lo <= v < lo + n:
                struct.pack_into('<I', b, off, v - lo + to)
                return
    for i in range(4):
        for f in (0x24, 0x28, 0x2C, 0x30, 0x34):
            move(0x50 + 0x40 * i + f)
            move(SI + 0x40 * i + f)
    move(SI + 0x100)
    move(SI + 0x130)
    for t in range(8):
        base = SI + 0x140 + 0x50 * t
        for f in (0x20, 0x40, 0x44, 0x48, 0x4C):
            move(base + f)
        kind = b[base + 0x24]
        if kind & 0xC0 or kind & 7 in (0, 3):
            move(base + 0x28)
        if kind & 0x40:
            move(base + 0x2C)
    return bytes(b)


def main():
    args = sys.argv[1:]
    ranges = None
    if '--map' in args:
        i = args.index('--map')
        # (the smallest piece first: a pointer is its own piece's, though a
        # bigger one holds its bytes too, as BN5's drum kit's other data
        # copied as samples holds waves)
        ranges = sorted(json.load(open(args[i + 1])), key=lambda r: r[1])
        del args[i:i + 2]
    if len(args) < 3:
        sys.exit(__doc__)
    oracle, ours, start = load(args[0] + '.si'), load(args[1] + '.si'), int(args[2])
    frames = min(len(ours), len(oracle) - start, int(args[3]) if len(args) > 3 else 1 << 30)
    first, count = {}, {}
    for f in range(frames):
        po, pu = parts(moved(oracle[start + f], ranges) if ranges else oracle[start + f]), parts(ours[f])
        for k in po:
            if po[k] != pu[k]:
                count[k] = count.get(k, 0) + 1
                if k not in first:
                    first[k] = (f, po[k].hex(), pu[k].hex())
    if ranges:
        io_o, io_u = open(args[0] + '.io', 'rb').read(), open(args[1] + '.io', 'rb').read()
        for f in range(frames):
            a, b = io_o[0x50 * (start + f):0x50 * (start + f) + 0x40], io_u[0x50 * f:0x50 * f + 0x40]
            if a != b:
                count['psg registers'] = count.get('psg registers', 0) + 1
                if 'psg registers' not in first:
                    first['psg registers'] = (f, a.hex(), b.hex())
    print(f'{frames} frames from the oracle\'s {start}')
    if ranges and 'psg registers' not in first:
        print('the PSG\'s registers the same, every frame')
    if not first:
        print('PASS: every part the same, every frame')
        return 0
    for k, (f, o, u) in sorted(first.items(), key=lambda kv: kv[1][0]):
        print(f'FAIL {k}: first at frame {f} ({count[k]} frames)\n  game {o[:96]}\n  ours {u[:96]}')
    return 1


if __name__ == '__main__':
    sys.exit(main())
