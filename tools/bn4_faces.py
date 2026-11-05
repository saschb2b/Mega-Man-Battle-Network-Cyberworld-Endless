#!/usr/bin/env python3
"""bn4_faces.py [report] | show [NAVI@ID ...]: faces for BN4 Red Sun's
tournament bracket from the Navis of BN6 Gregar and BN5 Team Colonel, made
from the player's own ROMs (docs/multirom/bn4-tournament.md, Faces for every
entrant; the names in src/emu/bn4_tournament.h).

report (the default) reads BN4's plates as its bracket draws them (two
sprites of 24x32 plates, four palettes a plate, the frame's colour indexes)
and fails where the ROM is not as the doc says; then lists every entrant the
tournament's design allows with the face its own game has for him (a mugshot
of sprite list 8, or none) and the colours his plate keeps.

show converts each NAVI's face into a plate in memory and writes it over the
plate of entrant id ID in romlab's copy of BN4's ROM (its tiles and its four
palettes), then opens The Den Battle Tournament's bracket with those ids as
entrants twice (all in, then after the first round) and keeps the frames in
.build/bn4faces/. NAVI is a run's navi number (1-18 BN6's, 24-29 BN5's) or
bn4:N, BN4's own mugshot N (the control: BN4's GutsMan's mugshot beside his
own plate). Needs the cyberworld-romlab image, tools/romlab/romlab.bin and
tools/bn4_lab.py boot's state. Nothing of the games is written to git: the
faces exist only in memory and in the frames under .build/.

  python3 tools/bn4_faces.py
  python3 tools/bn4_faces.py show 1@1 26@5 bn4:0x42@8
"""
import argparse
import hashlib
import glob
import os
import struct
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bn4_lab  # noqa: E402

ROOT = bn4_lab.ROOT
ROMS = bn4_lab.ROMS
OUT = os.path.join(ROOT, '.build', 'bn4faces')
SHA1 = {'bn4': 'a97e96a7da03abd70f7953328e511b9fa29179f1', 'bn5': '5f472f78d8de2df01d5039e045c043cb40969a39',
        'bn6': '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6'}
LISTS = {'bn4': 0x0802793C, 'bn5': 0x0803272C, 'bn6': 0x08031CC4}   # XRomLayout/RomLayout.sprite_lists
LIST_BATTLE, LIST_FACES = 0, 8
# src/emu/bn4_tournament.h
PLATE_FRAMES, PLATE_SHEETS, PLATE_SPRITES = 0x0810414D, 0x08104127, 0x0810433F
PLATE_PALETTES, PLATE_SPECIAL, MENU_SPRITE_PTRS = 0x08104108, 0x08104146, 0x080281D0
PLATE_SHEET_A, PLATE_SHEET_B = 0x084BB768, 0x084BDB30
PLATE_W, PLATE_H, PLATE_TILES = 24, 32, 12
PLATE_OUTER, PLATE_INNER = 14, 15                  # the frame's colour indexes
FACE_X, FACE_Y, FACE_W, FACE_H = 3, 2, 18, 26      # the face's window inside the frames
PLATE_VARIANTS = 4                                 # +0 colour, +1 lighter, +2 grey (out), +3 the purple inner frame
ENTRANT_MEGAMAN = 0x1E
TOURNAMENTS, BRACKET, GAMESTATE = 0x02001460, 0x0200ACC0, 0x0200218C
# BN6's (src/layer/guardians.c, docs/rom/graphics.md, Mugshots): navi -> list 8's face; Falzar's five have none in Gregar
BN6_BLACK_FACE = 0x084F6BBC
NAVIS = {1: ('HeatMan', 'bn6', 0x47), 2: ('ElecMan', 'bn6', 0x49), 3: ('SlashMan', 'bn6', 0x4B), 4: ('EraseMan', 'bn6', 0x50),
         5: ('ChargeMan', 'bn6', 0x4F), 6: ('SpoutMan', 'bn6', None), 7: ('TomahawkMan', 'bn6', None),
         8: ('TenguMan', 'bn6', None), 9: ('GroundMan', 'bn6', None), 10: ('DustMan', 'bn6', None),
         12: ('BlastMan', 'bn6', 0x51), 13: ('DiveMan', 'bn6', 0x52), 14: ('CircusMan', 'bn6', 0x54),
         15: ('JudgeMan', 'bn6', 0x55), 16: ('ElementMan', 'bn6', 0x56), 18: ('Colonel', 'bn6', 0x53),
         # BN5's six (guardian_older_sprite: lists 6 and 8 by the same number)
         24: ('Colonel', 'bn5', 69), 25: ('ShadowMan', 'bn5', 75), 26: ('NumberMan', 'bn5', 71),
         27: ('TomahawkMan', 'bn5', 72), 28: ('KnightMan', 'bn5', 73), 29: ('ToadMan', 'bn5', 74)}
# Falzar's five in Gregar: an 18x26 window round the head of the battle sprite's standing frame, at its own scale (a
# battle sprite's head is already about a plate's), its top left from his feet, chosen by eye on a grid of the frame
HEAD_WINDOW = {6: (-11, -35), 7: (6, -46), 8: (-5, -72), 9: (-9, -58), 10: (-8, -53)}
DIMS = [[(8, 8), (16, 16), (32, 32), (64, 64)], [(16, 8), (32, 8), (32, 16), (64, 32)], [(8, 16), (8, 32), (16, 32), (32, 64)]]


def find_roms():
    out = {}
    for path in sorted(glob.glob(os.path.join(ROMS, '*.gba'))):
        with open(path, 'rb') as f:
            data = f.read()
        h = hashlib.sha1(data).hexdigest()
        for k, s in SHA1.items():
            if h == s:
                out[k] = data
    return out


def u32(d, a):
    return struct.unpack_from('<I', d, a & 0x1FFFFFF)[0]


def lz77(d, off):
    n, out, p = d[off + 1] | d[off + 2] << 8 | d[off + 3] << 16, bytearray(), off + 4
    while len(out) < n:
        flags = d[p]
        p += 1
        for b in range(8):
            if len(out) >= n:
                break
            if flags & 0x80 >> b:
                x = d[p] << 8 | d[p + 1]
                p += 2
                for _ in range((x >> 12) + 3):
                    out.append(out[-(x & 0xFFF) - 1])
            else:
                out.append(d[p])
                p += 1
    return bytes(out)


class Sprite:
    """a sprite of BN's format: a 4-byte header, then its animations' offsets (src/gfx/gfx.c, Sprites)"""

    def __init__(self, d, ptr):
        self.ptr = ptr
        self.b = lz77(d, ptr & 0x1FFFFFF)[8:] if ptr & 0x80000000 else d[(ptr & 0x1FFFFFF) + 4:]

    def frame(self, anim):
        b, f = self.b, u32(self.b, anim * 4)
        t, p, m, o = (u32(b, f + 4 * i) for i in range(4))
        lst = b[m + u32(b, m)]
        objs, oa = [], o + u32(b, o + lst * 4)
        while not (b[oa] == 0xFF and b[oa + 1] == 0xFF) and len(objs) < 64:
            objs.append(tuple(b[oa:oa + 5]))
            oa += 5
        return {'t': t, 'tiles': b[t + 4:t + 4 + u32(b, t)], 'p': p, 'pal': b[p + 4:p + 4 + u32(b, p)], 'objs': objs, 'mini': m}


def list_sprite(d, game, cat, idx):
    return u32(d, u32(d, LISTS[game] + 4 * cat) + 4 * idx)


def pixels(fr):
    """a frame's palette indexes by its objects (1D tiles), its origin's offset: (rows, minx, miny)"""
    boxes = []
    for o in fr['objs']:
        w, h = DIMS[o[4] & 3][o[3] & 3]
        boxes.append((struct.unpack('b', bytes([o[1]]))[0], struct.unpack('b', bytes([o[2]]))[0], w, h, o))
    minx, miny = min(b[0] for b in boxes), min(b[1] for b in boxes)
    W, H = max(b[0] + b[2] for b in boxes) - minx, max(b[1] + b[3] for b in boxes) - miny
    px = [[0] * W for _ in range(H)]
    for x0, y0, w, h, o in boxes:
        for ty in range(h // 8):
            for tx in range(w // 8):
                td = fr['tiles'][(o[0] + ty * (w // 8) + tx) * 32:][:32]
                for i in range(64 if len(td) == 32 else 0):
                    ci = td[i // 2] >> 4 if i & 1 else td[i // 2] & 15
                    X, Y = tx * 8 + i % 8, ty * 8 + i // 8
                    X, Y = (w - 1 - X) if o[3] & 0x40 else X, (h - 1 - Y) if o[3] & 0x80 else Y
                    if ci:
                        px[y0 - miny + Y][x0 - minx + X] = ci
    return px, minx, miny


def rgb(c):
    return (c & 31, c >> 5 & 31, c >> 10 & 31)


def bgr(r, g, b):
    return max(0, min(31, r)) | max(0, min(31, g)) << 5 | max(0, min(31, b)) << 10


def colours(pal):
    return [pal[i * 2] | pal[i * 2 + 1] << 8 for i in range(len(pal) // 2)]


# ---- BN4's plates as its bracket draws them

def plate_of(b4, eid):
    """entrant id -> (sheet's pointer, animation, palette base): BN4_BRACKET_PLATE's init (0x08104040) and BN4_PLATE_SET"""
    anim = b4[(PLATE_FRAMES & 0x1FFFFFF) + eid]
    sheet = u32(b4, MENU_SPRITE_PTRS + 4 * b4[(PLATE_SPRITES & 0x1FFFFFF) + b4[(PLATE_SHEETS & 0x1FFFFFF) + eid]])
    return sheet, anim, b4[(PLATE_PALETTES & 0x1FFFFFF) + eid]


def plate_places(b4, eid):
    """where in BN4's ROM an entrant's plate is read: its tiles' bus address, and each variant's palette's"""
    sheet, anim, base = plate_of(b4, eid)
    s = Sprite(b4, sheet)
    fr = s.frame(anim)
    at = (sheet & 0x1FFFFFF) + 0x08000000 + 4
    nib = fr['objs'][0][4] >> 4
    return s, fr, at + fr['t'] + 4, [at + fr['p'] + 4 + (base + v + nib) * 32 for v in range(PLATE_VARIANTS)]


def report_plates(b4):
    bad = []
    for sheet, n in ((PLATE_SHEET_A, 16), (PLATE_SHEET_B, 7)):
        s = Sprite(b4, sheet)
        anims = u32(s.b, 0) // 4
        if anims != n:
            bad.append(f'sheet {sheet:#x}: {anims} animations, not {n}')
        for a in range(anims):
            fr = s.frame(a)
            shapes = [(o[1], o[2], o[3] & 3, o[4] & 3) for o in fr['objs']]
            if len(fr['tiles']) != PLATE_TILES * 32 or shapes != [(0xF4, 0xF0, 2, 2), (0x04, 0xF0, 1, 2)]:
                bad.append(f'sheet {sheet:#x} animation {a}: {len(fr["tiles"])} bytes, objects {shapes}')
        pals = s.frame(0)['mini'] - s.frame(0)['p'] - 4
        print(f'sheet {sheet:#010x}: {anims} plates of {PLATE_W}x{PLATE_H} (a 16x32 and an 8x32 object, {PLATE_TILES} tiles, '
              f'uncompressed), {pals // 32} palettes of 16 colours')
    ids = []
    for eid in list(range(0x17)) + [ENTRANT_MEGAMAN]:
        s, fr, tiles, pals = plate_places(b4, eid)
        px, _, _ = pixels(fr)
        if (px[0][1], px[1][2]) != (PLATE_OUTER, PLATE_INNER):
            ids.append(f'{eid:#x}: frame {px[0][1]}/{px[1][2]}')
    special = []
    for i in range(8):
        v = b4[(PLATE_SPECIAL & 0x1FFFFFF) + i]
        if v == 0xFF:
            break
        special.append(v)
    print(f'ids 0x00-0x16 and MegaMan: their frame in indexes {PLATE_OUTER} (outer) and {PLATE_INNER} (inner) but '
          + (', '.join(ids) or 'none'))
    print(f'drawn with palette +3 in colour: ids {", ".join(f"{v:#x}" for v in special)}')
    return bad


# ---- a face from its own game

def face_of(roms, navi):
    """a Navi's face as his own game shows it: (40x48 palette indexes, 16 colours) or None"""
    if isinstance(navi, str):
        n = int(navi.split(':')[1], 0)
        return mugshot(roms['bn4'], 'bn4', n)
    name, game, num = NAVIS[navi]
    if game not in roms:
        return None
    if num is not None:
        return mugshot(roms[game], game, num)
    return portrait(roms['bn6'], navi)


def mugshot(d, game, num):
    ptr = list_sprite(d, game, LIST_FACES, num)
    if game == 'bn6' and ptr == BN6_BLACK_FACE:
        return None
    fr = Sprite(d, ptr).frame(0)
    px, _, _ = pixels(fr)
    if len(px) != 48 or len(px[0]) != 40:
        return None
    return px, colours(fr['pal'][:32])


def portrait(d, navi):
    """Falzar's five in Gregar (no mugshot there): HEAD_WINDOW over the battle sprite's standing frame (list 0,
    0x2E + the navi, animation 0's first frame), on a ground of the darkest of his own colours"""
    fr = Sprite(d, list_sprite(d, 'bn6', LIST_BATTLE, 0x2E + navi)).frame(0)
    px, minx, miny = pixels(fr)
    pal = colours(fr['pal'][:32])
    wx, wy = HEAD_WINDOW[navi]
    ground = min(range(1, 16), key=lambda i: sum(rgb(pal[i])))
    out = [[ground] * FACE_W for _ in range(FACE_H)]
    for y in range(FACE_H):
        for x in range(FACE_W):
            sx, sy = wx + x - minx, wy + y - miny
            if 0 <= sy < len(px) and 0 <= sx < len(px[0]) and px[sy][sx]:
                out[y][x] = px[sy][sx]
    return out, pal


# ---- the conversion: a 40x48 face into a 24x32 plate

def plate_from(face, frames):
    """face: (40x48 indexes, 16 colours); frames: BN4's own frame colours, outer and inner, per variant.
    The face halved (each 2x2 block its commonest index, a tie to the darker), its middle 18 columns and 24
    rows in the window, a row of its ground above and below; at most 13 colours (the least used merged into
    the nearest); then the four palettes by the rules BN4's own plates follow (docs/multirom/bn4-tournament.md)"""
    px, pal = face
    lum = [sum(rgb(c)) for c in pal]
    if len(px[0]) == FACE_W:   # a head window at its own scale (Falzar's five)
        win = px
    else:
        half = []
        for y in range(24):
            row = []
            for x in range(20):
                block = [px[2 * y + j][2 * x + i] for j in (0, 1) for i in (0, 1)]
                row.append(max(set(block), key=lambda c: (block.count(c), -lum[c])))
            half.append(row)
        edge = [c for c in half[0] + half[-1] + [r[0] for r in half] + [r[-1] for r in half]]
        ground = max(set(edge), key=edge.count)
        win = [[ground] * FACE_W] + [r[1:1 + FACE_W] for r in half] + [[ground] * FACE_W]
        win = [[c if c else ground for c in r] for r in win]
    used = {}
    for r in win:
        for c in r:
            used[c] = used.get(c, 0) + 1
    merge = {}
    while len(used) > PLATE_OUTER - 1:
        least = min(used, key=used.get)
        near = min((c for c in used if c != least), key=lambda c: sum((a - b) ** 2 for a, b in zip(rgb(pal[c]), rgb(pal[least]))))
        used[near] += used.pop(least)
        merge[least] = near
    order = sorted(used, key=lambda c: -used[c])
    slot = {c: i + 1 for i, c in enumerate(order)}
    for a in merge:
        b = a
        while b in merge:
            b = merge[b]
        slot[a] = slot[b]
    plate = [[0] * PLATE_W for _ in range(PLATE_H)]
    for y in range(30):
        for x in range(1, 23):
            plate[y][x] = PLATE_OUTER if y in (0, 29) or x in (1, 22) else PLATE_INNER
    for y in range(FACE_H):
        for x in range(FACE_W):
            plate[FACE_Y + y][FACE_X + x] = slot[win[y][x]]
    tiles = bytearray()
    for x0, w in ((0, 16), (16, 8)):
        for ty in range(4):
            for tx in range(w // 8):
                for py in range(8):
                    for p in range(0, 8, 2):
                        X, Y = x0 + tx * 8 + p, ty * 8 + py
                        tiles.append(plate[Y][X] | plate[Y][X + 1] << 4)
    face_cols = [0] * 16
    for c, i in slot.items():
        face_cols[i] = pal[c]
    pals = []
    for v in range(PLATE_VARIANTS):
        cs = []
        for i in range(16):
            r, g, b = rgb(face_cols[i])
            if v == 1:
                c = bgr(r + 5, g + 5, b + 5)
            elif v == 2:
                o = 5 + (r * 25 + g * 60 + b * 15) / 100 * 0.7
                c = bgr(round(o + 1), round(o), round(o - 1))
            else:
                c = face_cols[i]
            cs.append(c)
        cs[0], cs[PLATE_OUTER], cs[PLATE_INNER] = 0, frames[v][0], frames[v][1]
        pals.append(b''.join(struct.pack('<H', c) for c in cs))
    return bytes(tiles), pals, len(used) + len(merge), len(used)


def frame_colours(b4):
    """BN4's own frame colours for each variant, read from GutsMan's plate (id 0x00; 0x04 and 0x05 alone keep their
    frame in other indexes)"""
    _, _, _, pals = plate_places(b4, 0x00)
    out = []
    for a in pals:
        p = b4[(a & 0x1FFFFFF):(a & 0x1FFFFFF) + 32]
        out.append((p[PLATE_OUTER * 2] | p[PLATE_OUTER * 2 + 1] << 8, p[PLATE_INNER * 2] | p[PLATE_INNER * 2 + 1] << 8))
    return out


def report(roms):
    b4 = roms['bn4']
    bad = report_plates(b4)
    frames = frame_colours(b4)
    print('entrants (docs/game/tournament.md, The entrants) and their faces:')
    for navi, (name, game, num) in sorted(NAVIS.items()):
        face = face_of(roms, navi)
        if face is None:
            print(f'  {navi:2} {name:12} {game}: no ROM' if game not in roms else f'  {navi:2} {name:12} {game}: no face')
            if game in roms:
                bad.append(f'navi {navi}: no face')
            continue
        _, _, n, kept = plate_from(face, frames)
        src = f'mugshot, list 8 #{num:#04x}, halved' if num is not None else 'head of the battle sprite, list 0 #%#04x, 1:1' % (0x2E + navi)
        print(f'  {navi:2} {name:12} {game}: {src}, {n} colours, {kept} kept')
    for b in bad:
        print(f'  not as docs/multirom/bn4-tournament.md says: {b}')
    return 1 if bad else 0


# ---- shown in romlab

def png(path, ppm, scale=2):
    with open(ppm, 'rb') as f:
        data = f.read()
    parts = data.split(b'\n', 3)
    W, H = map(int, parts[1].split())
    px = parts[3]
    raw = b''
    for y in range(H * scale):
        row = px[(y // scale) * W * 3:(y // scale + 1) * W * 3]
        raw += b'\0' + b''.join(row[x * 3:x * 3 + 3] * scale for x in range(W))

    def chunk(t, b):
        return struct.pack('>I', len(b)) + t + b + struct.pack('>I', zlib.crc32(t + b) & 0xFFFFFFFF)
    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', W * scale, H * scale, 8, 2, 0, 0, 0))
                + chunk(b'IDAT', zlib.compress(raw)) + chunk(b'IEND', b''))


def show(roms, pairs):
    b4 = roms['bn4']
    os.makedirs(OUT, exist_ok=True)
    script = ['load /src/.build/bn4lab/room.state', 'run 2']
    frames = frame_colours(b4)
    ids = []
    for navi, eid in pairs:
        face = face_of(roms, navi)
        if face is None:
            sys.exit(f'bn4_faces: no face for {navi}')
        tiles, pals, n, kept = plate_from(face, frames)
        _, _, at, pal_at = plate_places(b4, eid)
        script.append(f'pokes {at:08x} {tiles.hex()}')
        for a, p in zip(pal_at, pals):
            script.append(f'pokes {a:08x} {p.hex()}')
        ids.append(eid)
        print(f'{navi} -> plate {eid:#04x}: tiles at {at:#010x}, palettes at {", ".join(f"{a:#010x}" for a in pal_at)}, '
              f'{kept} of {n} colours')
    rest = [i for i in (0x00, 0x03, 0x04, 0x09, 0x0B, 0x0D, 0x11) if i not in ids]
    eight = [ENTRANT_MEGAMAN] + ids + rest
    eight = eight[:8]
    shots = []
    for name, flags in (('all_in', [0x40] * 8), ('round1', [0x41, 0x00, 0x41, 0x00, 0x41, 0x00, 0x41, 0x00])):
        ent = ''.join(f'{e:02x}{f:02x}{i:02x}00' for i, (e, f) in enumerate(zip(eight, flags)))
        s = list(script) + [f'pokes {TOURNAMENTS:08x} {ent}', f'poke8 {BRACKET + 5:08x} 0', f'poke8 {BRACKET + 6:08x} 0',
                            f'poke8 {BRACKET + 7:08x} 0', f'poke8 {GAMESTATE:08x} 1c', 'run 400',
                            f'shot /src/.build/bn4faces/{name}.ppm']
        bn4_lab.romlab(bn4_lab.Rom(), s, f'faces-{name}')
        ppm = os.path.join(OUT, f'{name}.ppm')
        png(os.path.join(OUT, f'{name}.png'), ppm)
        shots.append(f'.build/bn4faces/{name}.png')
    print('entrants in bracket order:', ' '.join(f'{e:#04x}' for e in eight))
    print('frames:', ', '.join(shots))
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('cmd', nargs='?', default='report', choices=('report', 'show'))
    ap.add_argument('pairs', nargs='*', help='NAVI@ID: a navi number (or bn4:N, BN4 mugshot N) on entrant id ID')
    a = ap.parse_args()
    roms = find_roms()
    if 'bn4' not in roms:
        sys.exit(f'bn4_faces: no BN4 Red Sun (USA) in {ROMS}')
    if a.cmd == 'report':
        return report(roms)
    pairs = []
    for p in a.pairs or ['1@1', '26@5', 'bn4:0x42@8']:
        navi, eid = p.rsplit('@', 1)
        pairs.append((navi if navi.startswith('bn4:') else int(navi), int(eid, 0)))
    return show(roms, pairs)


if __name__ == '__main__':
    sys.exit(main())
