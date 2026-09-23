#!/usr/bin/env python3
"""Generate src/hud_layout.inc from romlab captures of the original battle UI.

Every cell is recorded as the ROM offset of its 8x8 tile plus a palette taken
from the ROM, so the table holds numbers only. Cells that the game fills per
chip (names, art, icons, codes, digits) are replaced by the neutral tiles the
engine draws its own content over.

Captures (in the lab directory, made with romlab):
  cv/cp/ci  Custom screen open            (field.ss)
  rv/rp/ri/ro  battle running, BATTLE START banner   (running.ss)
  yv1/yp1/yo1  ENEMY DELETED banner
  cap/L270v/p/i/o  MEGAMAN DELETED banner (flow capture, MegaMan at 0 HP)
  cap/L420v/p/i/o  GAME OVER screen
  xv/xp/xi  RESULT window
  tv/tp/ti  text window (BG0), from the intro dialogue
  Ev/Ep/Ei  FOLDER EDIT screen (frame on BG1)
  pet0v/pet0p + Pi/Po  PET menu, cursor on the first item
"""
import os
import struct
import sys

LAB = sys.argv[1] if len(sys.argv) > 1 else os.path.expanduser('~/.cache/mmbn-ref/lab')
ROM = open(sys.argv[2] if len(sys.argv) > 2 else os.path.expanduser('~/.cache/mmbn-ref/roms/bn6g.gba'), 'rb').read()
OUT = sys.argv[3] if len(sys.argv) > 3 else os.path.join(os.path.dirname(__file__), '..', '..', 'src', 'hud_layout.inc')


# LZ77 blocks that hold UI tiles the game decompresses into VRAM (found with
# proto/lzscan against captured tiles). Cells that come from them are stored
# as 0x80000000 | block index << 24 | byte offset into the decompressed data.
LZ_BLOCKS = [0x6C5FA0, 0x6C58CC, 0x6C7B20, 0x6C211C]


def lz77(off):
    n = ROM[off + 1] | ROM[off + 2] << 8 | ROM[off + 3] << 16
    out, p = bytearray(), off + 4
    while len(out) < n:
        flags = ROM[p]; p += 1
        for _ in range(8):
            if len(out) >= n:
                break
            if flags & 0x80:
                v = ROM[p] << 8 | ROM[p + 1]; p += 2
                for _ in range((v >> 12) + 3):
                    out.append(out[-((v & 0xFFF) + 1)])
            else:
                out.append(ROM[p]); p += 1
            flags = (flags << 1) & 0xFF
    return bytes(out)


LZ_DATA = [lz77(b) for b in LZ_BLOCKS]


def locate(data):
    a = ROM.find(data)
    if a >= 0:
        return a
    for i, blob in enumerate(LZ_DATA):
        j = blob.find(data)
        if j >= 0:
            return 0x80000000 | i << 24 | j
    return -1


def rd(name):
    return open(os.path.join(LAB, name), 'rb').read()


pal_list = []


def pal_index(rom_off):
    if rom_off not in pal_list:
        pal_list.append(rom_off)
    return pal_list.index(rom_off)


def find_pal(pal_ram, slot, obj=False):
    base = 512 if obj else 0
    row = pal_ram[base + slot * 32 + 2: base + slot * 32 + 32]
    a = ROM.find(row)
    if a >= 0:
        return a - 2
    # partly rewritten at runtime: match the leading colours
    for n in (24, 20, 18):
        a = ROM.find(row[:n - 2])
        if a >= 0:
            return a - 2
    raise SystemExit(f'palette slot {slot} not found in ROM')


def bg3_cells(prefix, rows, cols, layer=3):
    io, vram, pal = rd(prefix + 'i.bin'), rd(prefix + 'v.bin'), rd(prefix + 'p.bin')
    c = struct.unpack_from('<H', io, 8 + 2 * layer)[0]
    cb, sb = ((c >> 2) & 3) * 0x4000, ((c >> 8) & 31) * 0x800
    grid = []
    for ty in rows:
        row = []
        for tx in cols:
            e = struct.unpack_from('<H', vram, sb + (ty * 32 + tx) * 2)[0]
            data = vram[cb + (e & 0x3FF) * 32: cb + (e & 0x3FF) * 32 + 32]
            if data == bytes(32):
                row.append(None)
                continue
            a = locate(data)
            row.append([a, find_pal(pal, e >> 12), (e >> 10) & 3] if a >= 0 else 'dyn')
        grid.append(row)
    return grid


BLANK = 0x02A6BC      # solid fill inside boxes
ICON_PAL = 0x72AED0   # empty slot frames (colours 0-8 match the runtime icon palette)
SLOT = (0x6E3FA0, 0x6E3FC0, 0x6E3FE0, 0x6E4000)
BLANK_LETTER = (0x6E3F60, 0x6E3F80)

# ---- Custom window: rows 0-19, cols 0-14 ----
cw = bg3_cells('c', range(20), range(15))
p9 = cw[0][0][1]
for ty in (1, 2):
    for tx in range(2, 8):
        cw[ty][tx] = [BLANK, p9, 0]                       # chip name
for ty in range(3, 9):
    for tx in range(2, 9):
        cw[ty][tx] = None                                 # chip art
for tx in range(2, 9):
    for ty in (9, 10):
        cw[ty][tx] = [BLANK, p9, 0]                       # code, element, power
for k in range(5):                                        # picked-chip column
    ty = 3 + k * 2
    cw[ty][12], cw[ty][13], cw[ty + 1][12], cw[ty + 1][13] = ([SLOT[i], ICON_PAL, 0] for i in range(4))
for base in (13, 16):                                     # offer grid, two rows of five
    for k in range(5):
        tx = 1 + k * 2
        cw[base][tx], cw[base][tx + 1], cw[base + 1][tx], cw[base + 1][tx + 1] = ([SLOT[i], ICON_PAL, 0] for i in range(4))
        cw[base + 2][tx], cw[base + 2][tx + 1] = [BLANK_LETTER[0], p9, 0], [BLANK_LETTER[1], p9, 0]
assert not any(c == 'dyn' for r in cw for c in r), 'custom window still has dynamic cells'

# ---- RESULT window: rows 2-19, cols 3-26 ----
rw = bg3_cells('x', range(2, 20), range(3, 27))
# "PRESS A BUTTON" (row 16, cols 5-14) blinks; the engine draws it over the
# plain fill the game shows between blinks (a solid colour-2 tile).
for tx in range(2, 12):
    rw[14][tx] = [0x280E9C, rw[14][tx][1], 0]
# Time digits and the rank are opaque tiles; the engine draws over the captured ones.
assert not any(c == 'dyn' for r in rw for c in r), 'result window still has dynamic cells'

# ---- Text window: BG0 rows 12-19, the full width ----
tw = bg3_cells('t', range(12, 20), range(30), layer=0)
assert not any(c == 'dyn' or c is None for r in tw for c in r), 'text window incomplete'

# ---- FOLDER EDIT frame: BG1, the whole screen ----
fw = bg3_cells('E', range(20), range(30), layer=1)
missing = [(y, x) for y, r in enumerate(fw) for x, c in enumerate(r) if c == 'dyn']
assert not missing, f'folder frame has dynamic cells {missing[:5]}'

# ---- PET menu: BG0 frame; the buttons' palette is composed at runtime ----
import shutil
for ext in ('v', 'p'):
    shutil.copy(os.path.join(LAB, f'pet0{ext}.bin'), os.path.join(LAB, f'PM{ext}.bin'))
shutil.copy(os.path.join(LAB, 'Pi.bin'), os.path.join(LAB, 'PMi.bin'))
PET_BUTTON_PAL = 0x6C7CF8
io_p, v_p, pal_p = rd('PMi.bin'), rd('PMv.bin'), rd('PMp.bin')
c = struct.unpack_from('<H', io_p, 8)[0]
cb, sb = ((c >> 2) & 3) * 0x4000, ((c >> 8) & 31) * 0x800
pw = []
for ty in range(20):
    row = []
    for tx in range(30):
        e = struct.unpack_from('<H', v_p, sb + (ty * 32 + tx) * 2)[0]
        data = v_p[cb + (e & 0x3FF) * 32: cb + (e & 0x3FF) * 32 + 32]
        if data == bytes(32):
            row.append(None)
            continue
        a = locate(data)
        assert a >= 0, f'PET cell {ty},{tx} not in ROM'
        pal = PET_BUTTON_PAL if e >> 12 == 15 else find_pal(pal_p, e >> 12)
        row.append([a, pal, (e >> 10) & 3])
    pw.append(row)

# PET sprites: logo, SELECT, status panel, PLACE, EXIT and the eight icons
S2 = {(0, 0): (8, 8), (0, 1): (16, 16), (0, 2): (32, 32), (0, 3): (64, 64), (1, 0): (16, 8), (1, 1): (32, 8),
      (1, 2): (32, 16), (1, 3): (64, 32), (2, 0): (8, 16), (2, 1): (8, 32), (2, 2): (16, 32), (2, 3): (32, 64)}
oam_p = rd('Po.bin')
pal_obj = rd('pet0p.bin')
pet_tiles = []   # (x, y, tile, pal, group); group 0 = fixed, 1-8 = item icons
for k, group in [(15, 0), (11, 0), (10, 0), (32, 0), (31, 0), (30, 0), (29, 0), (28, 0), (9, 0), (8, 0),
                 (19, 0), (18, 0), (17, 0), (16, 0)] + [(27 - i, i + 1) for i in range(8)]:
    a0, a1, a2 = struct.unpack_from('<HHH', oam_p, k * 8)
    x, y = a1 & 511, a0 & 255
    w, h = S2[(a0 >> 14, a1 >> 14)]
    hf = (a1 >> 12) & 1
    pal = find_pal(pal_obj, a2 >> 12, obj=True)
    for i in range(w * h // 64):
        data = v_p[0x10000 + ((a2 & 1023) + i) * 32: 0x10000 + ((a2 & 1023) + i + 1) * 32]
        if data == bytes(32):
            continue
        tx, ty = i % (w // 8), i // (w // 8)
        if hf:
            tx = w // 8 - 1 - tx
        a = locate(data)
        assert a >= 0, f'PET OBJ {k} tile {i} not in ROM'
        pet_tiles.append((x + tx * 8, y + ty * 8 - (16 if group else 0) * 0, a, pal, group, hf))

# ---- Banners: five 32x16 OBJs each ----
SIZES = {(0, 0): (8, 8), (0, 1): (16, 16), (0, 2): (32, 32), (1, 1): (32, 8), (1, 2): (32, 16), (2, 1): (8, 32)}


def banner(prefix, suffix=''):
    oam, vram, pal = rd(prefix + 'o' + suffix + '.bin'), rd(prefix + 'v' + suffix + '.bin'), rd(prefix + 'p' + suffix + '.bin')
    parts = []
    for k in range(128):
        a0, a1, a2 = struct.unpack_from('<HHH', oam, k * 8)
        y, x = a0 & 255, a1 & 511
        if (a0 >> 8) & 3 == 2 or y != 64 or (a0 >> 14, a1 >> 14) != (1, 2):
            continue
        tiles = []
        for t in range(8):
            data = vram[0x10000 + ((a2 & 1023) + t) * 32: 0x10000 + ((a2 & 1023) + t + 1) * 32]
            tiles.append(ROM.find(data) if data != bytes(32) else 0)
        parts.append((x, tiles, find_pal(pal, a2 >> 12, obj=True)))
    parts.sort()
    assert len(parts) == 5 and all(t >= 0 for _, ts, _ in parts for t in ts), f'{prefix} banner incomplete'
    return parts


start = banner('r')
deleted = banner('y', '1')
lost = banner('cap/L270')   # MEGAMAN DELETED, captured with labtrace.cap_script

out = ['/* Original battle UI layouts, recorded from the game running in romlab and',
       ' * resolved to ROM tile offsets. Generated by tools/romlab/extract_hud.py;',
       ' * do not edit by hand. Cell: { tile ROM offset, palette index, flip bits }. */',
       'typedef struct { uint32_t tile; uint8_t pal; uint8_t flip; } HudCell;',
       'static const uint32_t hud_lz_blocks[] = { ' + ', '.join(f'0x{b:06X}' for b in LZ_BLOCKS) + ' };',
       'static const uint32_t hud_pals[] = {']
cells = {}
# ---- GAME OVER: BG1 streaked grid (scrolls) and BG2 lettering, 20 rows ----
gw = bg3_cells('cap/L420', range(20), range(32), layer=1)
gt = bg3_cells('cap/L420', range(20), range(30), layer=2)
assert not any(c == 'dyn' for g in (gw, gt) for r in g for c in r), 'game over layers have dynamic cells'

for name, grid in (('custom', cw), ('result', rw), ('text', tw), ('folder', fw), ('pet', pw), ('gameover_bg', gw), ('gameover_text', gt)):
    cells[name] = [[(c[0], pal_index(c[1]), c[2]) if c else (0, 0, 0) for c in row] for row in grid]
banners = {}
for name, parts in (('start', start), ('deleted', deleted), ('lost', lost)):
    banners[name] = [(x, tiles, pal_index(p)) for x, tiles, p in parts]
pet_objs = [(x, y, t, pal_index(p), g, f) for x, y, t, p, g, f in pet_tiles]
out.append('\t' + ', '.join(f'0x{p:06X}' for p in pal_list) + ',')
out.append('};')
for name, grid in cells.items():
    out.append(f'static const HudCell hud_{name}[{len(grid)}][{len(grid[0])}] = {{')
    for row in grid:
        out.append('\t{ ' + ', '.join(f'{{ 0x{t:06X}, {p}, {f} }}' for t, p, f in row) + ' },')
    out.append('};')
for name, parts in banners.items():
    out.append(f'static const struct {{ int16_t x; uint8_t pal; uint32_t tiles[8]; }} hud_banner_{name}[5] = {{')
    for x, tiles, p in parts:
        out.append(f'\t{{ {x}, {p}, {{ ' + ', '.join(f'0x{t:06X}' for t in tiles) + ' } },')
    out.append('};')
out.append('/* PET menu sprites as single tiles: x, y, tile, palette, group (0 fixed, n = icon of item n), hflip */')
out.append(f'static const struct {{ int16_t x, y; uint32_t tile; uint8_t pal, group, flip; }} hud_pet_objs[{len(pet_objs)}] = {{')
for x, y, t, p, g, f in pet_objs:
    out.append(f'\t{{ {x}, {y}, 0x{t:06X}, {p}, {g}, {f} }},')
out.append('};')
open(OUT, 'w').write('\n'.join(out) + '\n')
print('wrote', OUT, 'palettes', [hex(p) for p in pal_list])
