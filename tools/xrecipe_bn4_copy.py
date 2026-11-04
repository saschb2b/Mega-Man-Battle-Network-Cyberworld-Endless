"""xrecipe_bn4_copy.py: BN6's own object types copied for BN4's families, one changed instruction apart, by
tools/xrecipe_bn4.py (docs/rom/bn4.md, The thunder's copy; bn4_twins.COPIED6): a type of BN6's whose code a family's call
aims at but whose behaviour differs from BN4's in a number BN6's own users need, copied whole from BN6's ROM to the
families' code room, run through the second-level dispatch as a moved type (src/emu/xvirus_dispatch.h), its spawn
through the dispatch's spawner at a number of its own, BN6's own type left as it is.

Numbers only: addresses in BN6 Cybeast Gregar (USA), counts and a CRC32."""
import os
import re
import sys
import zlib

import bn4_twins
import xrecipe_thumb as T

T3_TYPES6 = 0x08003EC4   # BN6_T3_TYPES
SPAWNERS6 = {0x08003358: 'XV_SPAWN_3', 0x08003374: 'XV_SPAWN_3_LAST', 0x08003390: 'XV_SPAWN_3_FIRST'}   # BN6_SPAWN_3*


class Copy:
    def __init__(self, ty, c):
        self.ty, self.spawner, self.why = ty, c['spawner'], c['why']
        self.decisions, self.walks, self.tables, self.calls, self.imms, self.data = [], {}, {}, {}, [], []
        self.numbers = []


def region6(b6, ty):
    types = [b6.u32(T3_TYPES6 + 4 * i) for i in range(0x100)]
    starts = sorted(set(x & ~1 for x in types if x & 1 and 0x08000000 <= x < 0x08800000))
    lo = types[ty] & ~1
    return lo, starts[starts.index(lo) + 1]


def walk(b6, c):
    """its type routine and its spawner walked, their jump tables followed, within its region"""
    todo = [c.lo, c.spawner]
    while todo:
        e = todo.pop() & ~1
        if e in c.walks:
            continue
        w = T.walk(b6, e, region=(c.lo, c.hi))
        c.walks[e] = w
        for site, t in w.bls.items():
            (todo.append(t) if c.lo <= t < c.hi else c.calls.setdefault(t, []).append(site))
        for lit, v in w.lits.items():
            if v is None or v & 3 or not c.lo <= v < c.hi:
                continue
            ents = []
            for k in range(16):
                x = b6.u32(v + 4 * k)
                if not (x & 1 and c.lo <= (x & ~1) < c.hi):
                    break
                ents.append(x)
            if ents:
                c.tables[v] = ents
                todo += ents


def classify(b6, c):
    code, pools, tables = set(), set(), set()
    for w in c.walks.values():
        for a, i in w.insns.items():
            code.update(range(a, a + i.size))
        for lit in w.lits:
            pools.update(range(lit, lit + 4))
    for v, ents in c.tables.items():
        tables.update(range(v, v + 4 * len(ents)))
    kinds = ['CODE' if a in code else 'POOL' if a in pools else 'TABLE' if a in tables else 'RAW' for a in range(c.lo, c.hi)]
    starts = {b6.u32(x) & ~1 for x in range(c.lo, c.hi - 3, 4) if kinds[x - c.lo] == 'POOL'} | {c.lo}
    a = c.lo
    while a < c.hi:
        kd, b = kinds[a - c.lo], a + 1
        while b < c.hi and kinds[b - c.lo] == kd and (b not in starts or kd == 'RAW'):
            b += 1
        if kd != 'CODE':
            c.data.append((a, b, kd))
        a = b
    for a in range(c.lo, c.hi - 1, 2):
        if kinds[a - c.lo] == 'CODE' and (b6.u16(a) & 0xFFF8) == 0x7868:
            c.decisions.append(f'{a:08X}: its object\'s type read (the dispatch\'s, its number in the side table)')


def number_clear(root, kind, ty):
    """no BN5 track's recipe names its type `ty` of `kind` (an XvType row: moved, or BN6's twin), whose number in the
    dispatch's tables (one a kind, by its game's number) the copy takes"""
    emu = os.path.join(root, 'src', 'emu')
    for n in sorted(os.listdir(emu)):
        if n.endswith('.c') and ('bn5' in n or n == 'xvirus_families.c'):
            with open(os.path.join(emu, n), encoding='utf-8') as fh:
                if re.search(rf'\{{ {kind}, 0x{ty:02X}, ', fh.read(), re.I):
                    sys.exit(f'xrecipe_bn4: {n} names the type {kind}:{ty:#x} the copy of BN6\'s takes in the dispatch: give the copy a number of its own')


def derive(b6, ty, at, number):
    """BN6's type `ty` (third kind) copied to `at`, its spawn the dispatch's number `number`, its changed immediates
    (bn4_twins.COPIED6) checked against BN6's code"""
    c = Copy(ty, bn4_twins.COPIED6[ty])
    c.lo, c.hi = region6(b6, ty)
    c.at = at + (c.lo & 3)
    c.number = number
    walk(b6, c)
    classify(b6, c)
    c.crc = zlib.crc32(b6.bytes(c.lo, c.hi - c.lo)) & 0xFFFFFFFF
    # (its pools' words that name neither its own code nor data but read as RAM, I/O or ROM: masks of BN6's own code, kept)
    for lo, hi, kd in c.data:
        for a in range((lo + 3) & ~3, hi - 3, 4) if kd == 'POOL' else ():
            v = b6.u32(a)
            if not c.lo <= (v & ~1) < c.hi and 0x02000000 <= v < 0x0E000000:
                c.numbers.append((a, v))
    for imm in bn4_twins.COPIED6[ty]['imms']:
        i = T.decode(b6, imm['at'])
        if i.form != 'movs' or i.f[0] != imm['rd'] or i.f[1] != imm['was']:
            c.decisions.append(f'{imm["at"]:08X}: not movs r{imm["rd"]},#{imm["was"]:#x} ({i.text})')
        c.imms.append(dict(at=imm['at'], form='THUMB_MOVS', rd=imm['rd'], rn=0, was=imm['was'], now=imm['now'], why=imm['why']))
    sw = c.walks.get(c.spawner & ~1)
    spawns = [(s, t) for s, t in (sw.bls.items() if sw else ()) if t in SPAWNERS6]
    if len(spawns) != 1:
        c.decisions.append(f'its spawner {c.spawner:08X} spawns through {len(spawns)} of BN6\'s spawners')
    else:
        v, chain = T.back_value(sw, spawns[0][0], 0)
        if v != ty or len(chain) != 1 or chain[0][1] != 'movs':
            c.decisions.append(f'its spawner\'s type made by {chain}')
        elif number != ty:
            c.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=0, rn=0, was=ty, now=number,
                               why=f'its type {ty:#x}: copied, the dispatch\'s {number:#x}'))
    c.imms.sort(key=lambda x: x['at'])
    c.end = c.at + (c.hi - c.lo)
    c.veneers = (c.end + 3) & ~3
    c.room_end = c.veneers + 12 * len(c.calls)   # (THUMB_VENEER_SIZE a call out)
    return c


def moved(c, a):
    """where BN6's address a of the copy is (its Thumb bit kept)"""
    return a - c.lo + c.at


def c_text(c, xv_code):
    """the copy's recipe of code alone (xvirus_code_ok), C"""
    L = [f'/* ---- BN6\'s third-kind {c.ty:#x} copied for BN4\'s families (bn4_twins.COPIED6; docs/rom/bn4.md, The thunder\'s copy): '
         f'run as the dispatch\'s {c.number:#x}' + (', its own number (no BN5 track names it)' if c.number == c.ty else '') + ' ---- */']
    L += ['static const XvBlock bn6copy_blocks[] = {',
          f'\t{{ 0x{c.lo:08X}, 0x{c.hi:08X}, XV_CODE + 0x{c.at - xv_code:X}, 0x{c.crc:08X}u }},', '};']
    L += ['static const XvData bn6copy_data[] = {'] + [f'\t{{ 0x{a:08X}, 0x{b:08X}, XV_{k} }},' for a, b, k in c.data] + ['};']
    L += ['static const XvPair bn6copy_calls[] = {']
    for t in sorted(c.calls):
        L.append(f'\t{{ 0x{t:08X}, {SPAWNERS6[t]} }},   /* BN6\'s spawner: the dispatch\'s */' if t in SPAWNERS6 else
                 f'\t{{ 0x{t:08X}, 0x{t:08X} }},   /* BN6\'s own */')
    L += ['};', 'static const XvImm bn6copy_imms[] = {']
    L += [f'\t{{ 0x{i["at"]:08X}, {i["form"]}, {i["rd"]}, {i["rn"]}, 0x{i["was"]:X}, 0x{i["now"]:X} }},   /* {i["why"]} */' for i in c.imms]
    L += ['};', 'static const XvNumber bn6copy_numbers[] = {']
    L += [f'\t{{ 0x{a:08X}, 0x{v:08X}, 0x{v:08X} }},   /* a mask of BN6\'s own code, kept */' for a, v in c.numbers]
    L += ['};', 'static const XvType bn6copy_types[] = {',
          f'\t{{ 3, 0x{c.number:02X}, XV_TYPE_MOVED, 0x{c.ty:02X} }},   /* BN6\'s third-kind {c.ty:#x}, copied, the dispatch\'s {c.number:#x} */', '};']
    n = lambda a: f'(int)(sizeof bn6copy_{a} / sizeof *bn6copy_{a})'
    L += ['const XVirusFamily xvirus_bn4_bn6copy = {', f'\t.name = "BN6\'s third-kind {c.ty:#x} for BN4\'s families",', '\t.game = XV_GAME_BN6,']
    L += [f'\t.{x} = bn6copy_{x}, .n{x} = {n(x)},' for x in ('blocks', 'data', 'calls', 'imms', 'numbers', 'types')]
    L += ['};', f'const uint32_t xvirus_bn4_bn6copy_veneers = XV_CODE + 0x{c.veneers - xv_code:X};',
          f'const int xvirus_bn4_bn6copy_nveneers = {len(c.calls)};', '']
    return '\n'.join(L)
