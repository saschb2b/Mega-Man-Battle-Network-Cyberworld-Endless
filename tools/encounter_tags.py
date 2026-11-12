#!/usr/bin/env python3
"""encounter_tags.py dump|sample|report|engine [--write|--check]: the encounter groups' tags
(docs/game/progression/encounter-groups.md; docs/devtools/encounter-tags.md).

Every virus family a run's engine holds, of every ROM read (BN6's own, BN5's
and BN4's moved in), tagged by what it does, and what the tags can build:

  dump    the host build's --encounter-tags with every ROM beside BN6's
          (src/dev/encounter_tags.c) into tools/encounter_data/tags.tsv: each
          family's versions (HP, hit, element, both wheels' weakness, reward
          row), where its game's records set it, the acts' bands, the areas'
          own battles. Needs the ROMs (local only).
  sample  .build/encounter/battles.tsv for tools/encounter_danger.py: each
          act's own battles and fillings of every template, to be fought.
  report  the families' tags, each ROM set's pool by act, the templates and
          how many ways each fills, and the danger measured, as Markdown
          (--write: into docs/game/progression/encounter-groups-measured.md).
  engine  src/core/group_tags.c, the measured tags the engine's groups read
          (src/core/groups.h): each family's pressure and tricks, numbers
          alone (--check: fail where the file is not what it would write).

Numbers and our own tags alone: a family is its net and BN6's AI index
(6:01, 5:33, 4:64), never a name, text or picture of the games'."""
import argparse
import collections
import itertools
import os
import random
import statistics
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA = os.path.join(ROOT, 'tools', 'encounter_data')
WORK = os.path.join(ROOT, '.build', 'encounter')
REPORT = os.path.join(ROOT, 'docs', 'game', 'progression', 'encounter-groups-measured.md')
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))

NETS = {0: '6', 1: '5', 2: '4'}
SETS = [('BN6', {0}), ('BN6+BN5', {0, 1}), ('BN6+BN4', {0, 2}), ('BN6+BN5+BN4', {0, 1, 2})]
ELEMENTS = ['none', 'fire', 'aqua', 'elec', 'wood']
BEATS = {1: 2, 2: 3, 3: 4, 4: 1}   # an element, and the one that hits it twice as hard (BN6's first wheel)
WHEEL2 = {0x80: 'sword', 0x40: 'cursor', 0x20: 'wind', 0x10: 'breaker'}
ROLES = ['free', 'back', 'front']
PANELS = {'b': 'hole', 'c': 'plain', 'd': 'cracked', 'e': 'poison', 'f': 'holy', 'g': 'grass', 'h': 'ice', 'i': 'volcano'}
ACTS = ['1', '2', '3', '4', '5', '6', 'Nest']
# (the engine's room, docs/game/progression/encounter-groups.md: BN4's late families' sounds shared one pool a battle
# until 11 November 2026, one of them a battle; they have slots of their own since, so no family is held apart)
POOLED = set()
SAMPLE_SEED = 20261111


class Family:
    def __init__(self, net, ai):
        self.net, self.ai, self.key = net, ai, f'{NETS[net]}:{ai:02d}'
        self.summoned, self.ver = False, {}
        self.role, self.cols, self.stage, self.bound, self.placed = 0, (0, 0, 0), 0, False, False
        self.pressure, self.tricks, self.fight = None, [], None

    @property
    def lean(self):
        """Where its records set it most: 2 the front (the enemy's first column, two placements in five or more
        and no column more), 1 the back (the last, likewise), 0 neither. The visitors' role (src/core/visitors.c)
        asks three in four for the back and most for the front, which leaves five families of 65 a role."""
        n = sum(self.cols)
        if not n:
            return 0
        if self.cols[2] * 5 >= n * 2 and self.cols[2] >= max(self.cols):
            return 1
        return 2 if self.cols[0] * 5 >= n * 2 and self.cols[0] >= max(self.cols) else 0


def load(path=None):
    """The dump: the families by key, the acts' bands, the areas' pools and their battles."""
    fams, bands, pools, forms, xareas, xforms = {}, [], {}, collections.defaultdict(list), {}, collections.defaultdict(list)
    shared = set()
    with open(path or os.path.join(DATA, 'tags.tsv')) as f:
        for line in f:
            c = line.rstrip('\n').split('\t')
            if c[0] == 'band':
                bands.append(dict(zip(['act', 'depth', 'lo', 'hi', 'cap', 'easy', 'vlo', 'vhi', 'stage'], map(int, c[1:]))))
            elif c[0] == 'pool':
                pools[int(c[1])] = [int(x) for x in c[2:]]
            elif c[0] == 'fam':
                fam = Family(int(c[1]), int(c[2]))
                fam.summoned = c[4] == '1'
                fams[fam.key] = fam
            elif c[0] == 'ver':
                drops = [int(c[12][i:i + 4], 16) for i in range(0, 80, 4)]
                fams[f'{NETS[int(c[1])]}:{int(c[2]):02d}'].ver[int(c[3])] = {
                    'id': int(c[4]), 'hp': int(c[5]), 'hit': int(c[6]), 'elem': int(c[7]) if 0 < int(c[7]) <= 4 else 0,
                    'elem2': int(c[8]), 'weak2': int(c[9]), 'flags': int(c[10]), 'drops': drops}
            elif c[0] == 'kin':
                net, ai = int(c[1]), int(c[2])
                own = f'{NETS[net]}:{ai:02d}'
                if own in fams:
                    fam = fams[own]
                    fam.role, fam.cols, fam.stage, fam.bound, fam.placed = int(c[3]), tuple(map(int, c[4:7])), int(c[7]), c[8] == '1', True
                elif ai <= 29:
                    shared.add((net, f'6:{ai:02d}'))   # (a family both games have by name: one family, BN6's)
            elif c[0] == 'form':
                forms[int(c[1])].append({'field': int(c[2]), 'weight': int(c[3]), 'navi': c[4] == '1',
                                         'ents': [tuple(map(int, e.split(':'))) for e in c[5:]]})
            elif c[0] == 'xarea':
                xareas[int(c[1])] = dict(zip(['xrom', 'like', 'held', 'native'], map(int, c[2:])))
            elif c[0] == 'xform':
                xforms[int(c[1])].append([tuple(map(int, e.split(':'))) for e in c[4:]])
    return fams, bands, pools, forms, xareas, xforms, shared


def measured(fams, path=None):
    """Each family's pressure and tricks from its battle alone (tools/encounter_danger.py families)."""
    path = path or os.path.join(DATA, 'families.tsv')
    if not os.path.exists(path):
        return False
    rows = read_tsv(path)
    for r in rows:
        fam = fams.get('{}:{:02d}'.format(NETS[int(r['label'].split(':')[0])], int(r['label'].split(':')[1])))
        if not fam:
            continue
        if r['mode'] == 'idle':
            fam.pressure = int(r['pressure20'])
            for p in r['own_panels'].replace('-', ''):
                fam.tricks.append('panels:' + PANELS.get(p, p))
            for p in r['foe_panels'].replace('-', ''):
                fam.tricks.append('field:' + PANELS.get(p, p))
            if int(r['foes_most']) > int(r['foes']):
                fam.tricks.append('summons')
        else:
            fam.fight = (int(r['won']), int(r['frames']), int(r['hp_lost']))
        if int(r['taken']) and 'area' not in fam.tricks:
            fam.tricks.append('area')
    for fam in fams.values():
        # (tough: the autopilot's starting folder does not delete its V1, alone, in 50 seconds)
        if fam.fight and not fam.fight[0]:
            fam.tricks.append('tough')
    pairs_load(fams)
    return True


def read_tsv(path):
    with open(path) as f:
        head = f.readline().rstrip('\n').split('\t')
        return [dict(zip(head, line.rstrip('\n').split('\t'))) for line in f if line.strip()]


def top_version(fam, band):
    """The highest version index a family takes in an act: BN6's the act's, the older nets' by tier (V1 lets V1EX, ...)."""
    return band['vhi'] if fam.net == 0 else min(5, 2 * band['vhi'] + 1)


def versions(fam, band):
    """The versions of a family a battle of the act may hold: up to the act's, its hit under the cap."""
    return [v for v in sorted(fam.ver) if v <= top_version(fam, band) and fam.ver[v]['hit'] <= band['cap'] and fam.ver[v]['hp'] <= band['hi']]


def pool(fams, nets, band):
    """The families of the nets present a battle of the act may hold."""
    return [f for f in fams.values() if f.net in nets and not f.summoned and not f.bound and f.stage <= band['stage'] and versions(f, band)]


# The pairs' pass (tools/encounter_danger.py pairs, tools/encounter_data/pairs.tsv): two families that press a standing
# MegaMan harder together than their own pressures add up to, by PAIR_EXTRA HP in 20 seconds or more at V1: the pair
# (its two keys, sorted) -> that excess. A group that holds such a pair is predicted that much harder.
PAIR_EXTRA = 40
PAIRS = {}


def pairs_load(fams, path=None):
    """PAIRS from the pairs' pass; every pair's row (label, measured, predicted) for the report. False where the pass
    was not run."""
    path = path or os.path.join(DATA, 'pairs.tsv')
    PAIRS.clear()
    if not os.path.exists(path):
        return []
    rows = []
    for r in read_tsv(path):
        a, b = r['label'].split('+')
        if a not in fams or b not in fams or fams[a].pressure is None or fams[b].pressure is None:
            continue
        own, got = fams[a].pressure + fams[b].pressure, int(r['pressure20'])
        rows.append((a, b, got, own))
        if got - own >= PAIR_EXTRA:
            PAIRS[tuple(sorted((a, b)))] = got - own
    return rows


def pressure_of(group, vs):
    """A battle's pressure as its families' own add up: each one's measured alone at V1, by its hit at the version
    fought over its V1's, and for each two of them the pairs' pass found pressing harder together (PAIRS) that
    excess, by the larger of the two's hits over their V1's; None where a family's was not measured."""
    if any(f.pressure is None for f in group):
        return None
    scale = [f.ver[v]['hit'] / max(1, f.ver[0]['hit']) for f, v in zip(group, vs)]
    total = sum(f.pressure * f.ver[v]['hit'] / max(1, f.ver[0]['hit']) for f, v in zip(group, vs))
    for i in range(len(group)):
        for k in range(i + 1, len(group)):
            extra = PAIRS.get(tuple(sorted((group[i].key, group[k].key))))
            if extra:
                total += extra * max(scale[i], scale[k])
    return round(total)


def cost_of(group, vs):
    """What a battle costs, as its families' own costs add up: the HP the test autopilot loses deleting each one's V1
    alone with the starting folder (50 seconds at most), by its hit at the version fought over its V1's; None where a
    family's was not fought."""
    if any(f.fight is None for f in group):
        return None
    return round(sum(f.fight[2] * f.ver[v]['hit'] / max(1, f.ver[0]['hit']) for f, v in zip(group, vs)))


def fit(group, band, test=None, cap=None, cost=None):
    """A group's versions inside the act's band: the most HP together from its floor to its top, each hit under its
    cap, the template's own test passed and, where `cap` is given, the pressure predicted at most that. The versions
    and the HP, None where no versions do."""
    best = None
    for vs in itertools.product(*[versions(f, band) for f in group]):
        hp = sum(f.ver[v]['hp'] for f, v in zip(group, vs))
        if not band['lo'] <= hp <= band['hi'] or (best and hp <= best[1]) or (test and not test(group, band, vs)):
            continue
        if cap is not None and (pressure_of(group, vs) is None or pressure_of(group, vs) > cap):
            continue
        if cost is not None and (cost_of(group, vs) is None or cost_of(group, vs) > cost):
            continue
        best = (vs, hp)
    return best


# ---- the templates: a name, its slots (what a family must be to take each), and what the group must be ----

def is_front(f):
    return f.lean == 2 or 'tough' in f.tricks


def is_back(f):
    return f.lean == 1


def is_free(f):
    return f.lean == 0


def breaks(f):
    return any(t.startswith('panels:') for t in f.tricks)


def shapes(f):
    return 'area' in f.tricks or any(t.startswith('field:') for t in f.tricks)


def adjacent(group, band, vs):
    a, b = (f.ver[v]['elem'] for f, v in zip(group, vs))
    return a and b and (BEATS[a] == b or BEATS[b] == a)


def same_element(group, band, vs):
    e = [f.ver[v]['elem'] for f, v in zip(group, vs)]
    return e[0] and len(set(e)) == 1


def glass(group, band, vs):
    return all(f.ver[v]['hit'] * 4 >= band['cap'] * 3 for f, v in zip(group, vs)) and sum(f.ver[v]['hp'] for f, v in zip(group, vs)) * 2 <= band['lo'] + band['hi']


def bulwark(group, band, vs):
    hp = [f.ver[v]['hp'] for f, v in zip(group, vs)]
    return hp[0] * 5 >= sum(hp) * 3 and group[0].ver[vs[0]]['hit'] * 2 <= band['cap'] and group[1].ver[vs[1]]['hit'] * 4 >= band['cap'] * 3


def two_nets(group, band, vs):
    return len({f.net for f in group}) == len(group)


def anything(f):
    return True


TEMPLATES = [
    ('wall and artillery', [is_front, is_back], None, 'a front-row family or one the buster is slow to delete, and a back-row shooter behind it'),
    ('panel war', [breaks, is_free], None, "a family that changes MegaMan's panels, and one that roams"),
    ('summoner and guard', [lambda f: 'summons' in f.tricks, is_front], None, 'a family that brings more enemies, behind a front or tough one'),
    ('clashing answers', [anything, anything], adjacent, 'two elements next to each other on the wheel: the chip that answers one is the other\'s own'),
    ('one element', [anything, anything], same_element, 'two families of one element: the act\'s answer chip hits the whole battle twice as hard'),
    ('pincer', [is_front, is_back, is_free], None, 'the front, the back and a roamer: three roles at once'),
    ('swarm', [anything], None, 'three of one family (the arc\'s own shape: alone, a pair, three)'),
    ('held ground', [shapes, is_back], None, 'a family that takes or reshapes panels, and a back-row shooter'),
    ('glass cannons', [anything, anything], glass, 'two families that hit near the act\'s cap, their HP together in the band\'s lower half'),
    ('bulwark', [anything, anything], bulwark, 'one family three fifths of the battle\'s HP hitting softly, one hitting near the cap'),
    ('nets meet', [is_free, is_free], two_nets, 'a roamer of each of two nets side by side (no net alone fills it)'),
]


def fillings(template, fams_in, band, cap=None, cost=None):
    """Every way the act's pool fills a template: (group, versions, HP), a family once a group; under `cap`, the
    pressure predicted at most that."""
    name, slots, test, _ = template
    out = []
    if name == 'swarm':
        for f in fams_in:
            for v in reversed(versions(f, band)):
                p = pressure_of((f, f, f), (v, v, v))
                c = cost_of((f, f, f), (v, v, v))
                if band['lo'] <= 3 * f.ver[v]['hp'] <= band['hi'] and (cap is None or (p is not None and p <= cap)) and \
                        (cost is None or (c is not None and c <= cost)):
                    out.append(((f, f, f), (v, v, v), 3 * f.ver[v]['hp']))
                    break
        return out
    seen = set()
    for group in itertools.product(*[[f for f in fams_in if s(f)] for s in slots]):
        keys = tuple(f.key for f in group)
        if len(set(keys)) < len(keys) or sum(k in POOLED for k in keys) > 1:
            continue
        if len({s for s in slots}) == 1 and tuple(sorted(keys)) in seen:   # (alike slots: a pair once)
            continue
        got = fit(group, band, test, cap, cost)
        if not got:
            continue
        seen.add(tuple(sorted(keys)))
        out.append((group, got[0], got[1]))
    return out


def natives(forms, fams, band, biome):
    """Area `biome`'s own battles as the act takes them: each formation at the highest version up to the act's that
    keeps it to the band (docs/game/progression/structure.md, Choosing battles), those reaching its floor."""
    out = []
    for form in forms.get(biome, []):
        virus = [(e, fams.get(f'6:{e[3]:02d}')) for e in form['ents'] if e[2] == 0 and 1 <= e[3] <= 29]
        virus = [(e, f) for e, f in virus if f and e[4] in f.ver and f.ver[e[4]]['id'] == e[0]]
        if form['navi'] or not virus or any(f.stage > band['stage'] for _, f in virus):
            continue
        for v in range(band['vhi'], -1, -1):
            if any(v not in f.ver for _, f in virus):
                continue
            hp, hit = sum(f.ver[v]['hp'] for _, f in virus), max(f.ver[v]['hit'] for _, f in virus)
            if hp <= band['hi'] and hit <= band['cap']:
                if hp >= band['lo']:
                    out.append(([(f, v, e[1] & 15, e[1] >> 4) for e, f in virus], hp, form['weight']))
                break
    return out


def area_families(forms, fams, biome):
    return {f'6:{e[3]:02d}' for form in forms.get(biome, []) for e in form['ents'] if e[2] == 0 and 1 <= e[3] <= 29 and f'6:{e[3]:02d}' in fams}


def act_areas(pools, act):
    """The BN6 areas an act draws on (BIOME_*): the surface's by its pool, the Undernet, the Graveyard, the Nest."""
    return pools.get(act) or {4: [5], 5: [4], 6: [7]}[act]


def spec_of(placed):
    return '/'.join(f'{f.ver[v]["id"]:X}@{col}{row}' for f, v, col, row in placed)


def place(group, vs):
    """A template's group on the enemy's panels: the front's column 4, the back's 6, a roamer's 5; rows 2, 1, 3."""
    rows, out = [2, 1, 3], []
    for i, (f, v) in enumerate(zip(group, vs)):
        col = 4 if f.lean == 2 else 6 if f.lean == 1 else 5
        if len(set(group)) == 1:
            col = 4 + i
        out.append((f, v, col, rows[i]))
    return out


def predicted(placed):
    """pressure_of for a battle's placed foes."""
    return pressure_of([p[0] for p in placed], [p[1] for p in placed])


def own_pressures(fams, bands, pools, forms, act):
    """The pressure predicted of every own battle of the act's areas, sorted."""
    return sorted(p for b in act_areas(pools, act) for placed, _, _ in natives(forms, fams, bands[act], b) for p in [predicted(placed)] if p is not None)


def own_costs(fams, bands, pools, forms, act):
    """What every own battle of the act's areas is predicted to cost (cost_of), sorted."""
    out = []
    for b in act_areas(pools, act):
        for placed, _, _ in natives(forms, fams, bands[act], b):
            c = cost_of([p[0] for p in placed], [p[1] for p in placed])
            if c is not None:
                out.append(c)
    return sorted(out)


def quarter_mark(values):
    """The three-quarter mark of sorted values: the proposed cap (docs/game/progression/encounter-groups.md, Danger)."""
    return values[min(len(values) - 1, len(values) * 3 // 4)]


def cmd_dump(a):
    os.makedirs(os.path.join(WORK, 'data'), exist_ok=True)
    out = os.path.join(WORK, 'tags.tsv')
    cmd = [sys.executable, os.path.join(ROOT, 'build.py'), 'shot', '--data-dir', '/src/.build/encounter/data', '--encounter-tags', '/src/.build/encounter/tags.tsv']
    r = subprocess.run(cmd, cwd=ROOT, env=dict(os.environ, CYBERWORLD_ROM_DIR=ROM_DIR), capture_output=True, text=True)
    if r.returncode or not os.path.exists(out):
        sys.exit('encounter_tags: the dump failed\n' + r.stderr[-2000:])
    os.makedirs(DATA, exist_ok=True)
    with open(out) as f, open(os.path.join(DATA, 'tags.tsv'), 'w') as g:
        g.write(f.read())
    fams = load()[0]
    print(f'{len(fams)} families into tools/encounter_data/tags.tsv:', ', '.join(f'{n} of BN{NETS[k]}' for k, n in sorted(collections.Counter(f.net for f in fams.values()).items())))
    return 0


def cmd_sample(a):
    fams, bands, pools, forms, _, _, _ = load()
    measured(fams)
    rng = random.Random(SAMPLE_SEED)
    lines = []
    for act in range(6):
        band = bands[act]
        own = [(biome, n) for biome in act_areas(pools, act) for n in natives(forms, fams, band, biome)]
        for i, (biome, (placed, hp, _)) in enumerate(rng.sample(own, min(a.natives, len(own)))):
            lines.append((f'a{act + 1}/own/{i}/b{biome}', spec_of(placed)))
        if act == 0 or act == 5:   # (act 1 holds its own; act 6's pool is act 5's but for the versions)
            continue
        everything = pool(fams, {0, 1, 2}, band)
        for t in TEMPLATES:
            got = fillings(t, everything, band)
            # (the mixed ones first: a filling of two nets is what the rework adds)
            mixed = [g for g in got if len({f.net for f in g[0]}) > 1] or got
            for i, (group, vs, hp) in enumerate(rng.sample(mixed, min(a.fillings, len(mixed)))):
                lines.append((f'a{act + 1}/{t[0].replace(" ", "-")}/{i}/' + '+'.join(f.key for f in group), spec_of(place(group, vs))))
    os.makedirs(WORK, exist_ok=True)
    with open(os.path.join(WORK, 'battles.tsv'), 'w') as f:
        f.writelines(f'{label}\t{spec}\n' for label, spec in lines)
    print(f'{len(lines)} battles into .build/encounter/battles.tsv')
    return 0


# ---- the report ----

def table(head, rows):
    out = ['| ' + ' | '.join(head) + ' |', '| ' + ' | '.join('---' for _ in head) + ' |']
    return out + ['| ' + ' | '.join(str(c) for c in r) + ' |' for r in rows]


def reward_text(drops):
    """A reward row in short: of its ten entries above 3/8 HP, how many are chips, and the zenny's most."""
    chips = {(d & 0x1FF) if d >> 14 == 0 else 512 + ((d >> 14) - 2) * 256 + (d & 0xFF) for d in drops[:10] if d != 0xFFFF and (d >> 14 == 0 or (d >> 14 >= 2 and d & 0x2000))}
    zenny = [d & 0x3FFF for d in drops[:10] if d != 0xFFFF and d >> 14 == 1]
    n = sum(1 for d in drops[:10] if d != 0xFFFF and (d >> 14 == 0 or (d >> 14 >= 2 and d & 0x2000)))
    return n, len(chips), max(zenny) if zenny else 0


def report_families(fams, have):
    rows = []
    for f in sorted(fams.values(), key=lambda f: (f.net, f.ai)):
        v = sorted(f.ver)
        elems = '/'.join(dict.fromkeys(ELEMENTS[f.ver[x]['elem']] for x in v if x <= (2 if f.net == 0 else 5)))
        weak = '/'.join(dict.fromkeys(ELEMENTS[BEATS[f.ver[x]['elem']]] if f.ver[x]['elem'] else '-' for x in v if x <= (2 if f.net == 0 else 5)))
        weak2 = ','.join(n for bit, n in WHEEL2.items() if f.ver[0]['weak2'] & bit) or '-'
        last = max(x for x in v if x <= (2 if f.net == 0 else 5))
        chips = [reward_text(f.ver[x]['drops']) for x in v if x <= last]
        note = 'summoned only' if f.summoned else 'special panels only' if f.bound else '' if f.placed else 'not in the records read'
        rows.append([f.key, len([x for x in v if x <= last]), f'{f.ver[0]["hp"]}-{f.ver[last]["hp"]}', f'{f.ver[0]["hit"]}-{f.ver[last]["hit"]}', elems, weak, weak2,
                     ROLES[f.role], ROLES[f.lean] + ' ' + '/'.join(map(str, f.cols)), f.stage if f.placed else '-',
                     f.pressure if f.pressure is not None else '-', (f'{f.fight[1] // 60} s, -{f.fight[2]}' if f.fight[0] else f'open, -{f.fight[2]}') if f.fight else '-',
                     ' '.join(dict.fromkeys(f.tricks)) or '-',
                     f'{chips[0][0]}-{chips[-1][0]} ({max(c[1] for c in chips)})', note])
    head = ['Family', 'Versions', 'HP', 'Hit', 'Element', 'Weak to', 'Second wheel', "Role (the visitors')", 'Lean (front/middle/back placements)', 'Stage', 'Pressure', 'Autopilot at V1 (time, HP)', 'Tricks', 'Chip entries of 10, V1 to top (kinds a row at most)', 'Note']
    return table(head, rows)


def report_pool(fams, bands):
    out = []
    for name, nets in SETS:
        rows = []
        for act, band in enumerate(bands):
            p = pool(fams, nets, band)
            roles = collections.Counter(ROLES[f.lean] for f in p)
            elems = collections.Counter(ELEMENTS[f.ver[versions(f, band)[-1]]['elem']] for f in p)
            tricks = collections.Counter(t.split(':')[0] for f in p for t in set(x.split(':')[0] for x in f.tricks))
            rows.append([ACTS[act], f'{band["lo"]}-{band["hi"]}', band['cap'], len(p), roles['front'], roles['back'], roles['free'],
                         *[elems[e] for e in ELEMENTS], tricks['panels'], tricks['field'] + tricks['area'], tricks['summons'], tricks['tough']])
        out += [f'**{name}**', ''] + table(['Act', 'Band', 'Cap', 'Families', 'Front', 'Back', 'Free', 'None', 'Fire', 'Aqua', 'Elec', 'Wood',
                                             'Panels', 'Field or area', 'Summons', 'Tough'], rows) + ['']
    return out


def report_templates(fams, bands, pools, forms):
    out = []
    for name, nets in SETS:
        rows = []
        for t in TEMPLATES:
            row = [t[0]]
            for act, band in enumerate(bands):
                p = pool(fams, nets, band)
                got = fillings(t, p, band)
                mixed = sum(1 for g in got if len({f.net for f in g[0]}) > 1)
                areas = act_areas(pools, act)
                held = [sum(1 for g in got if any(f.key in area_families(forms, fams, b) for f in g[0])) for b in areas]
                cell = f'{len(got)}' + (f' ({mixed})' if mixed else '')
                if got and act < 4:
                    cell += f' [{min(held)}-{max(held)}]'
                row.append(cell if got else '**none**')
            rows.append(row)
        out += [f'**{name}**', ''] + table(['Template'] + ['Act ' + a if a != 'Nest' else a for a in ACTS], rows) + ['']
    return out


def report_capped(fams, bands, pools, forms):
    """The own battles' predicted pressure by act, and the templates' fillings under its three-quarter mark."""
    own = [own_pressures(fams, bands, pools, forms, act) for act in range(len(bands))]
    rows = [[ACTS[act], len(o), o[0], o[len(o) // 2], quarter_mark(o), o[-1], 100 * (act + 1)] for act, o in enumerate(own)]
    out = table(['Act', "Own battles of the act's areas", 'Predicted pressure: least', 'Median', 'Three-quarter mark (the cap)', 'Most', "MegaMan's max HP there"], rows) + ['']
    costs = [own_costs(fams, bands, pools, forms, act) for act in range(len(bands))]
    out += ['The same battles by what they are predicted to cost the test autopilot (each family\'s HP lost deleting its V1 alone, '
            'added up): the second cap, on what the pressure cap does not see, how long a fight lasts and what lands on a MegaMan '
            'who lines up to shoot.', '']
    out += table(['Act', 'Predicted cost: least', 'Median', 'Three-quarter mark (the cap)', 'Most'],
                 [[ACTS[act], o[0], o[len(o) // 2], quarter_mark(o), o[-1]] for act, o in enumerate(costs)]) + ['']
    for name, nets in (SETS[0], SETS[3]):
        rows = []
        for t in TEMPLATES:
            row = [t[0]]
            for act, band in enumerate(bands):
                n = len(fillings(t, pool(fams, nets, band), band, quarter_mark(own[act]), quarter_mark(costs[act])))
                row.append(n or '**none**')
            rows.append(row)
        out += [f'**{name}, under both caps**', ''] + table(['Template'] + ['Act ' + a if a != 'Nest' else a for a in ACTS], rows) + ['']
    return out


def report_signatures(fams, bands):
    rows = []
    for name, nets in SETS:
        row = [name]
        for band in bands:
            p = pool(fams, nets, band)
            count = collections.Counter()
            for t in TEMPLATES:
                for key in {f.key for g in fillings(t, p, band) for f in g[0]}:
                    count[key] += 1
            row.append(f'{sum(1 for k in count if count[k] >= 3)} of {len(p)}')
        rows.append(row)
    return table(['ROM set'] + ['Act ' + a if a != 'Nest' else a for a in ACTS], rows)


def report_danger(fams, bands):
    path = os.path.join(DATA, 'danger.tsv')
    if not os.path.exists(path):
        return ['Not measured yet (`tools/encounter_danger.py battles`).'], []
    by_id = {f.ver[v]['id']: (f, v) for f in fams.values() for v in f.ver}
    rows = collections.defaultdict(dict)
    for r in read_tsv(path):
        act, kind = r['label'].split('/')[:2]
        e = rows[(int(act[1:]), kind, r['label'])]
        placed = [(*by_id[int(s.split('@')[0], 16)], int(s.split('@')[1][0]), int(s.split('@')[1][1])) for s in r['spec'].split('/')]
        e['placed'] = placed
        if r['mode'] == 'idle':
            e['idle'] = int(r['pressure20'])
        else:
            e.setdefault('fights', []).append((int(r['won']), int(r['frames']), int(r['hp_lost'])))
    summary, detail = [], []
    for act in range(1, 7):
        band = bands[act - 1]
        hp_max = 100 * act
        kinds = collections.defaultdict(list)
        for (a, kind, label), e in rows.items():
            if a == act and 'idle' in e:
                kinds[kind].append((label, e))
        own = sorted(e['idle'] for _, e in kinds.get('own', []))
        for kind in ['own'] + [k for k in sorted(kinds) if k != 'own']:
            es = kinds.get(kind)
            if not es:
                continue
            idle = sorted(e['idle'] for _, e in es)
            fights = [x for _, e in es for x in e.get('fights', [])]
            ratio = [e['idle'] / predicted(e['placed']) for _, e in es if predicted(e['placed'])]
            summary.append([act, kind.replace('-', ' '), len(es), f'{idle[0]} / {statistics.median(idle):.0f} / {idle[-1]}',
                            f'{100 * statistics.median(idle) / hp_max:.0f}%',
                            f'{statistics.median(ratio):.2f}' if ratio else '-',
                            f'{sum(w for w, _, _ in fights)} of {len(fights)}' if fights else '-',
                            f'{statistics.median(h for _, _, h in fights):.0f}' if fights else '-',
                            f'{max(h for _, _, h in fights)}' if fights else '-',
                            sum(1 for _, _, h in fights if h >= hp_max) if fights else '-',
                            f'{sum(1 for _, e in es if own and e["idle"] > own[-1])}' if kind != 'own' and own else '-'])
            for label, e in sorted(es):
                p = predicted(e['placed'])
                detail.append([act, kind.replace('-', ' '), ' + '.join(f'{f.key} v{v + 1}' for f, v, _, _ in e['placed']),
                               sum(f.ver[v]['hp'] for f, v, _, _ in e['placed']), max(f.ver[v]['hit'] for f, v, _, _ in e['placed']),
                               e['idle'], p if p is not None else '-', f'{e["idle"] / p:.2f}' if p else '-',
                               ' '.join(f'{"won" if w else "open"} {fr // 60}s -{h}' for w, fr, h in e.get('fights', []))])
    head = ['Act', 'Battles', 'Sampled', 'Pressure: least / median / most', "Median, of the act's max HP", 'Median measured / predicted',
            'Autopilot won in 50 s', 'Autopilot HP lost, median', '... most', "Lost the act's max HP", 'Past the own battles\' most']
    head2 = ['Act', 'Battles', 'Families (version index from 1)', 'HP', 'Hit', 'Pressure', 'Predicted', 'Ratio', 'Autopilot (per seed)']
    return table(head, summary), table(head2, detail)


# The fields (tools/encounter_danger.py battles, tools/encounter_data/fields.tsv): one pair that is harmless on the plain
# field, fought by the test autopilot on every field BN6's areas' random battles use, four seeds each. A field is fair
# where it wins every one and loses FIELD_FAIR HP at most in any (the plain field's own worst): a group stands on an
# area's field only where it is fair, since a group is not authored for the field its area's records are.
FIELD_FAIR = 20


def fields_load(path=None):
    """Each field fought: its number -> (fights, won, the most HP lost, the slowest win in seconds); empty where the
    pass was not run."""
    path = path or os.path.join(DATA, 'fields.tsv')
    out = {}
    if not os.path.exists(path):
        return out
    for r in read_tsv(path):
        if r['mode'] != 'fight':
            continue
        n, won, lost, secs = out.get(int(r['label'], 16), (0, 0, 0, 0))
        out[int(r['label'], 16)] = (n + 1, won + int(r['won']), max(lost, int(r['hp_lost'])), max(secs, int(r['frames']) // 60))
    return out


def fair_fields(fields):
    return sorted(f for f, (n, won, lost, _) in fields.items() if won == n and lost <= FIELD_FAIR)


def report_fields():
    fields = fields_load()
    if not fields:
        return ['The fields were not fought (`tools/encounter_danger.py battles`).', '']
    fair = set(fair_fields(fields))
    lines = [f'{len(fields)} fields, every one a random battle of BN6\'s areas uses, each fought four times by the test autopilot against one pair '
             f'that is harmless on the plain field (BN6\'s family 1 beside BN5\'s 51, 110 HP). {len(fair)} are fair: it wins every fight and '
             f'loses {FIELD_FAIR} HP at most in any, as on the plain field. On the other {len(fields) - len(fair)} the same pair costs more or cannot '
             f'be deleted in 50 seconds: what a field does to MegaMan\'s side (holes, poison, ice, panels he cannot stand on) no measure of '
             f'the families sees. A group takes an area\'s field only where it is fair.', '']
    rows = [[f'0x{f:02X}', f'{won}/{n}', lost, secs] for f, (n, won, lost, secs) in sorted(fields.items()) if f not in fair]
    return lines + table(['Field, not fair', 'Won', 'Most HP lost', 'Slowest, seconds'], rows) + ['']


def report_pairs(fams):
    """The pairs' pass: every two families of the pool fought together at V1, against what their own pressures add
    up to."""
    rows = pairs_load(fams)
    if not rows:
        return ['The pairs\' pass was not run (`tools/encounter_danger.py pairs`).', '']
    ratios = sorted(got / own for _, _, got, own in rows if own)
    excess = sorted(got - own for _, _, got, own in rows)
    lines = [f'{len(rows)} pairs, every two families of the pool once, each at V1 where its lean puts it, MegaMan standing: the HP he '
             f'loses in 20 seconds against the sum of the two families\' own pressures. The median pair presses '
             f'{statistics.median(ratios):.2f} of its sum ({statistics.median(excess):+.0f} HP); {sum(r > 1.25 for r in ratios)} press past 1.25 of it, '
             f'and the most any presses past it is {excess[-1]} HP. A pair {PAIR_EXTRA} HP or more past its sum is tagged '
             f'({len(PAIRS)} pairs): a group that holds it is predicted that much harder, by the larger of the two families\' '
             f'hits at the version fought over their V1\'s.', '']
    flagged = sorted(((got - own, a, b, got, own) for a, b, got, own in rows if got - own >= PAIR_EXTRA), reverse=True)
    lines += table(['Pair', 'Together', 'Their own, added', 'Past the sum'], [[f'{a} + {b}', got, own, f'+{d}'] for d, a, b, got, own in flagged])
    return lines + ['']


def cmd_report(a):
    fams, bands, pools, forms, xareas, xforms, shared = load()
    have = measured(fams)
    counts = collections.Counter(f.net for f in fams.values())
    lines = ['## The families', '',
             f'{len(fams)} families in the engine with every ROM read: ' + ', '.join(f'{n} of BN{NETS[k]}' for k, n in sorted(counts.items())) + '.',
             'Read from the ROMs by `tools/encounter_tags.py dump`; pressure and tricks fought by `tools/encounter_danger.py families`.', '']
    lines += report_families(fams, have) + ['']
    lines += ['## Each ROM set\'s pool by act', ''] + report_pool(fams, bands)
    lines += ['## The templates, and how many ways each fills', ''] + report_templates(fams, bands, pools, forms)
    lines += ['## Families that can carry an act', ''] + report_signatures(fams, bands) + ['']
    summary, detail = report_danger(fams, bands)
    lines += ['## Danger', ''] + summary + ['']
    lines += ['### The pairs\' pass', ''] + report_pairs(fams)
    lines += ['### The fields', ''] + report_fields()
    lines += ['### A cap on pressure', ''] + report_capped(fams, bands, pools, forms)
    if detail:
        lines += ['### Every battle sampled', ''] + detail + ['']
    text = '\n'.join(lines)
    if a.write:
        with open(REPORT) as f:
            old = f.read()
        mark = '<!-- measured: tools/encounter_tags.py report --write -->\n'
        if mark not in old:
            sys.exit(f'{os.path.relpath(REPORT, ROOT)} lacks the mark the tables go after')
        with open(REPORT, 'w') as f:
            f.write(old[:old.index(mark) + len(mark)] + '\n' + text)
        print('written:', os.path.relpath(REPORT, ROOT))
    else:
        print(text)
    return 0


ENGINE = os.path.join(ROOT, 'src', 'core', 'group_tags.c')
TRICK_BITS = (('GROUP_BREAKS', lambda f: any(t.startswith('panels:') for t in f.tricks)),
              ('GROUP_SHAPES', lambda f: 'area' in f.tricks or any(t.startswith('field:') for t in f.tricks)),
              ('GROUP_SUMMONS', lambda f: 'summons' in f.tricks), ('GROUP_TOUGH', lambda f: 'tough' in f.tricks))


def cmd_engine(a):
    """The measured tags as the engine reads them: a row a family measured, its net, BN6's AI index, its pressure and
    its tricks."""
    fams = load()[0]
    if not measured(fams):
        sys.exit('no tools/encounter_data/families.tsv: tools/encounter_danger.py families first')
    rows = []
    for f in sorted(fams.values(), key=lambda f: (f.net, f.ai)):
        if f.pressure is None:
            continue
        tricks = ' | '.join(name for name, has in TRICK_BITS if has(f)) or '0'
        rows.append(f'\t{{ {f.net}, {f.ai}, {f.pressure}, {tricks}, {f.fight[2] if f.fight else -1} }},')
    fair_bits = [0] * 32
    for field in fair_fields(fields_load()) or [0]:
        fair_bits[field >> 3] |= 1 << (field & 7)
    pairs = []
    for (one, two), extra in sorted(PAIRS.items(), key=lambda kv: (fams[kv[0][0]].net, fams[kv[0][0]].ai, fams[kv[0][1]].net, fams[kv[0][1]].ai)):
        pairs.append(f'\t{{ {fams[one].net}, {fams[one].ai}, {fams[two].net}, {fams[two].ai}, {extra} }},')
    text = '\n'.join([
        '/* groups.h: the families\' measured tags, a row each: its net (0 BN6\'s, 1 BN5\'s, 2 BN4\'s), BN6\'s AI index, its',
        ' * pressure (the HP a standing MegaMan loses in 20 seconds to its V1 alone), its tricks and its cost (the HP the test',
        ' * autopilot loses deleting its V1 alone with the starting folder, 50 seconds at most; -1 not fought).',
        ' * Written by tools/encounter_tags.py engine from tools/encounter_data/ (docs/devtools/encounter-tags.md); do not',
        ' * edit: measure again and write it again (--check says whether this is what it writes). */',
        '#include "groups.h"', '',
        'const GroupTag group_tags[] = {'] + rows + ['};', 'const int group_tags_count = (int)(sizeof group_tags / sizeof *group_tags);', '',
        '/* The pairs that press harder together than their own pressures add up to (the pairs\' pass: tools/encounter_danger.py',
        f' * pairs), {PAIR_EXTRA} HP in 20 seconds or more at V1: each family\'s net and BN6\'s AI index, and that excess. */',
        'const GroupPair group_pairs[] = {'] + pairs + ['};', 'const int group_pairs_count = (int)(sizeof group_pairs / sizeof *group_pairs);', '',
        '/* BN6\'s battlefields a group may stand on, a bit each by the field\'s number (the fields\' pass: a pair harmless on the',
        f' * plain field fought on each, fair where the test autopilot wins every fight and loses {FIELD_FAIR} HP at most). */',
        'const uint8_t group_fair_fields[32] = {', '\t' + ', '.join(f'0x{b:02X}' for b in fair_bits) + ',', '};', ''])
    if a.check:
        if not os.path.exists(ENGINE) or open(ENGINE).read() != text:
            sys.exit(f'{os.path.relpath(ENGINE, ROOT)} is not what tools/encounter_tags.py engine writes')
        print(f'{os.path.relpath(ENGINE, ROOT)} is what the tool writes ({len(rows)} families)')
        return 0
    with open(ENGINE, 'w') as f:
        f.write(text)
    print(f'written: {os.path.relpath(ENGINE, ROOT)} ({len(rows)} families)')
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('what', choices=['dump', 'sample', 'report', 'engine'])
    ap.add_argument('--write', action='store_true', help='report: into the concept, after its mark')
    ap.add_argument('--check', action='store_true', help='engine: write nothing, fail where the file differs')
    ap.add_argument('--natives', type=int, default=8, help="sample: each act's own battles")
    ap.add_argument('--fillings', type=int, default=2, help="sample: each template's fillings an act")
    a = ap.parse_args()
    return {'dump': cmd_dump, 'sample': cmd_sample, 'report': cmd_report, 'engine': cmd_engine}[a.what](a)


if __name__ == '__main__':
    sys.exit(main())
