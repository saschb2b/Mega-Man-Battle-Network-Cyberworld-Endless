#!/usr/bin/env python3
"""Every chip a run can hand out, used by MegaMan turned round on a
Surrounded field (issue #838; docs/rom/chips-code.md, A chip used turned
round).

BN6 never turns MegaMan, so its chips are not all written for it: a
liberation's Surrounded battle (BN5's field 0x33, the enemy's columns 1-2
and 5-6) turns him on L or R. Each chip is fought in BN6's battle
alone (the differential's --xdiff with game=bn6, its trace read by
tools/xdiff_snap.py), three times, against a still CanGard:

  turned   Surrounded, MegaMan at column 3 turned left (L at fight frame
           10), the foes at columns 2 and 5 of row 2
  mirror   the same, MegaMan at column 4 facing right
  plain    BN6's own field, MegaMan at column 3, the foe at column 4: the
           oracle, a foe a column ahead as BN6 fights it

and where none of them struck, again with the foe three columns ahead
(MegaMan at 4 turned, 3 facing right, 1 on the plain field; the foes at
columns 1 and 6, the plain one at 4), for a chip thrown that far;

then the chip used (A at frame 90). A chip that strikes the way he faces
takes off the foe ahead turned, and facing right, what it takes in BN6's
own battle, and leaves the two Surrounded runs the same mirrored: the HP
the foe behind lost, the panels' types and sides (column c against
7 - c) and MegaMan's column at the end.

  python3 tools/turned_sweep.py              # every chip: .build/turned/sweep.tsv, its misses listed
  python3 tools/turned_sweep.py 120 0x1A4    # some ids (decimal or hex)
  python3 tools/turned_sweep.py --jobs 3

Needs the host build (build/host/cyberworld) and the ROMs (local only):
CYBERWORLD_ROM_DIR, else ~/.cache/mmbn-ref/roms.
"""
import argparse
import concurrent.futures
import os
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from xdiff_snap import read_trace  # noqa: E402

ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
WORK = os.environ.get('CYBERWORLD_TURNED_WORK', os.path.join(ROOT, '.build', 'turned'))
BIN = os.environ.get('CYBERWORLD_TURNED_BIN', os.path.join(ROOT, 'build', 'host', 'cyberworld'))
BN6_RECORDS = 0x021DA8     # BN6's chip records (ROM offset, RomLayout's), 0x2C each: +0..3 the codes (26 *, 0xFF none)
BN5_RECORDS = 0x01E210     # BN5_CHIP_RECORDS (src/emu/bn5.h)
BN4_RECORDS = 0x0197EC     # BN4_CHIP_RECORDS less the bus (src/emu/bn4.h)
RECORD = 0x2C
FOE = '4F'                 # BN5's CanGard (BN6's moved 0x200): it never leaves its panel
COL, ROW = 0x12, 0x13      # a battle object's PanelX and PanelY (bn6.h's BN6_T1_PANEL_X)
ALLIANCE, HP = 0x16, 0x24
SPEC = ('game=bn6;foes={foes};mm={mm}2;sides={sides};hp=2000;safe=1;frames=360;seed=12345678:9ABCDEF0;'
        'folder6={deck};deck6={deck};cust6=1:A.2/-.8/START.2/-.8/A.2;keys={keys}90:A:4;out={out}')


def name_of(hexbytes):
    """a chip's name in BN's letters (the dump's hex, to its 0xE6)"""
    out = []
    for c in bytes.fromhex(hexbytes):
        if c == 0xE6:
            break
        out.append(' ' if c == 0 else chr(0x30 + c - 1) if c <= 0x0A else chr(0x41 + c - 0x0B) if c <= 0x24 else
                   chr(0x61 + c - 0x26) if 0x26 <= c <= 0x3F else '-')
    return ''.join(out)


def chips():
    """(id, game, name, code) of every chip a run can hand out: BN6's own ids with a code (no Program Advance, no
    pseudo-chip past 319), and the registry's moved BN5 and BN4 chips (the host build's --chip-texts dump)"""
    os.makedirs(os.path.join(WORK, 'data'), exist_ok=True)
    tsv = os.path.join(WORK, 'texts.tsv')
    subprocess.run([BIN, '--headless', '--rom-dir', ROM_DIR, '--data-dir', os.path.join(WORK, 'data'), '--chip-texts', tsv],
                   check=True, capture_output=True)
    roms = {g: open(os.path.join(ROM_DIR, f), 'rb').read() for g, f in ((6, 'bn6g.gba'), (5, 'bn5c.gba'), (4, 'bn4rs.gba'))}
    base = {6: BN6_RECORDS, 5: BN5_RECORDS, 4: BN4_RECORDS}
    out = []
    for line in open(tsv):
        f = line.rstrip('\n').split('\t')
        cid, game, index = int(f[0]), int(f[1]), int(f[2])
        if not game or (game == 6 and not 1 <= cid < 320):
            continue
        rec = roms[game][base[game] + index * RECORD:base[game] + index * RECORD + 4]
        codes = [c for c in rec if c <= 26]
        if codes:
            out.append((cid, game, name_of(f[4]), codes[0]))
    return out


RUNS = {   # (MegaMan's column, the L press, the foes, the field's sides): the foe ahead one column away
    'turned': (3, '10:L:2,', f'{FOE}@22,{FOE}@52', '33'),
    'mirror': (4, '', f'{FOE}@22,{FOE}@52', '33'),
    'plain': (3, '', f'{FOE}@42', '38'),
}
FAR = {    # ... three away, for a chip thrown there (where the near runs struck nothing)
    'turned': (4, '10:L:2,', f'{FOE}@12,{FOE}@62', '33'),
    'mirror': (3, '', f'{FOE}@12,{FOE}@62', '33'),
    'plain': (1, '', f'{FOE}@42', '38'),
}


def run(cid, code, kind, far=False):
    out = os.path.join(WORK, 'runs', f'{cid}.{kind}{".far" if far else ""}')
    deck = f'{cid:X}:{code:X}' if cid > 511 else f'{cid | code << 9:04X}'
    mm, keys, foes, sides = (FAR if far else RUNS)[kind]
    spec = SPEC.format(foes=foes, mm=mm, sides=sides, deck=deck, keys=keys, out=out)
    r = subprocess.run(['nice', '-n', '10', BIN, '--headless', '--rom-dir', ROM_DIR, '--data-dir', os.path.join(WORK, 'data'),
                        '--xdiff', spec], capture_output=True, text=True)
    if r.returncode or not os.path.exists(out + '.bn6'):
        return None
    return read_trace(out + '.bn6')


def summary(trace):
    """the HP each foe lost by its first column, the panels' (type, side) at the end by (column, row), MegaMan's last
    column, the frames traced"""
    frames = trace['frames']
    first, lost = {}, {}
    mm_col = None
    for fr in frames:
        for o in fr['objs']:
            if o.kind != 'T1':
                continue
            if o.o8(ALLIANCE) == 0:
                mm_col = o.o8(COL)
                continue
            first.setdefault(o.slot, (o.o8(COL), o.o16(HP)))
            lost[o.slot] = first[o.slot][1] - o.o16(HP)
    by_col = {}
    for slot, (col, _) in first.items():
        by_col[col] = max(by_col.get(col, 0), lost.get(slot, 0))
    panels = {}
    if frames:
        for r in range(1, 4):
            for c in range(1, 7):
                p = frames[-1]['panels'][r * 8 + c]
                panels[(c, r)] = (p[2], p[3])
    return by_col, panels, mm_col, len(frames)


def verdict(cid, code):
    v, why = judge(cid, code, False)
    if v != 'NOHIT':
        return v, why
    v, far = judge(cid, code, True)
    return (v, 'far: ' + far) if v != 'NOHIT' else ('NOHIT', why + ' | far: ' + far)


def judge(cid, code, far):
    """OK where the foe ahead loses what BN6's own battle takes off it, turned and facing right, and the two runs on the
    Surrounded field leave the same mirrored; TURNED where turned left alone it does not, BOTH where neither does,
    RIGHT where facing right alone (between the two sides); NOHIT where the plain battle's foe ahead is not struck
    either (no attack, or not one a column ahead), then held to the mirror alone"""
    t, m, p = run(cid, code, 'turned', far), run(cid, code, 'mirror', far), run(cid, code, 'plain', far)
    if t is None or m is None or p is None:
        return 'NORUN', ''
    (ht, pt, mt, _), (hm, pm, mmc, _), (hp, _, _, _) = summary(t), summary(m), summary(p)
    ahead = hp.get(4, 0)
    a, b = (1, 6) if far else (2, 5)
    tl, ml = (ht.get(a, 0), ht.get(b, 0)), (hm.get(b, 0), hm.get(a, 0))   # (the foe he faces, the one behind him)
    notes = [f'plain {ahead}, turned (ahead {tl[0]}, behind {tl[1]}), right (ahead {ml[0]}, behind {ml[1]})']
    bad = sorted(f'{c}{r}' for (c, r), v in pt.items() if v != pm.get((7 - c, r)))
    if bad:
        notes.append('panels unlike ' + ','.join(bad))
    if mt and mmc and mt != 7 - mmc:
        notes.append(f'megaman at {mt} turned, {mmc} facing right')
    mirrored = tl == ml and not bad and len(notes) == 1
    if not ahead:
        return ('NOHIT' if mirrored else 'UNLIKE'), '; '.join(notes)
    turned_ok, right_ok = tl[0] == ahead, ml[0] == ahead
    v = ('OK' if mirrored else 'UNLIKE') if turned_ok and right_ok else 'TURNED' if right_ok else 'RIGHT' if turned_ok else 'BOTH'
    return v, '; '.join(notes)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0], formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('ids', nargs='*')
    ap.add_argument('--jobs', type=int, default=3)
    a = ap.parse_args()
    os.makedirs(os.path.join(WORK, 'runs'), exist_ok=True)
    pool = chips()
    if a.ids:
        want = {int(x, 0) for x in a.ids}
        pool = [c for c in pool if c[0] in want]
    with concurrent.futures.ThreadPoolExecutor(a.jobs) as ex:
        results = list(ex.map(lambda c: (c, verdict(c[0], c[3])), pool))
    lines = []
    for (cid, game, name, code), (v, why) in results:
        lines.append(f'{cid}\t{cid:#05x}\tbn{game}\t{name}\t{chr(65 + code) if code < 26 else "*"}\t{v}\t{why}')
    with open(os.path.join(WORK, 'sweep.tsv'), 'w') as f:
        f.write('\n'.join(lines) + '\n')
    for l in lines:
        if l.split('\t')[5] not in ('OK', 'NOHIT') or a.ids:
            print(l)
    counts = {}
    for _, (v, _) in results:
        counts[v] = counts.get(v, 0) + 1
    print(f'{len(results)} chips: ' + ', '.join(f'{n} {v}' for v, n in sorted(counts.items())) + f' ({WORK}/sweep.tsv)')


if __name__ == '__main__':
    main()
