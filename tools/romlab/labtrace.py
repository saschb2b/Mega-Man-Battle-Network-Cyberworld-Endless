#!/usr/bin/env python3
"""Research helper: find where on-screen tiles of a romlab capture live in the ROM.

A capture is four dumps made by a romlab script (see cap_script):
  NAMEv.bin VRAM, NAMEp.bin palette RAM, NAMEi.bin IO registers, NAMEo.bin OAM.
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(__file__))
LAB = os.path.expanduser('~/.cache/mmbn-ref/lab')
ROM = open(os.path.expanduser('~/.cache/mmbn-ref/roms/bn6g.gba'), 'rb').read()


def cap_script(path):
    return (f'dump 6000000 18000 {path}v.bin\ndump 5000000 400 {path}p.bin\n'
            f'dump 4000000 60 {path}i.bin\ndump 7000000 400 {path}o.bin\n')


def lz77(off):
    if ROM[off] != 0x10:
        return None
    n = ROM[off + 1] | ROM[off + 2] << 8 | ROM[off + 3] << 16
    if n == 0 or n > 0x40000:
        return None
    out, p = bytearray(), off + 4
    try:
        while len(out) < n:
            flags = ROM[p]; p += 1
            for _ in range(8):
                if len(out) >= n:
                    break
                if flags & 0x80:
                    v = ROM[p] << 8 | ROM[p + 1]; p += 2
                    d = (v & 0xFFF) + 1
                    if d > len(out):
                        return None
                    for _ in range((v >> 12) + 3):
                        out.append(out[-d])
                else:
                    out.append(ROM[p]); p += 1
                flags = (flags << 1) & 0xFF
    except IndexError:
        return None
    return bytes(out)


_lz = None


def lz_blocks():
    """All plausible LZ77 blocks referenced by a ROM pointer (cached)."""
    global _lz
    if _lz is not None:
        return _lz
    cache = os.path.join(LAB, 'lzblocks.idx')
    offs = []
    if os.path.exists(cache):
        offs = [int(x, 16) for x in open(cache).read().split()]
    else:
        seen = set()
        for i in range(0, len(ROM) - 4, 4):
            v = struct.unpack_from('<I', ROM, i)[0]
            for base in (0x08000000, 0x88000000):
                a = v - base
                if 0 <= a < len(ROM) - 4 and a % 4 == 0 and ROM[a] == 0x10 and a not in seen:
                    seen.add(a)
        for a in sorted(seen):
            if lz77(a):
                offs.append(a)
        open(cache, 'w').write(' '.join('%x' % a for a in offs))
    _lz = [(a, lz77(a)) for a in offs]
    return _lz


def locate(data, want_all=False):
    """ROM offset of raw data, or ('lz', block, offset)."""
    hits = []
    a = ROM.find(data)
    while a >= 0:
        hits.append(a)
        if not want_all:
            return a
        a = ROM.find(data, a + 1)
    if hits:
        return hits
    for blk, blob in lz_blocks():
        j = blob.find(data)
        if j >= 0:
            if not want_all:
                return ('lz', blk, j)
            hits.append(('lz', blk, j))
    return hits if want_all else None


class Cap:
    def __init__(self, name):
        rd = lambda s: open(os.path.join(LAB, name + s), 'rb').read()
        self.v, self.p, self.i, self.o = rd('v.bin'), rd('p.bin'), rd('i.bin'), rd('o.bin')

    def reg(self, a):
        return struct.unpack_from('<H', self.i, a)[0]

    def bg_cell(self, layer, tx, ty):
        c = self.reg(8 + 2 * layer)
        cb, sb = ((c >> 2) & 3) * 0x4000, ((c >> 8) & 31) * 0x800
        size = c >> 14
        if size & 1 and tx >= 32:
            sb += 0x800; tx -= 32
        if size & 2 and ty >= 32:
            sb += 0x800 * (2 if size == 3 else 1); ty -= 32
        e = struct.unpack_from('<H', self.v, sb + (ty * 32 + tx) * 2)[0]
        t = e & 0x3FF
        return e, self.v[cb + t * 32: cb + t * 32 + 32]

    def bg_screen(self, layer, x, y):
        """Map entry and tile data under screen pixel (x, y) on a text BG."""
        hx, hy = self.reg(0x10 + 4 * layer) & 0x1FF, self.reg(0x12 + 4 * layer) & 0x1FF
        c = self.reg(8 + 2 * layer)
        w = 512 if c >> 14 & 1 else 256
        h = 512 if c >> 14 & 2 else 256
        return self.bg_cell(layer, ((x + hx) % w) // 8, ((y + hy) % h) // 8)

    def pal(self, bank, obj=False):
        b = 512 if obj else 0
        return self.p[b + bank * 32: b + bank * 32 + 32]

    def objs(self):
        """Visible OAM entries as dicts."""
        out = []
        sizes = {(0, 0): (8, 8), (0, 1): (16, 16), (0, 2): (32, 32), (0, 3): (64, 64),
                 (1, 0): (16, 8), (1, 1): (32, 8), (1, 2): (32, 16), (1, 3): (64, 32),
                 (2, 0): (8, 16), (2, 1): (8, 32), (2, 2): (16, 32), (2, 3): (32, 64)}
        for k in range(128):
            a0, a1, a2 = struct.unpack_from('<HHH', self.o, k * 8)
            if a0 & 0x300 == 0x200:
                continue
            w, h = sizes[(a0 >> 14, a1 >> 14)]
            x = a1 & 0x1FF
            if x >= 240: x -= 512
            y = a0 & 0xFF
            if y >= 160: y -= 256
            out.append(dict(k=k, x=x, y=y, w=w, h=h, tile=a2 & 0x3FF, pal=a2 >> 12, prio=(a2 >> 10) & 3,
                            affine=bool(a0 & 0x100), mode=(a0 >> 10) & 3, hflip=bool(a1 & 0x1000) and not a0 & 0x100,
                            vflip=bool(a1 & 0x2000) and not a0 & 0x100, aff=(a1 >> 9) & 31))
        return out

    def obj_tiles(self, ob):
        """Tile data of an OBJ (1D mapping, 4bpp), row-major 8x8 tiles."""
        n = ob['w'] // 8 * ob['h'] // 8
        base = 0x10000 + ob['tile'] * 32
        return [self.v[base + i * 32: base + i * 32 + 32] for i in range(n)]


def fmt(loc):
    if loc is None:
        return 'dyn'
    if isinstance(loc, tuple):
        return 'lz%06x+%x' % (loc[1], loc[2])
    return '%06x' % loc


_spr = None


def sprites():
    """{(cat, idx): decompressed sprite bytes} for every sprite in the ROM."""
    global _spr
    if _spr is not None:
        return _spr
    _spr = {}
    lists = 0x031CC4
    starts = [struct.unpack_from('<I', ROM, lists + c * 4)[0] - 0x08000000 for c in range(10)]
    for cat in range(10):
        lst = starts[cat]
        nxt = min([a for a in starts if a > lst] or [lst + 1024])
        for idx in range((nxt - lst) // 4):
            p = struct.unpack_from('<I', ROM, lst + idx * 4)[0]
            if p & 0x80000000:
                d = lz77((p & 0x7FFFFFFF) - 0x08000000)
                if d: _spr[(cat, idx)] = d
            elif 0x08000000 <= p < 0x08800000:
                a = p - 0x08000000
                _spr[(cat, idx)] = ROM[a:a + 0x20000]
            else:
                break
    return _spr


def find_sprite(tile):
    """Sprites whose data contain this 8x8 tile."""
    if tile == bytes(32):
        return []
    return [k for k, d in sprites().items() if d.find(tile) >= 0]


def match_obj(tiles, top=3):
    """Rank sprites by how many of these tiles they contain."""
    score = {}
    for t in tiles:
        if t == bytes(32):
            continue
        for k in find_sprite(t):
            score[k] = score.get(k, 0) + 1
    return sorted(score.items(), key=lambda kv: -kv[1])[:top]


def find_obj(tiles):
    """Sprites containing all of an OBJ's tiles consecutively."""
    blob = b''.join(tiles)
    if blob.count(0) == len(blob):
        return []
    return [k for k, d in sprites().items() if d.find(blob) >= 0]


def sprite_starts():
    out = []
    lists = 0x031CC4
    starts = [struct.unpack_from('<I', ROM, lists + c * 4)[0] - 0x08000000 for c in range(10)]
    for cat in range(10):
        lst = starts[cat]
        nxt = min([a for a in starts if a > lst] or [lst + 1024])
        for idx in range((nxt - lst) // 4):
            p = struct.unpack_from('<I', ROM, lst + idx * 4)[0]
            if not p & 0x80000000 and 0x08000000 <= p < 0x08800000:
                out.append((p - 0x08000000, cat, idx))
    return sorted(out)


def owner(tiles):
    """The sprite (cat, idx) that owns an OBJ's tiles, raw sprites resolved by address."""
    blob = b''.join(tiles)
    if blob.count(0) == len(blob):
        return None
    a = ROM.find(blob)
    if a >= 0:
        best = None
        for s, c, i in sprite_starts():
            if s <= a: best = (c, i)
        return best
    for k, d in sprites().items():
        if d.find(blob) >= 0 and struct.unpack_from('<I', ROM, 0x031CC4 + k[0] * 4)[0]:
            return k + ('lz',)
    return None


def sprite_frames(cat, idx):
    """[(anim, frame, tile bytes)] of a sprite, walking its animation table."""
    d = sprites().get((cat, idx))
    if d is None:
        return []
    base = 8 if struct.unpack_from('<I', ROM, 0x031CC4 + cat * 4)[0] and d[:4] != ROM[:4] else 4
    # LZ sprites keep a 4-byte size prefix before the header; raw ones start with it
    p = struct.unpack_from('<I', ROM, struct.unpack_from('<I', ROM, 0x031CC4 + cat * 4)[0] - 0x08000000 + idx * 4)[0]
    base = 8 if p & 0x80000000 else 4
    b = d[base:]
    out = []
    n = struct.unpack_from('<I', b, 0)[0] // 4
    for a in range(min(n, 256)):
        f = struct.unpack_from('<I', b, a * 4)[0]
        for k in range(256):
            rec = b[f + 20 * k:f + 20 * k + 20]
            t = struct.unpack_from('<I', rec, 0)[0]
            ln = struct.unpack_from('<I', b, t)[0]
            out.append((a, k, b[t + 4:t + 4 + ln]))
            if rec[18] & 0x80:
                break
    return out


def which_frame(cat, idx, tiles):
    """(anim, frame) of sprite (cat, idx) whose tiles contain all these OBJ tiles."""
    want = [t for t in tiles if t != bytes(32)]
    return [(a, k) for a, k, data in sprite_frames(cat, idx) if want and all(data.find(t) >= 0 for t in want)]
