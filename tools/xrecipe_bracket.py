#!/usr/bin/env python3
"""xrecipe_bracket.py [--json]: BN4 Red Sun's bracket screen (main mode 0x24) as a move into BN6 Gregar would take it,
derived from both ROMs (docs/multirom/bn4-tournament.md, Moved into BN6; docs/devtools/bracket-diff.md).

The screen's code is walked from its roots (its set-up, its mode routine and states, the plates' menu-sprite type, the
menu-sprite runner: src/emu/bn4_tournament.h's names), every routine they reach inside the screen's own regions taken
whole with its literal pool and jump tables, the rest its calls out. Each call out is paired with BN6's routine of the
same body: the project's pairs first (tools/bn4_twins.py), then tools/xrecipe_pairs.py's search by body, then a
near-body search over BN6's mapped routines (the symbol map's entries, .build/symbols/match; their names never
written), each candidate compared again by tools/xrecipe_thumb.py (SAME, LAYOUT: but for the offsets of loads and
stores, or DIFF). A pair SAME or LAYOUT gives its RAM twins too: the RAM words the two bodies load at the same place
(BN4's palette buffer and BN6's, for one). What the screen's literals name is classed: its own RAM (to be placed in
the RAM BN6 leaves alone while the screen runs), RAM a twin names, I/O, VRAM, its own data, and BN4's art (tiles,
palette, maps, sprites: copied from the player's ROM at install). A call into BN4's own logic around the screen (the
draw's next opponent, the script's return, the map's re-entry, the chat box) is the host's to replace (REPLACED).

  python3 tools/xrecipe_bracket.py          the census: .build/xrecipe/bracket_report.md, the move's open decisions
                                            counted; exit 1 while any is open
  python3 tools/xrecipe_bracket.py --json   ... and .build/xrecipe/bracket.json, the pairs and classes as numbers

Local only: both ROMs. Nothing of either is written: addresses, classes and counts."""
import argparse
import csv
import difflib
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import xrecipe_thumb as T  # noqa: E402
from xrecipe_bn4 import MATCH, ROOT, WORK, Pairs4, find_roms  # noqa: E402

# ---- the screen (src/emu/bn4_tournament.h) ----
BRACKET_SETUP, BRACKET_SCREEN, BRACKET_STATES, BRACKET_PLATE = 0x080438F8, 0x0804395C, 0x08043970, 0x08103FFC
MENU_SPRITE_SPAWN, MENU_SPRITE_TYPES = 0x08054056, 0x08053E24
# the menu sprites' runner: its set-up (all freed, then cleared), its spawn by a list, its run and its draw
MENU_SPRITE_INIT, MENU_SPRITE_LIST, MENU_SPRITE_RUN, MENU_SPRITE_DRAW = 0x0805404A, 0x08053F8C, 0x08053FA8, 0x08053FE4
ROOTS = (BRACKET_SETUP, BRACKET_SCREEN, BRACKET_PLATE, MENU_SPRITE_INIT, MENU_SPRITE_LIST, MENU_SPRITE_RUN, MENU_SPRITE_DRAW,
         MENU_SPRITE_SPAWN, 0x08001BE8)
STATES = 3
# the screen's own regions: a routine reached in one is the screen's, moved with it
OWN = ((0x080438F8, 0x08043F34, 'the screen and its set-up'), (0x08103F90, 0x08104710, 'the plates\' menu-sprite type'),
       (0x08053F68, 0x08054180, 'the menu sprites\' runner'), (0x080455C0, 0x080455E0, 'the Free Tournament\'s plates and lines'),
       # (BN4's engine routines moved with it where BN6's twin reads other data: the menu sprite's sprite by BN4's table
       # (BN6's of the same body reads BN6's), the HBlank DMA's set (BN6's keeps two fields more its caller gives))
       (0x08002908, 0x08002934, 'the menu sprite\'s sprite set (BN6\'s twin reads its own table)'),
       (0x08001BE8, 0x08001C04, 'the BG controls\' preset set (BN6\'s twin reads its own presets)'))
# BN4's own logic around the screen, which a run's host replaces: the call made the host's (docs/multirom/bn4-tournament.md)
REPLACED = {
    0x08044744: 'BN4_NEXT_OPPONENT: the set-up\'s next opponent for BN4\'s cutscene (a run picks its own)',
    0x0804E558: 'the set-up\'s chat box closed (BN4\'s text engine: the host\'s map has its own)',
    0x08053908: 'the set-up\'s chat box flags cleared (0x40)',
    0x08005572: 'the leave\'s warp back to the map (the host returns to BN6\'s map)',
    0x08005664: 'the leave\'s script resumed where one waits (BN4_BRACKET_FROM_SCRIPT; the host\'s own return)',
    0x0802DF2C: 'the leave\'s map entered again (the host\'s own return)',
    0x080A6D62: 'MegaMan\'s overworld player unlocked as the screen leaves (the host\'s map)',
    0x08002814: 'the leave\'s OBJ palette put back for BN4\'s map (its body copies BN4\'s own 0x20 bytes; the host\'s return '
                'reloads BN6\'s map)',
}
# the RAM words the screen names as its own: placed in RAM BN6 leaves alone while it runs (a decision of the move)
OWN_RAM = {
    0x0200ACC0: ('BN4_BRACKET', 0x30), 0x02008BB0: ('BN4_MENU_SPRITES', 0x58 * 24), 0x02001460: ('BN4_TOURNAMENTS', 0x80),
    0x02009440: ('BN4_BRACKET_BLEND', 0x140),
}
NEAR_RATIO = 0.95   # a near-body candidate's ratio, its body's instructions with offsets and branch places masked
# calls made through a shim of the move's: BN4's routine, the shim, why
SHIMS = {
    0x080060A4: ('XBR_STUB_HBLANK', 'the HBlank DMA set (r0 its source, r1 its register, r2 its count, r3 its control into the state at '
                                    'BN4\'s 0x02008920, BN6\'s 0x020081B0): BN6\'s 0x080068E8 sets two fields more from r4 and r5 (BN6\'s '
                                    'own caller, 0x080068EC, its line buffer 0x02008E80 and 0x500), which the shim gives as the source '
                                    'and the screen\'s 0x140 bytes'),
}
# calls whose BN6 twin was read by hand, their bodies differing (each DIFF until an audit says it serves)
CANDIDATES = {
    0x08000568: (0x080005D4, 'the music\'s start: the song r0 kept at the game state\'s +0x19 (BN6\'s +0xF), its "none" 0xFF (BN6\'s '
                             '0x63); the screen plays BN4\'s song 0x09, which a run copies into a BN6 slot (src/audio/xsong.c)'),
    0x08001E18: (0x080018D0, 'a veneer to IWRAM\'s fill of a BG map\'s rect with r3 (BN4\'s 0x03005A5D, BN6\'s 0x03005EBB: the same '
                             'instructions, both games\' IWRAM dumped in romlab); BN6\'s 0x080018C2, of the same veneer body, copies a '
                             'rect instead'),
}


def own_region(a):
    for lo, hi, what in OWN:
        if lo <= a < hi:
            return lo, hi, what
    return None


def mapped_bn6():
    """BN6's mapped routines' entries (the symbol map's functions, Thumb): their addresses alone"""
    path = os.path.join(MATCH, 'bn6f-gregar.csv')
    out = set()
    if os.path.exists(path):
        with open(path, newline='', encoding='utf-8') as f:
            for r in csv.DictReader(f):
                if r['kind'] == 'function' and r['address'] and r['mode'] != 'arm':
                    try:
                        out.add(int(r['address'], 16) & ~1)
                    except ValueError:
                        pass
    return sorted(out)


def shape(rom, a):
    n, w = T.norm(rom, a)
    out = []
    for x in n:
        if x[0] in T.MEM_FORMS:
            out.append(repr((x[0], x[1])))
        elif x[0] in ('b', 'bcc'):
            out.append(x[0])
        else:
            out.append(repr(x))
    return out


class Census:
    def __init__(self, b4, b6):
        self.b4, self.b6 = b4, b6
        self.pairs = Pairs4(b4, b6)
        self.walks, self.calls, self.tables, self.lits = {}, {}, {}, {}
        self._bn6 = None

    # ---- the walk ----
    def walk(self):
        todo = list(ROOTS) + [self.b4.u32(BRACKET_STATES + 4 * k) for k in range(STATES)]
        while todo:
            e = todo.pop() & ~1
            if e in self.walks:
                continue
            w = T.walk(self.b4, e)
            self.walks[e] = w
            for site, t in w.bls.items():
                (todo.append(t) if own_region(t) else self.calls.setdefault(t, []).append(site))
            for lit, v in w.lits.items():
                self.lits[lit] = (v, e)
                # (an odd word is a routine where it starts as one, push {.., lr}: the plates' byte tables sit at odd
                # addresses too, BN4_PLATE_SHEETS, BN4_PLATE_FRAMES, BN4_PLATE_SPRITES)
                if v and v & 1 and own_region(v & ~1) and self.b4.u16(v & ~1) & 0xFF00 == 0xB500:
                    todo.append(v)
                if v and not v & 3 and own_region(v):
                    ents = []
                    while own_region(v + 4 * len(ents)) and len(ents) < 32:
                        x = self.b4.u32(v + 4 * len(ents))
                        if not (x & 1 and own_region(x & ~1)):
                            break
                        ents.append(x)
                    if ents:
                        self.tables[v] = ents
                        todo += ents

    def spans(self):
        """the screen's code as spans: each routine's instructions and literals, merged where they touch"""
        pts = []
        for e, w in self.walks.items():
            for a, i in w.insns.items():
                pts.append((a, a + i.size))
            for lit in w.lits:
                pts.append((lit, lit + 4))
        for v, ents in self.tables.items():
            pts.append((v, v + 4 * len(ents)))
        pts.sort()
        out = []
        for lo, hi in pts:
            if out and lo <= out[-1][1] + 2:
                out[-1][1] = max(out[-1][1], hi)
            else:
                out.append([lo, hi])
        return out

    # ---- the pairs ----
    def bn6_shapes(self):
        if self._bn6 is None:
            self._bn6 = {}
            for a in mapped_bn6():
                try:
                    self._bn6[a] = shape(self.b6, a)
                except (IndexError, KeyError, ValueError):
                    pass
        return self._bn6

    def near(self, a4):
        """BN6's mapped routine nearest in body (ratio of their shapes), or None"""
        s4 = shape(self.b4, a4)
        best = (0.0, None)
        for a6, s6 in self.bn6_shapes().items():
            if abs(len(s6) - len(s4)) > max(2, len(s4) // 8):
                continue
            r = difflib.SequenceMatcher(None, s4, s6, autojunk=False).ratio()
            if r > best[0]:
                best = (r, a6)
        return best if best[0] >= NEAR_RATIO else (best[0], None)

    def pair(self, t):
        if t in SHIMS:
            return dict(bn4=t, cls='SHIM', how='the move\'s shim', why=SHIMS[t][1])
        if t in REPLACED:
            return dict(bn4=t, cls='REPLACED', how='the host\'s', why=REPLACED[t])
        tw, how, cls, diff = self.pairs.twin(t)
        if tw is None and t in CANDIDATES:
            # (a twin read by hand whose body differs: compared, its class DIFF until an audit says it serves)
            tw, how = CANDIDATES[t][0], f'read: {CANDIDATES[t][1]}'
            cls, diff, _ = T.compare(self.b4, t, self.b6, tw)
        elif tw is None:
            ratio, tw = self.near(t)
            if tw is None:
                return dict(bn4=t, cls='NONE', how=f'no body within {NEAR_RATIO} (best {ratio:.2f})')
            cls, diff, _ = T.compare(self.b4, t, self.b6, tw)
            how = f'near body ({ratio:.2f})'
        return dict(bn4=t, bn6=tw, cls=cls, how=how, diff=str(diff) if cls != 'SAME' else '')

    def toolkit_sites(self):
        """its reads of eToolkit past +0x3C (mov rN,r10 then ldr rN,[rN,#off]): BN4's toolkit is BN6's from there shifted, so
        each names a field BN6 keeps elsewhere (+0x3C: BN4's tournaments, BN6's game state)"""
        out = []
        for e, w in sorted(self.walks.items()):
            addrs = sorted(w.insns)
            for k, a in enumerate(addrs[:-1]):
                i, j = w.insns[a], w.insns[addrs[k + 1]]
                if i.form == 'hi_mov' and i.f[1] == 10 and j.form == 'ldr' and j.f[1] == i.f[0] and j.f[2] >= 0x3C:
                    # (the shim is a call: its routine must keep lr on its stack, push {.., lr} first)
                    first = w.insns[min(w.insns)]
                    if first.form != 'push' or not first.f[1]:
                        sys.exit(f'xrecipe_bracket: the toolkit read at {a:08X} is in {e:08X}, which does not push lr')
                    out.append((a, e, i.f[0], j.f[2]))
        return sorted({x[0]: x for x in out}.values())

    def data_tables(self, lits):
        """the data its literals name (its own regions' tables that are not code, and BN4's ROM data out of them): each
        table's words that point at ROM or RAM, read to the first word that is neither a pointer nor a small number"""
        out = {}
        for v in sorted({x for x in lits if x and not x & 3 and 0x08000000 <= x < 0x0A000000 and x not in self.tables}):
            if own_region(v) and any(v in w.insns for w in self.walks.values()):
                continue
            ptrs = []
            for k in range(32):
                x = self.b4.u32(v + 4 * k)
                if 0x02000000 <= x < 0x0A000000:
                    ptrs.append((v + 4 * k, x))
                elif x > 0xFFFF and (x & 0xFFFF) != 0xFFFF and x >> 16 != 0xFFFF:
                    break
            out[v] = ptrs
        return out

    def ram_twins(self, pairs):
        """BN4's RAM words and BN6's, loaded at the same place by a pair's two bodies (SAME or LAYOUT)"""
        out = {}
        for p in pairs:
            if p.get('cls') not in ('SAME', 'LAYOUT'):
                continue
            w4, w6 = T.walk(self.b4, p['bn4']), T.walk(self.b6, p['bn6'])
            l4 = [w4.lits[k] for k in sorted(w4.lits)]
            l6 = [w6.lits[k] for k in sorted(w6.lits)]
            for x, y in zip(l4, l6):
                if x and y and 0x02000000 <= x < 0x04000000 and 0x02000000 <= y < 0x04000000:
                    out.setdefault(x, set()).add((y, p['bn4']))
        return out

    # ---- the literals ----
    def classify(self, v, ram):
        if v is None:
            return 'number', ''
        if own_region(v & ~1):
            return 'own', ''
        if v in OWN_RAM:
            return 'own RAM', f'{OWN_RAM[v][0]}, {OWN_RAM[v][1]:#x} bytes'
        if v in ram:
            ys = {y for y, _ in ram[v]}
            return ('RAM twin', f'BN6 {min(ys):08X}') if len(ys) == 1 else ('RAM twins differ', ', '.join(f'{y:08X}' for y in sorted(ys)))
        if 0x02000000 <= v < 0x04000000:
            return 'RAM', 'no twin'
        if 0x04000000 <= v < 0x05000000:
            return 'I/O', ''
        if 0x05000000 <= v < 0x08000000:
            return 'video', ''
        if 0x08000000 <= v < 0x0A000000:
            return ('code', 'out of the screen') if v & 1 else ('ROM data', 'to copy')
        return 'number', ''


# ---- the recipe: src/emu/xbracket_bn4.c ----

# BN4's map left for a screen (BN4_MAP_LEAVE, which the map's sub-state 0x1C runs before the set-up): its calls' twins the entry
# makes after BN6's own way out of the map, and BN4's BG controls' preset the bracket draws on (BN4_BRACKET_BG_PRESET)
MAP_LEAVE, BG_PRESETS_SET, BG_PRESET = 0x08005040, 0x08001BE8, 0x13

OUT_C = os.path.join(ROOT, 'src', 'emu', 'xbracket_bn4.c')
# BN6's places for it (src/emu/xbracket.h, src/emu/free_space.h): its region, its stubs, veneers, code and art
FREE_BRACKET = 0x08800000 + 0x790000
STUB_NONE, STUB_RETURN, STUBS_END = FREE_BRACKET + 0x00, FREE_BRACKET + 0x30, FREE_BRACKET + 0x40
CODE, CODE_SLOT, ART = FREE_BRACKET + 0x1000, 0x800, FREE_BRACKET + 0x6000
# the RAM a mode screen takes while the map waits (BN6's shop decompresses into it as it runs: free_space.h's RAM_BRACKET)
RAM_BRACKET = 0x02029A00
RAM_PLAN = {0x0200ACC0: (0x000, 'BN4_BRACKET'), 0x02001460: (0x040, 'BN4_TOURNAMENTS'), 0x02009440: (0x100, 'BN4_BRACKET_BLEND'),
            0x02008BB0: (0x300, 'BN4_MENU_SPRITES'), 0x0202E000: (0x1000, 'BN4_BRACKET_TILES_RAM')}
# data the code reads by its literals (BN4's address, its end, its kind): pointer lists as pools, a word each
POOLS = ((0x08043814, 0x08043884, 'the palette cycles: the lit lines\' and the titles\' by tournament'),
         (0x08043A78, 0x08043A88, 'the titles\' cycles by tournament'), (0x08043AFC, 0x08043B14, 'BN4_BRACKET_MAPS'),
         (0x08053E24, 0x08053F1C, 'BN4_MENU_SPRITE_TYPES, its types 0-0x3D'), (0x080281D0, 0x080281E8, 'BN4_MENU_SPRITE_PTRS\' first six'))
RAWS = ((0x08044844, 0x08044888, 'the Free Tournament\'s spawn lists'),
        (0x08001C04, 0x08001C04 + 8 * (BG_PRESET + 1), 'BN4_BG_PRESETS, to the bracket\'s'))
# BN4's art the screen draws, copied whole from the player's ROM: its tiles (LZ77), palettes and BG maps; the plates' sprites
ART_BLOCKS = ((0x086C4E40, 0x086C792C, 'BN4_BRACKET_TILES to the last of BN4_BRACKET_MAPS\' maps'),
              (0x084B7E50, 0x084BEADC, 'BN4_MENU_SPRITE_PTRS 2-5, one after another: the lit lines\' sprites (2, 3) and the plates\' (4, 5), '
                                       'BN4_PLATE_SPRITES\' values'))


def groups(c):
    """the blocks: each own region with the pools it reads beside it, then the data and art regions"""
    out = []
    for lo, hi, what in OWN:
        g = [lo, hi, what]
        for plo, phi, _ in POOLS + RAWS:
            if lo - 0x100 <= phi <= lo or hi <= plo <= hi + 0x40:
                g[0], g[1] = min(g[0], plo), max(g[1], phi)
        out.append(g)
    for plo, phi, what in POOLS + RAWS:
        if not any(g[0] <= plo and phi <= g[1] for g in out):
            out.append([plo, phi, what])
    code = [(lo, hi, what, CODE + k * CODE_SLOT + (lo & 0x3F)) for k, (lo, hi, what) in enumerate(out)]
    art, at = [], ART
    for lo, hi, what in ART_BLOCKS:
        art.append((lo, hi, what, at + (lo & 0x3F)))
        at = (at + (lo & 0x3F) + hi - lo + 0x3F) & ~0x3F
    for lo, hi, _, at in code:
        assert hi - lo + (lo & 0x3F) <= CODE_SLOT, hex(lo)
    assert at <= FREE_BRACKET + 0x10000, hex(at)
    return code, art


def data_ranges(c, b4, code, art):
    """each block's ranges that are not code: the literals and pools (a word each), the jump tables, the rest raw"""
    blocks = [(lo, hi) for lo, hi, _, _ in code + art]

    def in_blocks(v):
        return any(lo <= (v & ~1) < hi for lo, hi in blocks)
    insn = set()
    for w in c.walks.values():
        for a, i in w.insns.items():
            insn.update(range(a, a + i.size))
    out = []
    for lo, hi, _, _ in code:
        a = lo
        while a < hi:
            if a in insn:
                a += 2
                continue
            if a in c.lits or any(plo <= a < phi for plo, phi, _ in POOLS) or (not a & 3 and a + 4 <= hi and not (insn & {a, a + 2})
                                                                              and in_blocks(b4.u32(a))):
                out.append((a, a + 4, 'XV_POOL'))
                a += 4
                continue
            tab = next((t for t in c.tables if t <= a < t + 4 * len(c.tables[t])), None)
            if tab is not None:
                end = tab + 4 * len(c.tables[tab])
                out.append((a, end, 'XV_TABLE'))
                a = end
                continue
            e = a
            while e < hi and e not in insn and e not in c.lits and not any(plo <= e < phi for plo, phi, _ in POOLS) and \
                    not any(t <= e < t + 4 * len(c.tables[t]) for t in c.tables) and \
                    not (not e & 3 and e + 4 <= hi and not (insn & {e, e + 2}) and in_blocks(b4.u32(e))):
                e += 2
            out.append((a, e, 'XV_RAW'))
            a = e
    for lo, hi, _, _ in art:
        out.append((lo, hi, 'XV_RAW'))
    # (a raw range's words never point into a block: checked here as the installer will)
    for lo, hi, k in out:
        if k == 'XV_RAW':
            for a in range((lo + 3) & ~3, hi - 3, 4):
                if in_blocks(b4.u32(a)):
                    sys.exit(f'xrecipe_bracket: raw data at {a:08X} points into a block ({b4.u32(a):08X})')
    return sorted(out)


def words_of(c, b4, r, code, art, data):
    """the words its pools name out of its blocks: RAM by its twin (or by distance into a twin's buffer), its own RAM by the
    plan, I/O and video as they are, a jump table's entry or a sprite of BN4's the screen never runs by the stub or 0"""
    blocks = [(lo, hi) for lo, hi, _, _ in code + art]
    twins = {x: min(y for y, _ in ys) for x, ys in r['ram'].items()}
    twins.update({x: y for x, (y, _) in RAM_READ.items()})
    out = {}
    for lo, hi, k in data:
        if k == 'XV_RAW':
            continue
        for a in range(lo, hi, 4):
            v = b4.u32(a)
            if any(blo <= (v & ~1) < bhi for blo, bhi in blocks) or not 0x02000000 <= v < 0x0E000000:
                continue
            if v in RAM_PLAN:
                out[v] = (RAM_BRACKET + RAM_PLAN[v][0], f'{RAM_PLAN[v][1]}: the plan\'s')
            elif v in twins:
                out[v] = (twins[v], 'its twin\'s (a pair\'s bodies)')
            elif 0x02000000 <= v < 0x04000000:
                near = [(v - x, x) for x in twins if 0 < v - x < 0x400]
                if not near:
                    sys.exit(f'xrecipe_bracket: RAM {v:08X} at {a:08X} has no twin')
                d, x = min(near)
                out[v] = (twins[x] + d, f'{x:08X}\'s twin\'s, {d:#x} on')
            elif 0x04000000 <= v < 0x08000000:
                out[v] = (v, 'I/O or video, as it is')
            elif 0x08053E24 <= a < 0x08053F1C:
                out[v] = (STUB_NONE | 1, 'a menu sprite type the screen never spawns: the stub')
            elif 0x080281D0 <= a < 0x080281E8:
                out[v] = (0, 'a menu sprite the screen never draws')
            elif v == 0x08043884:
                out[v] = (0, 'the replaced script return\'s argument')
            else:
                sys.exit(f'xrecipe_bracket: the word {v:08X} at {a:08X} names nothing the recipe places')
    for lit, (v, _) in c.lits.items():
        if v == 0x08043884:
            out[v] = (0, 'the replaced script return\'s argument')
    return out


def calls_of(c, r):
    out = []
    for p in r['pairs']:
        t = p['bn4']
        if p['cls'] == 'SHIM':
            to, why = SHIMS[t][0], 'the move\'s shim: ' + SHIMS[t][1]
        elif p['cls'] == 'REPLACED':
            to, why = (STUB_RETURN if t == 0x0802DF2C else STUB_NONE), 'the host\'s: ' + REPLACED[t]
        elif p['cls'] == 'SAME' or t in AUDITED:
            to, why = p['bn6'] & ~1, f'{p["how"]}, {p["cls"]}' + (f'; audited: {AUDITED[t]}' if t in AUDITED else '')
        else:
            sys.exit(f'xrecipe_bracket: the call to {t:08X} is {p["cls"]} and no audit says it serves')
        out.append((t, to, why))
    return out


# RAM twins read by hand where no pair's bodies give them (BN4's, BN6's, how)
RAM_READ = {0x02008920: (0x020081B0, 'the HBlank DMA\'s state: BN4\'s 0x080060BC and BN6\'s 0x08006910 stop it, the same body')}
# a DIFF pair taken with what its audit read
AUDITED = {0x08000568: 'BN6\'s plays song r0 as BN4\'s does, its own song numbers: the screen\'s song is BN4\'s 0x09 copied into a '
                       'BN6 slot (src/audio/xsong.c), its immediate set to that slot as the screen installs'}


def leave_of(c, b4):
    """BN4_MAP_LEAVE's calls in order, each with the r0 a movs before it gives: BN6's twin of the same body, or the preset's set moved
    with the screen; one with neither left out (its reason in the list)"""
    out, arg = [], 0
    w = T.walk(b4, MAP_LEAVE)
    for a in sorted(w.insns):
        i = w.insns[a]
        if i.form == 'bl':
            t = i.target
            if t == BG_PRESETS_SET:
                out.append((arg, 'own', t, 'BN4\'s BG controls\' preset, moved'))
            else:
                tw, how, cls, _ = c.pairs.twin(t)
                if tw is None:
                    ratio, tw = c.near(t)
                    cls = T.compare(b4, t, c.b6, tw)[0] if tw else None
                    how = f'near body ({ratio:.2f})'
                out.append((arg, 'BN6' if cls == 'SAME' else None, tw if cls == 'SAME' else t, f'{t:08X}: {how}, {cls}'))
            arg = 0
        elif i.text.startswith('movs r0, #'):
            arg = int(i.text.split('#')[1], 0)
        elif i.form == 'ldr_lit' and i.f[0] == 1:
            # (its last step: DISPCNT in the render info, [eToolkit +8] +0, made its word: every layer off as the screen sets up)
            out.append((b4.u32(i.lit), 'display', 'XBR_STUB_DISPLAY', f'the render info\'s DISPCNT made {b4.u32(i.lit):#x}'))
    return out


def crc(b4, lo, hi):
    import zlib
    return zlib.crc32(b4.bytes(lo, hi - lo)) & 0xFFFFFFFF


HEADER = '''/* xbracket_bn4.c: BN4 Red Sun's bracket screen (main mode 0x24) moved into BN6, written by tools/xrecipe_bracket.py from
 * BN4's and BN6's ROMs (python3 tools/xrecipe_bracket.py --write; --check says whether this is what it writes). Do not edit:
 * change the generator and write it again. Numbers alone: addresses in BN4 Red Sun (USA) and BN6 Cybeast Gregar (USA) and
 * CRC32s; the code and art are the player's ROM's, copied as the core starts (src/emu/xbracket.c; docs/multirom/
 * bn4-tournament.md, Moved into BN6). */
#include "xbracket_bn4.h"

#include "rom.h"
#include "xbracket.h"
'''


def c_file(c, b4, r):
    code, art = groups(c)
    data = data_ranges(c, b4, code, art)
    words = words_of(c, b4, r, code, art, data)
    calls = calls_of(c, r)
    L = [HEADER, 'static const XvBlock blocks[] = {']
    for lo, hi, what, at in code + art:
        base = 'XBR_CODE' if at < ART else 'XBR_ART'
        off = at - (CODE if at < ART else ART)
        L.append(f'\t{{ 0x{lo:08X}, 0x{hi:08X}, {base} + 0x{off:X}, 0x{crc(b4, lo, hi):08X}u }},   /* {what} */')
    L += ['};', 'static const XvData data[] = {']
    L += [f'\t{{ 0x{lo:08X}, 0x{hi:08X}, {k} }},' for lo, hi, k in data]
    L += ['};', 'static const XvPair calls[] = {']
    for t, to, why in calls:
        name = to if isinstance(to, str) else 'XBR_STUB_RETURN' if to == STUB_RETURN else 'XBR_STUB_NONE' if to == STUB_NONE else f'0x{to:08X}'
        L.append(f'\t{{ 0x{t:08X}, {name} }},   /* {why} */')
    L += ['};', 'static const XvPair words[] = {']
    for v, (to, why) in sorted(words.items()):
        name = (f'RAM_BRACKET + 0x{to - RAM_BRACKET:X}' if RAM_BRACKET <= to < RAM_BRACKET + 0x4000 else
                'XBR_STUB_NONE | 1u' if to == STUB_NONE | 1 else f'0x{to:08X}')
        L.append(f'\t{{ 0x{v:08X}, {name} }},   /* {why} */')
    L += ['};', '/* its reads of eToolkit +0x3C (BN4\'s tournaments, BN6\'s game state): each a call to the shim that loads the tournaments\' place */',
          'const XbrSite xbracket_sites[] = {']
    L += [f'\t{{ 0x{a:08X}, {rd} }},' for a, _, rd, _ in r['sites']]
    L += ['};', 'const int xbracket_site_count = (int)(sizeof xbracket_sites / sizeof *xbracket_sites);', '',
          '/* BN4_MAP_LEAVE\'s calls the entry makes after BN6\'s own way out of the map: BN6\'s twins, and BN4\'s preset\'s set moved */',
          'const XbrCall xbracket_leave[] = {']
    for arg, kind, t, why in leave_of(c, b4):
        if kind == 'display':
            L.append(f'\t{{ 0x{arg:X}, XBR_STUB_DISPLAY | 1u, false }},   /* {why} */')
        elif kind == 'own':
            L.append(f'\t{{ 0x{arg:X}, 0x{t:08X}, true }},   /* {why} */')
        elif kind == 'BN6':
            L.append(f'\t{{ 0x{arg:X}, 0x{t | 1:08X}, false }},   /* {why} */')
        else:
            L.append(f'\t/* (left out: {why}, no twin of the same body) */')
    L += ['};', 'const int xbracket_leave_count = (int)(sizeof xbracket_leave / sizeof *xbracket_leave);', '',
          'const XVirusFamily xbracket_bn4 = {', '\t.name = "BN4\'s bracket screen",', '\t.game = XROM_BN4_RED_SUN_US,',
          '\t.blocks = blocks, .nblocks = (int)(sizeof blocks / sizeof *blocks),',
          '\t.data = data, .ndata = (int)(sizeof data / sizeof *data),',
          '\t.calls = calls, .ncalls = (int)(sizeof calls / sizeof *calls),',
          '\t.words = words, .nwords = (int)(sizeof words / sizeof *words),', '};', '']
    return '\n'.join(L)


def report(c, r, path):
    spans, pairs, ram, lits, sites, tables = r['spans'], r['pairs'], r['ram'], r['lits'], r['sites'], r['tables']
    n = sum(hi - lo for lo, hi in spans)
    by = ', '.join(f'{k} {sum(1 for p in pairs if p["cls"] == k)}' for k in ('SAME', 'LAYOUT', 'DIFF', 'NONE', 'REPLACED', 'SHIM'))
    lines = ['# BN4\'s bracket screen: the move\'s census', '',
             f'Written by `tools/xrecipe_bracket.py` from both ROMs. {len(c.walks)} routines walked, {n} bytes in {len(spans)} '
             f'spans; {len(pairs)} calls out: {by}.', '', '## Spans', '']
    lines += [f'- `{lo:08X}`-`{hi:08X}` ({hi - lo} bytes, {(own_region(lo) or (0, 0, "out of the regions"))[2]})' for lo, hi in spans]
    lines += ['', '## Calls out', '', '| BN4 | BN6 | class | how | sites | the difference |', '| --- | --- | --- | --- | --- | --- |']
    for p in pairs:
        lines.append(f'| `{p["bn4"]:08X}` | {("`%08X`" % p["bn6"]) if p.get("bn6") else "-"} | {p["cls"]} | {p["how"]} | '
                     f'{len(c.calls[p["bn4"]])} | {p.get("diff") or p.get("why", "")} |')
    lines += ['', '## RAM twins (a pair\'s two bodies, the same place)', '']
    lines += [f'- `{x:08X}` -> ' + ', '.join(f'`{y:08X}` (by `{a:08X}`)' for y, a in sorted(ys)) for x, ys in sorted(ram.items())]
    lines += ['', '## eToolkit past +0x3C', '', 'BN4\'s toolkit is BN6\'s from +0x3C shifted: each read names a field BN6 keeps '
              'elsewhere (+0x3C BN4\'s tournaments, BN6\'s game state), the two instructions a call to a shim of the move\'s that '
              'loads the field\'s place (the routine pushes lr).', '']
    lines += [f'- `{a:08X}` (in `{e:08X}`): r{rd} = [eToolkit +{off:#x}]' for a, e, rd, off in sites]
    lines += ['', '## Literals out of the screen', '', '| at | word | class | note |', '| --- | --- | --- | --- |']
    lines += [f'| `{a:08X}` | `{v:08X}` | {k} | {note} |' for a, v, k, note in lits]
    lines += ['', '## Data it reads, and what that data points at', '']
    for v, ptrs in sorted(tables.items()):
        lines.append(f'- `{v:08X}`: ' + (', '.join(f'+{a - v:#x} `{x:08X}`' for a, x in ptrs) or 'numbers only'))
    n_open = open_count(r)
    lines += ['', f'## Open: {n_open}', '',
              'A DIFF or NONE call is copied (BN4\'s engine routine moved with the screen), shimmed, or audited as serving; a '
              'toolkit read is shimmed; the screen\'s own RAM is placed in RAM BN6 leaves alone while it runs; its data and '
              'what that points at in BN4\'s ROM (tiles, palettes, maps, the plates\' sprites) is copied from the player\'s ROM.']
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w') as f:
        f.write('\n'.join(lines) + '\n')


def open_count(r):
    return (sum(1 for p in r['pairs'] if p['cls'] in ('NONE', 'DIFF', 'LAYOUT')) + len(r['sites'])
            + sum(1 for x in r['lits'] if x[2] in ('RAM', 'ROM data', 'RAM twins differ', 'own RAM')))


def census(b4, b6):
    c = Census(b4, b6)
    c.walk()
    pairs = [c.pair(t) for t in sorted(c.calls)]
    ram = c.ram_twins(pairs)
    lits = []
    for at, (v, _) in sorted(c.lits.items()):
        if v is None or not 0x02000000 <= v < 0x0A000000 or own_region(v & ~1):
            continue
        k, note = c.classify(v, ram)
        lits.append((at, v, k, note))
    own_data = [v for v, _ in c.lits.values() if v and own_region(v) and not v & 3]
    tables = c.data_tables([x[1] for x in lits if x[2] == 'ROM data'] + own_data)
    return c, dict(spans=c.spans(), pairs=pairs, ram=ram, lits=lits, sites=c.toolkit_sites(), tables=tables)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--json', action='store_true')
    ap.add_argument('--write', action='store_true', help='write src/emu/xbracket_bn4.c')
    ap.add_argument('--check', action='store_true', help='fail where src/emu/xbracket_bn4.c is not what --write writes')
    a = ap.parse_args()
    b4, b6 = find_roms()
    c, r = census(b4, b6)
    path = os.path.join(WORK, 'bracket_report.md')
    report(c, r, path)
    if a.json:
        with open(os.path.join(WORK, 'bracket.json'), 'w') as f:
            json.dump(dict(spans=r['spans'], pairs=r['pairs'],
                           ram={f'{x:08X}': sorted(f'{y:08X}' for y, _ in ys) for x, ys in r['ram'].items()},
                           sites=[dict(at=a_, routine=e, rd=rd, off=off) for a_, e, rd, off in r['sites']],
                           lits=[dict(at=at, v=v, cls=k, note=n) for at, v, k, n in r['lits']],
                           tables={f'{v:08X}': [[a_, x] for a_, x in p] for v, p in r['tables'].items()}), f, indent=1)
    if a.write or a.check:
        text = c_file(c, b4, r)
        old = open(OUT_C).read() if os.path.exists(OUT_C) else ''
        if a.check:
            print(f'xrecipe_bracket: {os.path.relpath(OUT_C, ROOT)} {"in step" if old == text else "NOT in step"}')
            return 0 if old == text else 1
        with open(OUT_C, 'w') as f:
            f.write(text)
        print(f'xrecipe_bracket: wrote {os.path.relpath(OUT_C, ROOT)}')
    by = {k: sum(1 for p in r['pairs'] if p['cls'] == k) for k in ('SAME', 'LAYOUT', 'DIFF', 'NONE', 'REPLACED', 'SHIM')}
    n_open = open_count(r)
    print(f'xrecipe_bracket: {len(c.walks)} routines, {sum(hi - lo for lo, hi in r["spans"])} bytes; calls out {by}; '
          f'{len(r["ram"])} RAM twins; {len(r["sites"])} toolkit reads; {n_open} open ({os.path.relpath(path, ROOT)})')
    return 1 if n_open else 0


if __name__ == '__main__':
    sys.exit(main())
