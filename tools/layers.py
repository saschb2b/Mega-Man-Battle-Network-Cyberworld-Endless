#!/usr/bin/env python3
"""The code's layers: which part of src/ includes which, measured and held.

Every `#include "x.h"` in src/ is resolved to the folder that owns x.h (each
folder is on the include path, so a header's name is its address), and to
its unit: the folder, or the part of it tests/lint/layers.txt names. No ROM,
no build, about a second.

  python3 tools/layers.py              the report: folders, pairs, cycles, units, wide headers
  python3 tools/layers.py --edges      every edge between folders with the files that cause it
  python3 tools/layers.py --why A B    the includes that make folder A depend on B, and the names each
                                       uses (--units: the rule's units, as where a name has a dot)
  python3 tools/layers.py --uses H     who includes header H, and which of its names they use
  python3 tools/layers.py --cuts       the baseline by edge: what each costs, what it would end
  python3 tools/layers.py --generated  the files a tool writes, and their lines
  python3 tools/layers.py --lint       the rule: no include tests/lint/layers.txt does not allow,
                                       but those listed in tests/lint/layer_violations.txt
                                       (build.py lint runs it; --update writes the list)
  python3 tools/layers.py --record LABEL   a row of numbers into docs/code/layers.md (a release's)

The map, the rule and how to fix a violation: docs/code/layers.md.
"""
import argparse
import fnmatch
import os
import re
import sys
from collections import Counter, defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'src')
RULES = os.path.join(ROOT, 'tests', 'lint', 'layers.txt')
BASELINE = os.path.join(ROOT, 'tests', 'lint', 'layer_violations.txt')
CONCEPT = os.path.join(ROOT, 'docs', 'code', 'layers.md')
NUMBERS_END = '<!-- numbers: tools/layers.py --record adds a row above this line -->'

# The files of src/ a tool writes, each with its generator (docs/code/layers.md,
# Generated code). --generated names a file whose first lines say a tool wrote
# it and which is missing here.
GENERATED = {
    'audio/intro_chime.c': 'tools/intro_chime.py',
    'audio/intro_chime.h': 'tools/intro_chime.py',
    'core/app_icon.h': 'tools/app_icon.py',
    'core/group_tags.c': 'tools/encounter_tags.py',
    'core/library_pages_data.c': 'tools/library_pages.py',
    'core/xchips_moved.c': 'tools/xchiprecipe.py',
    'core/xchips_pas.c': 'tools/xchiprecipe.py',
    'core/xchips_registry_bn4.c': 'tools/xreg_bn4.py',
    'emu/free_space.h': 'tools/free_space.py',
    'emu/soulcode_bn5.c': 'tools/xsoul.py',
    'emu/xbracket_bn4.c': 'tools/xrecipe_bracket.py',
    'emu/xchips_bn4.c': 'tools/xchiprecipe_bn4.py',
    'emu/xchips_bn5.c': 'tools/xchiprecipe.py',
    'emu/xnavi_bn4.c': 'tools/xnavirecipe_bn4.py',
    'emu/xnavi_bn5.c': 'tools/xnavirecipe.py',
    'emu/xvirus_bn4.c': 'tools/xrecipe_bn4.py',
    'emu/xvirus_bn5.c': 'tools/xrecipe.py',
}
WRITTEN = re.compile(r'(?:written|drawn|WRITTEN) (?:BY|by)\s+(?:\*\s*)?(tools/\w+\.py)')

INCLUDE = re.compile(r'^[ \t]*#[ \t]*include[ \t]+"([^"]+)"', re.M)
WAY_OUT = ('the way out: move what is needed into a header the lower part owns, pass it in as an\n'
           '         argument, or have the upper part register a callback (docs/code/layers.md, Fixing a violation)')


class Tree:
    """src/ read once: each file's folder, lines and includes; each header's owner."""

    def __init__(self, root=SRC):
        self.files = []            # 'folder/name', sorted
        self.lines = {}
        self.includes = {}         # file -> the names it includes, in order
        self.owner = {}            # header name -> 'folder/name'
        self.twice = []            # header names two folders own
        for folder in sorted(os.listdir(root)):
            path = os.path.join(root, folder)
            if not os.path.isdir(path):
                continue
            for name in sorted(os.listdir(path)):
                if not name.endswith(('.c', '.h', '.m')):
                    continue
                key = f'{folder}/{name}'
                with open(os.path.join(path, name), errors='replace') as f:
                    text = f.read()
                self.files.append(key)
                self.lines[key] = text.count('\n')
                self.includes[key] = [os.path.basename(h) for h in INCLUDE.findall(text)]
                if name.endswith('.h'):
                    if name in self.owner:
                        self.twice.append((name, self.owner[name], key))
                    self.owner[name] = key
        self.folders = sorted({f.split('/')[0] for f in self.files})
        self.outside = Counter(h for f in self.files for h in self.includes[f] if h not in self.owner)

    def pairs(self):
        """Every (file, header file) of src/ one includes of the other, a file's own folder too."""
        for f in self.files:
            for h in self.includes[f]:
                if h in self.owner:
                    yield f, self.owner[h]

    def text(self, key):
        with open(os.path.join(SRC, key), errors='replace') as f:
            return f.read()


def folder_of(key):
    return key.split('/')[0]


class Rules:
    """tests/lint/layers.txt: the parts of folders, and what each unit may include."""

    def __init__(self, path=RULES):
        self.parts = []            # (unit, pattern), in the file's order
        self.order = []            # units, lowest first
        self.allowed = {}
        self.errors = []
        if not os.path.exists(path):
            self.errors.append(f'{os.path.relpath(path, ROOT)} is missing')
            return
        with open(path) as f:
            for n, raw in enumerate(f, 1):
                line = raw.split('#')[0].strip()
                if not line:
                    continue
                if '=' in line:
                    unit, patterns = (s.strip() for s in line.split('=', 1))
                    self.parts += [(unit, p) for p in patterns.split()]
                elif ':' in line:
                    unit, names = (s.strip() for s in line.split(':', 1))
                    if unit in self.allowed:
                        self.errors.append(f'line {n}: {unit} has two rules')
                    if names == '*':                 # every unit on the lines above
                        names = ' '.join(self.order)
                    for name in names.split():
                        if name not in self.allowed:
                            self.errors.append(f'line {n}: {unit} may include {name}, which has no rule above it: '
                                               f'a unit names only units on earlier lines, so the rules hold no cycle')
                    self.order.append(unit)
                    self.allowed[unit] = set(names.split())
                else:
                    self.errors.append(f'line {n}: neither "unit = files" nor "unit: units"')

    def unit(self, key):
        for unit, pattern in self.parts:
            if fnmatch.fnmatchcase(key, pattern):
                return unit
        return folder_of(key)

    def check(self, tree):
        """What is wrong with the rules themselves, against the tree."""
        errors = list(self.errors)
        units = {self.unit(f) for f in tree.files}
        for unit in sorted(units - set(self.allowed)):
            some = next(f for f in tree.files if self.unit(f) == unit)
            errors.append(f'{unit} (src/{some}) has no rule: give it a line saying what it may include, '
                          f'at its place in the order')
        for unit in sorted(set(self.allowed) - units):
            errors.append(f'{unit} has a rule and no file')
        for unit, pattern in self.parts:
            if not any(fnmatch.fnmatchcase(f, pattern) for f in tree.files):
                errors.append(f'{unit} = {pattern} matches no file')
        for name, a, b in tree.twice:
            errors.append(f'{name} is owned twice (src/{a}, src/{b}): an include of it cannot be resolved')
        return errors

    def violations(self, tree):
        """(file, header file) for every include the rules do not allow."""
        found = []
        for f, h in tree.pairs():
            a, b = self.unit(f), self.unit(h)
            if a != b and b not in self.allowed.get(a, ()):
                found.append((f, h))
        return found

    def kind(self, a, b):
        if a not in self.order or b not in self.order:
            return f'{a if a not in self.order else b} has no rule yet'
        if a in self.order and b in self.order and self.order.index(b) > self.order.index(a):
            return f'{b} is above {a}'
        return f'{b} is below {a}, and not one of the units {a} is allowed'


def graph(tree, of=folder_of):
    """{(a, b): [(file, header file)]} between the units `of` gives."""
    edges = defaultdict(list)
    for f, h in tree.pairs():
        a, b = of(f), of(h)
        if a != b:
            edges[(a, b)].append((f, h))
    return edges


def components(nodes, edges):
    """The strongly connected components of more than one node (Tarjan's), each sorted."""
    out = defaultdict(set)
    for a, b in edges:
        out[a].add(b)
    index, low, stack, on, found, count = {}, {}, [], set(), [], [0]

    def visit(v):
        index[v] = low[v] = count[0]
        count[0] += 1
        stack.append(v)
        on.add(v)
        for w in sorted(out[v]):
            if w not in index:
                visit(w)
                low[v] = min(low[v], low[w])
            elif w in on:
                low[v] = min(low[v], index[w])
        if low[v] == index[v]:
            comp = []
            while True:
                w = stack.pop()
                on.discard(w)
                comp.append(w)
                if w == v:
                    break
            if len(comp) > 1:
                found.append(sorted(comp))

    for v in sorted(nodes):
        if v not in index:
            visit(v)
    return sorted(found)


def triangles(edges):
    """Cycles of three with no pair among them including each other: what
    would stay a cycle were every mutual pair cut."""
    out = defaultdict(set)
    for a, b in edges:
        out[a].add(b)
    found = set()
    for a in out:
        for b in out[a]:
            for c in out[b]:
                if a in out[c] and len({a, b, c}) == 3:
                    if a in out[b] or b in out[c] or c in out[a]:
                        continue
                    i = min((a, b, c))
                    cyc = {a: (a, b, c), b: (b, c, a), c: (c, a, b)}[i]
                    found.add(cyc)
    return sorted(found)


def lines_by_folder(tree):
    hand, made = Counter(), Counter()
    for f in tree.files:
        (made if f in GENERATED else hand)[folder_of(f)] += tree.lines[f]
    return hand, made


def numbers(tree, rules):
    """What a release records: the hand-written lines, the folders' pairs, the baseline."""
    hand, made = lines_by_folder(tree)
    edges = graph(tree)
    pairs = sum(1 for (a, b) in edges if a < b and (b, a) in edges)
    comps = components(tree.folders, edges)
    units = graph(tree, rules.unit)
    return {
        'files': len(tree.files),
        'hand': sum(hand.values()),
        'generated': sum(made.values()),
        'pairs': pairs,
        'unit_pairs': sum(1 for (a, b) in units if a < b and (b, a) in units),
        'knot': max((len(c) for c in comps), default=0),
        'violations': len(rules.violations(tree)),
    }


def table(rows, right=()):
    widths = [max(len(str(r[i])) for r in rows) for i in range(len(rows[0]))]
    for r in rows:
        print('  ' + '  '.join(str(c).rjust(w) if i in right else str(c).ljust(w)
                               for i, (c, w) in enumerate(zip(r, widths))).rstrip())


def report(tree, rules):
    hand, made = lines_by_folder(tree)
    edges = graph(tree)
    n = numbers(tree, rules)
    print(f"src/: {n['files']} files, {n['hand'] + n['generated']} lines: {n['hand']} written by hand, "
          f"{n['generated']} by a tool ({len(GENERATED)} files)")
    print(f"folders: {len(tree.folders)}, {len(edges)} edges between them, {n['pairs']} pairs including each other, "
          f"the largest knot {n['knot']} folders")
    print(f"the rule (tests/lint/layers.txt): {len(rules.order)} units, {n['violations']} includes it does not allow\n")

    print('Folders (fan-out: the folders it includes; fan-in: the folders that include it)')
    rows = [('folder', 'files', 'by hand', 'by a tool', 'out', 'in', 'includes')]
    for m in tree.folders:
        out = sorted((b, len(v)) for (a, b), v in edges.items() if a == m)
        fan_in = sum(1 for (a, b) in edges if b == m)
        rows.append((m, sum(1 for f in tree.files if folder_of(f) == m), hand[m], made[m] or '', len(out), fan_in,
                     ' '.join(f'{b}:{c}' for b, c in out)))
    table(rows, right=(1, 2, 3, 4, 5))

    print('\nPairs that include each other (a -> b, b -> a: the includes each way)')
    rows = [(a, b, len(edges[(a, b)]), len(edges[(b, a)])) for (a, b) in sorted(edges) if a < b and (b, a) in edges]
    table(rows, right=(2, 3))

    print('\nKnots (folders that all reach each other)')
    for comp in components(tree.folders, edges):
        print(f'  {len(comp)}: {" ".join(comp)}')
    free = sorted(set(tree.folders) - {m for c in components(tree.folders, edges) for m in c})
    print(f'  in none: {" ".join(free) or "-"}')
    tri = triangles(edges)
    print(f'\nCycles of three with no pair among them: {len(tri)}')
    for a, b, c in tri:
        print(f'  {a} -> {b} -> {c} -> {a}')

    print('\nUnits (tests/lint/layers.txt, lowest first; "not allowed": includes of this unit the rule forbids)')
    bad = Counter(rules.unit(f) for f, h in rules.violations(tree))
    uedges = graph(tree, rules.unit)
    rows = [('unit', 'files', 'lines', 'not allowed', 'may include')]
    for u in rules.order:
        mine = [f for f in tree.files if rules.unit(f) == u]
        rows.append((u, len(mine), sum(tree.lines[f] for f in mine), bad[u] or '',
                     ' '.join(x for x in rules.order if x in rules.allowed[u]) or '-'))
    table(rows, right=(1, 2, 3))
    upairs = sum(1 for (a, b) in uedges if a < b and (b, a) in uedges)
    print(f'  {len(uedges)} edges between units, {upairs} pairs including each other')

    print('\nThe headers most included (directly; "reach": the files that get it through other headers too)')
    direct = defaultdict(set)
    for f, h in tree.pairs():
        direct[h].add(f)
    reach = reach_of(tree)
    rows = [('header', 'direct', 'reach', 'folders')]
    top = sorted(direct, key=lambda h: (-len(direct[h]), h))[:25]
    for h in top:
        rows.append((h, len(direct[h]), len(reach[h]), len({folder_of(f) for f in direct[h]})))
    table(rows, right=(1, 2, 3))
    if tree.outside:
        print('\nIncluded from outside src/: ' + ', '.join(f'{h} ({c})' for h, c in sorted(tree.outside.items())))


def reach_of(tree):
    """header file -> every file that includes it, directly or through headers."""
    seen = {}

    def closure(f):
        if f in seen:
            return seen[f]
        seen[f] = got = set()
        for h in tree.includes[f]:
            if h in tree.owner and tree.owner[h] != f:
                got.add(tree.owner[h])
                got |= closure(tree.owner[h])
        return got

    reach = defaultdict(set)
    for f in tree.files:
        for h in closure(f):
            reach[h].add(f)
    return reach


def show_edges(tree, of=folder_of, only=None):
    """The edges with the files that cause each; for one edge (--why), each
    include with the header's names its file uses."""
    edges = graph(tree, of)
    if only and only not in edges:
        print(f'{only[0]} includes nothing of {only[1]}')
    for (a, b) in sorted(edges):
        if only and (a, b) != only:
            continue
        print(f'{a} -> {b}: {len(edges[(a, b)])}')
        by = defaultdict(list)
        for f, h in edges[(a, b)]:
            by[h].append(f)
        for h in sorted(by, key=lambda x: (-len(by[x]), x)):
            if not only:
                print(f'  {os.path.basename(h)} ({len(by[h])}): {" ".join(sorted(os.path.basename(f) for f in by[h]))}')
                continue
            names = declared(tree.text(h))
            for f in sorted(by[h]):
                used = sorted(set(re.findall(r'[A-Za-z_]\w*', strip(tree.text(f)))) & names)
                print(f'  src/{f} includes {os.path.basename(h)}, and uses: {" ".join(used) or "nothing of its own"}')


# ---- what an includer uses of a header ----

KEYWORDS = set('''auto break case char const continue default do double else enum extern float for goto if inline int
long register restrict return short signed sizeof static struct switch typedef union unsigned void volatile while
bool true false NULL defined uint8_t uint16_t uint32_t uint64_t int8_t int16_t int32_t int64_t size_t'''.split())


def strip(text):
    """C without its comments and string literals."""
    text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.S)
    text = re.sub(r'//[^\n]*', ' ', text)
    return re.sub(r'"(?:\\.|[^"\\\n])*"', '""', text)


def declared(text):
    """The names a header gives its includers: macros, enumerators, types,
    functions and objects (read roughly: enough to tell who uses what)."""
    text = strip(text)
    names = set(re.findall(r'^[ \t]*#[ \t]*define[ \t]+(\w+)', text, re.M))
    body = re.sub(r'^[ \t]*#.*(?:\\\n.*)*', '', text, flags=re.M)
    for block in re.findall(r'\benum\b[^{;]*\{([^}]*)\}', body):
        names |= set(re.findall(r'(?:^|,)\s*(\w+)\s*(?:=[^,]*)?(?=,|$)', block.strip()))
    flat = body
    while True:                                   # struct and enum bodies give no names of their own
        smaller = re.sub(r'\{[^{}]*\}', ' ', flat)
        if smaller == flat:
            break
        flat = smaller
    for stmt in flat.split(';'):
        stmt = stmt.strip()
        if not stmt:
            continue
        m = re.search(r'\(\s*\*\s*(\w+)\s*\)\s*\(', stmt)                # a function pointer
        if not m:
            m = re.search(r'(\w+)\s*\(', stmt)                             # a function
        if not m:
            m = re.search(r'(\w+)\s*(?:\[[^\]]*\]\s*)*(?:=.*)?$', stmt, re.S)   # a type or an object
        if m:
            names.add(m.group(1))
        names |= set(re.findall(r'\b(?:struct|union|enum)\s+(\w+)', stmt))
    return {n for n in names if n not in KEYWORDS and not n.isdigit() and not re.fullmatch(r'[A-Z0-9_]+_H_?', n)}


def uses(tree, name, show=12):
    """Who includes a header, and which of its names (and of the headers it
    includes in turn) each includer uses."""
    if name not in tree.owner:
        sys.exit(f'no header {name} in src/')
    head = tree.owner[name]
    parts = [head] + [tree.owner[h] for h in tree.includes[head] if h in tree.owner]
    names = {p: declared(tree.text(p)) for p in parts}
    for p in parts[1:]:
        names[p] -= names[head]
    includers = sorted(f for f, h in tree.pairs() if h == head and f != head)
    words = {f: set(re.findall(r'[A-Za-z_]\w*', strip(tree.text(f)))) for f in includers}
    by_folder = Counter(folder_of(f) for f in includers)
    print(f'{name} (src/{head}, {tree.lines[head]} lines, {len(names[head])} names of its own): '
          f'{len(includers)} files include it, {len(reach_of(tree)[head])} reach it')
    print('  by folder: ' + ' '.join(f'{m}:{c}' for m, c in sorted(by_folder.items(), key=lambda x: (-x[1], x[0]))))
    if len(parts) > 1:
        print('\n  The headers it includes, and its includers that use names of each:')
        patterns = Counter()
        rows = []
        for p in parts:
            users = [f for f in includers if words[f] & names[p]]
            rows.append((os.path.basename(p) + (' itself' if p == head else ''), len(names[p]), len(users)))
        table([('header', 'names', 'includers using it')] + rows, right=(1, 2))
        for f in includers:
            patterns[tuple(os.path.basename(p) for p in parts if words[f] & names[p])] += 1
        print('\n  What an includer uses, by how many do:')
        for pat, c in sorted(patterns.items(), key=lambda x: (-x[1], x[0]))[:show]:
            print(f'    {c:4d}  {" + ".join(pat) or "nothing of them"}')
    count = Counter()
    for f in includers:
        for n in words[f] & names[head]:
            count[n] += 1
    none = [f for f in includers if not any(words[f] & names[p] for p in parts)]
    few = Counter(len(words[f] & names[head]) for f in includers)
    print(f'\n  Its own names by how many includers use each ({len(names[head]) - len(count)} used by none):')
    for n, c in sorted(count.items(), key=lambda x: (-x[1], x[0]))[:show * 3]:
        print(f'    {c:4d}  {n}')
    print(f'\n  Includers using 0, 1, 2, 3 of its own names: {few[0]}, {few[1]}, {few[2]}, {few[3]}')
    print(f'  Includers using no name of it or of what it includes: {len(none)}')
    for f in none:
        print(f'    src/{f}')
    return includers, words, names


# ---- the rule ----

def baseline():
    if not os.path.exists(BASELINE):
        return Counter()
    with open(BASELINE) as f:
        return Counter(line.rstrip('\n') for line in f if line.strip() and not line.startswith('#'))


def entry(f, h):
    return f'src/{f} -> {h}'


def lint(tree, rules, update):
    errors = rules.check(tree)
    for e in errors:
        print(f'  tests/lint/layers.txt: {e}')
    found_pairs = rules.violations(tree)
    found = Counter(entry(f, h) for f, h in found_pairs)
    if update and not errors:
        with open(BASELINE, 'w') as out:
            out.write('# the includes tests/lint/layers.txt does not allow, as they stood when the rule came\n'
                      '# (build.py lint, docs/code/layers.md): "file -> folder/header". This list only shrinks:\n'
                      '# none may join; turn one around, then build.py lint --update.\n')
            out.write(''.join(f'{line}\n' for line in sorted(found.elements())))
        print(f'layers: {sum(found.values())} listed in tests/lint/layer_violations.txt')
        return True
    base = baseline()
    new, gone = found - base, base - found
    for f, h in sorted(found_pairs):
        if entry(f, h) not in new:
            continue
        a, b = rules.unit(f), rules.unit(h)
        may = ' '.join(x for x in rules.order if x in rules.allowed.get(a, ())) or 'nothing'
        print(f'  new: src/{f} includes "{os.path.basename(h)}" (src/{h}): {a} may not include {b}\n'
              f'         the rule (tests/lint/layers.txt): {rules.kind(a, b)}; {a} may include: {may}\n'
              f'         {WAY_OUT}')
    if gone:
        print(f'  ({sum(gone.values())} fixed since the baseline: build.py lint --update drops them)')
    print(f'layers: {sum(found.values())} includes the rule does not allow, {sum(new.values())} new')
    return not new and not errors


def cuts(tree, rules):
    """The baseline by edge between units, the smallest first: its includes,
    the headers they are of, and the pair of folders it would end."""
    by = defaultdict(list)
    for f, h in rules.violations(tree):
        by[(rules.unit(f), rules.unit(h))].append((f, h))
    fedges = graph(tree)
    uedges = graph(tree, rules.unit)
    print('The includes the rule does not allow, by edge (the fewest first). "ends": the folders\' pair\n'
          'that would no longer include each other were the edge gone.\n')
    for (a, b), v in sorted(by.items(), key=lambda x: (len(x[1]), x[0])):
        fa, fb = {folder_of(f) for f, _ in v}, {folder_of(h) for _, h in v}
        ends = []
        for x in sorted(fa):
            for y in sorted(fb):
                whole = fedges.get((x, y), [])
                if whole and (y, x) in fedges and all(p in v for p in whole):
                    ends.append(f'{x} <-> {y}')
        back = len(uedges.get((b, a), []))
        heads = Counter(os.path.basename(h) for _, h in v)
        print(f'{a} -> {b}: {len(v)} ({rules.kind(a, b)}; {b} -> {a}: {back})'
              + (f'; ends {", ".join(ends)}' if ends else ''))
        print('    of: ' + ', '.join(f'{h}×{c}' if c > 1 else h for h, c in sorted(heads.items(), key=lambda x: (-x[1], x[0]))))
        print('    in: ' + ' '.join(sorted({f for f, _ in v})))


def generated(tree):
    total = 0
    rows = [('file', 'lines', 'written by')]
    for f in sorted(GENERATED, key=lambda x: (-tree.lines.get(x, 0), x)):
        if f not in tree.lines:
            rows.append((f'src/{f}', 'gone', GENERATED[f]))
            continue
        total += tree.lines[f]
        rows.append((f'src/{f}', tree.lines[f], GENERATED[f]))
    table(rows, right=(1,))
    print(f'  {len(GENERATED)} files, {total} lines')
    for f in tree.files:
        if f in GENERATED:
            continue
        head = ' '.join(tree.text(f).split('\n')[:6])
        m = WRITTEN.search(head)
        own = os.path.basename(f)
        if m and ('Do not edit' in head or 'do not edit' in head or re.search(rf'{re.escape(own)}:', head)):
            print(f'  not listed, and its first lines say {m.group(1)} wrote it: src/{f}')


def record(tree, rules, label):
    n = numbers(tree, rules)
    row = (f"| {label} | {n['hand']} | {n['generated']} | {n['files']} | {n['pairs']} | {n['unit_pairs']} | {n['violations']} |")
    with open(CONCEPT) as f:
        text = f.read()
    if NUMBERS_END not in text:
        sys.exit(f'{os.path.relpath(CONCEPT, ROOT)} has no "{NUMBERS_END}" line')
    if f'| {label} |' in text:
        sys.exit(f'{label} is recorded already')
    with open(CONCEPT, 'w') as f:
        f.write(text.replace(NUMBERS_END, f'{row}\n{NUMBERS_END}'))
    print(row)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--edges', action='store_true')
    ap.add_argument('--units', action='store_true', help="with --edges or --why: the rule's units, not the folders")
    ap.add_argument('--why', nargs=2, metavar=('A', 'B'))
    ap.add_argument('--uses', metavar='HEADER')
    ap.add_argument('--cuts', action='store_true')
    ap.add_argument('--generated', action='store_true')
    ap.add_argument('--numbers', action='store_true')
    ap.add_argument('--lint', action='store_true')
    ap.add_argument('--update', action='store_true')
    ap.add_argument('--record', metavar='LABEL')
    a = ap.parse_args()
    tree, rules = Tree(), Rules()
    if a.lint or a.update:
        return 0 if lint(tree, rules, a.update) else 1
    if a.why:
        of = rules.unit if a.units or not set(a.why) <= set(tree.folders) else folder_of
        show_edges(tree, of, tuple(a.why))
    elif a.edges:
        show_edges(tree, rules.unit if a.units else folder_of)
    elif a.uses:
        uses(tree, a.uses)
    elif a.cuts:
        cuts(tree, rules)
    elif a.generated:
        generated(tree)
    elif a.numbers:
        print(' '.join(f'{k}={v}' for k, v in numbers(tree, rules).items()))
    elif a.record:
        record(tree, rules, a.record)
    else:
        report(tree, rules)
    return 0


if __name__ == '__main__':
    sys.exit(main())
