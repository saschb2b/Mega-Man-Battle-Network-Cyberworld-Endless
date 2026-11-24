#!/usr/bin/env python3
"""folder_hands.py [--list] [--trials N]: the starting folders measured as numbers (docs/game/meta/folders-measured.md).

The run's six folders (src/core/meta.c's tables, Standard BN6's own first preset), BN6's other five preset folders,
BN5 Team Colonel's six and BN4 Red Sun's NEW GAME folder, read from the ROMs where GiveFolder finds them
(docs/rom/chips.md, Chip folders and Other games' folders), each counted (kinds, codes, chips in *, the folder's attack
power and heals) and drawn: a Monte Carlo of the Custom screen, five chips a turn, a chip not picked kept for the next,
each turn's pick the legal set (one chip's copies, or one code with * among it) of the most damage and half its heal,
a Program Advance of BN6's table formed where its parts are in the pick. Power is the record's (+0x1A), one hit: a
Vulcan's or a delayed chip's whole damage is not modelled, nor range, foes or the custom gauge's growth.

--grass: a pick's strongest Fire chip after a GrasSeed in it hits double (BN6's Fire on grass; the seed lands where the
foe stands, which the model takes as given).

--list: each preset's chips and codes, and for BN5's and BN4's chips the id a run plays them as (the registry's,
src/core/xchips_registry*.c, or BN6's own by name) and whether BN6's record has the code. Local only: the ROMs
(CYBERWORLD_ROM_DIR, else ~/.cache/mmbn-ref/roms). Prints names read from the ROMs; writes nothing."""
import argparse
import itertools
import os
import random
import re
import struct
import sys
from collections import Counter

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import library_pages as lp  # noqa: E402  (the ROMs and the chips' names)

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REC6 = 0x021DA8                                  # BN6's chip records (docs/rom/chips.md)
BN6_PRESETS = (0x0213AC, 0x0213E8, 0x02158C, 0x021604, 0x0215C8, 0x021640)   # BN6_FOLDER_TABLE's six
BN5_PRESETS = (0x01D814, 0x01D850, 0x01D9F4, 0x01DA6C, 0x01DA30, 0x01DAA8)   # BN5_FOLDER_TABLE's six
BN4_START = 0x017CC8                             # BN4_FOLDERS, its first of twelve
HEAL = {154: 10, 155: 30, 156: 50, 157: 80, 158: 120}   # Recov10 to Recov120
ATK = {192: 10}                                  # Atk+10
NO_DAMAGE = {131, 146, 162, 163, 168, 175, 177, 178, 179, 184, 298}   # support: no damage of their own here
PAS = [(339, (71, 72, 73), 4), (339, (71, 74, 75), 4), (320, (1,), 0), (321, (2,), 0), (322, (3,), 0), (337, (23,), 0),
       (323, (20,), 0), (326, (107,), 0),
       (324, (21,), 0), (327, (108,), 0), (329, (92,), 0), (335, (144,), 0)]   # BN6's table (0x02BA60), the ones in reach
FIRE = {20, 21, 22, 46, 60, 76, 107, 108, 109, 110, 111, 112}   # BN6's Fire attack chips among the commons
GRASSEED = 68
GRASS = False
CODES = {c: k for k, c in enumerate('ABCDEFGHIJKLMNOPQRSTUVWXYZ')} | {'*': 26}


def code(c):
    return '*' if c == 26 else chr(65 + c)


class Folders:
    def __init__(self):
        self.r = lp.roms()
        self.rom = self.r['bn6']
        reg = open(os.path.join(ROOT, 'src/core/xchips_registry.c')).read()
        self.reg5 = {int(a): (int(b), h) for a, b, h in re.findall(r'\{\s*(\d+),\s*(\d+),\s*(XREG_\w+)', reg)}
        reg4 = open(os.path.join(ROOT, 'src/core/xchips_registry_bn4.c')).read()
        self.reg4 = {int(a): (int(b), h) for a, b, h in re.findall(r'\{\s*(\d+),\s*(\d+),\s*(XREG_\w+)', reg4)}
        self.n6 = {}
        for i in range(1, 411):
            self.n6.setdefault(self.name('bn6', i), i)

    def name(self, game, i):
        return lp.spell(lp.name(self.r[game], game, i))

    def power(self, i):
        return struct.unpack_from('<H', self.rom, REC6 + i * 0x2C + 0x1A)[0] if i < 411 else 0

    def codes6(self, i):
        return [code(c) for c in self.rom[REC6 + i * 0x2C:REC6 + i * 0x2C + 4] if c != 0xFF]

    def preset(self, game, o):
        return [(e & 0x1FF, e >> 9) for e in struct.unpack_from('<30H', self.r[game], o)]

    def as_run(self, game, i):
        """the id a run plays game's chip i as, and how"""
        if game == 'bn6':
            return i, 'BN6'
        if game == 'bn5':
            return self.reg5.get(i, (None, 'none'))
        if i in self.reg4:
            return self.reg4[i]
        nm = self.name('bn4', i)
        if nm in self.n6:
            return self.n6[nm], 'BN6 by name'
        return next(((b, 'BN5 row by name') for a, (b, h) in self.reg5.items() if self.name('bn5', a) == nm), (None, 'none'))

    def run_folders(self):
        src = open(os.path.join(ROOT, 'src/core/meta.c')).read()
        ids = dict(re.findall(r'\b([A-Z0-9]+) = (\d+)', src))
        out = {'Standard': self.preset('bn6', BN6_PRESETS[0])}
        for f in re.findall(r'static const uint16_t (\w+)\[30\] = \{', src):
            body = re.search(r'static const uint16_t %s\[30\] = \{(.*?)\};' % f, src, re.S).group(1)
            out[f.capitalize()] = [(int(ids[a]), CODES[b]) for a, b in re.findall(r"C\((\w+), '(.)'\)", body)]
        return out

    def all(self):
        f = self.run_folders()
        for k, o in enumerate(BN6_PRESETS[1:], 1):
            f[f'BN6 preset {k}'] = self.preset('bn6', o)
        for game, o, label in (('bn5', BN5_PRESETS[0], 'BN5 start'), ('bn4', BN4_START, 'BN4 start')):
            f[label] = [(self.as_run(game, i)[0], c) for i, c in self.preset(game, o)]
        return f


def legal(sel):
    return len({i for i, c in sel}) == 1 or len({c for i, c in sel if c != 26}) <= 1


def value(F, sel):
    """(damage, heal, Program Advances) of a pick sent in its best order"""
    ids = [i for i, c in sel]
    pa = None
    for res, parts, kind in PAS:
        if kind == 4 and all(p in ids for p in parts):
            pa = (res, list(parts))
        elif kind == 0:
            cs = sorted(c for i, c in sel if i == parts[0] and c != 26)
            if any(cs[a + 1] == cs[a] + 1 and cs[a + 2] == cs[a] + 2 for a in range(len(cs) - 2)):
                pa = (res, [parts[0]] * 3)
        if pa:
            break
    rest, dmg = list(ids), 0
    if pa:
        for p in pa[1]:
            rest.remove(p)
        dmg = F.power(pa[0])
    attacks = [i for i in rest if i not in HEAL and i not in ATK and i not in NO_DAMAGE and F.power(i) > 0]
    dmg += sum(F.power(i) for i in attacks)
    # (--grass: a GrasSeed sent first, the strongest Fire chip after it on grass: BN6 adds its damage once more and
    # burns the grass plain, docs/research/sources.md)
    fire = [F.power(i) for i in attacks if i in FIRE]
    if GRASS and GRASSEED in rest and fire:
        dmg += max(fire)
    if attacks or pa:
        dmg += sum(ATK.get(i, 0) for i in rest)
    return dmg, sum(HEAL.get(i, 0) for i in rest), int(pa is not None)


def pick(F, hand):
    best, top = None, -1
    for n in range(1, len(hand) + 1):
        for sel in itertools.combinations(hand, n):
            if legal(sel):
                d, h, p = value(F, sel)
                if d + h / 2 > top:
                    top, best = d + h / 2, (sel, d, h, p)
    return best


def draw(F, folder, trials, turns=3):
    rng, tot = random.Random(865), Counter()
    for _ in range(trials):
        deck, hand = folder[:], []
        rng.shuffle(deck)
        for t in range(turns):
            while len(hand) < 5 and deck:
                hand.append(deck.pop())
            sel, d, h, p = pick(F, hand)
            for s in sel:
                hand.remove(s)
            if t == 0:
                tot['d1'] += d
                tot['n1'] += len(sel)
            tot['d'] += d
            tot['h'] += h
            tot['n'] += len(sel)
            tot['pa'] += p
    return {k: v / trials for k, v in tot.items()}


def listing(F):
    for game, offs in (('bn6', BN6_PRESETS), ('bn5', BN5_PRESETS), ('bn4', (BN4_START,))):
        for k, o in enumerate(offs):
            print(f'{game} preset {k} at {o:#08x}')
            for (i, c), m in sorted(Counter(F.preset(game, o)).items()):
                run, how = F.as_run(game, i)
                note = ''
                # (a moved chip's record of BN6's is blank: its codes are the registry row's)
                if run is not None and run < 411 and game != 'bn6' and F.codes6(run) and code(c) not in F.codes6(run):
                    note = f'  (BN6 record codes {" ".join(F.codes6(run))})'
                print(f'  {m}x {F.name(game, i)} {code(c)}: {game} {i}, run {run} {how}{note}')


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--list', action='store_true')
    ap.add_argument('--trials', type=int, default=4000)
    ap.add_argument('--grass', action='store_true', help='a Fire chip sent after a GrasSeed in one pick hits the grass for double')
    a = ap.parse_args()
    global GRASS
    GRASS = a.grass
    F = Folders()
    if a.list:
        listing(F)
        return
    print('folder | kinds | codes | in * | attack power | heal | turn 1: damage, chips | 3 turns: damage, heal, chips, PAs')
    for k, f in F.all().items():
        s = draw(F, f, a.trials)
        atk = sum(F.power(i) for i, c in f if i not in HEAL and i not in ATK and i not in NO_DAMAGE)
        print(f"{k} | {len({i for i, c in f})} | {len({c for i, c in f if c != 26})} | {sum(c == 26 for i, c in f)} | "
              f"{atk} | {sum(HEAL.get(i, 0) for i, c in f)} | {s['d1']:.0f}, {s['n1']:.2f} | "
              f"{s['d']:.0f}, {s['h']:.0f}, {s['n']:.2f}, {s['pa']:.2f}")


if __name__ == '__main__':
    main()
