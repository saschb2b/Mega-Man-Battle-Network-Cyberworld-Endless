#!/usr/bin/env python3
"""bn5_songs.py [SONG ...]: where Battle Network 5 Team Colonel (USA) starts each
of its music player's songs, read from the player's own ROM (docs/ROM_DATA.md,
Where BN5 plays its songs; the Jukebox's names, docs/PET.md). Prints numbers
and addresses only: nothing of the ROM is written anywhere.

What starts a song, each found as the ROM has it:
  code     an immediate (or a register set to one) in r0 before a call of
           PlayMusic 0x080005A0, its forced form 0x080005BE, its fade-in
           0x0800062A or PlaySoundEffect 0x08000598
  map      the map-music lists 0x080361C8 (by game state +7)
  battle   a battle record's music byte (+2)
  ms, cs   a map script's 0x30/0x31, a cutscene's 0x4C/0x4D: every script of
           every map group's tables (0x08034E44, 0x08034EBC) decoded, with
           the cutscenes they and the code start and every jump, and the
           Liberation Missions' own (their records' pointers 12 and 13)
  ns       an object script's 0x47, the scripts found by their shape
  text     a text script's 0xFD 01 (or 0B, faded in), every text archive
           (LZ77 or not) found by its shape
The command sizes are BN5's own handlers' (each one's advances of the
script's pointer), measured once and kept here as numbers; a decode that
meets a command it does not know stops there, and is counted.

  python3 tools/bn5_songs.py          every song 0x01-0x24: where it starts
  python3 tools/bn5_songs.py 18 24    those songs, every place listed
"""
import os
import re
import struct
import sys
from collections import defaultdict

ROMS = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
B = 0x08000000


def bn5_rom():
    for name in sorted(os.listdir(ROMS)):
        path = os.path.join(ROMS, name)
        if name.endswith('.gba') and os.path.getsize(path) == 0x800000:
            with open(path, 'rb') as f:
                data = f.read()
            if data[0xAC:0xB0] == b'BRKE':
                return data
    sys.exit('bn5_songs: no BN5 Team Colonel ROM in ' + ROMS)


ROM = bn5_rom()


def u8(a): return ROM[a - B]
def u16(a): return struct.unpack_from('<H', ROM, a - B)[0]
def u32(a): return struct.unpack_from('<I', ROM, a - B)[0]
def rom_ptr(v): return B <= v < B + len(ROM)


def sizes(text):
    return {int(k, 16): int(v) for k, v in (p.split(':') for p in text.split())}


# Each command's size, its handler's: the cutscenes' (0x08037844), the map
# scripts' (0x08035934, many of them the cutscenes' handlers), the objects'
# (0x080A66D4). The ones whose size hangs on a sub-command are functions.
CS = sizes('00:1 01:1 02:3 03:4 04:2 05:2 06:1 07:1 08:1 09:3 0a:2 0b:1 0c:1 0d:3 0e:3 0f:1 10:1 11:6 12:2 13:5 14:5 15:7 16:8 17:8 '
           '18:8 19:8 1b:7 1c:8 1d:10 1e:6 1f:6 20:6 21:6 22:6 23:6 24:6 25:6 26:4 27:3 28:4 29:4 2a:4 2b:4 2c:5 2d:5 2e:6 2f:7 30:9 '
           '31:3 32:3 33:4 34:3 35:5 36:5 37:2 39:1 3a:3 3b:1 3c:5 3d:2 3e:4 3f:10 40:5 41:3 42:5 45:4 46:3 49:5 4b:3 4c:3 4d:5 4e:3 '
           '4f:1 50:4 51:3 53:3 58:13 59:1 5a:4 5b:2 5c:1 5d:1 5e:1 5f:1 60:2 61:1 66:3 67:4 68:3 69:1 6a:2 6b:4 6c:2 6d:1 6e:1 6f:4 '
           '70:4 71:2 72:2 73:6 74:1')
CS.update({
    0x1A: lambda a: {0: 11, 1: 12, 2: 14}.get(u8(a + 1), 11),   # a byte's, hword's or word's compare
    0x38: lambda a: 3 if u8(a + 1) == 0xFF else 2,              # a text script, from memory or not
    0x43: lambda a: {0: 5, 1: 2}.get(u8(a + 1), 6),
    0x44: lambda a: 2 if u8(a + 1) == 1 else 4,
    0x47: lambda a: {0: 13, 1: 13, 2: 14}.get(u8(a + 1) >> 4, 2),   # a map object spawned or freed
    0x48: lambda a: {1: 3, 3: 2, 4: 2, 5: 2}.get(u8(a + 1), 6),
    0x4A: lambda a: 2 if u8(a + 1) & 0x40 else 7,               # a warp
    0x52: lambda a: 6 if u8(a + 1) == 0 else 2,                 # a camera script run or stopped
})
CS_DEST = {0x14: 1, 0x15: 3, 0x16: 4, 0x17: 4, 0x18: 4, 0x19: 4, 0x1A: 6, 0x1B: 3, 0x1C: 4, 0x1D: 6, 0x1E: 2, 0x1F: 2,
           0x20: 2, 0x21: 2, 0x22: 2, 0x23: 2, 0x24: 2, 0x25: 2}
CS_STOP = {0x00, 0x01, 0x10, 0x14, 0x3B}
MS = sizes('00:1 01:5 02:7 03:8 04:8 05:8 06:8 08:8 09:10 0a:6 0b:6 0c:6 0d:6 0e:6 0f:6 10:6 11:6 12:7 13:7 14:7 15:7 16:6 17:13 '
           '18:6 19:6 1a:5 1b:5 1c:5 1d:4 1e:3 1f:4 20:4 21:4 22:4 23:5 24:5 25:9 26:9 27:6 28:7 29:9 2a:3 2b:3 2c:5 2d:5 2e:2 2f:3 '
           '30:3 31:5 32:3 33:1 34:4 35:3 37:13 38:1 3a:2 3b:1 41:4 42:4 43:2 44:2 45:2')
MS.update({0x07: CS[0x1A], 0x39: lambda a: 2 if u8(a + 1) == 1 else 6, 0x40: lambda a: 3 if u8(a + 1) == 1 else 6})
MS_DEST = {0x01: (1,), 0x02: (3,), 0x03: (4,), 0x04: (4,), 0x05: (4,), 0x06: (4,), 0x07: (6,), 0x08: (4,), 0x09: (6,),
           0x0A: (2,), 0x0B: (2,), 0x0C: (2,), 0x0D: (2,), 0x10: (2,), 0x11: (2,), 0x17: (1, 5, 9), 0x18: (2,), 0x19: (2,),
           0x1A: (1,), 0x1B: (1,), 0x1C: (1,)}
MS_STOP = {0x00, 0x01, 0x17}
NS = sizes('00:1 02:5 03:1 04:7 05:7 06:3 07:3 08:1 09:1 0a:2 0b:2 0c:4 0d:1 0e:1 0f:2 10:2 11:3 12:1 13:1 14:7 15:4 16:2 17:2 '
           '18:2 19:1 1a:1 1b:1 1c:5 1d:2 1e:2 1f:1 20:1 21:1 22:1 23:1 25:3 26:1 27:1 28:3 29:3 2a:2 2b:3 2d:3 2e:2 2f:4 30:1 31:2 '
           '32:5 33:2 34:1 35:5 36:5 37:9 38:8 39:2 3a:1 3b:7 3c:3 3d:7 3e:3 40:1 41:1 44:6 45:3 46:2 47:3 48:1 49:1 4a:4 4b:6 4c:2 '
           '4d:9 4e:5 4f:4 50:5')
NS_DEST = {0x02: (1,), 0x04: (3,), 0x05: (3,), 0x36: (1,), 0x38: (4,), 0x3B: (3,), 0x3D: (3,), 0x4B: (2,), 0x4D: (1, 5)}
NS_STOP = {0x00, 0x02, 0x03, 0x34, 0x3A, 0x4D}
KINDS = {'cs': (CS, CS_STOP), 'ms': (MS, MS_STOP), 'ns': (NS, NS_STOP)}
MUSIC = {('cs', 0x4C): 'cs', ('cs', 0x4D): 'cs fade', ('ms', 0x30): 'ms', ('ms', 0x31): 'ms fade', ('ns', 0x47): 'ns'}


def size(kind, op, a):
    s = KINDS[kind][0].get(op)
    return s(a) if callable(s) else s


def decode(start, kind, limit=400):
    a, out = start, []
    for _ in range(limit):
        if not rom_ptr(a):
            return out, 'outside'
        op = u8(a)
        n = size(kind, op, a)
        if not n:
            return out, 'unknown %02x' % op
        out.append((a, op, n))
        if op in KINDS[kind][1]:
            return out, 'end'
        a += n
    return out, 'limit'


def follows(kind, a, op):
    """the scripts a command goes on to: (address, kind)"""
    if kind == 'cs':
        d = [(u32(a + CS_DEST[op]), 'cs')] if op in CS_DEST else []
        if op == 0x3B:   # a switch: its destinations while they are pointers
            k = 1
            while k < 33 and rom_ptr(u32(a + k)) and u32(a + k) < 0x08100000:
                d.append((u32(a + k), 'cs'))
                k += 4
        if op == 0x11: d.append((u32(a + 2), 'cs'))
        if op == 0x13 and u32(a + 1): d.append((u32(a + 1), 'cs'))
        return d
    if kind == 'ms':
        d = [(u32(a + k), 'ms') for k in MS_DEST.get(op, ())]
        if op == 0x26: d.append((u32(a + 1), 'cs'))   # a cutscene started
        return d
    return [(u32(a + k), 'ns') for k in NS_DEST.get(op, ())]


def walk(starts):
    """every command reachable from (address, kind, label): {(address, kind): (op, size, label)}, and the decodes that stopped short"""
    seen, short, todo = {}, 0, list(starts)
    while todo:
        s, kind, label = todo.pop()
        if (s, kind) in seen or not rom_ptr(s):
            continue
        out, why = decode(s, kind)
        short += why != 'end'
        for a, op, n in out:
            if (a, kind) in seen:
                break
            seen[(a, kind)] = (op, n, label)
            todo += [(x, k, label) for x, k in follows(kind, a, op) if rom_ptr(x)]
    return seen, short


# ---- the code ----

def bl_target(off):
    h1, h2 = struct.unpack_from('<HH', ROM, off)
    if (h1 & 0xF800) == 0xF000 and (h2 & 0xF800) == 0xF800:
        hi = h1 & 0x7FF
        if hi & 0x400: hi -= 0x800
        return B + off + 4 + (hi << 12) + ((h2 & 0x7FF) << 1)
    return None


CALLS = {}
for off in range(0, 0x200000, 2):
    t = bl_target(off)
    if t is not None:
        CALLS.setdefault(t, []).append(B + off)


def r_set(call, reg, back=8):
    """the immediate `reg` holds at `call`: a movs reg,#n (or movs rX,#n and adds reg,rX,#0) before it, with no call,
    branch or load between; None where it is not one (a register a routine filled, a number past a byte)"""
    src = reg
    for k in range(2, 2 * back, 2):
        h = u16(call - k)
        if (h & 0xF800) == 0x2000 and (h >> 8) & 7 == src:     # movs src, #imm
            return h & 0xFF
        if (h & 0xFFC0) == 0x1C00 and h & 7 == src:            # adds src, rN, #0
            src = (h >> 3) & 7
            continue
        if (h & 0xF800) in (0xF000, 0xF800) or (h & 0xF000) == 0xD000 or (h & 0xF800) == 0xE000 or (h & 0xFE00) in (0xB400, 0xBC00):
            return None   # a call, a branch, a push or pop: the trail ends
        if h & 7 == src and (h & 0xE000) in (0x0000, 0x6000, 0x8000) or (h & 0xF800) == 0x4800 and (h >> 8) & 7 == src:
            return None   # src written by anything else
    return None


def literal_r0(call):
    for k in range(2, 18, 2):
        h = u16(call - k)
        if (h & 0xFF00) == 0x4800:
            return u32(((call - k + 4) & ~3) + (h & 0xFF) * 4)
    return None


STARTERS = {0x080005A0: 'PlayMusic', 0x080005BE: 'PlayMusic again', 0x0800062A: 'PlayMusic faded', 0x08000598: 'PlaySoundEffect'}


def code_songs(places):
    for fn, name in STARTERS.items():
        for call in CALLS.get(fn, []):
            song = r_set(call, 0)
            if song is not None and 1 <= song <= 0x24 and (u16(call - 2) & 0xFF00) != 0x3000:   # (not movs r0,#n; adds r0,#m: a sound past 0xFF)
                places[song].append(('code', call, name))
    # a liberation's battle's win: the song in r2 (0x19, 0x1F, 0x24), to PlayMusic
    for call in CALLS.get(0x080005A0, []):
        if u16(call - 2) == 0x1C10:   # adds r0, r2, #0: the nearest movs r2 r_set found, the others on other paths
            nearest = r_set(call, 0)
            for k in range(4, 40, 2):
                h = u16(call - k)
                if (h & 0xFF00) == 0x2200 and 1 <= (h & 0xFF) <= 0x24:
                    if h & 0xFF == nearest:
                        nearest = None
                        continue
                    places[h & 0xFF].append(('code', call - k, 'a battle\'s win, r2'))


# ---- the scripts ----

def map_script_starts():
    starts = []
    for base, first, n in ((0x08034E44, 0x00, 12), (0x08034EBC, 0x80, 0x15)):
        for g in range(n):
            ta, tb = u32(base + 8 * g), u32(base + 8 * g + 4)
            if not (rom_ptr(ta) and rom_ptr(tb)):
                continue
            for which, t in (('entered', ta), ('each frame', tb)):
                for m in range(40):
                    p = u32(t + 4 * m)
                    if rom_ptr(p):
                        starts.append((p, 'ms', 'map %02x:%d %s' % (first + g, m, which)))
    for c in CALLS.get(0x0803731C, []):   # StartCutscene
        p = literal_r0(c)
        if p and rom_ptr(p):
            starts.append((p, 'cs', 'code %08x' % c))
    for m in range(9):   # the missions' own (BN5_LIB_MISSIONS, 0x5C each)
        for k in (12, 13):
            p = u32(0x0804F628 + 0x5C * m + 0xC + 4 * k)
            if rom_ptr(p):
                starts.append((p, 'ms', 'mission %d' % (m + 1)))
    return starts


def object_script_starts():
    words = {struct.unpack_from('<I', ROM, o)[0] for o in range(0, len(ROM) - 3, 4)}
    starts = []
    for v in sorted(w for w in words if rom_ptr(w)):
        out, why = decode(v, 'ns', 300)
        if why != 'end' or len(out) < 3:
            continue
        if all(B <= x < 0x08800000 for a, op, n in out for x, k in follows('ns', a, op)) and any(op in (0x14, 0x16, 0x17, 0x18, 0x4D) for a, op, n in out):
            starts.append((v, 'ns', 'object script'))
    return starts


def script_songs(places):
    seen, short = walk(map_script_starts() + object_script_starts())
    for (a, kind), (op, n, label) in seen.items():
        how = MUSIC.get((kind, op))
        if how:
            song = u16(a + 1)
            if 1 <= song <= 0x24:
                places[song].append((how, a, label))
    return len(seen), short


# ---- the text ----

def lz77(off, maxout=0x20000):
    if ROM[off] != 0x10: return None
    size = ROM[off + 1] | ROM[off + 2] << 8 | ROM[off + 3] << 16
    if not 16 <= size <= maxout: return None
    out, p = bytearray(), off + 4
    try:
        while len(out) < size:
            flags = ROM[p]; p += 1
            for b in range(8):
                if len(out) >= size: break
                if flags & (0x80 >> b):
                    v = ROM[p] << 8 | ROM[p + 1]; p += 2
                    n, disp = (v >> 12) + 3, (v & 0xFFF) + 1
                    if disp > len(out): return None
                    for _ in range(n): out.append(out[-disp])
                else:
                    out.append(ROM[p]); p += 1
    except IndexError:
        return None
    return bytes(out)


def archive_shape(arc):
    if len(arc) < 4: return False
    t0 = arc[0] | arc[1] << 8
    if t0 % 2 or not 2 <= t0 < len(arc): return False
    offs = [arc[2 * i] | arc[2 * i + 1] << 8 for i in range(t0 // 2)]
    return offs == sorted(offs) and all(t0 <= x <= len(arc) for x in offs)


def text_songs(places):
    n = 0
    pat = re.compile(rb'\xFD(?:(\x01)([\x01-\x24])\x00|(\x0B)([\x01-\x24]))', re.S)
    for o in range(0, len(ROM) - 8, 4):
        if ROM[o] != 0x10: continue
        data = lz77(o)
        if not data or data[0] or (data[1] | data[2] << 8 | data[3] << 16) != len(data) or not archive_shape(data[4:]):
            continue
        n += 1
        for m in pat.finditer(data[4:]):
            song = (m.group(2) or m.group(4))[0]
            places[song].append(('text' + (' fade' if m.group(3) else ''), B + o, 'archive +%04x' % m.start()))
    return n


# ---- lists and records ----

def map_counts():
    """each map group's maps: the entries of its table of scripts as each map is entered (0x08034E44, 0x08034EBC), which
    end where the next table begins (its maps' music bytes run on into the next group's, or a list's next array)"""
    tables = []
    for base, first, n in ((0x08034E44, 0x00, 12), (0x08034EBC, 0x80, 0x15)):
        for g in range(n):
            ta, tb = u32(base + 8 * g), u32(base + 8 * g + 4)
            if rom_ptr(ta) and rom_ptr(tb) and not ta & 1:
                tables.append((first + g, ta, tb))
    starts = sorted({p for g, ta, tb in tables for p in (ta, tb)})
    return {g: min(([p for p in starts if p > ta] or [ta])[0] - ta, 0x100) // 4 for g, ta, tb in tables}


def map_list_songs(places):
    counts = map_counts()
    for i in range(4):
        a = l = u32(0x080361C8 + 4 * i)
        while u8(a) != 0xFF and a - l < 0x200:
            g, p = u8(a), u32(a + 4)
            for m in range(counts.get(g, 0)):
                s = u8(p + m)
                if 1 <= s <= 0x24: places[s].append(('map', p + m, 'list %d group %02x map %d' % (i, g, m)))
            a += 8


def record_songs(places):
    for o in range(0x113000, 0x123000, 4):
        p = u32(B + o + 12)
        if not rom_ptr(p) or u8(p) != 0x00 or u8(p + 1) != 0x22: continue   # (MegaMan's entry first)
        q, ok = p, False
        for _ in range(16):
            t = u8(q)
            if t == 0xF0: ok = q > p; break
            if t not in (0x00, 0x01, 0x10, 0x11, 0x12, 0x20, 0x30): break
            q += 4
        if ok and 1 <= ROM[o + 2] <= 0x24:
            places[ROM[o + 2]].append(('battle', B + o, 'record'))


def main():
    want = [int(a, 16) for a in sys.argv[1:]]
    places = defaultdict(list)
    code_songs(places)
    map_list_songs(places)
    record_songs(places)
    ncmds, short = script_songs(places)
    narcs = text_songs(places)
    print('bn5_songs: %d script commands decoded (%d decodes stopped short: pointer tables read as scripts), %d text archives'
          % (ncmds, short, narcs))
    for song in want or range(1, 0x25):
        ps = places.get(song, [])
        kinds = defaultdict(int)
        for how, a, label in ps: kinds[how] += 1
        line = ', '.join('%s %d' % (k, v) for k, v in sorted(kinds.items())) or 'never started'
        print('0x%02X  %s' % (song, line))
        if want:
            for how, a, label in sorted(ps, key=lambda p: (p[0], p[1])):
                print('        %-8s %08x  %s' % (how, a, label))


if __name__ == '__main__':
    main()
