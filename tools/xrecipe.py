#!/usr/bin/env python3
"""xrecipe.py [--check] [--report] [--ai N ...] [--no-search]: BN5's virus families' recipes, made from the two ROMs
(issue #82; docs/MULTIROM.md, Transplanted families; docs/DEVTOOLS.md, The recipe generator).

A family's recipe (src/emu/xvirus_recipe.h) is numbers alone: where its code lies in BN5 and where it goes, which of its
bytes are data, its calls' and pointers' twins in BN6, the immediates BN6 numbers otherwise, its states and hooks in
BN6's layout, its records, what it draws and spawns. This derives each from BN5 Team Colonel's ROM and BN6 Cybeast
Gregar's (found by their SHA-1s in CYBERWORLD_ROM_DIR or ~/.cache/mmbn-ref/roms): every routine its states and hooks
reach, walked instruction by instruction (tools/xrecipe_thumb.py, the project's own decoder); every call out paired with
BN6's twin (tools/xrecipe_pairs.py: the project's pairs in tools/bn5_twins.py first, the symbol map of
tools/bn6f_match.py where it was made, then a BN6 routine of the same body), each pair's bodies compared and a pair
whose bodies differ taken only where its audit says it serves (bn5_twins.AUDITS, docs/ROM_DATA.md); the objects it
spawns BN6 has no twin of moved with it, through the second-level dispatch (src/emu/xvirus_dispatch.h); its sprites,
effects and sounds by their bytes. Numbers are allocated in install order (FAMILIES: an AI index from 32, six ids from
0x200, a home, code and art places, sprite numbers, effect ids), as the static model wants them (docs/MULTIROM.md).

  python3 tools/xrecipe.py              write src/emu/xvirus_bn5.c: FAMILIES' recipes, whole (no decision open)
  python3 tools/xrecipe.py --check      write nothing: fail where src/emu/xvirus_bn5.c is not what it would write
  python3 tools/xrecipe.py --report     every BN5 family derived (AI 0-34), in install order after FAMILIES: what each
                                        takes and what still needs a decision, into .build/xrecipe/report.md and .json

Local only: both ROMs. Nothing of either is written: the C file holds addresses, counts and CRC32s, and the report the
same. tests/test_core.c compares the generated CanGard and Handi with the hand-made recipes the build installs
(src/emu/xvirus_families.c), field for field.
"""
import argparse
import hashlib
import json
import os
import re
import struct
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bn5_objects as OB  # noqa: E402
import bn5_twins  # noqa: E402
import xrecipe_thumb as T  # noqa: E402
from xrecipe_family import BN5, BN6, GENERIC5, GENERIC6, HOOKS6, Deriver, Family, kind_name  # noqa: E402
from xrecipe_pairs import Pairs  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_C = os.path.join(ROOT, 'src', 'emu', 'xvirus_bn5.c')
OUT_JSON = os.path.join(ROOT, 'tests', 'xdiff', 'families.json')
WORK = os.path.join(ROOT, '.build', 'xrecipe')
MATCH = os.path.join(ROOT, '.build', 'symbols', 'match')
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
SHA1 = {'bn6': '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6', 'bn5': '5f472f78d8de2df01d5039e045c043cb40969a39'}

# The families written into the C file, in install order (each its slot: AI 32 + slot, ids 0x200 + 6 x slot, ...): the
# ones installed (src/emu/xvirus_families.c, xvirus_bn5) and checked against BN5's own (tools/xdiff.py). Each is BN5's AI
# index and the project's own name for it, which the C file takes: the names BN5's archive gives are read to check
# them and shown in the local report, never written into the C file
FAMILIES = ((14, 'CanGard'), (4, 'Handi'), (2, 'Powie'), (3, 'Bugtank'), (5, 'Dominerd'), (6, 'MetFire'), (7, 'Shakey'), (9, 'NinJoy'),
            (11, 'Lark'), (12, 'BomBoy'), (13, 'Zomon'), (16, 'Drixol'), (17, 'Batty'), (18, 'Skarab'), (20, 'Draggin'), (21, 'Whirly'),
            (22, 'Marina'), (23, 'Flashy'), (24, 'Eleogre'), (25, 'Cactikil'), (26, 'WuNote'), (27, 'Appley'), (28, 'BigBrute'),
            (29, 'TinHawk'), (30, 'Bladia'))
# BN5's families not moved, and why (the report says so for each): the five BN6 has by name are BN6's own (docs/MULTIROM.md,
# the finer calls), and these no battle of BN5's names (a scan of every entity list a word of BN5's ROM points at, 386 of
# them, names no id of AI 31-34: TestVirs's 0x00 is its only id, AI 31 and 32 have none, AI 33's 0xB5-0xC6 and 0xCD-0xFF
# and Tuby's 0xC7-0xCC appear in none)
NOT_MOVED = {0: 'TestVirs, a test family no battle names', 31: 'no enemy id names AI 31', 32: 'no enemy id names AI 32',
             33: 'no battle of BN5\'s names its ids (0xB5-0xC6, 0xCD-0xFF)', 34: 'Tuby: no battle of BN5\'s names its ids (0xC7-0xCC)'}

# The static layout's places (src/emu/xvirus_recipe.h)
EMU_FREE = 0x08800000
XV_AT = EMU_FREE + 0x330000
XV_HOMES, XV_CODE, XV_ART, XV_END = XV_AT + 0x18000, XV_AT + 0x38000, XV_AT + 0x50000, XV_AT + 0xD0000
XV_HOME_SIZE, XV_FIRST_ID, XV_FIRST_AI = 0x800, 0x200, 32
CODE_STEP = 0x200      # each block begins at the next multiple of this past the last (its distance from a word kept)
ART_STEP = 0x800       # each family's art begins at the next multiple of this
ROOM = {'battle': 0x1000, 'load': 0x800, 'effect': 0x400}   # a sprite's room: its bytes rounded up to this
SONG_STEP = 0x100      # a copied song's room: its bytes rounded up to this
SONGS_EMPTY_LAST = 0x62   # BN6's empty song slots end (0x26-0x62)
# BN5's sounds BN6 has no song of, each at one slot of BN6's empty ones whatever track copies it (issue #82's shared
# allocations, binding: the first install copies it, the others find it there); a sound of a family's not here is a decision
SOUND_SLOTS = {0xC0: 0x62, 0xB7: 0x61, 0xBA: 0x60}
REGISTRY_C = os.path.join(ROOT, 'src', 'core', 'xchips_registry.c')
# the numbers the grown lists and tables give the families, in turn (xvirus_recipe.h: BN6's placeholders first)
# (issue #82's shared allocations, binding: list 1 BN6's placeholders 0x00 and 0x1E, then upward from BN6's end; list 2
# upward from 0x30, the Navis' 0x20-0x2F skipped; lists 4 and 5 upward from BN6's ends, list 5's at 0x80 as BN6 reads it
# (BN6_SPRITES_LIST5_THROUGH: the real world's people, 5:0x20-0x7F, are list 6's table read through list 5's word); the
# chips take theirs down from 0xFF)
LIST_NUMBERS = {1: [0x1E, 0x00] + list(range(0x20, 0x100)), 2: list(range(0x30, 0x100)),
                4: list(range(0x5C, 0x100)), 5: list(range(0x80, 0x100))}
FIRST_EFFECT = 0x6C
BN6_EFFECT_LIST = 4    # the grown list an effect's sprite of an ungrown list is listed in (the attacks' and effects')
XV_SPAWN = {(1, 0x08003140): 'XV_SPAWN_1', (1, 0x0800315C): 'XV_SPAWN_1_LAST', (1, 0x080031E8): 'XV_SPAWN_1_LAST',
            (3, 0x08003178): 'XV_SPAWN_3', (3, 0x08003194): 'XV_SPAWN_3_LAST', (3, 0x080031B0): 'XV_SPAWN_3_FIRST',
            (4, 0x080031CC): 'XV_SPAWN_4'}
BN5_SPAWN_NAME = {0x08003140: 'BN5_SPAWN_1', 0x0800315C: 'BN5_SPAWN_1_LAST', 0x080031E8: 'BN5_SPAWN_1_LAST2',
                  0x08003178: 'BN5_SPAWN_3', 0x08003194: 'BN5_SPAWN_3_LAST', 0x080031B0: 'BN5_SPAWN_3_FIRST', 0x080031CC: 'BN5_SPAWN_4'}


# ---- the ROMs ----

def find_roms():
    found = {}
    for name in sorted(os.listdir(ROM_DIR)) if os.path.isdir(ROM_DIR) else []:
        path = os.path.join(ROM_DIR, name)
        if name.lower().endswith('.gba') and os.path.isfile(path) and os.path.getsize(path) == 0x800000:
            with open(path, 'rb') as f:
                data = f.read()
            for k, v in SHA1.items():
                if hashlib.sha1(data).hexdigest() == v:
                    found[k] = data
    missing = [k for k in SHA1 if k not in found]
    if missing:
        sys.exit(f'xrecipe: no {" or ".join(missing).upper()} ROM in {ROM_DIR} (BN6 Cybeast Gregar and BN5 Team Colonel, USA)')
    return T.Rom(found['bn5'], 'bn5'), T.Rom(found['bn6'], 'bn6')


# ---- sprites (LZ77 decompressed), as src/layer/xnavi.c sizes them ----

def lz77(rom, a):
    d = rom.data
    o = a - T.BUS
    if d[o] != 0x10:
        return None
    size = d[o + 1] | d[o + 2] << 8 | d[o + 3] << 16
    out, i = bytearray(), o + 4
    while len(out) < size:
        flags = d[i]
        i += 1
        for bit in range(8):
            if len(out) >= size:
                break
            if flags & (0x80 >> bit):
                b1, b2 = d[i], d[i + 1]
                i += 2
                n, disp = (b1 >> 4) + 3, ((b1 & 0xF) << 8 | b2) + 1
                for _ in range(n):
                    out.append(out[-disp])
            else:
                out.append(d[i])
                i += 1
    return bytes(out[:size])


def sprite_len(b, n):
    """xnavi.c's xnavi_sprite_len on bytes b (its four bytes of size first): how much of them the sprite takes"""
    g32 = lambda o: int.from_bytes(b[o:o + 4], 'little')
    if n < 8:
        return 0
    base, m = 4, n - 4
    anims = g32(base) // 4
    end = 4 * anims
    if not anims or anims > 256 or end > m:
        return 0
    for a in range(anims):
        f = g32(base + 4 * a)
        for _ in range(256):
            if f + 20 > m:
                return 0
            end = max(end, f + 20)
            for part in range(2):
                at = g32(base + f + 4 * part)
                if at + 4 > m or g32(base + at) > m - at - 4:
                    return 0
                end = max(end, at + 4 + g32(base + at))
            tab = g32(base + f + 12)
            if tab + 4 > m or g32(base + tab) > m - tab:
                return 0
            for j in range(g32(base + tab) // 4):
                o = tab + g32(base + tab + 4 * j)
                while o + 2 <= m and not (b[base + o] == 0xFF and b[base + o + 1] == 0xFF):
                    o += 5
                if o + 2 > m:
                    return 0
                end = max(end, o + 2)
            end = max(end, g32(base + f + 8) + 8)
            if b[base + f + 18] & 0x80:
                break
            f += 20
    return end + 4


def sprite(rom, lists, lst, idx):
    """(its pointer word, its bytes as the installer copies them: decompressed, its own size word dropped) or (word, None)"""
    base = rom.u32(lists + 4 * lst)
    p = rom.u32(base + 4 * idx)
    a = p & 0x7FFFFFFF
    if not rom.has(a, 4):
        return p, None
    if p & 0x80000000:
        raw = lz77(rom, a)
        if not raw or len(raw) <= 8:
            return p, None
        raw = raw[4:]
        return p, raw if sprite_len(raw, len(raw)) else None
    n = sprite_len(rom.data[a - T.BUS:a - T.BUS + 0x40000], 0x40000)
    return p, rom.bytes(a, n) if n else None


# ---- names, ids, held chips ----

def text(rom, archive, i):
    """an entry of a text archive, its letters and digits (others by their code), for the report"""
    p = archive + rom.u16(archive + 2 * i)
    s = ''
    for c in rom.bytes(p, 24):
        if c >= 0xE5:
            break
        s += chr(ord('A') + c - 0x0B) if 0x0B <= c <= 0x24 else chr(ord('a') + c - 0x26) if 0x26 <= c <= 0x3F else \
            chr(ord('0') + c - 0x01) if 0x01 <= c <= 0x0A else ' ' if c == 0 else f'{{{c:02x}}}'
    return s


def ids_of(b5, ai):
    out = {}
    for i in range(BN5['nids']):
        v, kind, a = b5.bytes(BN5['ids'] + 3 * i, 3)
        if kind == 0 and a == ai and v not in out:
            out[v] = i
    return [out.get(v) for v in range(6)]


def chip_names(rom, archives, n):
    return {i: text(rom, archives[i >> 8], i & 0xFF) for i in range(1, n)}


def registry():
    """the chip registry (src/core/xchips_registry.c, numbers only): BN5's chip id -> (BN6's id, how it comes in)"""
    out = {}
    with open(REGISTRY_C, encoding='utf-8') as f:
        for m in re.finditer(r'\{\s*(\d+),\s*(\d+),\s*XREG_(\w+)', f.read()):
            out[int(m.group(1))] = (int(m.group(2)), m.group(3))
    return out


# ---- a family's recipe ----

# The tracks whose copies of a moved type install before the families' (src/emu/xinstall.c, xinstall_core)
BEFORE_FAMILIES = ('navis', 'chips')


class Alloc:
    """the numbers the families take, in install order"""

    def __init__(self):
        self.code, self.art, self.effect = XV_CODE, XV_ART, FIRST_EFFECT
        self.lists = {k: list(v) for k, v in LIST_NUMBERS.items()}
        # (the numbers tools/bn5_objects.py gives the moved objects' copies of the families' kinds, from the families' range:
        # placed by that install, before any family's)
        for lst, num, track in OB.IMAGE_COPIED.values():
            if track == 'families' and num in self.lists.get(lst, ()):
                self.lists[lst].remove(num)
        self.memo = {}   # (what, BN5's number) -> BN6's, one BN5 thing one BN6 number across families
        # (kind, BN5's type) -> the family that moved it first: those after spawn its copy; the moved objects' first of all
        # (tools/bn5_objects.py), then the types whose copy tools/bn5_twins.py's TYPES gives the Navis or the chips' code, all
        # installed before any family (src/emu/xinstall.c, xinstall_core)
        self.moved_types = {(m['kind'], m['type']): 'the moved objects' for m in OB.MODULES}
        for key, rec in bn5_twins.TYPES.items():
            if rec[0] == 'moved' and rec[2] in BEFORE_FAMILIES:
                self.moved_types.setdefault(key, f'the {rec[2]}\' copy')

    def number(self, kind, key):
        k = (kind, key)
        if k in self.memo:
            return self.memo[k], False
        v = self.lists[kind].pop(0) if isinstance(kind, int) else self.take_effect()
        self.memo[k] = v
        return v, True

    def take_effect(self):
        v = self.effect
        self.effect += 1
        return v

    def song(self, v):
        """BN6's empty slot for BN5's song v (SOUND_SLOTS: one slot a song across families and tracks): (slot or 0, fresh)"""
        k = ('song', v)
        if k in self.memo:
            return self.memo[k], False
        if v not in SOUND_SLOTS:
            return 0, False
        self.memo[k] = SOUND_SLOTS[v]
        return self.memo[k], True

    def bn6_sprites(self, b6, lst):
        """BN6's list `lst`: its sprites by their bytes' hash (the first number of each)"""
        if not hasattr(self, '_sprites'):
            self._sprites = {}
        if lst not in self._sprites:
            base, out = b6.u32(BN6['sprites'] + 4 * lst), dict(bytes={})
            for k in range(256):
                q = b6.u32(base + 4 * k)
                if not 0x08000000 <= q & 0x7FFFFFFF < 0x08800000:
                    break
                _, c = sprite(b6, BN6['sprites'], lst, k)
                if c:
                    out['bytes'].setdefault(hashlib.sha1(c).hexdigest(), k)
            self._sprites[lst] = out
        return self._sprites[lst]

    def song_twins(self, b6):
        """BN6's songs by how they sound (song_sig): the first number of each"""
        if not hasattr(self, '_twins'):
            self._twins = {}
            for i in range(0x300):
                try:
                    sig = song_sig(b6, BN6['songs'], i)
                except (IndexError, struct.error):
                    sig = None
                if sig and i not in range(SONGS_EMPTY_LAST - 0x3C, SONGS_EMPTY_LAST + 1):
                    self._twins.setdefault(sig, i)
        return self._twins


def rounded(n, step):
    return (n + step - 1) // step * step


class Recipe:
    pass


def art_same(b5, b6, alloc, a5, a6):
    """BN5's sprite and BN6's, each (its list's byte offset, its number) as an object's code loads them, draw the same
    (their bytes, or BN6's drawing BN5's animations: sprite_canon); a number made otherwise draws the same only where both
    are made so"""
    if None in a5 or None in a6:
        return a5 == a6
    (l5, n5), (l6, n6) = a5, a6
    if l5 != l6 or l5 % 4:
        return False
    _, s5 = sprite(b5, 0x0803272C, l5 // 4, n5)
    _, s6 = sprite(b6, BN6['sprites'], l6 // 4, n6)
    return bool(s5) and s5 == s6


def derive(b5, b6, pairs, ai, slot, alloc, reg):
    d = Deriver(b5, b6, pairs, alloc.moved_types, lambda x, y: art_same(b5, b6, alloc, x, y))
    f = Family(ai)
    t = [d.table(ai, k) for k in range(9)]
    f.st, f.s1, f.s2, f.hooks5 = t[0], t[1], t[2], t[3:9]
    nxt = [x for x in d.s1s if x > f.s1]
    f.region = (f.s1, nxt[0] if nxt else f.s1 + 0x1000)
    f.last = not nxt
    f.ids5 = ids_of(b5, ai)
    f.name = text(b5, BN5['names'], f.ids5[0]) if f.ids5[0] is not None else ''
    f.states5 = []
    for k in range(24):
        v = b5.u32(f.st + 4 * k)
        if not T.is_code_ptr(v, b5):
            break
        f.states5.append(v)
    d.walk_family(f)
    blocks = d.blocks_of(f)
    for _ in range(8):
        roots = [r for r in d.dead_roots(f, blocks) if r not in f.walks]
        if not roots:
            break
        f.notes += [f'dead code at {r:08X} (nothing names it): walked and fixed up as if live' for r in roots]
        d.walk_family(f, extra=roots)
        blocks = d.blocks_of(f)
    f.data = d.classify(f, blocks)
    d.scan(f)
    d.scan_words(f, blocks)
    for key in sorted(f.needs):
        # (a type of the moved objects its code reaches through their spawner: run as their copy, which the install checks is there)
        f.types.setdefault(key, dict(bn6=None, routine=None, shared='the moved objects',
                                     why='the moved objects\' (tools/bn5_objects.py), installed before any family: their copy run for this one'))
    d.type_decisions(f)
    d.kind_decisions(f)
    r = Recipe()
    r.f, r.ai5, r.name, r.slot = f, ai, f.name, slot
    r.ai, r.first_id, r.home = XV_FIRST_AI + slot, XV_FIRST_ID + 6 * slot, XV_HOMES + XV_HOME_SIZE * slot
    r.blocks = []
    at = alloc.code
    for lo, hi in blocks:
        at = rounded(at, CODE_STEP) + (lo & 3)
        r.blocks.append((lo, hi, at, zlib.crc32(b5.bytes(lo, hi - lo)) & 0xFFFFFFFF))
        at += hi - lo
    alloc.code = at
    r.data = f.data
    calls_of(r, d, blocks)
    r.clones = []
    states_of(r, d, alloc)
    hooks_of(r, d)
    r.s1, r.s2 = f.s1, f.s2
    r.names = f.ids5
    s1 = b5.bytes(f.s1, 8)
    r.traits = dict(mode=s1[4], shadow=bool(s1[5]))
    if s1[6] or s1[7]:
        f.decide('struct1', f'its struct1 +6/+7 are {s1[6]:02x} {s1[7]:02x} (not 0): unknown in BN5\'s layout')
    if s1[4] >= 32:
        f.notes.append(f'its struct1\'s mode {s1[4]:#x} is past BN6\'s 32: BN6\'s record takes 0 (XV_MODE_PAST)')
    if s1[0] != 4:
        f.decide('struct1', f'its battle sprite is of list {s1[0] // 4}, not list 1')
    held_of(r, b5, reg)
    r.more_imms = []
    chip_compares(r, reg)
    art_of(r, b5, b6, alloc, s1)
    types_of(r, alloc)
    # (the third kind's objects its code spawns, through the dispatch or an engine routine, that run an obstacle's states)
    spawned3 = {t5 for k, t5, _, _ in r.types if k == 3} | {v for t in r.f.calls for k, v in [d.spawned_by(t)] if k == 3}
    r.t3_states = sorted(t for t in spawned3 if d.runs_states(3, t))
    sounds_of(r, b5, b6, alloc)
    conditions_of(r, pairs)
    name_ranges(r)
    r.imms = unique_imms(r.f.imms + r.more_imms)
    r.words = sorted((v, to) for v, (to, why, tw) in f.words.items())
    r.numbers = sorted((lit, was, now) for lit, (was, now, why) in f.numbers.items())
    if f.chips and not r.held_chips:
        f.decide('chips', f'it writes a chip\'s numbers for its attack ({len(f.chips)} calls) and holds none')
    r.prestep = 0
    for lo, hi, at in dispatcher_cases(b5):
        if any(x is not None and lo <= x <= hi for x in f.ids5):
            if at in bn5_twins.PRESTEP and all(x is not None and lo <= x <= hi for x in f.ids5) and not r.prestep:
                r.prestep = at
                f.notes.append(f'BN5\'s virus dispatcher acts for its ids {lo:#x}-{hi:#x} before its state each frame ({at:08X}): '
                               f'the installer\'s port of it runs as its pre-step from the hook on BN6\'s dispatcher')
            else:
                f.decide('pre-step', f'BN5\'s virus dispatcher acts for its ids {lo:#x}-{hi:#x} before its state each frame ({at:08X}), '
                                     f'which no port of the installer\'s does (bn5_twins.PRESTEP)')
    if ai in SHARED:
        f.notes.insert(0, f'BN6 has this family by name ({SHARED[ai]}): BN5\'s records name BN6\'s (docs/MULTIROM.md, the finer calls); not moved')
    return r


def unique_imms(imms):
    seen, out = set(), []
    for i in imms:
        if i['at'] not in seen:
            seen.add(i['at'])
            out.append(i)
    return sorted(out, key=lambda i: i['at'])


def calls_of(r, d, blocks):
    """each BL out of its blocks: its twin, or the dispatch's spawner for its game's spawner"""
    f = r.f
    inside = lambda v: v in f.walks   # (a routine walked: its own, or copied; any other is BN6's, through its twin)
    outs = {}
    for w in f.walks.values():
        for site, t in w.bls.items():
            if not inside(t):
                outs.setdefault(t, []).append(site)
    r.calls = []
    for t, sites in sorted(outs.items(), key=lambda x: min(x[1])):
        if t in BN5['spawners']:
            r.calls.append(dict(bn5=t, bn6=None, spawner=XV_SPAWN[(BN5['spawners'][t], t)], source='the dispatch', cls=None, sites=sites))
            continue
        tw = f.calls.get(t) or d.pairs.twin(t)
        r.calls.append(dict(bn5=t, bn6=tw[0], spawner=None, source=tw[1], cls=tw[2], detail=tw[3], sites=sites))


def states_of(r, d, alloc):
    """its state table BN6's way: 0-7 BN6's generic (or its own where it overrides one of 0-5), its own from 8; where its
    paralysis (4) is a wrapper of its own round BN5's generic one, BN6's frozen (6) and bubbled (7) clones of it round
    BN6's generic ones"""
    f, out = r.f, []
    for k in range(8):
        if k < 6 and k < len(f.states5) and (f.states5[k] & ~1) != GENERIC5[k]:
            s = f.states5[k] & ~1
            kind, tw = d.kind_of(f, s)
            out.append(('BN6', tw[0] | 1) if kind == 'engine' else ('OWN', s | 1))
        elif k in (6, 7) and len(f.states5) > 4 and (f.states5[4] & ~1) != GENERIC5[4] and d.kind_of(f, f.states5[4])[0] != 'engine':
            c = clone_of(r, d, alloc, f.states5[4] & ~1, GENERIC6[k], k)
            out.append(('CLONE', f.states5[4] | 1, len(r.clones)) if c else ('BN6', GENERIC6[k] | 1))
        else:
            out.append(('BN6', GENERIC6[k] | 1))
    for s in f.states5[6:]:
        kind, tw = d.kind_of(f, s)
        out.append(('BN6', tw[0] | 1) if kind == 'engine' else ('OWN', s | 1))
    r.states = out


def dispatcher_cases(b5, at=0x08104550):
    """the NameID ranges BN5's virus dispatcher (the first kind's type 0's per-frame step for viruses) acts for before a
    virus's state: each cmp of [r5,#0x28] with a low bound, then a high one; (lo, hi, where)"""
    w = T.walk(b5, at)
    addrs, out = sorted(w.insns), []
    for n, a in enumerate(addrs):
        i = w.insns[a]
        nxt = [w.insns[x] for x in addrs[n + 1:n + 4]]
        if i.form == 'cmp' and len(nxt) >= 2 and nxt[0].form == 'bcc' and nxt[1].form == 'cmp' and nxt[1].f[0] == i.f[0] \
                and COND_LT.get(nxt[0].f[0]) == 'lo':
            # (a register the routine loads the virus's NameID, +0x28, into before: the dispatcher's own branches skip the
            # instructions between, so the order of addresses alone says too little)
            if any(w.insns[x].form == 'ldrh' and w.insns[x].f == (i.f[0], 5, 0x28) for x in addrs[:n]):
                out.append((i.f[1], nxt[1].f[1], a))
    return out


COND_LT = {11: 'lo', 3: 'lo'}   # blt, bcc: the check of a low bound

# BN5's families BN6 has by name: BN5's records name BN6's (docs/MULTIROM.md, the finer calls)
SHARED = {1: 'Mettaur', 8: 'Trumpy', 10: 'WindBox', 15: 'Catack', 19: 'Champy'}


def clone_of(r, d, alloc, start, to, k):
    """its paralysis wrapper (a routine of its code, code alone) copied for BN6's state k (6 frozen, 7 bubbled), its call
    of BN5's generic paralysis aimed at BN6's generic state k, a compare of CurAction with 4 (the paralysis it is in)
    made k: the clone's place after its blocks"""
    f = r.f
    w = T.walk(d.b5, start)
    end = max(a + i.size for a, i in w.insns.items())
    calls = [a for a, t in w.bls.items() if t == GENERIC5[4]]
    if w.lits or w.tables or w.indirect or min(w.insns) != start or sum(i.size for i in w.insns.values()) != end - start or len(calls) != 1:
        f.decide('status wrappers', f'its paralysis wrapper {start:08X} is no plain routine to copy (pools, tables or no one call of '
                                    f'BN5\'s paralysis): BN6\'s state {k} needs one of its own')
        return None
    imm = None
    addrs = sorted(w.insns)
    for n, a in enumerate(addrs):
        i = w.insns[a]
        if i.form == 'ldrb' and i.f[1:] == (5, 9):
            for b in addrs[n + 1:n + 4]:
                j = w.insns[b]
                if j.form == 'cmp' and j.f[0] == i.f[0] and j.f[1] == 4:
                    imm = dict(at=b, form='THUMB_CMP', rd=j.f[0], rn=0, was=4, now=k, why=f'CurAction 4 compared: this clone\'s state, {k}')
                    break
    at = rounded(alloc.code, CODE_STEP) + (start & 3)
    alloc.code = at + (end - start)
    r.clones.append(dict(start=start, end=end, at=at, call=GENERIC5[4], to=to, imm=imm, state=k))
    return r.clones[-1]


def name_ranges(r):
    """its look-ups of an enemy by a range of NameIDs, as BN6 numbers them: its own ids the family's, every id or every
    other id to 0x3FF; each bound BN6's where its pool holds it (a number), else the call aimed at a shim of its home that
    loads both of BN6's bounds (XvRange: BN6's ids past an immediate's 255)"""
    f = r.f
    r.ranges = []
    own = [x for x in r.names if x is not None]
    lo5, hi5 = (min(own), max(own)) if own else (None, None)
    for a, lo, hi in f.nameranges:
        if lo is None or hi is None:
            f.decide('name ids', f'{a:08X}: an enemy looked up by a NameID range made otherwise')
            continue
        if (lo, hi) == (lo5, hi5):
            want = (r.first_id, r.first_id + 5)
        elif lo <= 1 and hi >= 0x17E:
            want = (lo, hi if hi >= 0x7FFF else 0x3FF)
        elif lo <= 1 and lo5 is not None and hi == lo5 - 1:
            want = (lo, r.first_id - 1)
        elif hi >= 0x17E and lo5 is not None and lo == hi5 + 1:
            want = (r.first_id + 6, 0x3FF)
        else:
            f.decide('name ids', f'{a:08X}: an enemy looked up by its NameID from {lo:#x} to {hi:#x}: no rule for BN6\'s ids')
            continue
        w = next(w for w in f.walks.values() if a in w.insns)
        lits = {}
        for val, new, reg in ((lo, want[0], 1), (hi, want[1], 2)):
            _, chain = T.back_value(w, a, reg)
            lits[reg] = next((w.insns[x].lit for x, form, v in chain if form == 'ldr_lit'), None)
        if any(lits[reg] is None and val != new for val, new, reg in ((lo, want[0], 1), (hi, want[1], 2))):
            r.ranges.append(dict(at=a, lo=want[0], hi=want[1], why=f'NameIDs {lo:#x}-{hi:#x} in BN5, {want[0]:#x}-{want[1]:#x} in BN6'))
            continue
        for val, new, reg in ((lo, want[0], 1), (hi, want[1], 2)):
            if val != new:
                f.numbers[lits[reg]] = (val, new, f'an enemy id\'s bound: {val:#x} in BN5, {new:#x} in BN6')


def hooks_of(r, d):
    f = r.f
    h5 = {3 + k: f.hooks5[k] for k in range(6)}
    out = []
    for k6 in range(3, 11):
        h = h5[HOOKS6[k6]] & ~1
        if h == BN5['nullsub'] & ~1:
            out.append(('BN6', BN6['nullsub']))
            continue
        kind, tw = d.kind_of(f, h)
        out.append(('BN6', tw[0] | 1) if kind == 'engine' else ('OWN', h | 1))
    r.hooks = out


def held_of(r, b5, reg):
    """its viruses' held chips: BN5's lists, each chip as the chip registry has it in BN6's tables (BN6's own id for a chip
    both games name, else the registry's own id: its record, and its code once moved); a chip the registry lacks held out"""
    f = r.f
    table = b5.u32(BN5['held'])
    p = b5.u32(table + 4 * r.ai5)
    if p != f.s1 + 0x2C:
        f.notes.append(f'its held chips are at {p:08X}, not its records\' lists ({f.s1 + 0x2C:08X})')
    lists = [[b5.u16(p + 0x20 * v + 2 * k) for k in range(16)] for v in range(6)]
    chips = sorted(set(c & 0x1FF for l in lists for c in l if c != 0xFFFF))
    r.held_chips, r.held_out, r.held_how = None, [], {}
    if not chips:
        return
    out = []
    for l in lists:
        row = []
        for c in l:
            if c == 0xFFFF:
                break
            to = reg.get(c & 0x1FF)
            if to is None:
                r.held_out.append(c & 0x1FF)
                continue
            r.held_how[c & 0x1FF] = to
            row.append(to[0] | (c & ~0x1FF))
        out.append(row + [0xFFFF] * (16 - len(row)))
    r.held_chips = out
    for c in sorted(set(r.held_out)):
        f.decide('held chips', f'BN5\'s chip {c} is not in the chip registry: held out')
    pending = sorted(c for c, (_, how) in r.held_how.items() if how == 'PENDING')
    if pending:
        f.notes.append('its held chips ' + ', '.join(f'{c} (BN6 {r.held_how[c][0]})' for c in pending) + ' are the registry\'s, '
                       'their code pending: held and used by BN5\'s rule, their records the registry\'s (an inert family '
                       'until their code is moved)')


def chip_compares(r, reg):
    """its compares of a held chip's id with a number (Deriver.chip_compares): the immediate BN6's id where it fits, else a
    call to a shim of its home that compares with BN6's id and branches as the instruction after did (XvCmp); and the chip
    ids its code gives its object by hand (Deriver.chip_store), BN6's in the immediate"""
    f = r.f
    r.cmps = []
    for at, rd, c5 in sorted(set(f.chip_stores)):
        # (a chip id its code gives its object by hand: BN6's id in the movs where it fits)
        to = reg.get(c5)
        if to is None:
            f.decide('chip', f'{at:08X}: BN5\'s chip {c5} given its object, which the registry lacks')
        elif to[0] > 0xFF:
            f.decide('chip', f'{at:08X}: BN5\'s chip {c5} given its object, BN6\'s {to[0]} past an immediate\'s 255')
        elif to[0] != c5:
            r.more_imms.append(dict(at=at, form='THUMB_MOVS', rd=rd, rn=0, was=c5, now=to[0],
                                    why=f'a chip given its object (+0x2A): BN5\'s chip {c5}, BN6\'s {to[0]} (the chip registry)'))
    for at, rd, c5, entry in sorted(set(f.chip_cmps)):
        to = reg.get(c5)
        if to is None:
            f.decide('chip compare', f'{at:08X}: a held chip compared with BN5\'s {c5}, which the registry lacks')
            continue
        c6 = to[0]
        if c6 == c5:
            continue
        if c6 <= 0xFF:
            r.more_imms.append(dict(at=at, form='THUMB_CMP', rd=rd, rn=0, was=c5, now=c6,
                                    why=f'a held chip compared: BN5\'s chip {c5}, BN6\'s {c6} (the chip registry)'))
            continue
        w = f.walks[entry]
        nxt = w.insns.get(at + 2)
        first = w.insns.get(entry)
        if not nxt or nxt.form != 'bcc' or nxt.f[0] not in (0, 1) or not first or first.form != 'push' or not first.f[1]:
            f.decide('chip compare', f'{at:08X}: BN6\'s id {c6} is past an immediate and the compare is not followed by beq or '
                                     f'bne in a routine that keeps lr on the stack')
            continue
        r.cmps.append(dict(at=at, reg=rd, was=c5, now=c6, why=f'a held chip compared: BN5\'s chip {c5}, BN6\'s {c6} (the chip '
                                                                f'registry), past an immediate\'s 255'))


def art_of(r, b5, b6, alloc, s1):
    """what it draws: its battle sprite (list 1), the sprites its code loads by number, its effects' sprites, and the ids its
    code reads from a table of its own (bn5_twins.ID_TABLES: byte fix-ups); each sprite copied at a number of BN6's lists,
    or BN6's own where BN6 has the same bytes; its effects new ids past BN6's, or BN6's of the same sprite"""
    f = r.f
    lists5, lists6 = 0x0803272C, BN6['sprites']
    six = {}
    r.sprites, r.effects, r.bytes, r.effect_ids = [], [], [], {}
    art = rounded(alloc.art, ART_STEP)
    table_sites = {site for t in bn5_twins.ID_TABLES for site in t['sites']}

    def bn6_twin(lst, idx):
        p, b = sprite(b5, lists5, lst, idx)
        if not b:
            return None, 0
        same = alloc.bn6_sprites(b6, lst)
        # (the same bytes: a sprite of BN6's drawing what BN5's does at other places in its data, as BN6's versions of the
        # engine's puffs and sparks would, is no twin, since the differential reads a sprite's state by those places)
        return same['bytes'].get(hashlib.sha1(b).hexdigest()), len(b)

    def place(lst, idx, room_kind):
        nonlocal art
        p, b = sprite(b5, lists5, lst, idx)
        if not b:
            f.decide('sprite', f'its sprite {lst}:{idx:#x} could not be read')
            return None
        # (a sprite of a list BN6 has not grown, its third: listed among the attacks' and effects', list 4, which the
        # effect's row names in BN6, or its load's list in BN6's numbers)
        to_list = lst if lst in LIST_NUMBERS else BN6_EFFECT_LIST
        to, fresh = alloc.number(to_list, (lst, idx))
        if fresh:
            room = rounded(len(b), ROOM[room_kind])
            r.sprites.append(dict(list=lst, index=idx, to_list=to_list, to_index=to, at=art, room=room, lz=bool(p & 0x80000000)))
            art += room
        elif room_kind == 'battle':
            f.decide('battle sprite', f'its battle sprite {lst}:{idx:#x} is a family\'s before it (listed at {to:#x}): one record for both wanted')
        else:
            f.notes.append(f'its sprite {lst}:{idx:#x} is listed by a family before it ({lst}:{to:#x}): installed with it')
        return to

    def sprite_to(lst, v):
        """BN6's number for BN5's sprite (list, number) its code loads, in its list's grown copy (list 4's, where BN6 has
        not grown its list: placed_list)"""
        same, _ = bn6_twin(lst, v)
        if same is not None:
            return same
        mine = next((x['to_index'] for x in r.sprites if (x['list'], x['index']) == (lst, v)), None)
        return mine if mine is not None else place(lst, v, 'load')

    def placed_list(lst, v):
        """the list BN6 lists BN5's sprite (list, number) in: its own, or list 4 where BN6 has not grown its own and has no
        twin of the sprite in it"""
        if lst in LIST_NUMBERS or bn6_twin(lst, v)[0] is not None:
            return lst
        return BN6_EFFECT_LIST

    def effect_to(e):
        """BN6's effect id for BN5's effect e its code spawns"""
        e6 = effect_twin(b5, b6, e, bn6_twin)
        if e6 is None:
            row = b5.bytes(BN5['effects'] + 4 * e, 4)
            lst, idx = row[0] // 4, row[1]
            e6, fresh = alloc.number('effect', e)
            if not fresh:
                f.notes.append(f'its effect {e:#x} is listed by a family before it (BN6\'s {e6:#x}): installed with it')
            if fresh:
                r.effects.append((e, e6))
                if (lst, idx) not in [(s['list'], s['index']) for s in r.sprites] and \
                        (lst if lst in LIST_NUMBERS else BN6_EFFECT_LIST, (lst, idx)) not in alloc.memo:
                    place(lst, idx, 'effect')
        r.effect_ids[e] = e6
        return e6

    tw, n = bn6_twin(s1[0] // 4, s1[1])
    if tw is not None:
        f.decide('battle sprite', f'BN6 has its battle sprite as list 1 {tw:#x}: its struct1 could name BN6\'s')
    place(s1[0] // 4, s1[1], 'battle')
    for (l, v), sites in sorted(f.loads.items(), key=lambda x: str(x[0])):
        if l is None or v is None:
            if not all(site in table_sites for site, _ in sites):
                f.decide('sprite', f'a sprite loaded by a number made otherwise at {", ".join(f"{s:08X}" for s, _ in sites)}')
            continue
        lst = l // 4
        to, to_list = sprite_to(lst, v), placed_list(lst, v)
        if to is not None and to != v:
            for s, chain in sites:
                imm_for(r, chain, 2, v, to, f'its sprite: list {lst} {v:#x} in BN5, {to:#x} in BN6')
        if to_list != lst:
            # (its load's list too: BN5's list offset in r1, BN6's list 4's)
            for s, _ in sites:
                w = next(w for w in f.walks.values() if s in w.insns)
                _, chain1 = T.back_value(w, s, 1)
                imm_for(r, chain1, 1, l, 4 * to_list, f'its sprite\'s list: {lst} in BN5, {to_list} in BN6 (its own not grown)')
    for e, sites in sorted(f.effects.items(), key=lambda x: str(x[0])):
        if e is None:
            if not all(site[0] in table_sites for site in sites):
                f.decide('effect', f'an effect id made otherwise at {", ".join(f"{site[0]:08X}" for site in sites)}')
            continue
        e6 = effect_to(e)
        if e6 != e:
            # (an effect id in r4 as the effect's spawner takes it, or stored for the shared teleport: its register with it)
            for site in sites:
                imm_for(r, site[1], site[2] if len(site) > 2 else 4, e, e6, f'its effect: {e:#x} in BN5, {e6:#x} in BN6')
    # (the ids its code reads from a table of its own)
    in_block = lambda a: any(lo <= a < hi for lo, hi, _, _ in r.blocks)
    raw = lambda a: any(lo <= a < hi and k == 'RAW' for lo, hi, k in r.data)
    r.tables, lists_moved = [], {}
    for t in bn5_twins.ID_TABLES:
        if not in_block(t['at']):
            continue
        if not all(in_block(x) for x in t['sites']) or not in_block(t['at'] + t['size'] - 1):
            f.decide('table', f'{t["at"]:08X}: an id table of its code not copied whole, or read from code not its own')
            continue
        r.tables.append(t['at'])
        for k in t['live']:
            base = t['at'] + t['step'] * k
            fields = dict((kind, off) for off, kind in t['fields'])
            if 'effect' in fields:
                at = base + fields['effect']
                e5 = b5.u8(at)
                e6 = effect_to(e5)
                if e6 != e5:
                    r.bytes.append(dict(at=at, was=e5, now=e6, why=f'an effect id of its table {t["at"]:08X}: {e5:#x} in BN5, {e6:#x} in BN6'))
            if 'sprite' in fields:
                at = base + fields['sprite']
                lst, v = (t['list'] if 'list' in t else b5.u8(base + fields['list']) // 4), b5.u8(at)
                to, to_list = sprite_to(lst, v), placed_list(lst, v)
                if to is not None and to != v:
                    r.bytes.append(dict(at=at, was=v, now=to, why=f'a sprite of its table {t["at"]:08X}: list {lst} {v:#x} in BN5, {to:#x} in BN6'))
                if to_list != lst and 'list' in t:
                    # (the list its code loads the table's sprites from, an immediate: BN6's list 4 where its own is not grown)
                    lists_moved.setdefault((t['at'], lst), set()).add(to_list)
                elif to_list != lst:
                    # (a sprite of a list BN6 has not grown, listed in list 4: the entry's list byte BN6's too)
                    r.bytes.append(dict(at=base + fields['list'], was=lst * 4, now=to_list * 4,
                                        why=f'a sprite of its table {t["at"]:08X}: its list {lst} in BN5, {to_list} in BN6 (its own not grown)'))
    for (at, lst), to_lists in sorted(lists_moved.items()):
        t = next(x for x in bn5_twins.ID_TABLES if x['at'] == at)
        if len(to_lists) != 1:
            f.decide('table', f'{at:08X}: its sprites of list {lst} placed in more than one list ({sorted(to_lists)})')
            continue
        for site in t['sites']:
            w = next((w for w in f.walks.values() if site in w.insns), None)
            _, chain1 = T.back_value(w, site, 1) if w else (None, [])
            imm_for(r, chain1, 1, 4 * lst, 4 * min(to_lists), f'its table {at:08X}\'s sprites\' list: {lst} in BN5, {min(to_lists)} in BN6 (its own not grown)')
    for y in r.bytes:
        if not raw(y['at']):
            f.decide('table', f'{y["at"]:08X}: an id of its table that is not raw data in its blocks')
    alloc.art = art


def effect_twin(b5, b6, e5, bn6_twin):
    """BN6's effect of the same sprite (by its bytes), animation and palette, else None"""
    if e5 >= BN5['neffects']:
        return None
    r5 = b5.bytes(BN5['effects'] + 4 * e5, 4)
    tw, _ = bn6_twin(r5[0] // 4, r5[1])
    if tw is None:
        return None
    for e6 in range(BN6['neffects']):
        q = b6.bytes(BN6['effects'] + 4 * e6, 4)
        if q[0] == r5[0] and q[1] == tw and q[2] == r5[2] and q[3] == r5[3]:
            return e6
    return None


def imm_for(r, chain, reg, was, now, why):
    """the immediate a value's chain (back_value's) sets its low byte by, given BN6's number"""
    for a, form, v in chain:
        if form in ('movs', 'adds8') and v == was:
            r.more_imms.append(dict(at=a, form='THUMB_MOVS' if form == 'movs' else 'THUMB_ADDS', rd=reg, rn=0, was=was, now=now, why=why))
            return
    r.f.decide('immediate', f'{why}: made by {chain}, not one instruction')


def types_of(r, alloc=None):
    """the object types its code spawns: BN6's twin, moved through the dispatch, or moved by a family before it (its copy
    run for this one too); each one it moves told to `alloc` for the families after it (a family's: the Navis' keep none)"""
    r.types = []
    for (kind, t5), v in sorted(r.f.types.items()):
        if v['bn6'] is not None:
            r.types.append((kind, t5, v['bn6'], v['why']))
        elif v.get('shared'):
            r.types.append((kind, t5, 'SHARED', v['why']))
        elif v['routine'] is None:
            r.f.decide('spawn', f'the {kind_name(kind)} kind\'s type {t5:#x}: {v["why"]}')
        else:
            r.types.append((kind, t5, None, v['why']))
            if alloc is not None:
                alloc.moved_types.setdefault((kind, t5), r.name)


def sounds_of(r, b5, b6, alloc):
    """the sounds its code plays, each by its song as it sounds (song_sig): BN6's of the same number, else BN6's twin of
    another number, else BN5's copied into one of BN6's empty song slots (XvSong: the families' own, from the top), the
    immediate that names it BN6's"""
    r.sounds, r.songs = [], []
    for v, sites in sorted(r.f.sounds.items(), key=lambda x: str(x[0])):
        if v is None:
            r.f.decide('sound', f'a sound made otherwise at {", ".join(f"{s:08X}" for s, _ in sites)}')
            continue
        sig = song_sig(b5, BN5['songs'], v)
        if sig == song_sig(b6, BN6['songs'], v) and v not in SOUND_SLOTS:
            r.sounds.append((v, v, 'the same'))
            continue
        twin = alloc.song_twins(b6).get(sig)
        if twin is not None and v not in SOUND_SLOTS:
            to, how = twin, f'BN6\'s {twin:#x}, the same song'
        else:
            # (a sound the shared allocations give a slot: copied there, BN5's own, even where BN6's of its number is the
            # same but for its player, as Skarab's 0xB7)
            to, fresh = alloc.song(v)
            how = f'BN5\'s copied into BN6\'s empty slot {to:#x}' if to else 'no slot'
            if not to:
                r.f.decide('sound', f'its sound {v:#x} is no song of BN6\'s and has no slot of the shared allocations (SOUND_SLOTS): ask')
                continue
            if fresh:
                room = rounded(song_bytes(b5, v), SONG_STEP)
                alloc.art = rounded(alloc.art, 4)
                r.songs.append(dict(song=v, slot=to, at=alloc.art, room=room))
                alloc.art += room
        r.sounds.append((v, to, how))
        for site, chain in sites:
            imm_for(r, chain, 0, v, to, f'its sound {v:#x}: {how}')


def song_bytes(rom, v):
    """the bytes a song takes copied as xsong.c lays it out (its sequence, voice group and samples): an upper bound"""
    hp = rom.u32(BN5['songs'] + 8 * v)
    nt = rom.u8(hp)
    starts = [rom.u32(hp + 8 + 4 * k) for k in range(nt)]
    n = hp + 8 + 4 * nt - min(starts) + 4
    vg = rom.u32(hp + 4)
    programs = {0}
    for t in starts:
        tr = song_track(rom, t)
        programs |= {tr[k + 1] for k in range(len(tr) - 1) if tr[k] == 0xBD}
    n += 12 * (max(programs) + 1) + 4

    def voice(e, sub=False):
        typ, p = e[0], int.from_bytes(e[4:8], 'little')
        if typ & 0xC0:
            if sub:
                return 0
            if typ & 0x80:
                return 128 * 12 + sum(voice(rom.bytes(p + 12 * k, 12), True) for k in range(128))
            q = int.from_bytes(e[8:12], 'little')
            m = max(rom.bytes(q, 128)) + 1
            return 128 + 12 * m + sum(voice(rom.bytes(p + 12 * k, 12), True) for k in range(m)) + 8
        if typ & 7 == 0 and rom.has(p, 16):
            return 16 + rom.u32(p + 12) + 1 + 4
        return 16 + 4 if typ & 7 == 3 else 0
    for prog in programs:
        n += voice(rom.bytes(vg + 12 * prog, 12))
    return n


def song_track(rom, a):
    """a track's bytes to its end, its jumps' and calls' pointers (0xB2, 0xB3) as distances from its start"""
    out, i = bytearray(), a
    for _ in range(6000):
        if not rom.has(i):
            break
        c = rom.u8(i)
        if c in (0xB2, 0xB3) and rom.has(i + 1, 4):
            out += bytes([c]) + ((rom.u32(i + 1) - a) & 0xFFFFFFFF).to_bytes(4, 'little')
            i += 5
            if c == 0xB2:
                break
            continue
        out.append(c)
        i += 1
        if c == 0xB1:
            break
    return bytes(out)


def song_sig(rom, table, i, player=False):
    """a song as it sounds: its header's numbers, its tracks (their pointers relative) and the voices they select (a
    sample's bytes by their hash); with `player`, the music player its entry names too (the Navis' sounds: one BN6 plays
    on another player is copied, BN5's own)"""
    hp = rom.u32(table + 8 * i)
    if not rom.has(hp, 8):
        return None
    tracks, blocks, prio, rev = rom.bytes(hp, 4)
    if tracks > 16:
        return None
    vg = rom.u32(hp + 4)
    tr = [song_track(rom, rom.u32(hp + 8 + 4 * k)) for k in range(tracks)]
    voices = set()
    for t in tr:
        for k in range(len(t) - 1):
            if t[k] == 0xBD:
                voices.add(t[k + 1])
    vs = []
    for v in sorted(voices):
        if not rom.has(vg + 12 * v, 12):
            continue
        e = rom.bytes(vg + 12 * v, 12)
        smp = ''
        if e[0] in (0, 8, 0x10, 0x18):
            sp = int.from_bytes(e[4:8], 'little')
            if rom.has(sp, 16):
                n = rom.u32(sp + 12)
                if n < 0x100000 and rom.has(sp, 16 + n):
                    smp = hashlib.sha1(rom.bytes(sp, 16 + n)).hexdigest()[:12]
        vs.append(e[:4].hex() + e[8:].hex() + smp)
    # (the music player it plays on is each song table's own: tests/xdiff/legit.json's sound)
    return hashlib.sha1(repr(((rom.u16(table + 8 * i + 4),) if player else ()) + (tracks, prio, rev, tr, vs)).encode()).hexdigest()


def conditions_of(r, pairs):
    """each call's pair as its bodies compare: SAME and LAYOUT taken, DIFF where its audit takes it and its conditions
    hold for this family's code"""
    f = r.f
    for c in r.calls:
        if c['spawner'] or c['bn6'] is None:
            continue
        if c['cls'] != 'DIFF':
            continue
        audit = pairs.audit(c['bn5'], c['bn6'])
        if audit is None:
            f.decide('pair', f'{c["bn5"]:08X} -> {c["bn6"]:08X} ({c["source"]}): the bodies differ ({c["detail"]}) and no audit says it serves')
            continue
        verdict, why, conds = audit
        c['audit'] = why
        if verdict != 'ok':
            f.decide('pair', f'{c["bn5"]:08X} -> {c["bn6"]:08X}: its audit says no ({why})')
        for cond in conds:
            bad = condition_fails(r, cond)
            if bad:
                f.decide('pair', f'{c["bn5"]:08X} -> {c["bn6"]:08X} serves only where {cond}: {bad}')


def condition_fails(r, cond):
    f = r.f
    if cond == 'no_flip' and f.flip:
        return 'it writes its object\'s +0x17 at ' + ', '.join(f'{a:08X}' for a in f.flip)
    if cond == 'flags_free':
        bad = [(a, v) for a, v in f.flags_set if v is None or v & 0x80010000]
        if bad:
            return 'it sets flags ' + ', '.join(f'{a:08X}: {v if v is None else hex(v)}' for a, v in bad)
    if cond == 'no_attack6' and f.attack6:
        return 'it calls setAttack6 at ' + ', '.join(f'{a:08X}' for a in f.attack6)
    if cond == 'panel_types':
        bad = [(a, v) for a, v in f.panel_types if v not in bn5_twins.SAME_PANEL_TYPES]
        if bad:
            return 'it sets panel types ' + ', '.join(f'{a:08X}: {v}' for a, v in bad)
    if cond == 'held_bn6' and r.held_out:
        return 'its held chips ' + ', '.join(str(c) for c in sorted(set(r.held_out))) + ' have no BN6 ids (held out)'
    return None


# ---- the C file ----

def c_name(name, ai):
    n = re.sub(r'[^a-z0-9]', '', name.lower())
    return n or f'ai{ai}'


def ref_text(ref):
    k, a = ref[0], ref[1]
    if k == 'CLONE':
        return f'CLONE({ref[2]}, 0x{a:08X})'
    if k == 'BN6' and a == BN6['nullsub']:
        return 'BN6(BN6_FAMILY_NULLSUB)'
    return f'{k}(0x{a:08X})'


def addr_text(a):
    """an address the C file writes: one of the moved objects' copies by its place (XN_OBJECTS, tools/bn5_objects.py), else
    the number"""
    if OB.OBJECTS_AT <= (a & ~1) < OB.OBJECTS_END:
        return f'XN_OBJECTS + 0x{a - OB.OBJECTS_AT:X}'
    return f'0x{a:08X}'


def c_recipe(r):
    n = c_name(r.label, r.ai5)
    L = [f'/* ---- BN5\'s {r.label} (AI {r.ai5} there, its enemy ids {", ".join(f"{x:#x}" for x in r.names)}), as AI {r.ai} with ids '
         f'{r.first_id:#x}-{r.first_id + 5:#x} ---- */', '']
    L.append(f'static const XvBlock {n}_blocks[] = {{')
    for lo, hi, at, crc in r.blocks:
        L.append(f'\t{{ 0x{lo:08X}, 0x{hi:08X}, XV_CODE + 0x{at - XV_CODE:X}, 0x{crc:08X}u }},')
    L += ['};', f'static const XvData {n}_data[] = {{']
    for lo, hi, k in r.data:
        L.append(f'\t{{ 0x{lo:08X}, 0x{hi:08X}, XV_{k} }},')
    L += ['};', f'static const XvPair {n}_calls[] = {{']
    for c in r.calls:
        if c['spawner']:
            L.append(f'\t{{ {BN5_SPAWN_NAME[c["bn5"]]}, {c["spawner"]} }},   /* the dispatch\'s spawner */')
        else:
            how = c['cls'] + (' (audited)' if c.get('audit') else '')
            L.append(f'\t{{ 0x{c["bn5"]:08X}, {addr_text(c["bn6"] or 0)} }},   /* {c["source"]}, {how} */')
    L.append('};')
    if r.words:
        L.append(f'static const XvPair {n}_words[] = {{')
        for v, to in r.words:
            L.append(f'\t{{ 0x{v:08X}, {addr_text(to)} }},')
        L.append('};')
    if r.imms:
        L.append(f'static const XvImm {n}_imms[] = {{')
        for i in r.imms:
            L.append(f'\t{{ 0x{i["at"]:08X}, {i["form"]}, {i["rd"]}, {i["rn"]}, 0x{i["was"]:X}, 0x{i["now"]:X} }},   /* {i["why"]} */')
        L.append('};')
    if r.numbers:
        L.append(f'static const XvNumber {n}_numbers[] = {{')
        for lit, was, now in r.numbers:
            L.append(f'\t{{ 0x{lit:08X}, 0x{was:08X}, 0x{now:08X} }},')
        L.append('};')
    if r.bytes:
        L.append(f'static const XvByte {n}_bytes[] = {{')
        for y in r.bytes:
            L.append(f'\t{{ 0x{y["at"]:08X}, 0x{y["was"]:02X}, 0x{y["now"]:02X} }},   /* {y["why"]} */')
        L.append('};')
    if r.ranges:
        L.append(f'static const XvRange {n}_ranges[] = {{')
        for x in r.ranges:
            L.append(f'\t{{ 0x{x["at"]:08X}, 0x{x["lo"]:03X}, 0x{x["hi"]:03X} }},   /* {x["why"]} */')
        L.append('};')
    if r.cmps:
        L.append(f'static const XvCmp {n}_cmps[] = {{')
        for x in r.cmps:
            L.append(f'\t{{ 0x{x["at"]:08X}, {x["reg"]}, 0x{x["was"]:X}, 0x{x["now"]:X} }},   /* {x["why"]} */')
        L.append('};')
    if r.songs:
        L.append(f'static const XvSong {n}_songs[] = {{')
        for x in r.songs:
            L.append(f'\t{{ 0x{x["song"]:X}, 0x{x["slot"]:02X}, XV_ART + 0x{x["at"] - XV_ART:X}, 0x{x["room"]:X} }},   /* its sound {x["song"]:#x}, '
                     f'BN5\'s song copied */')
        L.append('};')
    if r.clones:
        L.append(f'static const XvClone {n}_clones[] = {{')
        for c in r.clones:
            i = c['imm']
            imm = f'{{ 0x{i["at"]:08X}, {i["form"]}, {i["rd"]}, {i["rn"]}, 0x{i["was"]:X}, 0x{i["now"]:X} }}' if i else '{ 0 }'
            L.append(f'\t{{ 0x{c["start"]:08X}, 0x{c["end"]:08X}, XV_CODE + 0x{c["at"] - XV_CODE:X}, 0x{c["call"]:08X}, 0x{c["to"]:08X}, {imm} }},'
                     f'   /* its paralysis wrapper for BN6\'s state {c["state"]} */')
        L.append('};')
    L.append(f'static const XvRef {n}_states[] = {{')
    L.append('\t' + ', '.join(ref_text(x) for x in r.states) + ',')
    L.append('};')
    L.append(f'static const XvSprite {n}_sprites[] = {{')
    for x in r.sprites:
        L.append(f'\t{{ {x["list"]}, 0x{x["index"]:02X}, {x["to_list"]}, 0x{x["to_index"]:02X}, XV_ART + 0x{x["at"] - XV_ART:X}, 0x{x["room"]:X} }},'
                 + ('   /* (LZ77 in BN5) */' if x['lz'] else ''))
    L.append('};')
    if r.effects:
        L.append(f'static const XvId {n}_effects[] = {{ ' + ', '.join(f'{{ 0x{a:02X}, 0x{b:02X} }}' for a, b in r.effects) + ' };')
    if r.types:
        L.append(f'static const XvType {n}_types[] = {{ ' + ', '.join(
            f'{{ {k}, 0x{t5:02X}, {"XV_TYPE_MOVED" if t6 is None else "XV_TYPE_SHARED" if t6 == "SHARED" else f"0x{t6:02X}"} }}'
            for k, t5, t6, _ in r.types) + ' };')
    if r.held_chips:
        L.append(f'static const uint16_t {n}_held[XV_VERSIONS * 16] = {{')
        for row in r.held_chips:
            L.append('\t' + ', '.join(f'0x{c:04X}' for c in row) + ',')
        L.append('};')
    L += ['', f'static const XVirusFamily bn5_{n} = {{', f'\t.name = "{r.label} (BN5\'s)",', '\t.game = XROM_BN5_COLONEL_US,',
          f'\t.src_ai = {r.ai5},', f'\t.ai = {r.ai},', f'\t.first_id = XV_FIRST_ID + 0x{r.first_id - XV_FIRST_ID:X},',
          f'\t.home = XV_HOMES + 0x{r.home - XV_HOMES:X},']
    count = lambda a: f'(int)(sizeof {n}_{a} / sizeof *{n}_{a})'
    for field, have in (('blocks', True), ('data', True), ('calls', True), ('words', r.words), ('imms', r.imms), ('numbers', r.numbers),
                        ('bytes', r.bytes), ('ranges', r.ranges), ('cmps', r.cmps), ('songs', r.songs), ('clones', r.clones), ('states', True)):
        if have:
            L.append(f'\t.{field} = {n}_{field}, .n{field} = {count(field)},')
    L.append('\t.hooks = { ' + ', '.join(ref_text(x) for x in r.hooks) + ' },')
    L.append(f'\t.struct1 = 0x{r.s1:08X}, .struct2 = 0x{r.s2:08X},')
    L.append('\t.names = { ' + ', '.join(f'0x{x:02X}' for x in r.names) + ' },')
    if r.held_chips:
        L.append(f'\t.held_chips = {n}_held,')
    if r.prestep:
        L.append(f'\t.prestep = 0x{r.prestep:08X},   /* BN5\'s dispatcher\'s case for its ids, ported (bn5_twins.PRESTEP) */')
    L.append('\t.ice = 0,')
    L.append(f'\t.traits = {{ .laid = true, .mode = 0x{r.traits["mode"]:02X}, .element = 0, .weakness = 0, .shadow = {"true" if r.traits["shadow"] else "false"} }},')
    L.append(f'\t.sprites = {n}_sprites, .nsprites = {count("sprites")},')
    if r.effects:
        L.append(f'\t.effects = {n}_effects, .neffects = {count("effects")},')
    if r.types:
        L.append(f'\t.types = {n}_types, .ntypes = {count("types")},')
    L.append('};')
    return '\n'.join(L), f'&bn5_{n}'


HEADER = '''/* xvirus_bn5.c: BN5's virus families' recipes, written by tools/xrecipe.py from the two ROMs (python3 tools/xrecipe.py;
 * --check says whether this is what it writes). Do not edit: change the generator (or tools/bn5_twins.py, its pairs and
 * their audits) and write it again. Numbers alone (docs/MULTIROM.md, Transplanted families): addresses in BN5 Team
 * Colonel (USA) and BN6 Cybeast Gregar (USA), counts and the CRC32s of the ranges the install copies from the player's
 * ROM; nothing of either game's. The families here are the generator's, whole (no decision open), in install order
 * (each its AI index, ids, home, code and art places by its slot): the ones installed wherever BN5's ROM is read
 * (src/emu/xvirus.c), and tests/test_core.c holds the generated CanGard and Handi to the recipes made by hand
 * (src/emu/xvirus_families.c), field for field. */
#include "xvirus_bn5.h"

#include "bn5.h"
#include "bn6.h"
#include "rom.h"
#include "xnavi_recipe.h"

#define OWN(a) { (a), true, 0 }
#define BN6(a) { (a), false, 0 }
#define CLONE(k, a) { (a), true, (k) }
'''


def c_file(recipes):
    parts, refs = [HEADER], []
    for r in recipes:
        text_, ref = c_recipe(r)
        parts.append(text_)
        refs.append(ref)
    parts.append('const XVirusFamily *const xvirus_bn5[] = { ' + ', '.join(refs) + ' };\n'
                 'const int xvirus_bn5_count = (int)(sizeof xvirus_bn5 / sizeof *xvirus_bn5);\n')
    return '\n'.join(parts)


# ---- the report ----

def report(recipes, path_md, path_json):
    lines = ['# BN5\'s virus families as tools/xrecipe.py derives them', '',
             'Each in install order after the families installed (its slot\'s numbers); what it derives, then what still needs '
             'a decision.', '']
    js = []
    for r in recipes:
        f = r.f
        cls = {}
        for c in r.calls:
            k = 'dispatch' if c['spawner'] else 'none' if c['bn6'] is None else c['cls'] + ('+audit' if c.get('audit') else '')
            cls[k] = cls.get(k, 0) + 1
        moved = [f'{kind_name(k)} {t5:#x}' + ('' if t6 is None else ' (moved by a family before it)' if t6 == 'SHARED' else f' as BN6\'s {t6:#x}')
                 for k, t5, t6, _ in r.types]
        lines.append(f'## AI {r.ai5}: {r.name or "(unnamed)"} (slot {r.slot}: AI {r.ai}, ids {r.first_id:#x}-{r.first_id + 5:#x})')
        lines.append('')
        lines.append(f'- code: {len(r.blocks)} blocks, {sum(b - a for a, b, _, _ in r.blocks)} bytes; calls {len(r.calls)} '
                     f'({", ".join(f"{v} {k}" for k, v in sorted(cls.items()))}); immediates {len(r.imms)}; words {len(r.words)}; '
                     f'numbers kept {len(r.numbers)}')
        lines.append(f'- states {len(r.states)} ({sum(1 for x in r.states if x[0] == "OWN")} its own, '
                     f'{sum(1 for x in r.states if x[0] == "CLONE")} clones of its paralysis wrapper); hooks: '
                     + ', '.join('-' if (k == 'BN6' and a == BN6['nullsub']) else f'{k} {a:08X}' for k, a in r.hooks))
        lines.append(f'- draws: {len(r.sprites)} sprites (' + ', '.join(f'{s["list"]}:{s["index"]:#x} as {s["to_list"]}:{s["to_index"]:#x}'
                                                                       for s in r.sprites) + f'); effects {r.effects or "-"}; '
                     f'objects spawned: {", ".join(moved) or "-"}; sounds ' + (', '.join(f'{v:#x}' + ('' if to == v else f' as {to:#x} ({how})')
                                                                                for v, to, how in r.sounds) or '-'))
        held_out = sorted(set(r.held_out))
        lines.append(f'- draws random numbers: {len(f.rng)} calls; held chips: '
                     + ('-' if not r.held_chips else f'{sum(1 for l in r.held_chips for c in l if c != 0xFFFF)} as BN6\'s'
                        + (f', held out: ' + ', '.join(str(c) for c in held_out) if held_out else '')
                        + (f' (the registry\'s: ' + ', '.join(f'{c} as {to} {how.lower()}' for c, (to, how) in sorted(r.held_how.items()))
                           + ')' if r.held_how else '')))
        if f.notes:
            lines.append('- notes: ' + '; '.join(dict.fromkeys(f.notes)))
        lines.append('- decisions: ' + ('none' if not f.decisions else ''))
        for k, d in f.decisions:
            lines.append(f'  - {k}: {d}')
        lines.append('')
        js.append(dict(ai=r.ai5, name=r.name, slot=r.slot, blocks=[[hex(a), hex(b)] for a, b, _, _ in r.blocks],
                       calls=[dict(bn5=hex(c['bn5']), bn6=hex(c['bn6']) if c['bn6'] else c['spawner'], source=c['source'], cls=c['cls'],
                                   audit=c.get('audit')) for c in r.calls],
                       decisions=[list(d) for d in f.decisions], notes=list(dict.fromkeys(f.notes))))
    os.makedirs(os.path.dirname(path_md), exist_ok=True)
    with open(path_md, 'w', encoding='utf-8') as o:
        o.write('\n'.join(lines) + '\n')
    with open(path_json, 'w', encoding='utf-8') as o:
        json.dump(js, o, indent=1)


def xdiff_json(recipes):
    """the families' numbers as the differential reads them (tests/xdiff/families.json): each family's ids, AI index, effects,
    sounds and moved types in both games, so a spec's maps are its families' own"""
    fams = []
    for r in recipes:
        fams.append(dict(
            name=r.label, ai5=r.ai5, ai6=r.ai, ids5=[f'{x:#x}' for x in r.names], ids6=[f'{r.first_id + v:#x}' for v in range(6)],
            effects={f'{e5:#x}': f'{e6:#x}' for e5, e6 in sorted(r.effect_ids.items())},
            # (a sound BN6 has under another number; one copied into its slot is told by its own number in BN6's trace too,
            # src/audio/xsong.c's xsong_copy_of)
            sounds={f'{v:#x}': f'{to:#x}' for v, to, _ in r.sounds if to != v and v not in SOUND_SLOTS},
            code_twins={f'{v:#x}': f'{to:#x}' for v, to in r.words if v & 1},
            effect_bytes=[f'{x:#x}' for x in sorted(r.f.effect_bytes)],
            t3_actions=[f'{t:#x}' for t in r.t3_states],
            moved_types=[f'{k}:{t5:#x}' for k, t5, t6, _ in r.types if t6 is None or t6 == 'SHARED'],
            struct1=f'{r.f.s1:#x}', struct2=f'{r.f.s2:#x}', states=len(r.states),
            art={f'{x["at"]:#x}': [x['list'], f'{x["index"]:#x}'] for x in r.sprites}))
    return json.dumps(dict(about='The moved families\' numbers in both games as tools/xdiff.py reads a spec\'s, written by '
                                 'tools/xrecipe.py with src/emu/xvirus_bn5.c (numbers only: never by hand): each family\'s '
                                 'enemy ids, AI index, effects its code spawns, sounds it plays copied or by another number, '
                                 'the engine\'s routines its code names as callbacks and their twins (each pair\'s bodies '
                                 'compared), the object types it moved, BN5\'s and BN6\'s, where its struct1 and struct2 are in '
                                 'BN5 (its battle sprite\'s arm points and its versions\' HP, tools/xfamily.py), how many states its table holds in BN6 and the '
                                 'sprites it copied (BN6\'s list word: BN5\'s list and number)', families=fams), indent=1) + '\n'


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--check', action='store_true', help='write nothing: fail where the C file is not what this writes')
    ap.add_argument('--report', action='store_true', help='every BN5 family: .build/xrecipe/report.md and .json')
    ap.add_argument('--ai', type=int, nargs='*', help='the report\'s families (BN5 AI indexes), FAMILIES first')
    ap.add_argument('--no-search', action='store_true', help='pair by the project\'s pairs and the symbol map alone')
    a = ap.parse_args()
    b5, b6 = find_roms()
    pairs = Pairs(b5, b6, MATCH, WORK, search=not a.no_search)
    if not pairs.map_ok:
        print(f'xrecipe: no symbol map in {MATCH} (build.py symbols --bn6f DIR makes it): pairing by the project\'s pairs and bodies')
    reg = registry()
    alloc = Alloc()
    order = [ai for ai, _ in FAMILIES]
    if a.report:
        order += [x for x in (a.ai if a.ai else range(BN5['nfam'])) if x not in order]
    recipes = []
    for slot, ai in enumerate(order):
        recipes.append(derive(b5, b6, pairs, ai, slot, alloc, reg))
    whole = recipes[:len(FAMILIES)]
    for r, (ai, label) in zip(whole, FAMILIES):
        if r.name != label:
            sys.exit(f'xrecipe: BN5\'s AI {ai} is not {label} (its names archive says otherwise): FAMILIES is wrong')
        r.label = label
        if r.f.decisions and not a.report:
            sys.exit(f'xrecipe: {r.name} (AI {r.ai5}) has decisions open: ' + '; '.join(f'{k}: {d}' for k, d in r.f.decisions))
    for r in recipes[len(FAMILIES):]:
        if r.ai5 in SHARED or r.ai5 in NOT_MOVED:
            r.f.notes.insert(0, 'not moved: ' + (f'BN6 has it by name ({SHARED[r.ai5]})' if r.ai5 in SHARED else NOT_MOVED[r.ai5]))
    text_, json_ = c_file(whole), xdiff_json(whole)
    if a.report:
        report(recipes, os.path.join(WORK, 'report.md'), os.path.join(WORK, 'report.json'))
        print(f'xrecipe: {len(recipes)} families derived; {sum(1 for r in recipes if not r.f.decisions)} with no decision open; '
              f'{os.path.join(WORK, "report.md")}')
    if a.check:
        bad = 0
        for path, want in ((OUT_C, text_), (OUT_JSON, json_)):
            with open(path, encoding='utf-8') as f:
                same = f.read() == want
            print(f'xrecipe: {os.path.relpath(path, ROOT)} {"is" if same else "is NOT"} what the generator writes')
            bad += not same
        return 1 if bad else 0
    if not a.report:
        for path, want in ((OUT_C, text_), (OUT_JSON, json_)):
            with open(path, 'w', encoding='utf-8') as f:
                f.write(want)
        print(f'xrecipe: {", ".join(r.label for r in whole)} written into {os.path.relpath(OUT_C, ROOT)} and '
              f'{os.path.relpath(OUT_JSON, ROOT)}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
