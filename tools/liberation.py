#!/usr/bin/env python3
"""liberation.py [--seeds N] [--act A] [--show K] [--draw DIR] [--json FILE]:
the liberation generator's ROM-free prototype (docs/LIBERATION.md).

Makes Battle Network 5's Liberation Missions afresh from a seed, by the rules
dissected from its ROM and its players' guides: a board of panels laid out in
the net's shape language (platforms, one-wide walkways, pads on spurs), its
kinds placed as BN5 places them (a start zone at an edge, the Darkloid's 3x3
on the far side, DarkHoles in rings of dark panels with their guardians, dark
panels thickening away from the start, items in the dark, BonusPanels at the
ends of spurs, numbered barriers whose keys lie on the near side), checked
against the fairness rules and rejected where one fails. A planner then plays
each mission many times by BN5's rules, abstracted (a team acting once a
phase, three-turn liberations, the one-turn bonus, Order Points, the enemy's
phase), and its phases set the mission's par. Prints the measures the design
promises over the seeds: phases against par, every ability's worth, route
variety, rejections. Numbers only; nothing of BN5's is read here
(tools/liberation_bn5.py measures BN5's own nine missions the same way).
"""
import argparse
import collections
import json
import math
import os
import random
import statistics
import sys

FREE, DARK, ITEM, BONUS, HOLE, RING, BOSS, BAR1, BAR2, BAR3, START = range(11)
BARS = (BAR1, BAR2, BAR3)
DIRS = ((1, 0), (0, 1), (-1, 0), (0, -1))
AROUND = tuple((dx, dy) for dx in (-1, 0, 1) for dy in (-1, 0, 1) if dx or dy)

# Per act of the endless net (docs/PROGRESSION.md), the BN5 mission it stands
# for and the sizes BN5's missions have there (docs/LIBERATION.md, The
# missions): panels, DarkHoles, team, barrier kinds, the share of dark panels,
# items, BonusPanels, guardians beyond one a hole, the enemies' damage and HP.
ACTS = {
    2: dict(bn5=1, panels=(95, 120), holes=2, team=2, bars=0, dark=(0.40, 0.55), items=7, bonus=2, roam=0,
            kinds=('brute',), hp=200, ally_hp=200, virus=14, boss_hp=400, boss_range=2, boss_dmg=40),
    3: dict(bn5=3, panels=(165, 215), holes=3, team=3, bars=1, dark=(0.45, 0.58), items=8, bonus=3, roam=1,
            kinds=('brute', 'hawk'), hp=300, ally_hp=300, virus=20, boss_hp=600, boss_range=2, boss_dmg=50),
    4: dict(bn5=5, panels=(205, 240), holes=4, team=4, bars=2, dark=(0.52, 0.66), items=12, bonus=3, roam=2,
            kinds=('brute', 'hawk', 'bladia'), hp=400, ally_hp=350, virus=28, boss_hp=900, boss_range=3, boss_dmg=80),
    5: dict(bn5=7, panels=(215, 250), holes=5, team=5, bars=2, dark=(0.58, 0.68), items=16, bonus=3, roam=2,
            kinds=('brute', 'hawk', 'bladia'), hp=500, ally_hp=450, virus=34, boss_hp=1200, boss_range=2, boss_dmg=80),
    6: dict(bn5=9, panels=(215, 250), holes=5, team=5, bars=3, dark=(0.55, 0.68), items=14, bonus=3, roam=3,
            kinds=('brute', 'hawk', 'bladia'), hp=600, ally_hp=650, virus=40, boss_hp=1500, boss_range=3, boss_dmg=100),
}
# The guardians, as BN5's guides give them (docs/LIBERATION.md, The enemy's phase): steps, reach, damage by act.
GUARDS = {
    'brute': dict(move=2, reach=1, all_in_reach=True, hp=(120, 180, 220, 250, 300), dmg=(20, 40, 50, 60, 80)),
    'hawk': dict(move=3, reach=3, all_in_reach=False, hp=(100, 150, 180, 200, 250), dmg=(20, 30, 40, 60, 70)),
    'bladia': dict(move=2, reach=1, all_in_reach=False, hp=(200, 230, 230, 300, 340), dmg=(50, 50, 70, 100, 120), darkens=True),
}
# Every Navi's liberation ability, its kind and its shape (docs/LIBERATION.md, The Navis).
ABILITIES = {
    'MegaMan': 'longswrd', 'Colonel': 'screndiv', 'ProtoMan': 'wideswrd', 'ShadowMan': 'sneak',
    'NumberMan': 'numbercheck', 'TomahawkMan': 'tomahawk', 'KnightMan': 'stonebody', 'ToadMan': 'melody',
    'GyroMan': 'incoming', 'NapalmMan': 'napalm', 'SearchMan': 'search', 'MagnetMan': 'barrier', 'Meddy': 'twin',
}
ALLIES = ('Colonel', 'ShadowMan', 'NumberMan', 'TomahawkMan', 'KnightMan', 'ToadMan',
          'ProtoMan', 'GyroMan', 'NapalmMan', 'SearchMan', 'MagnetMan', 'Meddy')
FLIERS = ('ShadowMan', 'GyroMan')
# The planner's skill: a liberation battle's chance to be won in one turn on a panel with
# no dark around, and to fail (each dark panel around takes 0.05 and adds 0.035).
SKILL = [0.75, 0.04]
# BN5's par over the planner's median phases on BN5's own nine boards (tools/liberation_bn5.py
# prints both): the planner plays slower than BN5's par expects, so a generated mission's par is
# its median phases times this, BN5's yardstick.
PAR_SCALE = 0.62


def shape(ability, pos, d):
    """The panels an ability reaches from pos facing d (dx, dy): its BN5 shape."""
    (x, y), (dx, dy) = pos, d
    px, py = -dy, dx  # to the side
    ahead = lambda k, s=0: (x + dx * k + px * s, y + dy * k + py * s)
    if ability == 'longswrd':
        return [ahead(1), ahead(2)]
    if ability in ('wideswrd', 'screndiv'):
        return [ahead(1, -1), ahead(1), ahead(1, 1)]
    if ability in ('tomahawk', 'numbercheck'):
        return [ahead(k, s) for k in (1, 2) for s in (-1, 0, 1)]
    if ability == 'napalm':
        return [ahead(1), ahead(2), ahead(3), ahead(4), ahead(3, -1), ahead(3, 1)]
    if ability == 'melody':
        return [ahead(k) for k in range(1, 6)]
    return []


class Board:
    def __init__(self, act, seed):
        self.act, self.seed = act, seed
        self.cells = {}          # (x, y) -> kind
        self.items = {}          # (x, y) -> (type, amount)
        self.guards = []         # dicts: kind, pos, hole
        self.boss = None
        self.start = []
        self.spur_ends = []
        self.platform = set()
        self.secret = None       # the sealed route's gate panel, if any
        self.par = None

    def floor(self, c):
        return c in self.cells

    def neighbours(self, c):
        return [(c[0] + dx, c[1] + dy) for dx, dy in DIRS if (c[0] + dx, c[1] + dy) in self.cells]


def carve_line(cells, a, b, rng):
    """A one-wide walkway from a to b: straight runs bending once at a right angle."""
    (x0, y0), (x1, y1) = a, b
    path = []
    if rng.random() < 0.5:
        for x in range(x0, x1, 1 if x1 > x0 else -1): path.append((x, y0))
        for y in range(y0, y1, 1 if y1 > y0 else -1): path.append((x1, y))
    else:
        for y in range(y0, y1, 1 if y1 > y0 else -1): path.append((x0, y))
        for x in range(x0, x1, 1 if x1 > x0 else -1): path.append((x, y1))
    path.append(b)
    for c in path: cells.add(c)
    return path


def lay_out(act, rng):
    """The board's floor in the net's shape language (docs/LEVEL_DESIGN.md): platforms,
    walkways between their sides, a start zone at the left edge, the Darkloid's
    block at the right, spurs ending in pads or stubs."""
    spec = ACTS[act]
    want = rng.randint(*spec['panels'])
    w = int(math.sqrt(want * 2.6) * 1.25)
    h = int(want * 2.6 / w) + 2
    floor, plat = set(), set()
    rects = []
    tries = 0
    while len(floor) < want * 0.55 and tries < 400:
        tries += 1
        rw, rh = rng.randint(3, 6), rng.randint(3, 5)
        rx, ry = rng.randint(4, w - rw - 5), rng.randint(1, h - rh - 1)
        if any(rx < x + ww + 2 and x < rx + rw + 2 and ry < y + hh + 2 and y < ry + rh + 2 for x, y, ww, hh in rects):
            continue
        rects.append((rx, ry, rw, rh))
        for x in range(rx, rx + rw):
            for y in range(ry, ry + rh):
                floor.add((x, y)); plat.add((x, y))
    sy = rng.randint(2, h - 3)
    start = [(0, sy - 1), (0, sy), (0, sy + 1), (1, sy)]
    if rng.random() < 0.5: start += [(1, sy - 1), (1, sy + 1)]
    by = rng.randint(2, h - 3)
    bx = w + 1
    boss_block = [(bx + dx, by + dy) for dx in (-1, 0, 1) for dy in (-1, 0, 1)]
    for c in start + boss_block: floor.add(c)
    nodes = [(2, sy)] + [(x + ww // 2, y + hh // 2) for x, y, ww, hh in rects] + [(bx - 2, by)]
    floor.add((2, sy)); floor.add((bx - 2, by)); carve_line(floor, (1, sy), (2, sy), rng); floor.add((bx - 2, by))
    # A spanning tree by distance, then a loop or two (BN5's maps are mostly trees, docs/LEVEL_DESIGN.md).
    linked, edges = {0}, []
    while len(linked) < len(nodes):
        best = min(((abs(nodes[i][0] - nodes[j][0]) + abs(nodes[i][1] - nodes[j][1]), i, j)
                    for i in linked for j in range(len(nodes)) if j not in linked))
        edges.append(best[1:]); linked.add(best[2])
    for _ in range(rng.randint(1, 1 + act // 3)):
        i, j = rng.sample(range(len(nodes)), 2)
        if (i, j) not in edges and (j, i) not in edges: edges.append((i, j))
    for i, j in edges: carve_line(floor, nodes[i], nodes[j], rng)
    carve_line(floor, (bx - 2, by), (bx - 1, by), rng)
    # Spurs: pads of 3x3 and one-wide stubs off the walkways (where BN5 sets its BonusPanels).
    ends = []
    walk = [c for c in floor if c not in plat and c not in boss_block and c not in start]
    for k in range(rng.randint(2, 4) + act // 2):
        if not walk: break
        c = rng.choice(walk)
        d = rng.choice(DIRS)
        n = rng.randint(2, 4)
        path = [(c[0] + d[0] * i, c[1] + d[1] * i) for i in range(1, n + 1)]
        if any(p in floor or not (2 <= p[0] < w and 0 <= p[1] < h) for p in path): continue
        if any((p[0] + e[0], p[1] + e[1]) in floor for p in path[1:] for e in DIRS if (p[0] + e[0], p[1] + e[1]) not in path and (p[0] + e[0], p[1] + e[1]) != c):
            continue
        for p in path: floor.add(p)
        end = path[-1]
        if k % 2 == 0:
            pad = [(end[0] + d[0] + dx, end[1] + d[1] + dy) for dx in (-1, 0, 1) for dy in (-1, 0, 1)]
            if not any(p in floor for p in pad):
                for p in pad: floor.add(p); plat.add(p)
                continue
        ends.append(end)
    return floor, plat, start, boss_block, (bx, by), ends


def graph_dist(board, src, passable):
    """Breadth-first distances from src cells over panels passable(kind) allows."""
    dist = {s: 0 for s in src}
    q = collections.deque(src)
    while q:
        c = q.popleft()
        for n in board.neighbours(c):
            if n not in dist and passable(board.cells[n]):
                dist[n] = dist[c] + 1
                q.append(n)
    return dist


def generate(act, seed):
    """One mission, or None with the reason it was rejected."""
    rng = random.Random(seed * 7919 + act)
    spec = ACTS[act]
    floor, plat, start, boss_block, boss, ends = lay_out(act, rng)
    b = Board(act, seed)
    for c in floor: b.cells[c] = FREE
    b.platform = plat
    b.start = start
    for c in start: b.cells[c] = START
    for c in boss_block: b.cells[c] = RING
    b.cells[boss] = BOSS
    b.boss = boss
    n = len(b.cells)
    lo, hi = spec['panels']
    if not (lo * 0.8 <= n <= hi * 1.2):
        return None, 'size'
    reach = graph_dist(b, start, lambda k: True)
    if boss not in reach:
        return None, 'unreachable'
    far = max(reach.values())
    if reach[boss] < 0.6 * far:
        return None, 'boss-near'
    # DarkHoles: on platforms with a whole ring of floor, far from the start and from each other.
    cand = [c for c in plat if all((c[0] + dx, c[1] + dy) in b.cells and b.cells[(c[0] + dx, c[1] + dy)] == FREE for dx, dy in AROUND)
            and b.cells[c] == FREE and reach.get(c, 0) >= 0.3 * far]
    rng.shuffle(cand)
    holes = []
    for c in sorted(cand, key=lambda c: -reach[c] * rng.uniform(0.6, 1.0)):
        if all(abs(c[0] - h[0]) + abs(c[1] - h[1]) >= 6 for h in holes) and abs(c[0] - boss[0]) + abs(c[1] - boss[1]) >= 4:
            holes.append(c)
        if len(holes) == spec['holes']: break
    if len(holes) < spec['holes']:
        return None, 'holes'
    for i, h in enumerate(holes):
        b.cells[h] = HOLE
        for dx, dy in AROUND: b.cells[(h[0] + dx, h[1] + dy)] = DARK
        ring = [(h[0] + dx, h[1] + dy) for dx, dy in AROUND if dx == 0 or dy == 0]
        b.guards.append(dict(kind=rng.choice(spec['kinds']) if i else 'brute', pos=rng.choice(ring), hole=i))
    b.holes = holes
    # Dark panels: thicker away from the start, in clusters (one pass of a majority rule).
    d0, d1 = spec['dark']
    free = [c for c, k in b.cells.items() if k == FREE]
    for c in free:
        t = reach[c] / far
        if reach[c] <= 2: continue
        if rng.random() < d0 + (d1 - d0) * 2 * t - 0.25 + 0.25 * t: b.cells[c] = DARK
    for c in free:
        if reach[c] <= 2: continue
        around = [b.cells.get((c[0] + dx, c[1] + dy)) for dx, dy in AROUND]
        darks = sum(1 for k in around if k == DARK); floors = sum(1 for k in around if k is not None)
        if floors and darks / floors > 0.6 and rng.random() < 0.7: b.cells[c] = DARK
        elif floors and darks / floors < 0.2 and rng.random() < 0.5: b.cells[c] = FREE
    # BonusPanels at the ends of spurs (BN5's "?" at the end of its stubs).
    stubs = [e for e in ends if b.cells.get(e) in (FREE, DARK)]
    rng.shuffle(stubs)
    for e in stubs[:spec['bonus']]: b.cells[e] = BONUS
    # Barriers: walkway panels whose removal cuts off part of the board, the key on the near side.
    for k in range(spec['bars']):
        cut = find_cut(b, rng, min_region=8 + 2 * k)
        if not cut:
            return None, 'barrier'
        cells, region = cut
        for c in cells: b.cells[c] = BARS[k]
        near = graph_dist(b, start, lambda kk: kk not in BARS and kk not in (HOLE, RING, BOSS))
        spots = [c for c in near if b.cells[c] == DARK and c not in region and near[c] >= 4]
        if not spots:
            return None, 'key'
        key = rng.choice(spots)
        b.cells[key] = ITEM
        b.items[key] = ('key', k + 1)
        if k == spec['bars'] - 1 and act >= 4 and rng.random() < 0.5:
            gate = max(region, key=lambda c: abs(c[0] - start[0][0]) + abs(c[1] - start[0][1]))
            if b.cells.get(gate) in (FREE, DARK):
                b.secret = gate
    # Items in the dark: corners and dead ends first (BN5 tucks them there).
    darks = [c for c, kk in b.cells.items() if kk == DARK and c not in b.items]
    darks.sort(key=lambda c: (len(b.neighbours(c)), rng.random()))
    mix = item_mix(act, rng, spec['items'])
    for c, it in zip(darks[:len(mix) * 2:2] + darks[1:len(mix) * 2:2], mix):
        b.cells[c] = ITEM
        b.items[c] = it
    for i in range(spec['roam']):
        spots = [c for c, kk in b.cells.items() if kk == DARK and reach.get(c, 0) > 0.4 * far]
        if spots:
            b.guards.append(dict(kind=rng.choice(spec['kinds']), pos=rng.choice(spots), hole=None))
    why = check(b)
    if why:
        return None, why
    return b, None


def item_mix(act, rng, n):
    """What the item panels hold, in BN5's kinds (docs/LIBERATION.md, Item panels)."""
    out = [('op', rng.choice((2, 3)))]
    pool = [('heart', 1)] * 3 + [('op', 2), ('op', 3)] * 2 + [('zenny', 1)] * 3 + [('chip', 1)] * 2
    if act >= 4: pool += [('bugfrag', 1)] * 2 + [('trap', 1)] * 2
    while len(out) < n: out.append(rng.choice(pool))
    return out


def find_cut(b, rng, min_region):
    """One to three walkway panels that cut a region off the start's side (not the boss)."""
    walk = [c for c, k in b.cells.items() if k in (FREE, DARK) and len(b.neighbours(c)) == 2 and c not in b.start]
    rng.shuffle(walk)
    for c in walk[:120]:
        saved = b.cells[c]
        b.cells[c] = BAR1
        side = graph_dist(b, b.start, lambda k: k != BAR1 and k not in BARS)
        b.cells[c] = saved
        region = [x for x in b.cells if x not in side and x != c]
        if b.boss in region or any(x in region for x in b.start):
            continue
        if len(region) < min_region:
            continue
        # A wall of two or three where the walkway runs on (BN5 walls its barriers).
        cells = [c]
        for n in b.neighbours(c):
            if n in region and b.cells[n] in (FREE, DARK) and len(b.neighbours(n)) == 2 and len(cells) < 3:
                cells.append(n)
        return cells, set(region) - set(cells)
    return None


def check(b):
    """The fairness rules (docs/LIBERATION.md, Guarantees): a reason to reject, or None."""
    lib = lambda k: k not in (BOSS,)
    dist = graph_dist(b, b.start, lambda k: k not in BARS)
    for i, k in enumerate(BARS):
        keys = [c for c, it in b.items.items() if it == ('key', i + 1)]
        if any(kk == k for kk in b.cells.values()) and not any(c in dist for c in keys):
            return 'key-behind-lock'
    every = graph_dist(b, b.start, lib)
    for h in b.holes:
        if h not in every: return 'hole-cut'
    first = [c for c in b.cells if b.cells[c] in (DARK, ITEM)
             and any(n in graph_dist(b, b.start, lambda k: k in (FREE, START, BONUS)) for n in b.neighbours(c))]
    if not first:
        return 'nothing-to-liberate'
    if any(g['pos'] in graph_dist(b, b.start, lambda k: True) and graph_dist(b, b.start, lambda k: True)[g['pos']] <= 3 for g in b.guards):
        return 'guard-at-start'
    return None


# ---- the planner: BN5's rules, abstracted --------------------------------------------------

class Sim:
    def __init__(self, b, team, rng, act):
        self.b, self.rng, self.act = b, rng, act
        self.cells = dict(b.cells)
        self.items = dict(b.items)
        spec = ACTS[act]
        self.spec = spec
        tier = min(4, max(0, spec['bn5'] // 2))
        self.guards = [dict(g, hp=GUARDS[g['kind']]['hp'][tier], max=GUARDS[g['kind']]['hp'][tier],
                            dmg=GUARDS[g['kind']]['dmg'][tier]) for g in b.guards]
        self.boss_hp = spec['boss_hp']
        self.op = 3
        self.navis = []
        pos = list(b.start)
        for i, name in enumerate(team):
            hp = spec['hp'] if name == 'MegaMan' else spec['ally_hp'] + (150 if name == 'KnightMan' else 0)
            self.navis.append(dict(name=name, ab=ABILITIES[name], hp=hp, max=hp, pos=pos[i % len(pos)], down=0, stun=0))
        self.uses = collections.Counter()
        self.liberated_by = collections.Counter()
        self.shield = False
        self.melody = set()
        self.holes_order = []
        self.decisions = 0
        self.close_calls = 0

    # The board's state
    def holes_left(self):
        return [c for c, k in self.cells.items() if k == HOLE]

    def walkable(self, c):
        return self.cells.get(c) in (FREE, START, BONUS)

    def occupied(self):
        return {n['pos'] for n in self.navis} | {g['pos'] for g in self.guards}

    def reach(self, navi):
        """Where the Navi may end his move: clean panels joined to his (teammates passed, not
        stood on; guardians block; ShadowMan and GyroMan pass over dark panels too)."""
        fly = navi['name'] in FLIERS
        occ = self.occupied() - {navi['pos']}
        foes = {g['pos'] for g in self.guards}
        seen, q = {navi['pos']}, collections.deque([navi['pos']])
        while q:
            c = q.popleft()
            for n in self.b.neighbours(c):
                if n in seen or n in foes: continue
                k = self.cells[n]
                if self.walkable(n) or (fly and k in (DARK, ITEM)):
                    seen.add(n); q.append(n)
        return [c for c in seen if self.walkable(c) and (c not in occ)]

    def target_cost(self):
        """Per panel, the dark panels still to liberate from it to the nearest target (Dijkstra, 0-1)."""
        holes = self.holes_left()
        if holes:
            targets = holes
        else:
            targets = [self.b.boss]
        dist = {t: 0 for t in targets}
        dq = collections.deque(targets)
        while dq:
            c = dq.popleft()
            for n in self.b.neighbours(c):
                k = self.cells[n]
                if k in BARS: continue
                w = 1 if k in (DARK, ITEM, RING) else 0
                nd = dist[c] + w
                if nd < dist.get(n, 1 << 30):
                    dist[n] = nd
                    (dq.appendleft if w == 0 else dq.append)(n)
        return dist

    def libable(self, c):
        k = self.cells.get(c)
        if k in (DARK, ITEM): return True
        if k == RING: return not self.holes_left()
        return False

    def value(self, cells, cost):
        """What liberating these panels is worth: progress toward a target, items, a hole, the boss."""
        v = 0.0
        here = min((cost.get(n, 99) for c in cells for n in self.b.neighbours(c) if self.walkable(n)), default=99)
        for c in cells:
            k = self.cells.get(c)
            if k == RING and not self.holes_left():
                v += 6.0
            elif k in (DARK, ITEM, RING):
                v += 3.0 if cost.get(c, 99) < here or cost.get(c, 99) <= 1 else 0.6
                if k == ITEM:
                    it = self.items.get(c, ('', 0))[0]
                    v += {'op': 2.5 if self.op < 6 else 0.5, 'key': 4.0, 'heart': 1.5}.get(it, 0.8)
            elif k == HOLE:
                v += 14.0
        return v

    def difficulty(self, c):
        return sum(1 for dx, dy in AROUND if self.cells.get((c[0] + dx, c[1] + dy)) in (DARK, ITEM, HOLE, RING))

    def battle(self, navi, c, guard=None):
        """One liberation battle: 1 the one-turn win, 2 a win in two or three, 0 a failure; HP taken."""
        d = self.difficulty(c)
        if guard:
            p1, pf = 0.10, 0.30 + 0.02 * d
        elif self.cells.get(c) == HOLE:
            p1, pf = 0.0, 0.25
        else:
            p1, pf = max(0.08, SKILL[0] - 0.05 * d), min(0.45, SKILL[1] + 0.035 * d)
        r = self.rng.random()
        dmg = self.spec['virus'] * (1 + d / 4) * self.rng.uniform(0.3, 1.4)
        if r < p1: res, dmg = 1, dmg * 0.3
        elif r < 1 - pf: res = 2
        else: res, dmg = 0, dmg * 1.3
        self.hurt(navi, dmg)
        return res

    def hurt(self, navi, dmg):
        if navi['ab'] == 'stonebody' and not navi.get('in_battle'): dmg = 0
        navi['hp'] -= int(dmg)
        if navi['hp'] <= 0:
            navi['hp'], navi['down'] = 0, 2

    def take(self, navi, c):
        """A panel liberated: its item given."""
        k = self.cells.get(c)
        if k == ITEM:
            it, n = self.items.pop(c)
            if it == 'op': self.op = min(8, self.op + n)
            elif it == 'heart': navi['hp'] = min(navi['max'], navi['hp'] + navi['max'] // 2)
            elif it == 'key':
                for x, kk in self.cells.items():
                    if kk == BARS[n - 1]: self.cells[x] = FREE
            elif it == 'trap': self.hurt(navi, 50 + 10 * self.act)
        if k == HOLE:
            self.holes_order.append(c)
            self.guards = [g for g in self.guards if g['hole'] is None or self.b.holes[g['hole']] != c]
        if k in (DARK, ITEM, HOLE, RING):
            self.cells[c] = FREE
            self.liberated_by[navi['name']] += 1

    def liberate(self, navi, cells, bonus=True):
        cells = [c for c in cells if self.libable(c) or self.cells.get(c) == HOLE]
        if not cells: return
        main = cells[0]
        res = self.battle(navi, main)
        if res == 0: return
        for c in cells: self.take(navi, c)
        if res == 1 and bonus and self.cells.get(main) != HOLE:
            for dx, dy in AROUND:
                n = (main[0] + dx, main[1] + dy)
                if self.cells.get(n) in (DARK, ITEM): self.take(navi, n)

    def options(self, navi, cost):
        """Every move-and-act the Navi has this phase, valued."""
        opts = []
        ab = navi['ab']
        for p in self.reach(navi):
            for d in DIRS:
                t = (p[0] + d[0], p[1] + d[1])
                guard = next((g for g in self.guards if g['pos'] == t), None)
                if guard:
                    opts.append((0.6 * self.value([t], cost) + 4.0 * (1 - guard['hp'] / guard['max']) + 1.0, 'fight', p, d))
                    if ab == 'sneak' and self.op and guard['hp'] > 100:
                        opts.append((0.5 * self.value([t], cost) + 2.0, 'sneak', p, d))
                    continue
                if t == self.b.boss:
                    if not self.holes_left():
                        # The Darkloid heals 100 for every dark panel around him (BN5's guides): clear
                        # his ring first unless one fight can finish him.
                        heal = 100 * self.difficulty(self.b.boss)
                        opts.append((30.0 if self.boss_hp <= 0.45 * self.spec['boss_hp'] or heal < 200 else 3.0, 'boss', p, d))
                    continue
                if self.cells.get(t) == HOLE:
                    opts.append((self.value([t], cost) * 0.75, 'lib', p, d)); continue
                if not self.libable(t): continue
                p1 = max(0.08, SKILL[0] - 0.05 * self.difficulty(t))
                around = [(t[0] + dx, t[1] + dy) for dx, dy in AROUND if self.libable((t[0] + dx, t[1] + dy))]
                v = self.value([t], cost) + p1 * self.value(around, cost) * 0.8
                opts.append((v * (1 - 0.035 * self.difficulty(t)), 'lib', p, d))
                reach2 = navi['pos'] in self.melody
                if reach2:
                    opts.append((self.value(shape('melody', p, d), cost) * 0.8, 'long', p, d))
                if self.op and ab in ('longswrd', 'wideswrd', 'screndiv', 'tomahawk', 'napalm'):
                    cells = [c for c in shape(ab, p, d) if self.libable(c)]
                    if t in cells:
                        opts.append((self.value(cells, cost) * 0.85 - 1.5, 'ability', p, d))
                if self.op and ab == 'numbercheck':
                    cells = [c for c in shape(ab, p, d) if self.cells.get(c) == ITEM]
                    if cells:
                        opts.append((2.5 * len(cells) - 1.5, 'number', p, d))
                if self.op and ab in ('search', 'twin'):
                    line = self.line(t, d)
                    if len(line) >= 3:
                        opts.append((self.value(line, cost) * (0.7 if ab == 'twin' else 0.8) - 1.5, 'line', p, d))
            if self.op and ab == 'incoming' and self.cells.get(p) in (FREE,):
                pass
        if self.op and ab == 'incoming':
            for p in self.flyover(navi):
                if self.cells.get(p) == DARK:
                    opts.append((self.value([p], cost) + 1.0 - 1.5, 'incoming', p, None))
        if self.op and ab == 'barrier' and self.threat() > 0:
            opts.append((min(9.0, self.threat() / 15.0) - 1.5, 'barrier', navi['pos'], None))
        if self.op and ab == 'melody':
            mates = [m for m in self.navis if m is not navi and not m.get('acted') and not m['down']
                     and abs(m['pos'][0] - navi['pos'][0]) <= 1 and abs(m['pos'][1] - navi['pos'][1]) <= 1]
            if mates:
                opts.append((4.0, 'melody', navi['pos'], None))
        if navi['hp'] < navi['max'] * 0.4:
            opts.append((5.0 + 6.0 * (1 - navi['hp'] / navi['max']), 'rest', navi['pos'], None))
        return opts

    def flyover(self, navi):
        return [c for c in self.b.neighbours(navi['pos']) for c in [c] + self.b.neighbours(c) if self.cells.get(c) == DARK]

    def ring(self):
        bx, by = self.b.boss
        return [(bx + dx, by + dy) for dx, dy in AROUND]

    def line(self, t, d):
        out = []
        c = t
        while self.libable(c) and len(out) < 6:
            out.append(c)
            c = (c[0] + d[0], c[1] + d[1])
        return out

    def threat(self):
        return sum(g['dmg'] for g in self.guards for n in self.navis
                   if abs(g['pos'][0] - n['pos'][0]) + abs(g['pos'][1] - n['pos'][1]) <= GUARDS[g['kind']]['move'] + GUARDS[g['kind']]['reach'])

    def act_navi(self, navi, cost):
        opts = self.options(navi, cost)
        if not opts:
            return
        opts.sort(key=lambda o: -o[0])
        self.decisions += 1
        if len(opts) > 1 and opts[1][0] >= 0.9 * opts[0][0] and opts[1][2:] != opts[0][2:]:
            self.close_calls += 1
        v, kind, p, d = opts[0]
        navi['pos'] = p
        t = (p[0] + d[0], p[1] + d[1]) if d else None
        if kind == 'lib':
            self.liberate(navi, [t])
        elif kind == 'long':
            self.liberate(navi, shape('melody', p, d), bonus=False)
        elif kind == 'ability':
            self.op -= 1; self.uses[navi['name']] += 1
            self.liberate(navi, [t] + [c for c in shape(navi['ab'], p, d) if c != t], bonus=navi['ab'] == 'longswrd')
        elif kind == 'number':
            self.op -= 1; self.uses[navi['name']] += 1
            for c in shape('numbercheck', p, d):
                if self.cells.get(c) == ITEM:
                    it = self.items[c]
                    if it[0] != 'trap': self.take(navi, c); self.cells[c] = DARK
                    else: self.items.pop(c); self.cells[c] = DARK
        elif kind == 'line':
            self.op -= 1; self.uses[navi['name']] += 1
            self.liberate(navi, self.line(t, d), bonus=False)
        elif kind == 'incoming':
            self.op -= 1; self.uses[navi['name']] += 1
            self.take(navi, p); navi['pos'] = p
        elif kind == 'barrier':
            self.op -= 1; self.uses[navi['name']] += 1; self.shield = True
        elif kind == 'melody':
            self.op -= 1; self.uses[navi['name']] += 1
            for m in self.navis:
                if m is not navi and abs(m['pos'][0] - p[0]) <= 1 and abs(m['pos'][1] - p[1]) <= 1: self.melody.add(m['pos'])
        elif kind == 'rest':
            navi['hp'] = min(navi['max'], navi['hp'] + navi['max'] // 2)
        elif kind in ('fight', 'sneak'):
            g = next((g for g in self.guards if g['pos'] == t), None)
            if g:
                if kind == 'sneak':
                    self.op -= 1; self.uses[navi['name']] += 1
                    g['hp'] = max(1, g['hp'] - 100 * max(1, self.spec['bn5'] // 2))
                else:
                    navi['in_battle'] = True
                    res = self.battle(navi, t, guard=g)
                    navi['in_battle'] = False
                    # Liberating a guardian's panel is a fight with him: won, he is deleted and the
                    # panel liberated; his DarkHole sends another in the enemy's phase.
                    g['hp'] -= g['max'] if res else g['max'] // 3
                    if g['hp'] <= 0:
                        self.guards.remove(g)
                        self.take(navi, t)
                        if g['hole'] is not None and self.cells.get(self.b.holes[g['hole']]) == HOLE:
                            self.respawn.append(dict(g, hp=g['max']))
        elif kind == 'boss':
            navi['in_battle'] = True
            d_ = self.rng.uniform(0.25, 0.6) * self.spec['boss_hp'] * (1.4 if navi['name'] == 'MegaMan' else 1.0)
            self.hurt(navi, self.spec['boss_dmg'] * self.rng.uniform(1.0, 2.5))
            navi['in_battle'] = False
            self.boss_hp -= int(d_)

    def enemy_phase(self):
        for g in self.respawn:
            h = self.b.holes[g['hole']]
            spots = [(h[0] + dx, h[1] + dy) for dx, dy in AROUND if self.cells.get((h[0] + dx, h[1] + dy)) in (DARK, ITEM)
                     and (h[0] + dx, h[1] + dy) not in self.occupied()]
            if self.cells.get(h) == HOLE and spots:
                self.guards.append(dict(g, pos=spots[0]))
        self.respawn = []
        targets = [n for n in self.navis if not n['down']]
        if not targets: return
        for g in self.guards:
            st = GUARDS[g['kind']]
            near = min(targets, key=lambda n: abs(n['pos'][0] - g['pos'][0]) + abs(n['pos'][1] - g['pos'][1]))
            for _ in range(st['move']):
                dd = abs(near['pos'][0] - g['pos'][0]) + abs(near['pos'][1] - g['pos'][1])
                if dd <= st['reach']: break
                steps = [n for n in self.b.neighbours(g['pos']) if self.cells[n] in (DARK, ITEM) and n not in self.occupied()]
                if st.get('darkens'):
                    steps += [n for n in self.b.neighbours(g['pos']) if self.cells[n] == FREE and n not in self.occupied()]
                if not steps: break
                nxt = min(steps, key=lambda n: abs(near['pos'][0] - n[0]) + abs(near['pos'][1] - n[1]))
                if abs(near['pos'][0] - nxt[0]) + abs(near['pos'][1] - nxt[1]) >= dd: break
                g['pos'] = nxt
                if st.get('darkens') and self.cells[nxt] == FREE: self.cells[nxt] = DARK
            hit = [n for n in targets if abs(n['pos'][0] - g['pos'][0]) + abs(n['pos'][1] - g['pos'][1]) <= st['reach']]
            if not st['all_in_reach']: hit = hit[:1]
            for n in hit:
                if not self.shield: self.hurt(n, g['dmg'])
            g['hp'] = min(g['max'], g['hp'] + g['max'] // 2)
        bx, by = self.b.boss
        for n in targets:
            if abs(n['pos'][0] - bx) + abs(n['pos'][1] - by) <= self.spec['boss_range'] and not self.shield:
                self.hurt(n, self.spec['boss_dmg'])
        self.boss_hp = min(self.spec['boss_hp'], self.boss_hp + 100 * self.difficulty(self.b.boss))
        self.shield = False
        self.melody = set()

    def run(self, cap=40):
        self.respawn = []
        for phase in range(1, cap + 1):
            for navi in self.navis:
                navi['acted'] = False
            order = sorted(self.navis, key=lambda n: n['name'] != 'MegaMan')
            for navi in order:
                if navi['down']:
                    navi['down'] -= 1
                    if not navi['down']: navi['hp'] = navi['max'] // 2
                    continue
                cost = self.target_cost()
                self.act_navi(navi, cost)
                navi['acted'] = True
                for c, k in list(self.cells.items()):
                    if k == BONUS and any(n['pos'] == c for n in self.navis):
                        self.cells[c] = FREE
                        r = self.rng.randrange(5)
                        if r == 0: navi['hp'] = navi['max']
                        elif r == 1: self.op = min(8, self.op + 5)
                        elif r == 3 and self.guards: self.guards.remove(min(self.guards, key=lambda g: abs(g['pos'][0] - c[0]) + abs(g['pos'][1] - c[1])))
                        elif r == 4: self.shield = True
                if self.boss_hp <= 0:
                    return phase
            if all(n['down'] for n in self.navis):
                return None
            self.enemy_phase()
        return None


def team_for(act, rng, allies=ALLIES[:6]):
    n = ACTS[act]['team']
    return ['MegaMan'] + rng.sample(list(allies), n - 1)


def play(b, team, runs, seed, ban=None):
    """Phases to clear over runs of the planner, abilities used, holes' order."""
    out = []
    for r in range(runs):
        rng = random.Random((seed * 31 + r) * 104729)
        s = Sim(b, team, rng, b.act)
        if ban:
            for n in s.navis:
                if n['name'] == ban: n['ab'] = 'none'
        p = s.run()
        out.append((p, s))
    return out


def set_par(b, team, runs=24):
    res = play(b, team, runs, b.seed)
    ph = sorted(p for p, _ in res if p)
    if len(ph) < runs * 0.6:  # BN5's own mission 1 the planner wins about half the time
        return None, res
    b.par = max(5, round(ph[len(ph) // 2] * PAR_SCALE))
    return b.par, res


# ---- drawing (our own picture: coloured squares, nothing of BN5's art) ----------------------

COLOURS = {FREE: (120, 220, 235), DARK: (110, 50, 140), ITEM: (110, 50, 140), BONUS: (240, 200, 60), HOLE: (20, 10, 30),
           RING: (150, 40, 60), BOSS: (220, 40, 40), BAR1: (150, 150, 160), BAR2: (120, 120, 135), BAR3: (90, 90, 110),
           START: (60, 110, 230)}
CHARS = {FREE: '.', DARK: 'o', ITEM: 'i', BONUS: '?', HOLE: 'H', RING: 'B', BOSS: 'X', BAR1: '1', BAR2: '2', BAR3: '3', START: 's'}


def text(cells, guards=(), items=None):
    xs = [c[0] for c in cells]; ys = [c[1] for c in cells]
    g = {x['pos'] for x in guards}
    rows = []
    for y in range(min(ys), max(ys) + 1):
        rows.append(''.join(('g' if (x, y) in g else CHARS[cells[(x, y)]]) if (x, y) in cells else ' ' for x in range(min(xs), max(xs) + 1)))
    return '\n'.join(rows)


def draw(cells, path, guards=(), items=None, title='', scale=10):
    from PIL import Image, ImageDraw
    xs = [c[0] for c in cells]; ys = [c[1] for c in cells]
    x0, y0 = min(xs), min(ys)
    w, h = (max(xs) - x0 + 1) * scale, (max(ys) - y0 + 1) * scale
    im = Image.new('RGB', (w + 2 * scale, h + 3 * scale), (16, 16, 24))
    dr = ImageDraw.Draw(im)
    for (x, y), k in cells.items():
        px, py = (x - x0 + 1) * scale, (y - y0 + 2) * scale
        dr.rectangle([px, py, px + scale - 2, py + scale - 2], fill=COLOURS[k])
        if k == ITEM:
            dr.rectangle([px + scale // 3, py + scale // 3, px + scale - scale // 3 - 2, py + scale - scale // 3 - 2], fill=(90, 200, 220))
        if k == HOLE:
            dr.ellipse([px + 1, py + 1, px + scale - 3, py + scale - 3], outline=(200, 120, 255))
    for g in guards:
        x, y = g['pos']
        px, py = (x - x0 + 1) * scale, (y - y0 + 2) * scale
        dr.ellipse([px + 2, py + 2, px + scale - 4, py + scale - 4], fill=(255, 140, 0))
    if title:
        dr.text((scale, 2), title, fill=(230, 230, 230))
    im.save(path)


def measures(act, seeds, runs, show, drawdir):
    stats = collections.defaultdict(list)
    rejects = collections.Counter()
    worth = collections.defaultdict(list)
    usefreq = collections.Counter()
    present = collections.Counter()
    shown = 0
    tried = 0
    made = 0
    seed = 0
    while made < seeds and tried < seeds * 12:
        seed += 1; tried += 1
        b, why = generate(act, seed)
        if not b:
            rejects[why] += 1
            continue
        rng = random.Random(seed)
        team = team_for(act, rng)
        par, res = set_par(b, team, runs)
        if par is None:
            rejects['unwinnable'] += 1
            continue
        made += 1
        ph = [p * PAR_SCALE for p, _ in res if p]
        stats['panels'].append(len(b.cells))
        stats['dark'].append(sum(1 for k in b.cells.values() if k in (DARK, ITEM)) / len(b.cells))
        stats['par'].append(par)
        stats['best'].append(sum(1 for p in ph if p <= par - 2) / len(res))
        stats['mid'].append(sum(1 for p in ph if par - 2 < p <= par) / len(res))
        stats['over'].append(1 - stats['best'][-1] - stats['mid'][-1])
        stats['close'].append(statistics.mean(s.close_calls / max(1, s.decisions) for _, s in res))
        orders = collections.Counter(tuple(s.holes_order[:2]) for _, s in res)
        stats['orders'].append(len(orders))
        stats['secret'].append(1 if b.secret else 0)
        stats['bars'].append(sum(1 for k in b.cells.values() if k in BARS))
        for n in team[1:] + ['MegaMan']:
            present[n] += 1
            used = sum(s.uses[n] for _, s in res) / len(res)
            if used > 0.2: usefreq[n] += 1
            ban = play(b, team, max(8, runs // 3), seed, ban=n)
            bp = [p if p else 40 for p, _ in ban]
            worth[n].append(PAR_SCALE * (statistics.mean(bp) - statistics.mean(p if p else 40 for p, _ in res[:len(ban)])))
        if shown < show:
            shown += 1
            print('act %d seed %d: %d panels, par %d, team %s, phases (scaled) %s' % (act, seed, len(b.cells), par, ', '.join(team), [round(p, 1) for p in sorted(ph)]))
            print(text(b.cells, b.guards))
            print()
        if drawdir:
            os.makedirs(drawdir, exist_ok=True)
            draw(b.cells, os.path.join(drawdir, 'act%d_seed%03d.png' % (act, seed)), b.guards,
                 title='act %d seed %d  par %d  %s' % (act, seed, par, '+'.join(team)))
    return stats, rejects, worth, usefreq, present, tried


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--seeds', type=int, default=60)
    ap.add_argument('--runs', type=int, default=16)
    ap.add_argument('--act', type=int, default=0, help='one act (2-6); every act without it')
    ap.add_argument('--show', type=int, default=0)
    ap.add_argument('--draw', default='')
    ap.add_argument('--json', default='')
    a = ap.parse_args()
    report = {}
    for act in ([a.act] if a.act else sorted(ACTS)):
        stats, rejects, worth, usefreq, present, tried = measures(act, a.seeds, a.runs, a.show, a.draw)
        m = lambda k: statistics.mean(stats[k]) if stats[k] else 0
        print('act %d (BN5 mission %d): %d missions of %d tried, rejected %s' % (act, ACTS[act]['bn5'], len(stats['par']), tried, dict(rejects)))
        if not stats['par']:
            continue
        print('  panels %.0f, dark %.0f%%, par %.1f (%d-%d), barriers %.1f, sealed routes %.0f%%' % (
            m('panels'), 100 * m('dark'), m('par'), min(stats['par']), max(stats['par']), m('bars'), 100 * m('secret')))
        print('  the planner: two under par %.0f%%, at par %.0f%%, over %.0f%%; close calls %.0f%% of decisions; first holes taken %.1f ways' % (
            100 * m('best'), 100 * m('mid'), 100 * m('over'), 100 * m('close'), m('orders')))
        print('  phases an ability saves (its Navi without it), and the missions it was used in:')
        for n in sorted(worth, key=lambda n: -statistics.mean(worth[n])):
            print('    %-12s %+5.2f  used in %3.0f%%  (%d missions)' % (n, statistics.mean(worth[n]), 100 * usefreq[n] / present[n], present[n]))
        report[act] = dict(stats={k: m(k) for k in stats}, rejects=dict(rejects), worth={n: statistics.mean(v) for n, v in worth.items()})
    if a.json:
        with open(a.json, 'w') as f: json.dump(report, f, indent=1)
    return 0


if __name__ == '__main__':
    sys.exit(main())
