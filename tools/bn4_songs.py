#!/usr/bin/env python3
"""bn4_songs.py [SONG ...]: where Battle Network 4 Red Sun (USA) starts each of
its music player's songs, read from the player's own ROM (docs/rom/bn4.md,
Where BN4 plays its songs; the Jukebox's names, docs/game/pet.md). Prints
numbers and addresses only: nothing of the ROM is written anywhere. BN5's
twin is tools/bn5_songs.py, whose method this follows.

What starts a song, each found as the ROM has it:
  code     an immediate (or a register set to one) in r0 before a call of
           PlayMusic 0x08000568, its forced form 0x08000586,
           PlaySoundEffect 0x08000560 or the faded-in start 0x080005F2; and
           a battle's win, its song in r2
  map      the map-music lists 0x0802AFD0 (by the game state's +7)
  battle   a battle record's music byte (+3), or its mode's song from
           0x08007510 (modes 0x46 on)
  ms, cs   a map script's 0x2C/0x2D/0x2E, a cutscene's 0x49/0x4A/0x4B: every
           script of every map group's tables (0x08029C4C, 0x08029CC4)
           decoded, with the cutscenes they and the code start and every jump
  ns       an object script's 0x28/0x47, the scripts found by their shape
  text     a text script's 0xFD 00, 01, 06 or 07 (0B faded in), every text
           archive (LZ77) found by its shape
The command sizes are BN4's own handlers' (each one's advances of the
script's pointer: r7 a map script's and a cutscene's, r6 an object's),
measured once and kept here as numbers; a decode that meets a command it
does not know stops there, and is counted.

  python3 tools/bn4_songs.py          every song 0x00-0x21: where it starts
  python3 tools/bn4_songs.py 18 1E    those songs, every place listed
"""
import os
import re
import struct
import sys
from collections import defaultdict

ROMS = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
B = 0x08000000
LAST = 0x21   # the music player's last song (player 31's): 0x22 on are sounds


def bn4_rom():
    for name in sorted(os.listdir(ROMS)):
        path = os.path.join(ROMS, name)
        if name.endswith('.gba') and os.path.getsize(path) == 0x800000:
            with open(path, 'rb') as f:
                data = f.read()
            if data[0xAC:0xB0] == b'B4WE':
                return data
    sys.exit('bn4_songs: no BN4 Red Sun ROM in ' + ROMS)


ROM = bn4_rom()


def u8(a): return ROM[a - B]
def u16(a): return struct.unpack_from('<H', ROM, a - B)[0]
def u32(a): return struct.unpack_from('<I', ROM, a - B)[0]
def rom_ptr(v): return B <= v < B + len(ROM)


def sizes(text):
    return {int(k, 16): int(v) for k, v in (p.split(':') for p in text.split())}


# Each command's size, its handler's: the cutscenes' (0x0802C140, 0x67 of
# them), the map scripts' (0x0802A7D0, 0x40, many of them the cutscenes'
# handlers), the objects' (0x080A85D0, 0x4C, 0x01 and 0x2C none). The ones whose size
# hangs on a sub-command are functions.
CS = sizes('00:1 01:1 02:3 03:4 04:2 05:2 06:1 07:1 08:1 09:3 0a:2 0b:1 0c:1 0d:3 0e:3 0f:1 10:1 11:6 12:2 13:5 14:5 15:7 16:8 17:8 '
           '18:8 19:8 1b:7 1c:8 1d:10 1e:6 1f:6 20:6 21:6 22:6 23:6 24:4 25:3 26:4 27:4 28:4 29:4 2a:5 2b:5 2c:6 2d:7 2e:9 2f:3 '
           '30:3 31:4 32:3 33:5 34:5 35:2 37:1 38:3 39:1 3a:5 3b:2 3c:4 3d:10 3e:5 3f:3 40:5 43:4 44:3 45:13 47:5 49:3 4a:3 4b:5 '
           '4c:3 4d:1 4e:4 4f:3 51:3 52:2 53:2 54:4 55:2 56:13 57:1 58:4 59:2 5a:1 5b:1 5c:1 5d:1 5e:2 5f:1 60:1 61:1 62:1 63:1 '
           '64:3 65:4 66:3')
CS.update({
    0x1A: lambda a: {0: 11, 1: 12, 2: 14}.get(u8(a + 1), 11),   # a byte's, hword's or word's compare
    0x36: lambda a: 3 if u8(a + 1) == 0xFF else 2,              # a text script, from memory or not
    0x41: lambda a: {0: 5, 1: 2}.get(u8(a + 1), 6),
    0x42: lambda a: 2 if u8(a + 1) == 1 else 4,
    0x46: lambda a: {1: 3, 3: 2, 4: 2, 5: 2}.get(u8(a + 1), 6),
    0x48: lambda a: 2 if u8(a + 1) & 0x40 else 7,               # a warp
    0x50: lambda a: 6 if u8(a + 1) == 0 else 2,                 # a camera script run or stopped
})
CS_DEST = {0x14: 1, 0x15: 3, 0x16: 4, 0x17: 4, 0x18: 4, 0x19: 4, 0x1A: 6, 0x1B: 3, 0x1C: 4, 0x1D: 6, 0x1E: 2, 0x1F: 2,
           0x20: 2, 0x21: 2, 0x22: 2, 0x23: 2}
CS_STOP = {0x00, 0x01, 0x10, 0x14, 0x39}
MS = sizes('00:1 02:7 03:8 04:8 05:8 06:8 08:8 09:10 0a:6 0b:6 0c:6 0d:6 0e:6 0f:6 10:6 11:6 12:7 13:7 14:7 15:7 16:7 17:5 '
           '18:5 19:5 1a:4 1b:3 1c:4 1d:4 1e:4 1f:4 20:5 21:5 22:9 23:9 24:6 25:7 26:9 27:3 28:3 29:5 2a:5 2b:2 2c:3 2d:3 2e:5 '
           '2f:3 30:1 31:4 32:3 33:2 34:13 35:1 37:2 38:1 39:1 3a:1 3b:1 3c:1 3e:4 3f:3')
MS.update({0x01: 5, 0x07: CS[0x1A], 0x36: lambda a: 2 if u8(a + 1) == 1 else 6, 0x3D: lambda a: 3 if u8(a + 1) == 1 else 6})
MS_DEST = {0x01: (1,), 0x02: (3,), 0x03: (4,), 0x04: (4,), 0x05: (4,), 0x06: (4,), 0x07: (6,), 0x08: (4,), 0x09: (6,),
           0x0A: (2,), 0x0B: (2,), 0x0C: (2,), 0x0D: (2,), 0x0E: (2,), 0x0F: (2,), 0x10: (2,), 0x11: (2,), 0x12: (3,), 0x13: (3,), 0x14: (3,), 0x15: (3,), 0x16: (3,),
           0x17: (1,), 0x18: (1,), 0x19: (1,)}
MS_STOP = {0x00, 0x01}
NS = sizes('02:5 03:1 04:7 05:7 06:3 07:3 08:1 09:1 0a:2 0b:2 0c:4 0d:1 0e:1 0f:2 10:2 11:3 12:1 13:1 14:7 15:4 16:2 17:2 18:2 '
           '19:1 1a:1 1b:1 1c:5 1d:2 1e:2 1f:1 20:1 21:1 22:1 23:1 24:1 25:3 26:1 27:1 28:3 29:3 2a:2 2b:3 2d:3 2e:2 2f:4 30:1 '
           '31:2 32:5 33:2 34:1 35:5 36:5 37:9 38:8 39:2 3a:1 3b:7 3c:3 3d:7 3e:3 40:1 41:1 42:7 43:7 44:6 45:3 46:2 47:3 48:1 '
           '49:1 4a:4 4b:6')
NS[0x00] = 1   # a wait: the script stays on it
NS[0x3F] = lambda a: 1   # a switch on the story: its destinations follow (follows)
NS_DEST = {0x02: (1,), 0x04: (3,), 0x05: (3,), 0x36: (1,), 0x38: (4,), 0x3B: (3,), 0x3D: (3,), 0x42: (3,), 0x43: (3,),
           0x4B: (2,)}
NS_STOP = {0x00, 0x02, 0x03, 0x34, 0x3A, 0x3F}
KINDS = {'cs': (CS, CS_STOP), 'ms': (MS, MS_STOP), 'ns': (NS, NS_STOP)}
# the music commands: the song a hword after the command (0x49's, 0x2C's and 0x26's by PlaySoundEffect, a forced start)
MUSIC = {('cs', 0x49): 'cs', ('cs', 0x4A): 'cs', ('cs', 0x4B): 'cs fade', ('ms', 0x2C): 'ms', ('ms', 0x2D): 'ms',
         ('ms', 0x2E): 'ms fade', ('ns', 0x28): 'ns', ('ns', 0x47): 'ns'}


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


def switch(a, first):
    """a switch's destinations: its words from `first` while they are pointers into the scripts' code"""
    d, k = [], first
    while k < 33 and rom_ptr(u32(a + k)) and u32(a + k) < 0x08100000:
        d.append(u32(a + k))
        k += 4
    return d


def follows(kind, a, op):
    """the scripts a command goes on to: (address, kind)"""
    if kind == 'cs':
        d = [(u32(a + CS_DEST[op]), 'cs')] if op in CS_DEST else []
        if op == 0x39: d += [(x, 'cs') for x in switch(a, 1)]   # by a menu's answer
        if op == 0x11: d.append((u32(a + 2), 'cs'))             # a second thread
        if op == 0x13 and u32(a + 1): d.append((u32(a + 1), 'cs'))
        return d
    if kind == 'ms':
        d = [(u32(a + k), 'ms') for k in MS_DEST.get(op, ())]
        if op == 0x23: d.append((u32(a + 1), 'cs'))   # a cutscene started (BN4_START_CUTSCENE)
        return d
    d = [(u32(a + k), 'ns') for k in NS_DEST.get(op, ())]
    if op == 0x3F: d += [(x, 'ns') for x in switch(a, 1)]
    return d


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


STARTERS = {0x08000568: 'PlayMusic', 0x08000586: 'PlayMusic forced', 0x08000560: 'PlaySoundEffect', 0x080005F2: 'PlayMusic faded'}
WIN = 0x080071F8   # a battle's win: PlayMusic of r2, 0x18 or 0x1E (0x080071DE, 0x080071EA, 0x080071F2)


def code_songs(places):
    for fn, name in STARTERS.items():
        for call in CALLS.get(fn, []):
            song = r_set(call, 0)
            if song is not None and song <= LAST and (u16(call - 2) & 0xFF00) != 0x3000:   # (not movs r0,#n; adds r0,#m: a sound past 0xFF)
                places[song].append(('code', call, name))
    if u16(WIN - 2) == 0x1C10:   # adds r0, r2, #0
        for k in range(4, 40, 2):
            h = u16(WIN - k)
            if (h & 0xFF00) == 0x2200 and (h & 0xFF) <= LAST:   # movs r2, #song
                places[h & 0xFF].append(('code', WIN - k, 'a battle\'s win, r2'))


# ---- the scripts ----

# each map group's two tables of scripts, a word a map: as it is entered, then each frame (read at 0x08029EE6); the real
# world's 6 groups and the net's 21 (BN4's map loader's, docs/rom/bn4.md, Map tables), the net's groups without maps null
MAP_SCRIPTS = ((0x08029C4C, 0x00, 6), (0x08029CC4, 0x80, 21))


def script_tables():
    """(group, entered, each frame, its maps) for every map group with maps: the first table ends where the second
    begins, a word a map (group 0x00's 7, 0x94's 8: the Undernet 1-6 and Black Earth 1-2)"""
    out = []
    for base, first, n in MAP_SCRIPTS:
        for g in range(n):
            ta, tb = u32(base + 8 * g), u32(base + 8 * g + 4)
            if rom_ptr(ta) and rom_ptr(tb) and tb > ta:
                out.append((first + g, ta, tb, (tb - ta) // 4))
    return out


def map_counts():
    return {g: n for g, ta, tb, n in script_tables()}


def map_script_starts():
    starts = []
    for g, ta, tb, n in script_tables():
        for which, t in (('entered', ta), ('each frame', tb)):
            for m in range(n):
                p = u32(t + 4 * m)
                if rom_ptr(p):
                    starts.append((p, 'ms', 'map %02x:%d %s' % (g, m, which)))
    for c in CALLS.get(0x0802BD10, []):   # BN4_START_CUTSCENE
        p = literal_r0(c)
        if p and rom_ptr(p):
            starts.append((p, 'cs', 'code %08x' % c))
    return starts


def object_script_starts():
    words = {struct.unpack_from('<I', ROM, o)[0] for o in range(0, len(ROM) - 3, 4)}
    starts = []
    for v in sorted(w for w in words if rom_ptr(w)):
        out, why = decode(v, 'ns', 300)
        if why != 'end' or len(out) < 4:
            continue
        if all(B <= x < 0x08800000 for a, op, n in out for x, k in follows('ns', a, op)) and any(op in (0x14, 0x16, 0x17, 0x18) for a, op, n in out):
            starts.append((v, 'ns', 'object script'))
    return starts


def script_songs(places):
    seen, short = walk(map_script_starts() + object_script_starts())
    for (a, kind), (op, n, label) in seen.items():
        how = MUSIC.get((kind, op))
        if how:
            song = u16(a + 1)
            if song <= LAST:
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
    pat = re.compile(rb'\xFD(?:([\x00\x01\x06\x07])([\x00-\x21])\x00|(\x0B)([\x00-\x21]))', re.S)
    for o in range(0, len(ROM) - 8, 4):
        if ROM[o] != 0x10: continue
        data = lz77(o)   # (BN4's archives start at once: no header of their size, as BN5's and BN6's have)
        if not data or not archive_shape(data):
            continue
        n += 1
        for m in pat.finditer(data):
            song = (m.group(2) if m.group(1) else m.group(4))[0]
            places[song].append(('text' + (' fade' if m.group(3) else ''), B + o, 'archive +%04x' % m.start()))
    return n


# ---- lists and records ----

MAP_MUSIC = 0x0802AFD0   # BN4_MAP_MUSIC
MODE_SONGS = 0x08007510  # BN4_MODE_SONGS: modes 0x46 on


def map_list_songs(places):
    counts = map_counts()
    for i in range(4):
        a = l = u32(MAP_MUSIC + 4 * i)
        while u8(a) != 0xFF and a - l < 0x200:
            g, p = u8(a), u32(a + 4)
            for m in range(counts.get(g, 0)):
                s = u8(p + m)
                if s <= LAST: places[s].append(('map', p + m, 'list %d group %02x map %d' % (i, g, m)))
            a += 8


BATTLE_TABLES = 0x08017FB8   # BN4_BATTLE_TABLES: the real world's groups (6), the net's (21); a pointer per map index, a list of records each
STORY_BATTLES = 0x080FA900   # BN4_STORY_BATTLES: the scripts' records, 12 bytes each


def record_shape(r):
    """a battle record (BN4_RECORD_SIZE, 12 bytes): its entities' list (BN4_ENTITY_SIZE each, 0xFF ending it) first MegaMan"""
    p = u32(r + 8)
    return rom_ptr(p) and u8(p) == 0x00 and u8(p + 3) == 0x00 and 0xFF in ROM[p + 4 - B:p + 4 + 4 * 16 - B:4]


def battle_records():
    """every record the encounters roll (both lists, every group's map indices) and every story battle's"""
    out = []
    for which, groups in ((0, 6), (1, 21)):
        tables = u32(BATTLE_TABLES + 4 * which)
        for g in range(groups):
            lst = u32(tables + 4 * g)
            if not rom_ptr(lst): continue
            k = 0
            while k < 64 and rom_ptr(u32(lst + 4 * k)):
                r = u32(lst + 4 * k)
                while u8(r) != 0xFF and record_shape(r):
                    out.append((r, 'encounter'))
                    r += 12
                k += 1
    r = STORY_BATTLES
    while record_shape(r):
        out.append((r, 'story battle %d' % ((r - STORY_BATTLES) // 12)))
        r += 12
    return out


def record_songs(places):
    for r, label in battle_records():
        song, mode = u8(r + 3), u8(r + 4)
        if 0x46 <= mode < 0x46 + 7:
            song = u8(MODE_SONGS + mode - 0x46)
        if song <= LAST:
            places[song].append(('battle', r, label))


def main():
    want = [int(a, 16) for a in sys.argv[1:]]
    places = defaultdict(list)
    code_songs(places)
    map_list_songs(places)
    record_songs(places)
    ncmds, short = script_songs(places)
    narcs = text_songs(places)
    print('bn4_songs: %d script commands decoded (%d decodes stopped short: pointer tables read as scripts), %d text archives'
          % (ncmds, short, narcs))
    for song in want or range(0, LAST + 1):
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
