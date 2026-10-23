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
against the fairness rules and rejected where one fails. Its shape and its
dark keep to bands measured on BN5's own boards (fields, fill, the dark
around a dark panel). A planner then plays each mission many times by BN5's
rules, abstracted (a team acting once a phase, three-turn liberations, the
one-turn bonus, Order Points, the enemy's phase): its phases set the
mission's par, and the board is kept only where every teammate's command
earns its Order Point. Prints the measures the design promises over the
seeds: phases against par, every command's worth, route variety, the bands,
rejections. Numbers only; nothing of BN5's is read here
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

FREE, DARK, ITEM, BONUS, HOLE, RING, BOSS, BAR1, BAR2, BAR3, START, TRAP = range(12)
BARS = (BAR1, BAR2, BAR3)
DIRS = ((1, 0), (0, 1), (-1, 0), (0, -1))
DARKS = (DARK, ITEM, HOLE, RING, TRAP)
AROUND = tuple((dx, dy) for dx in (-1, 0, 1) for dy in (-1, 0, 1) if dx or dy)

# The staircase (docs/LIBERATION.md, The quality bar, 3): each act one rule more, its new
# question. Act 2 the basics (dark panels, DarkHoles, the Darkloid, Order Points, hearts);
# 'barriers' (act 3) the sealed region behind a BarrierPanel, its key off the way and its route;
# 'reach' (act 4) roaming guardians and a barrier across the way; 'tolls' (act 5) trap panels
# shown on the way and Bladia's darkening; 'maze' (act 6) a second barrier, its key past the
# first. Traps' harm: BN5's 50 in missions 5-6, 100 later.
TRAP_HARM = {5: 50, 6: 100}
TRAPS = {5: 4, 6: 4}
# Per act of the endless net (docs/PROGRESSION.md), the BN5 mission it stands
# for and the sizes BN5's missions have there (docs/LIBERATION.md, The
# missions): panels, DarkHoles, team, barrier kinds, the share of dark panels,
# items, BonusPanels, guardians beyond one a hole, the enemies' damage and HP.
# The layout's and the dark's bands are measured on BN5's own boards
# (tools/liberation_bn5.py): `fields` the share of panels in a 2x2 block of
# floor, `fill` the floor's share of its box, `dense` the dark panels around a
# dark panel, on average; `plat` the platforms' sides, `wide` the share of
# walkways two panels wide.
ACTS = {
    2: dict(bn5=1, rules=(), panels=(90, 112), holes=2, team=2, bars=0, dark=(0.28, 0.45), items=7, bonus=2, roam=0,
            plat=((3, 5), (3, 4)), wide=0.0, gap=2, fill=0.26, fields=(0.68, 0.86), dense=(3.3, 4.3),
            kinds=('brute',), hp=200, ally_hp=200, virus=14, boss_hp=300, boss_range=1, boss_dmg=30),
    3: dict(bn5=3, rules=('barriers',), panels=(165, 215), holes=3, team=3, bars=1, dark=(0.45, 0.58), items=8, bonus=3, roam=0,
            plat=((3, 6), (3, 5)), wide=0.2, gap=2, fill=0.27, fields=(0.70, 0.88), dense=(3.4, 4.5),
            kinds=('brute',), hp=300, ally_hp=300, virus=20, boss_hp=600, boss_range=2, boss_dmg=50),
    4: dict(bn5=5, rules=('barriers', 'reach'), panels=(210, 245), holes=4, team=4, bars=2, dark=(0.52, 0.66), items=12, bonus=3, roam=1,
            plat=((4, 8), (4, 6)), wide=0.6, gap=1, fill=0.42, fields=(0.86, 1.0), dense=(4.6, 6.2),
            kinds=('brute', 'hawk'), hp=400, ally_hp=350, virus=28, boss_hp=900, boss_range=3, boss_dmg=80),
    5: dict(bn5=7, rules=('barriers', 'reach', 'tolls'), panels=(215, 255), holes=5, team=5, bars=2, dark=(0.58, 0.68), items=16, bonus=3, roam=2,
            plat=((4, 8), (4, 6)), wide=0.6, gap=1, fill=0.44, fields=(0.86, 1.0), dense=(4.6, 6.2),
            kinds=('brute', 'hawk', 'bladia'), hp=500, ally_hp=450, virus=34, boss_hp=1200, boss_range=2, boss_dmg=80),
    6: dict(bn5=9, rules=('barriers', 'reach', 'tolls', 'maze'), panels=(220, 260), holes=5, team=5, bars=3, dark=(0.55, 0.68), items=14, bonus=3, roam=2,
            plat=((4, 8), (4, 6)), wide=0.6, gap=1, fill=0.45, fields=(0.86, 1.0), dense=(4.2, 6.2),
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
# The commands whose one-turn win still frees the panels around (Casebier's guides except
# Napalm, PanelSearch, TwinLiberation and TomahawkSwing from it; assumed for the rest).
BONUS_KEPT = ('longswrd', 'wideswrd', 'screndiv')
# The planner's skill: a liberation battle's chance to be won in one turn on a panel with
# no dark around, and to fail (each dark panel around takes 0.05 and adds 0.035).
SKILL = [0.75, 0.04]
# BN5's par over the planner's median phases on BN5's own boards, per act from the missions it
# stands for (tools/liberation_bn5.py prints them): the planner plays slower than BN5's par
# expects, so a generated mission's par is its median phases times its act's, BN5's yardstick.
PAR_SCALE = {2: 0.58, 3: 0.67, 4: 0.70, 5: 0.82, 6: 0.79}
# The command rule: a board is kept only where every teammate's command saves at least this
# many phases (scaled as the par) against the same Navi without it, the same plays paired.
EARN = -0.1
USE_MIN = {2: 0.5, 3: 0.5, 4: 0.25, 5: 0.25, 6: 0.25}   # ... and only where the planner uses every teammate's command,
# half a time a play at least, a quarter with four or five Navis to share the Order Points
# The quality bar's measures (docs/LIBERATION.md, The quality bar, 1 and 6): a board is rejected
# where more than this share of its decisions on the way have one sensible move, or where its
# sealed region's route costs the route's player fewer phases than ROUTE_COST's first (a gift,
# not a choice) or more than its second (a wall, not a choice), scaled as the par.
DEAD_MAX = 0.55
ROUTE_COST = (1.0, 8.0)
# The utility commands pay in safety and loot, which phases do not show (SneakAttack: a guardian
# weakened with no battle; NumberCheck: items taken and traps disarmed with none): held only to
# being used where their situation is and to costing the team no more than this.
UTILITY = ('sneak', 'numbercheck')
UTILITY_EARN = -0.25


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
        self.secret = None       # the sealed region's route panel, if any
        self.sealed = 0          # its barrier's number
        self.regions = set()     # the panels behind the sealed barrier
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
    area = want / spec['fill']
    w = int(math.sqrt(area * 1.4))
    h = int(area / w) + 2
    floor, plat = set(), set()
    rects = []
    tries = 0
    (pw0, pw1), (ph0, ph1) = spec['plat']
    g = spec['gap']
    share = 0.55 if spec['wide'] < 0.5 else 0.7
    while len(floor) < want * share and tries < 600:
        tries += 1
        rw, rh = rng.randint(pw0, pw1), rng.randint(ph0, ph1)
        rx, ry = rng.randint(4, max(4, w - rw - 5)), rng.randint(1, max(1, h - rh - 1))
        if any(rx < x + ww + g and x < rx + rw + g and ry < y + hh + g and y < ry + rh + g for x, y, ww, hh in rects):
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
    for i, j in edges:
        path = carve_line(floor, nodes[i], nodes[j], rng)
        if rng.random() < spec['wide']:
            # A field's way two panels wide, as BN5's later boards run (docs/LIBERATION.md, The board).
            for k, c in enumerate(path):
                nxt = path[k + 1] if k + 1 < len(path) else path[k - 1]
                if nxt[0] != c[0]: floor.add((c[0], c[1] + 1))
                else: floor.add((c[0] + 1, c[1]))
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


def density(b):
    """The dark panels around a dark panel, on average (of eight): how hard a liberation is."""
    ds = [sum(1 for dx, dy in AROUND if b.cells.get((c[0] + dx, c[1] + dy)) in (DARK, ITEM, HOLE, RING))
          for c, k in b.cells.items() if k in (DARK, ITEM)]
    return sum(ds) / len(ds) if ds else 0.0


def fields(cells):
    """The share of panels in a 2x2 block of floor, and the floor's share of its box."""
    fl = set(cells)
    blocks = set()
    for (x, y) in fl:
        if {(x + 1, y), (x, y + 1), (x + 1, y + 1)} <= fl:
            blocks |= {(x, y), (x + 1, y), (x, y + 1), (x + 1, y + 1)}
    xs = [c[0] for c in fl]; ys = [c[1] for c in fl]
    return len(blocks) / len(fl), len(fl) / ((max(xs) - min(xs) + 1) * (max(ys) - min(ys) + 1))


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


def generate(act, seed, team=None):
    """One mission for this team (the run's Souls when the layer is made), or None with the
    reason it was rejected."""
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
    deep = spec['dense'][0] >= 4.5
    for i, h in enumerate(holes):
        b.cells[h] = HOLE
        for dx, dy in AROUND: b.cells[(h[0] + dx, h[1] + dy)] = DARK
        if deep:  # a second ring where the floor runs on (BN5's later holes sit in fields of dark)
            for dx in range(-2, 3):
                for dy in range(-2, 3):
                    c = (h[0] + dx, h[1] + dy)
                    if b.cells.get(c) == FREE and rng.random() < 0.8: b.cells[c] = DARK
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
    if deep:  # the Darkloid's approach dark two deep
        bx, by = boss
        for dx in range(-3, 4):
            for dy in range(-3, 4):
                c = (bx + dx, by + dy)
                if b.cells.get(c) == FREE and reach.get(c, 0) > 2: b.cells[c] = DARK
    # The density rule: majority passes until a dark panel has as many dark around it as
    # BN5's boards of this act have (the act's `dense` band), so a one-turn win is no sure thing
    # and the commands earn their Order Points.
    lo_d, hi_d = spec['dense']
    for _ in range(8):
        dd = density(b)
        if lo_d <= dd <= hi_d: break
        for c in free:
            if reach[c] <= 2 or b.cells[c] not in (FREE, DARK): continue
            darks = sum(1 for dx, dy in AROUND if b.cells.get((c[0] + dx, c[1] + dy)) in (DARK, HOLE))
            floors = sum(1 for dx, dy in AROUND if (c[0] + dx, c[1] + dy) in b.cells)
            if not floors: continue
            if dd < lo_d and b.cells[c] == FREE and darks / floors >= 0.45 and rng.random() < 0.6: b.cells[c] = DARK
            elif dd < lo_d and b.cells[c] == DARK and darks / floors < 0.25 and rng.random() < 0.6: b.cells[c] = FREE
            elif dd > hi_d and b.cells[c] == DARK and darks / floors > 0.8 and rng.random() < 0.3: b.cells[c] = FREE
    if not lo_d <= density(b) <= hi_d:
        return None, 'density'
    # BonusPanels at the ends of spurs (BN5's "?" at the end of its stubs).
    stubs = [e for e in ends if b.cells.get(e) in (FREE, DARK)]
    rng.shuffle(stubs)
    for e in stubs[:spec['bonus']]: b.cells[e] = BONUS
    # The staircase's barriers (docs/LIBERATION.md, The quality bar, 3 and 6).
    why = barriers(b, rng, reach)
    if why:
        return None, why
    # Items in the dark: corners and dead ends first (BN5 tucks them there).
    darks = [c for c, kk in b.cells.items() if kk == DARK and c not in b.items]
    darks.sort(key=lambda c: (len(b.neighbours(c)), rng.random()))
    mix = item_mix(act, rng, spec['items'])
    for c, it in zip(darks[:len(mix) * 2:2] + darks[1:len(mix) * 2:2], mix):
        b.cells[c] = ITEM
        b.items[c] = it
    loot(b, rng)
    if 'tolls' in spec['rules']:
        traps(b, rng, TRAPS[act])
        if not any(k == TRAP for k in b.cells.values()):
            return None, 'traps'
    for i in range(spec['roam']):
        spots = [c for c, kk in b.cells.items() if kk == DARK and reach.get(c, 0) > 0.4 * far and c not in b.regions]
        if spots:
            b.guards.append(dict(kind=rng.choice(spec['kinds'][1:] or spec['kinds']), pos=rng.choice(spots), hole=None))
    if team:
        why = situations(b, team, rng, reach, far)
        if why:
            return None, why
    why = check(b)
    if why:
        return None, why
    return b, None


def cost_from(b, src):
    """Per panel, the dark panels to liberate on the way from src (a 0-1 walk; barriers shut)."""
    dist = {c: 0 for c in src}
    dq = collections.deque(src)
    while dq:
        c = dq.popleft()
        for n in b.neighbours(c):
            k = b.cells[n]
            if k in BARS: continue
            w = 1 if k in DARKS else 0
            if dist[c] + w < dist.get(n, 1 << 30):
                dist[n] = dist[c] + w
                (dq.appendleft if w == 0 else dq.append)(n)
    return dist


def way_on(b):
    """The panels on a cheapest way from the start to a DarkHole: where the team must go."""
    ds = cost_from(b, b.start)
    way = set()
    for h in b.holes:
        dh = cost_from(b, [h])
        if h not in ds: continue
        best = ds[h]
        way |= {c for c in b.cells if c in ds and c in dh and ds[c] + dh[c] <= best + 1}
    return way


def situations(b, team, rng, reach, far):
    """Each teammate's command its situation on this board (docs/LIBERATION.md, The command
    rule): a guardian in the way for SneakAttack, an item cache for NumberCheck, a dark line of
    five for LifeMelody, rows and blocks of dark for ScrenDiv and TomahawkSwing."""
    abilities = {ABILITIES[n] for n in team}
    way = way_on(b)
    near_hole = {(h[0] + dx, h[1] + dy) for h in b.holes for dx in range(-2, 3) for dy in range(-2, 3)}
    taken = {g['pos'] for g in b.guards}
    if 'sneak' in abilities:
        spots = [c for c in way if b.cells[c] == DARK and c not in near_hole and c not in taken
                 and reach.get(c, 0) > 0.35 * far and len(b.neighbours(c)) <= 3]
        if not spots:
            return 'situation-sneak'
        c = rng.choice(spots)
        b.guards.append(dict(kind=rng.choice(ACTS[b.act]['kinds']), pos=c, hole=None, blocks=True))
        taken.add(c)
    if 'numbercheck' in abilities:
        blocks = []
        for (x, y), k in b.cells.items():
            for w, h, stands in ((3, 2, ((x + 1, y - 1), (x + 1, y + 2))), (2, 3, ((x - 1, y + 1), (x + 2, y + 1)))):
                cells = [(x + i, y + j) for i in range(w) for j in range(h)]
                if all(b.cells.get(c) == DARK and c not in taken and c not in near_hole for c in cells) \
                        and any(b.cells.get(c) in (FREE, START) or (b.cells.get(c) == DARK and any(
                            b.cells.get(n) in (FREE, START) for n in b.neighbours(c))) for c in stands if c in b.cells):
                    blocks.append(cells)
        if not blocks:
            return 'situation-number'
        cache = rng.choice(blocks)
        kinds = [('op', 3), ('heart', 1), ('zenny', 1), ('chip', 1)]
        if b.act >= 4: kinds[2] = ('trap', 1)
        for c, it in zip(rng.sample(cache, 4), kinds):
            b.cells[c] = ITEM
            b.items[c] = it
    if 'melody' in abilities:
        lines = []
        for (x, y) in b.cells:
            for dx, dy in DIRS[:2]:
                run = [(x + dx * i, y + dy * i) for i in range(5)]
                if all(b.cells.get(c) in (DARK, FREE, ITEM) for c in run) and sum(c in way for c in run) >= 3 \
                        and all(reach.get(c, 0) > 2 for c in run) and not any(c in taken for c in run):
                    lines.append(run)
        if not lines:
            return 'situation-melody'
        for c in rng.choice(lines):
            if b.cells[c] == FREE: b.cells[c] = DARK
    for ab, cells_of, need in (('screndiv', lambda c, d: shape('screndiv', c, d), 3), ('tomahawk', lambda c, d: shape('tomahawk', c, d), 3)):
        if ab not in abilities: continue
        spots = sum(1 for c in b.cells for d in DIRS
                    if b.cells[c] in (FREE, START) and all(b.cells.get(t) in (DARK, ITEM) for t in cells_of(c, d)))
        spots += sum(1 for c in way for d in DIRS if b.cells.get(c) in (DARK, ITEM) and all(b.cells.get(t) in (DARK, ITEM) for t in cells_of(c, d)))
        if spots < need:
            return 'situation-' + ab
    return None


# What BN5's item panels of the missions an act stands for hold, counted from its records
# (tools/liberation_bn5.py): hearts, Order Points items, zenny, chips, BugFrags, traps (the keys
# come with the barriers). Its later missions hold 12-13 Order Points in items for five Navis.
ITEMS = {
    2: dict(heart=2, op=2, zenny=2, chip=1, bugfrag=0, trap=0),
    3: dict(heart=2, op=3, zenny=1, chip=2, bugfrag=0, trap=0),
    4: dict(heart=2, op=4, zenny=3, chip=2, bugfrag=1, trap=7),
    5: dict(heart=2, op=4, zenny=4, chip=3, bugfrag=2, trap=8),
    6: dict(heart=2, op=4, zenny=6, chip=2, bugfrag=1, trap=6),
}


def item_mix(act, rng, n=None):
    """What the item panels hold, BN5's counts for the act (docs/LIBERATION.md, Item panels)."""
    out = []
    for kind, count in ITEMS[act].items():
        for _ in range(count):
            out.append((kind, rng.choice((2, 3)) if kind == 'op' else 1))
    rng.shuffle(out)
    return out



def way_cells(b):
    """The panels on a cheapest way from the start to each DarkHole (within one dark panel)."""
    return way_on(b)


def barriers(b, rng, reach):
    """The act's barriers: the sealed region's (act 3 on: five to sixteen panels cut off by its
    own wall, nothing of the board's in them, its key off the way by two, its route at the far
    end), one across the way (act 4 on: a way round it, its key off the way), and the maze's
    second (act 6: its key past the first). A reason to reject, or None."""
    rules = ACTS[b.act]['rules']
    if 'barriers' not in rules:
        return None
    way = way_cells(b)
    cut = find_cut(b, rng)
    if not cut:
        return 'barrier'
    wall, region = cut
    near = graph_dist(b, b.start, lambda kk: kk not in BARS)
    spots = [c for c in near if b.cells[c] == DARK and c not in region and c not in wall and near[c] >= 4
             and off_way(way, c) >= 2]
    if not spots:
        return 'key'
    n = 1
    for c in wall: b.cells[c] = BARS[n - 1]
    key = rng.choice(spots)
    b.cells[key] = ITEM
    b.items[key] = ('key', n)
    b.sealed, b.regions = n, set(region)
    for c in region:
        if b.cells[c] == FREE and rng.random() < 0.8: b.cells[c] = DARK
    gate = max(region, key=lambda c: min(abs(c[0] - w[0]) + abs(c[1] - w[1]) for w in wall))
    b.cells[gate] = ITEM
    b.items[gate] = ('route', 1)
    b.secret = gate
    if 'reach' in rules and not across(b, rng, way, 2, past=False) and not across(b, rng, way, 2, past=False, cut_ok=True):
        return 'barrier'
    if 'maze' in rules and not any(across(b, rng, way, 3, past=p, cut_ok=k) for p, k in ((True, False), (False, False), (False, True))):
        return 'barrier'
    return None


def off_way(way, c):
    return min((abs(c[0] - w[0]) + abs(c[1] - w[1]) for w in way), default=99)


def across(b, rng, way, n, past, cut_ok=False):
    """Barrier n across a cheapest way: every target still reached round it (with `cut_ok`, a
    DarkHole may lie behind it alone, BN5's own barrier), its key off the way (the maze's, past
    the barrier before: the way to it longer with that one shut)."""
    cand = [c for c in way if b.cells.get(c) in (FREE, DARK) and c not in b.regions and c not in b.start
            and not any(abs(c[0] - h[0]) <= 1 and abs(c[1] - h[1]) <= 1 for h in b.holes)
            and abs(c[0] - b.boss[0]) + abs(c[1] - b.boss[1]) > 2 and not any(g['pos'] == c for g in b.guards)]
    rng.shuffle(cand)
    for c in cand[:60]:
        saved = b.cells[c]
        b.cells[c] = BARS[n - 1]
        side = graph_dist(b, b.start, lambda kk: kk not in BARS)
        # (BN5's own where the floor has no way round, as the prototype's tree-like boards
        # mostly have none: a DarkHole walled off, its key on the near side, never the Darkloid)
        if b.boss not in side or (not cut_ok and any(h not in side for h in b.holes)):
            b.cells[c] = saved
            continue
        opened = graph_dist(b, b.start, lambda kk: kk not in BARS or kk == BARS[n - 2]) if past else side
        spots = [x for x in side if b.cells[x] == DARK and x not in b.regions and side[x] >= 4 and off_way(way, x) >= 1
                 and (not past or side[x] > opened.get(x, 99))]
        if not spots:
            b.cells[c] = saved
            continue
        key = rng.choice(spots)
        b.cells[key] = ITEM
        b.items[key] = ('key', n)
        return True
    return False


def loot(b, rng):
    """The act's chips and zenny, off the way (a detour's worth)."""
    way = way_cells(b)
    spots = [c for c, k in b.cells.items() if k == DARK and c not in b.regions and off_way(way, c) >= 1]
    rng.shuffle(spots)
    n = {2: 1, 3: 2, 4: 2, 5: 3, 6: 3}[b.act]
    for i, c in enumerate(spots[:n]):
        b.cells[c] = ITEM
        b.items[c] = ('chip', 1) if i % 2 == 0 else ('zenny', 2 + b.act)


def traps(b, rng, want):
    """Trap panels on the way, shown (BN5 hides them; the quality bar's telegraph shows them),
    each with a way round a dark panel or two longer: the toll or the detour."""
    way = [c for c in way_cells(b) if b.cells.get(c) == DARK and c not in b.regions
           and abs(c[0] - b.start[0][0]) + abs(c[1] - b.start[0][1]) >= 4
           and not any(abs(c[0] - h[0]) <= 1 and abs(c[1] - h[1]) <= 1 for h in b.holes)]
    rng.shuffle(way)
    base = cost_from(b, b.start)
    best = min(base.get(h, 99) for h in b.holes)
    got = 0
    for c in way:
        if got >= want: break
        b.cells[c] = BAR3
        round_ = cost_from(b, b.start)
        b.cells[c] = DARK
        if min(round_.get(h, 99) for h in b.holes) <= best + 2:
            b.cells[c] = TRAP
            got += 1


def find_cut(b, rng, min_region=5):
    """One or two walkway panels that cut five to thirty panels off by themselves, nothing of the
    board's (holes, the Darkloid, items, guardians, the start) in them: the sealed region."""
    walk = [c for c, k in b.cells.items() if k in (FREE, DARK) and len(b.neighbours(c)) <= 3 and c not in b.start
            and not any(abs(c[0] - h[0]) <= 1 and abs(c[1] - h[1]) <= 1 for h in b.holes)]
    rng.shuffle(walk)
    walls = []
    for c in walk[:200]:
        walls.append([c])
        for d in ((1, 0), (0, 1)):
            n = (c[0] + d[0], c[1] + d[1])
            if b.cells.get(n) in (FREE, DARK): walls.append([c, n])
    rng.shuffle(walls)
    guards = {g['pos'] for g in b.guards}
    for wall in walls[:400]:
        saved = [b.cells[c] for c in wall]
        for c in wall: b.cells[c] = BAR1
        side = graph_dist(b, b.start, lambda k: k not in BARS)
        for c, k in zip(wall, saved): b.cells[c] = k
        region = [x for x in b.cells if x not in side and x not in wall]
        if not min_region <= len(region) <= 30 or b.boss in region:
            continue
        if any(b.cells[x] in (HOLE, ITEM, RING, BOSS, START) or x in guards for x in region):
            continue
        return wall, set(region)
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
        self.yields = collections.Counter()
        self.liberated_by = collections.Counter()
        self.shield = False
        self.melody = set()
        self.holes_order = []
        self.decisions = 0
        self.close_calls = 0
        self.taste = 0           # the route's player (1) goes for the sealed region's route first
        self.moves = 0           # sensible moves over the decisions (docs/LIBERATION.md, The quality bar, 1)
        self.dead = 0            # decisions with one sensible move, strikes apart
        self.strikes = 0         # ... a strike at a DarkHole or the Darkloid being the best
        self.best = collections.Counter()
        self.route_taken = False

    # The board's state
    def holes_left(self):
        return [c for c, k in self.cells.items() if k == HOLE]

    def walkable(self, c):
        return self.cells.get(c) in (FREE, START, BONUS)

    def route_targets(self):
        """The route's player's targets: the sealed barrier's key, then the route."""
        if not self.taste or not self.b.sealed: return []
        key = [c for c, it in self.items.items() if it == ('key', self.b.sealed)]
        if key: return key
        return [c for c, it in self.items.items() if it[0] == 'route']

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

    def blocking(self):
        """Per guardian, the dark panels more the way on costs with him standing in it (his panel
        cannot be liberated past him, only fought): what deleting him is worth."""
        out = {}
        if not self.guards: return out
        open_cost = self.target_cost()
        front = [c for n in self.navis for c in self.reach(n)]
        base = min((open_cost.get(c, 99) for c in front), default=99)
        for g in self.guards:
            walled = self.target_cost(walls={g['pos']})
            out[g['pos']] = max(0, min((walled.get(c, 99) for c in front), default=99) - base)
        return out

    def target_cost(self, walls=()):
        """Per panel, the dark panels still to liberate from it to the nearest target (Dijkstra, 0-1)."""
        holes = self.holes_left()
        targets = self.route_targets() or holes or [self.b.boss]
        dist = {t: 0 for t in targets}
        dq = collections.deque(targets)
        while dq:
            c = dq.popleft()
            for n in self.b.neighbours(c):
                k = self.cells[n]
                if k in BARS or n in walls: continue
                w = 1 if k in (DARK, ITEM, RING, TRAP) else 0
                nd = dist[c] + w
                if nd < dist.get(n, 1 << 30):
                    dist[n] = nd
                    (dq.appendleft if w == 0 else dq.append)(n)
        return dist

    def libable(self, c):
        k = self.cells.get(c)
        if k in (DARK, ITEM, TRAP): return True
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
            elif k in (DARK, ITEM, RING, TRAP):
                v += 3.0 if cost.get(c, 99) < here or cost.get(c, 99) <= 1 else 0.6
                if k == TRAP:
                    v -= 6.0 * TRAP_HARM.get(self.act, 50) / self.spec['hp']
                if k == ITEM:
                    it = self.items.get(c, ('', 0))
                    sealed = it == ('key', self.b.sealed) or it[0] == 'route'
                    v += 4.0 * self.taste if sealed else {'op': 2.5 if self.op < 6 else 0.5, 'key': 4.0, 'heart': 1.5, 'chip': 1.2}.get(it[0], 0.8)
            elif k == HOLE:
                v += 14.0
        return v

    def difficulty(self, c):
        return sum(1 for dx, dy in AROUND if self.cells.get((c[0] + dx, c[1] + dy)) in DARKS)

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

    def sneak_damage(self):
        """SneakAttack's strike: a hundred for every two missions (BN5's guides), 500 at the most."""
        return min(500, 100 * max(1, self.spec['bn5'] // 2))

    def fight_damage(self, top, navi):
        """What a battle against a guardian takes off him in its three turns."""
        return top * self.rng.uniform(0.55, 1.25) * (1.15 if navi['name'] == 'MegaMan' else 1.0)

    def kill_chance(self, hp, top, navi):
        lo, hi = 0.55 * top, 1.25 * top * (1.15 if navi['name'] == 'MegaMan' else 1.0)
        return 1.0 if hp <= lo else 0.0 if hp >= hi else (hi - hp) / (hi - lo)

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
            elif it == 'route': self.route_taken = True
        if k == TRAP:
            self.hurt(navi, TRAP_HARM.get(self.act, 50))
        if k == HOLE:
            self.holes_order.append(c)
            self.guards = [g for g in self.guards if g['hole'] is None or self.b.holes[g['hole']] != c]
        if k in DARKS:
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
                if self.cells.get(n) in (DARK, ITEM, TRAP): self.take(navi, n)

    def options(self, navi, cost):
        """Every move-and-act the Navi has this phase, valued."""
        opts = []
        ab = navi['ab']
        for p in self.reach(navi):
            for d in DIRS:
                t = (p[0] + d[0], p[1] + d[1])
                guard = next((g for g in self.guards if g['pos'] == t), None)
                if guard:
                    worth = 0.6 * self.value([t], cost) + 3.0 + 2.5 * min(4, self.blocks.get(t, 0))
                    opts.append((worth * self.kill_chance(guard['hp'], guard['max'], navi) + 0.5, 'fight', p, d))
                    if ab == 'sneak' and self.op and guard['hp'] > 1:
                        # SneakAttack leaves him weaker for the teammates still to act this phase.
                        after = max(1, guard['hp'] - self.sneak_damage())
                        gain = max(self.kill_chance(after, guard['max'], m) - self.kill_chance(guard['hp'], guard['max'], m)
                                   for m in self.navis if m is not navi and not m.get('acted') and not m['down']) \
                            if any(m is not navi and not m.get('acted') and not m['down'] for m in self.navis) else 0.0
                        opts.append((worth * gain - 1.0, 'sneak', p, d))
                    continue
                g2 = next((g for g in self.guards if g['pos'] == (p[0] + 2 * d[0], p[1] + 2 * d[1])), None)
                if ab == 'sneak' and self.op and g2 and t in self.cells and g2['hp'] > 1 and not guard:
                    worth2 = 0.6 * self.value([g2['pos']], cost) + 3.0 + 2.5 * min(4, self.blocks.get(g2['pos'], 0))
                    after = max(1, g2['hp'] - self.sneak_damage())
                    mates = [m for m in self.navis if m is not navi and not m.get('acted') and not m['down']]
                    if mates:
                        gain = max(self.kill_chance(after, g2['max'], m) - self.kill_chance(g2['hp'], g2['max'], m) for m in mates)
                        opts.append((worth2 * gain - 1.0, 'sneak2', p, d))
                if ab == 'sneak' and self.op and not self.holes_left():
                    for k in (1, 2):
                        if (p[0] + d[0] * k, p[1] + d[1] * k) == self.b.boss and self.boss_hp > 1:
                            opts.append((min(8.0, 8.0 * self.sneak_damage() / self.spec['boss_hp'] * 3) - 1.0, 'sneak_boss', p, d))
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
                        v = self.value(cells, cost)
                        if ab in BONUS_KEPT:
                            v += p1 * self.value([c for c in around if c not in cells], cost) * 0.8
                        opts.append((v * (1 - 0.035 * self.difficulty(t)) - 1.5, 'ability', p, d))
                if self.op and ab == 'numbercheck':
                    worth = sum({'op': 2.5 if self.op < 6 else 0.5, 'key': 4.0, 'heart': 1.5, 'trap': 2.0}.get(self.items[c][0], 0.8)
                                for c in shape(ab, p, d) if self.cells.get(c) == ITEM)
                    worth += 2.0 * sum(1 for c in shape(ab, p, d) if self.cells.get(c) == TRAP)
                    if worth:
                        opts.append((worth - 1.0, 'number', p, d))
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
            # LifeMelody: a teammate beside him liberates five ahead this phase, over anything.
            best = 0.0
            for m in self.navis:
                if m is navi or m.get('acted') or m['down']: continue
                if abs(m['pos'][0] - navi['pos'][0]) > 1 or abs(m['pos'][1] - navi['pos'][1]) > 1: continue
                for d in DIRS:
                    line = [c for c in shape('melody', m['pos'], d) if self.libable(c)]
                    best = max(best, self.value(line, cost) * 0.6)
            if best:
                opts.append((best - 1.5, 'melody', navi['pos'], None))
        if navi['hp'] < navi['max'] * 0.4:
            opts.append((5.0 + 6.0 * (1 - navi['hp'] / navi['max']), 'rest', navi['pos'], None))
        return opts

    def purpose(self, kind, p, d):
        """A move's class: its kind and what it is for (the DarkHole it heads to, the Darkloid, a
        guardian, a rest), and for a liberation whether it is a bold one (four or more dark round
        it: the one-turn win's gamble)."""
        t = (p[0] + d[0], p[1] + d[1]) if d else p
        if kind in ('rest', 'barrier', 'melody'): return (kind,)
        if kind in ('fight', 'sneak', 'sneak2'): return (kind, t)
        if kind in ('boss', 'sneak_boss'): return (kind,)
        holes = self.holes_left()
        aim = min(range(len(holes)), key=lambda i: abs(holes[i][0] - t[0]) + abs(holes[i][1] - t[1])) if holes else -1
        it = self.items.get(t, ('',))[0] if self.cells.get(t) == ITEM else ''
        return (kind, aim, it, self.difficulty(t) >= 4)

    def sensible(self, opts):
        """The decision's sensible moves: of different class, four fifths of the best or more."""
        best = opts[0]
        if best[0] <= 0: return
        seen = {self.purpose(o[1], o[2], o[3]) for o in opts if o[0] >= 0.8 * best[0]}
        self.moves += len(seen)
        t = (best[2][0] + best[3][0], best[2][1] + best[3][1]) if best[3] else None
        strike = best[1] == 'boss' or (best[1] == 'lib' and self.cells.get(t) == HOLE)
        self.strikes += strike
        self.dead += len(seen) <= 1 and not strike
        self.best[best[1]] += 1

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
        self.blocks = self.blocking()
        opts = self.options(navi, cost)
        if not opts:
            return
        opts.sort(key=lambda o: -o[0])
        self.decisions += 1
        if len(opts) > 1 and opts[1][0] >= 0.9 * opts[0][0] and opts[1][2:] != opts[0][2:]:
            self.close_calls += 1
        self.sensible(opts)
        v, kind, p, d = opts[0]
        navi['pos'] = p
        t = (p[0] + d[0], p[1] + d[1]) if d else None
        if kind == 'lib':
            self.liberate(navi, [t])
        elif kind == 'long':
            self.liberate(navi, shape('melody', p, d), bonus=False)
        elif kind == 'ability':
            self.op -= 1; self.uses[navi['name']] += 1
            self.liberate(navi, [t] + [c for c in shape(navi['ab'], p, d) if c != t], bonus=navi['ab'] in BONUS_KEPT)
        elif kind == 'number':
            self.op -= 1; self.uses[navi['name']] += 1
            self.yields['NumberCheck items'] += sum(1 for c in shape('numbercheck', p, d) if self.cells.get(c) == ITEM)
            for c in shape('numbercheck', p, d):
                if self.cells.get(c) == TRAP:
                    self.cells[c] = DARK   # (disarmed: NumberCheck's niche on the tolls' boards)
                    continue
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
                    self.yields['SneakAttack HP'] += g['hp'] - max(1, g['hp'] - self.sneak_damage())
                    g['hp'] = max(1, g['hp'] - self.sneak_damage())
                else:
                    navi['in_battle'] = True
                    self.battle(navi, t, guard=g)
                    navi['in_battle'] = False
                    # Liberating a guardian's panel is a fight with him, his HP carried: deleted
                    # within the three turns, the panel is liberated and his DarkHole sends another
                    # in the enemy's phase; not, he keeps what he has left.
                    g['hp'] -= int(self.fight_damage(g['max'], navi))
                    if g['hp'] <= 0:
                        self.guards.remove(g)
                        self.take(navi, t)
                        if g['hole'] is not None and self.cells.get(self.b.holes[g['hole']]) == HOLE:
                            self.respawn.append(dict(g, hp=g['max']))
        elif kind == 'sneak2':
            g = next((g for g in self.guards if g['pos'] == (p[0] + 2 * d[0], p[1] + 2 * d[1])), None)
            if g:
                self.op -= 1; self.uses[navi['name']] += 1
                self.yields['SneakAttack HP'] += g['hp'] - max(1, g['hp'] - self.sneak_damage())
                g['hp'] = max(1, g['hp'] - self.sneak_damage())
        elif kind == 'sneak_boss':
            self.op -= 1; self.uses[navi['name']] += 1
            self.yields['SneakAttack HP'] += self.boss_hp - max(1, self.boss_hp - self.sneak_damage())
            self.boss_hp = max(1, self.boss_hp - self.sneak_damage())
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
                steps = [n for n in self.b.neighbours(g['pos']) if self.cells[n] in (DARK, ITEM, TRAP) and n not in self.occupied()]
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
            # Support first (LifeMelody, SneakAttack), then MegaMan, then the rest.
            order = sorted(self.navis, key=lambda n: (ABILITIES[n['name']] not in ('melody', 'sneak'), n['name'] != 'MegaMan'))
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


def play(b, team, runs, seed, ban=None, taste=0):
    """Phases to clear over runs of the planner, abilities used, holes' order (with `taste`, a
    player who goes for the sealed region's route first)."""
    out = []
    for r in range(runs):
        rng = random.Random((seed * 31 + r) * 104729 + taste)
        s = Sim(b, team, rng, b.act)
        s.taste = taste
        if ban:
            for n in s.navis:
                if n['name'] == ban: n['ab'] = 'none'
        p = s.run()
        out.append((p, s))
    return out


def command_worth(b, team, runs, res):
    """Per Navi, the phases (scaled) his command saves: the planner's plays with it against
    the same plays with him and no command (the same random numbers, so paired)."""
    n_ban = max(8, runs // 2)
    base = [p if p else 40 for p, _ in res[:n_ban]]
    out = {}
    for n in team:
        ban = play(b, team, n_ban, b.seed, ban=n)
        out[n] = PAR_SCALE[b.act] * (statistics.mean(p if p else 40 for p, _ in ban) - statistics.mean(base))
    return out


def set_par(b, team, runs=24):
    res = play(b, team, runs, b.seed)
    ph = sorted(p for p, _ in res if p)
    if len(ph) < runs * 0.6:  # BN5's own mission 1 the planner wins about half the time
        return None, res
    b.par = max(5, round(ph[len(ph) // 2] * PAR_SCALE[b.act]))
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
    worth_all = collections.defaultdict(list)
    usefreq = collections.Counter()
    present = collections.Counter()
    shown = 0
    tried = 0
    made = 0
    seed = 0
    while made < seeds and tried < seeds * 12:
        seed += 1; tried += 1
        team = team_for(act, random.Random(seed))
        b, why = generate(act, seed, team)
        if not b:
            rejects[why] += 1
            continue
        par, res = set_par(b, team, runs)
        if par is None:
            rejects['unwinnable'] += 1
            continue
        way = sum(s.decisions - s.strikes for _, s in res)
        dead = sum(s.dead for _, s in res) / max(1, way)
        if dead > DEAD_MAX:
            rejects['dead'] += 1
            continue
        route = None
        if b.secret:
            rp = [p for p, s in play(b, team, max(4, runs // 4), b.seed, taste=1) if p and s.route_taken]
            route = PAR_SCALE[act] * (statistics.median(rp) - statistics.median([p for p, _ in res if p])) if rp else None
            if route is None or not ROUTE_COST[0] <= route <= ROUTE_COST[1]:
                rejects['route'] += 1
                continue
        w = command_worth(b, team, runs, res)
        for n, v in w.items(): worth_all[n].append(v)
        used = {n: sum(s.uses[n] for _, s in res) / len(res) for n in team}
        if any(w[n] < (UTILITY_EARN if ABILITIES[n] in UTILITY else EARN) or used[n] < USE_MIN[act]
               for n in team[1:] if ABILITIES[n] != 'stonebody'):
            rejects['commands'] += 1
            continue
        made += 1
        ph = [p * PAR_SCALE[act] for p, _ in res if p]
        stats['panels'].append(len(b.cells))
        f, fill = fields(b.cells)
        stats['fields'].append(f); stats['fill'].append(fill); stats['dense'].append(density(b))
        stats['dark'].append(sum(1 for k in b.cells.values() if k in (DARK, ITEM)) / len(b.cells))
        stats['par'].append(par)
        stats['best'].append(sum(1 for p in ph if p <= par - 2) / len(res))
        stats['mid'].append(sum(1 for p in ph if par - 2 < p <= par) / len(res))
        stats['over'].append(1 - stats['best'][-1] - stats['mid'][-1])
        stats['close'].append(statistics.mean(s.close_calls / max(1, s.decisions) for _, s in res))
        stats['dead'].append(dead)
        stats['moves'].append(sum(s.moves for _, s in res) / max(1, sum(s.decisions for _, s in res)))
        ph_sorted = sorted(ph)
        stats['margin'].append((ph_sorted[len(ph) * 3 // 4] - ph_sorted[len(ph) // 4]) if ph else 0)
        if route is not None: stats['route'].append(route)
        stats['traps'].append(sum(1 for k in b.cells.values() if k == TRAP))
        orders = collections.Counter(tuple(s.holes_order[:2]) for _, s in res)
        stats['orders'].append(len(orders))
        stats['secret'].append(1 if b.secret else 0)
        stats['bars'].append(sum(1 for k in b.cells.values() if k in BARS))
        for key in ('NumberCheck items', 'SneakAttack HP'):
            ab = 'numbercheck' if key.startswith('Number') else 'sneak'
            if any(ABILITIES[n] == ab for n in team):
                stats[key].append(statistics.mean(s.yields[key] for _, s in res))
        for n in team[1:] + ['MegaMan']:
            present[n] += 1
            used = sum(s.uses[n] for _, s in res) / len(res)
            if used > 0.2: usefreq[n] += 1
            worth[n].append(w[n])
        if shown < show:
            shown += 1
            print('act %d seed %d: %d panels, par %d, team %s, phases (scaled) %s' % (act, seed, len(b.cells), par, ', '.join(team), [round(p, 1) for p in sorted(ph)]))
            print(text(b.cells, b.guards))
            print()
        if drawdir:
            os.makedirs(drawdir, exist_ok=True)
            draw(b.cells, os.path.join(drawdir, 'act%d_seed%03d.png' % (act, seed)), b.guards,
                 title='act %d seed %d  par %d  %s' % (act, seed, par, '+'.join(team)))
    return stats, rejects, worth, worth_all, usefreq, present, tried


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--seeds', type=int, default=60)
    ap.add_argument('--runs', type=int, default=16)
    ap.add_argument('--act', type=int, default=0, help='one act (2-6); every act without it')
    ap.add_argument('--show', type=int, default=0)
    ap.add_argument('--draw', default='')
    ap.add_argument('--json', default='')
    ap.add_argument('--earn', type=float, default=None, help='the command rule\'s phases (default EARN; -99 turns it off)')
    a = ap.parse_args()
    global EARN
    if a.earn is not None: EARN = a.earn
    report = {}
    for act in ([a.act] if a.act else sorted(ACTS)):
        stats, rejects, worth, worth_all, usefreq, present, tried = measures(act, a.seeds, a.runs, a.show, a.draw)
        m = lambda k: statistics.mean(stats[k]) if stats[k] else 0
        print('act %d (BN5 mission %d): %d missions of %d tried, rejected %s' % (act, ACTS[act]['bn5'], len(stats['par']), tried, dict(rejects)))
        if not stats['par']:
            continue
        print('  panels %.0f, fields %.0f%%, fill %.0f%%, dark %.0f%% (%.1f around a dark panel), par %.1f (%d-%d), barriers %.1f, sealed routes %.0f%%' % (
            m('panels'), 100 * m('fields'), 100 * m('fill'), 100 * m('dark'), m('dense'), m('par'), min(stats['par']), max(stats['par']),
            m('bars'), 100 * m('secret')))
        print('  the planner: two under par %.0f%%, at par %.0f%%, over %.0f%%; close calls %.0f%% of decisions; first holes taken %.1f ways' % (
            100 * m('best'), 100 * m('mid'), 100 * m('over'), 100 * m('close'), m('orders')))
        print('  the quality bar: dead decisions on the way %.0f%%, sensible moves a decision %.1f, the par\'s middle half %.1f phases wide, the route %+.1f phases, traps %.1f' % (
            100 * m('dead'), m('moves'), m('margin'), m('route'), m('traps')))
        if stats['NumberCheck items'] or stats['SneakAttack HP']:
            print('  the utility commands\' yield a play: NumberCheck %.1f items, SneakAttack %.0f HP struck off' % (
                m('NumberCheck items'), m('SneakAttack HP')))
        print('  phases a command saves (its Navi without it) on the boards kept, on every board tried, and the kept missions it was used in:')
        for n in sorted(worth_all, key=lambda n: -statistics.mean(worth_all[n])):
            kept = statistics.mean(worth[n]) if worth[n] else float('nan')
            print('    %-12s %+5.2f  %+5.2f  used in %3.0f%%  (%d kept, %d tried)' % (
                n, kept, statistics.mean(worth_all[n]), 100 * usefreq[n] / max(1, present[n]), present[n], len(worth_all[n])))
        report[act] = dict(stats={k: m(k) for k in stats}, rejects=dict(rejects), worth={n: statistics.mean(v) for n, v in worth.items() if v},
                           worth_all={n: statistics.mean(v) for n, v in worth_all.items() if v})
    if a.json:
        with open(a.json, 'w') as f: json.dump(report, f, indent=1)
    return 0


if __name__ == '__main__':
    sys.exit(main())
