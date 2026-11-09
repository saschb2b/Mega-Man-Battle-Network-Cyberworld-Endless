#!/usr/bin/env python3
"""free_space.py [--write | --check | --fit SIZE [--zone Z] | --usage FILE]:
the free space's plan (src/emu/free_space.plan), the one place every region
the engine writes past BN6's 8 MB in the core's ROM copy, and in the RAM BN6
leaves alone, is named, placed and owned (docs/engine/emulation/free-space.md).

From it come src/emu/free_space.h (the FREE_* and RAM_* names the code uses,
and the tables tests/test_core.c and src/emu/free_space.c read) and the
offsets, names and writers of docs/engine/emulation/memory.md's two tables,
whose words stay the doc's own. Needs no ROM.

  python3 tools/free_space.py              the report: every zone, its regions, its gaps
                                           and its headroom (with --usage, what a run used)
  python3 tools/free_space.py --write      write free_space.h and memory.md's tables
  python3 tools/free_space.py --check      write nothing: fail where the plan is not
                                           whole (regions out of order, over each other,
                                           outside their zone, a writer or concept that is
                                           not there) or a file differs from what it would
                                           write (build.py lint)
  python3 tools/free_space.py --fit SIZE [--zone Z] [--align A]
                                           where a new region of SIZE bytes fits: the free
                                           gaps that hold it, in its kind's zone or all
  python3 tools/free_space.py --usage FILE the report with what a run used, from the file
                                           CYBERWORLD_FREE_SPACE=FILE wrote as it quit
                                           (src/emu/free_space.c: each region's last byte
                                           that is not the 0xFF fill)
"""
import argparse
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PLAN = 'src/emu/free_space.plan'
HEADER = 'src/emu/free_space.h'
DOC = 'docs/engine/emulation/memory.md'
EMU_FREE = 0x08800000
FREE_SIZE = 0x800000
FLAGS = ('end', 'free')


class Zone:
    def __init__(self, key, space, at, end, names, title, line):
        self.key, self.space, self.at, self.end, self.names, self.title, self.line = key, space, at, end, names, title, line
        self.regions = []


class Region:
    def __init__(self, name, at, end, flags, owners, concept, what, zone, line):
        self.name, self.at, self.end, self.owners, self.concept, self.what, self.zone, self.line = name, at, end, owners, concept, what, zone, line
        self.has_end = 'end' in flags
        self.free = 'free' in flags
        self.parent = flags.get('in')
        self.children = []


def number(text, space, line):
    """+0x... from EMU_FREE in the ROM copy, 0x... a RAM address"""
    if space == 'rom':
        if not text.startswith('+0x'):
            raise SystemExit(f'{PLAN}:{line}: a ROM place is +0x... from EMU_FREE, not {text}')
        return EMU_FREE + int(text[1:], 16)
    if not text.startswith('0x'):
        raise SystemExit(f'{PLAN}:{line}: a RAM place is its address 0x..., not {text}')
    return int(text, 16)


def parse_flags(text, line):
    flags = {}
    for f in ([] if text == '-' else text.split(',')):
        key, _, value = f.partition('=')
        if key not in FLAGS + ('in', 'start') or (key == 'in') != bool(value):
            raise SystemExit(f'{PLAN}:{line}: no such flag {f}')
        flags[key] = value or True
    return flags


def parse(path=None):
    zones, regions = [], []
    with open(path or os.path.join(ROOT, PLAN)) as f:
        for n, raw in enumerate(f, 1):
            text = raw.strip()
            if not text or text.startswith('#'):
                continue
            if text.startswith('zone '):
                parts = text.split(None, 6)
                if len(parts) < 7 or parts[2] not in ('rom', 'ram'):
                    raise SystemExit(f'{PLAN}:{n}: zone KEY rom|ram START END NAMES title')
                _, key, space, at, end, names, title = parts
                names = dict(x.split('=', 1) for x in ([] if names == '-' else names.split(',')))
                zones.append(Zone(key, space, number(at, space, n), number(end, space, n), names, title, n))
                continue
            parts = text.split(None, 6)
            if len(parts) < 7 or not zones:
                raise SystemExit(f'{PLAN}:{n}: NAME START END FLAGS OWNER CONCEPT what, after its zone')
            name, at, end, flags, owners, concept, what = parts
            z = zones[-1]
            r = Region(name, number(at, z.space, n), number(end, z.space, n), parse_flags(flags, n),
                       [] if owners == '-' else owners.split(','), concept, what, z, n)
            regions.append(r)
            z.regions.append(r)
    by = {r.name: r for r in regions}
    for r in regions:
        if r.parent:
            if r.parent not in by:
                raise SystemExit(f'{PLAN}:{r.line}: {r.name} is in {r.parent}, which the plan does not name')
            by[r.parent].children.append(r)
    return zones, regions


def problems(zones, regions, files=True):
    """what makes the plan not whole: each zone inside its space and past the
    one before it, each region inside its zone (a part inside its parent) and
    past the one before it, not empty, its name once; its writers' files and
    its concept there"""
    bad = []
    names = {}
    for r in regions:
        if r.name in names or (r.has_end and r.name + '_END' in names):
            bad.append(f'{r.name}: named twice')
        names[r.name] = r
        if r.has_end:
            names[r.name + '_END'] = r
    for z in zones:
        for k in z.names.values():
            if k in names:
                bad.append(f'{k}: named twice')
    last = {}
    for z in zones:
        lo, hi = (EMU_FREE, EMU_FREE + FREE_SIZE) if z.space == 'rom' else (0x02000000, 0x02040000)
        if not lo <= z.at < z.end <= hi:
            bad.append(f'zone {z.key}: {z.at:08x}-{z.end:08x} outside {lo:08x}-{hi:08x}')
        if z.space in last and z.at < last[z.space].end:
            bad.append(f'zone {z.key}: at {z.at:08x}, over zone {last[z.space].key}')
        last[z.space] = z
        tops = [r for r in z.regions if not r.parent]
        for i, r in enumerate(tops):
            if not z.at <= r.at < r.end <= z.end:
                bad.append(f'{r.name}: {r.at:08x}-{r.end:08x} empty or outside zone {z.key}, {z.at:08x}-{z.end:08x}')
            if i and r.at < tops[i - 1].end:
                bad.append(f'{r.name}: at {r.at:08x}, over {tops[i - 1].name}, which ends at {tops[i - 1].end:08x}')
        for p in z.regions:
            for i, c in enumerate(p.children):
                if not p.at <= c.at < c.end <= p.end:
                    bad.append(f'{c.name}: {c.at:08x}-{c.end:08x} empty or outside {p.name}, {p.at:08x}-{p.end:08x}')
                if i and c.at < p.children[i - 1].end:
                    bad.append(f'{c.name}: at {c.at:08x}, over {p.children[i - 1].name}')
        for r in z.regions:
            if r.parent and names[r.parent].zone is not z:
                bad.append(f'{r.name}: in {r.parent}, of another zone')
    if files:
        sources = {f for d, _, fs in os.walk(os.path.join(ROOT, 'src')) for f in fs}
        for r in regions:
            for o in r.owners:
                if o not in sources:
                    bad.append(f'{r.name}: its writer {o} is no file of src/')
            if not os.path.isfile(os.path.join(ROOT, 'docs', r.concept)):
                bad.append(f'{r.name}: its concept docs/{r.concept} is not there')
            if not r.owners and not r.free:
                bad.append(f'{r.name}: no writer, and not free')
    return bad


def gaps(z):
    """the spans of a zone no region holds, and its regions marked free"""
    out, at = [], z.at
    for r in sorted((r for r in z.regions if not r.parent), key=lambda r: r.at):
        if r.at > at:
            out.append((at, r.at, None))
        if r.free:
            out.append((r.at, r.end, r.name))
        at = max(at, r.end)
    if at < z.end:
        out.append((at, z.end, None))
    return out


# ---- free_space.h ----

def place(z, value):
    return f'(EMU_FREE + 0x{value - EMU_FREE:06X}u)' if z.space == 'rom' else f'0x{value:08X}u'


def span(z, at, end):
    if z.space == 'rom':
        return f'+0x{at - EMU_FREE:06X}-+0x{end - EMU_FREE:06X}'
    return f'0x{at:08X}-0x{end:08X}'


def define(name, value, what=''):
    head = f'#define {name:<20} {value}'
    return f'{head:<52} /* {what} */' if what else head


def header(zones, regions):
    out = [
        '/* The free space past the game\'s 8 MB in the core\'s ROM copy, and the RAM',
        ' * BN6 leaves alone: one plan every part of the engine follows. WRITTEN BY',
        ' * tools/free_space.py FROM src/emu/free_space.plan: edit the plan, run the',
        ' * tool, and build.py lint keeps the two in step (docs/engine/emulation/',
        ' * free-space.md: the zones, their headroom and how a region is placed).',
        ' * Each region is its writer\'s, [its start, its end), inside its zone; a span',
        ' * no region names is free. tests/test_core.c checks the regions in order',
        ' * and apart from FREE_SPACE_ROM and FREE_SPACE_RAM, emu.c\'s writes into the',
        ' * ROM copy\'s free space are told where they run past a region (free_space.c),',
        ' * and what the installers write as the core starts is claimed byte by byte',
        ' * as they write it (claims.h). emu.h includes this: the places are bus',
        ' * addresses of the core\'s copy. */',
        '#ifndef CW_FREE_SPACE_H',
        '#define CW_FREE_SPACE_H',
        '',
        define('EMU_FREE', '0x08800000u', 'first free bus address past an 8 MB game'),
        define('FREE_END', '(EMU_FREE + 0x800000u)', "the copy's end: 16 MB (EMU_ROM_SIZE)"),
    ]
    for z in zones:
        out.append('')
        out.append(f'/* ---- {z.key}: {span(z, z.at, z.end)}, {z.title} ---- */')
        for r in z.regions:
            who = ', '.join(r.owners[:3]) + (', ...' if len(r.owners) > 3 else '') or 'free'
            out.append(define(r.name, place(z, r.at), f'{r.what} ({who})'))
            if r.has_end:
                out.append(define(r.name + '_END', place(z, r.end)))
        for k, name in z.names.items():
            out.append(define(name, place(z, z.at if k == 'start' else z.end)))
    out += ['', '/* The plan as tables: X(name, start, end, parent, zone), in address order,',
            ' * a part after its parent ("" none); X(key, start, end) the zones */']
    for space, macro in (('rom', 'FREE_SPACE_ROM'), ('ram', 'FREE_SPACE_RAM')):
        rows = [f'\tX("{r.name}", {place(z, r.at)}, {place(z, r.end)}, "{r.parent or ""}", "{z.key}")'
                for z in zones if z.space == space for r in z.regions]
        out.append(f'#define {macro}(X) \\')
        out += [row + ' \\' for row in rows[:-1]] + [rows[-1]]
        zrows = [f'\tX("{z.key}", {place(z, z.at)}, {place(z, z.end)})' for z in zones if z.space == space]
        out.append(f'#define {macro}_ZONES(X) \\')
        out += [row + ' \\' for row in zrows[:-1]] + [zrows[-1]]
    out += ['', '#endif', '']
    return '\n'.join(out)


# ---- memory.md's tables ----

ROW = re.compile(r'^\| (`[^|]*`) \| (`\w+`) \| (.*) \| ([^|]*) \|$')


def doc_span(z, r):
    if z.space == 'rom':
        return f'`+0x{r.at - EMU_FREE:X}`-`+0x{r.end - EMU_FREE:X}`'
    return f'`0x{r.at:08X}`-`0x{r.end - 1:08X}`'


def doc_tables(zones, text):
    """memory.md with its two tables' rows made from the plan: a row a region
    (a part's parent's row tells its parts) and a row a gap, in order; each
    row's words kept from the row of that region, or the plan's own"""
    lines = text.split('\n')
    words = {}
    for line in lines:
        m = ROW.match(line)
        if m:
            words[m.group(2).strip('`')] = m.group(3)
    for space, head in (('rom', '| Offset | Region | Contents | Code |'), ('ram', '| Range | Region | Contents | Code |')):
        if head not in lines:
            raise SystemExit(f'{DOC}: no table starting "{head}"')
        i = lines.index(head)
        j = i + 2
        while j < len(lines) and lines[j].startswith('|'):
            j += 1
        rows = []
        for z in (z for z in zones if z.space == space):
            spans = [(r.at, r) for r in z.regions if not r.parent] + [(a, (a, e)) for a, e, name in gaps(z) if not name]
            for _, item in sorted(spans, key=lambda s: s[0]):
                if isinstance(item, tuple):
                    a, e = item
                    span = f'`+0x{a - EMU_FREE:X}`-`+0x{e - EMU_FREE:X}`' if space == 'rom' else f'`0x{a:08X}`-`0x{e - 1:08X}`'
                    rows.append(f'| {span} | | Free ({e - a:,} bytes; zone {z.key}) | |')
                    continue
                r = item
                code = ', '.join(f'`{o}`' for o in r.owners)
                rows.append(f'| {doc_span(z, r)} | `{r.name}` | {words.get(r.name, r.what[0].upper() + r.what[1:])} | {code} |')
        lines[i + 2:j] = rows
    return '\n'.join(lines)


ZONES_DOC = 'docs/engine/emulation/free-space.md'
ZONES_BEGIN = '<!-- tools/free_space.py writes this table from src/emu/free_space.plan -->'
ZONES_END = '<!-- end of the table tools/free_space.py writes -->'


def zones_table(zones, text):
    """free-space.md's table of the zones: each one's span, what it holds,
    its regions, its headroom and its largest gap, between its markers"""
    if ZONES_BEGIN not in text or ZONES_END not in text:
        raise SystemExit(f'{ZONES_DOC}: no markers for the zones\' table')
    rows = ['| Zone | Span | Holds | Regions | Headroom | Largest gap |', '| --- | --- | --- | --- | --- | --- |']
    for z in zones:
        g = gaps(z)
        free = sum(e - a for a, e, _ in g)
        big = max((e - a for a, e, _ in g), default=0)
        at = f'+0x{z.at - EMU_FREE:06X}' if z.space == 'rom' else f'0x{z.at:08X}'
        end = f'+0x{z.end - EMU_FREE:06X}' if z.space == 'rom' else f'0x{z.end:08X}'
        rows.append(f'| `{z.key}` | `{at}`-`{end}` ({size(z.end - z.at)}) | {z.title} | {len(z.regions)} | '
                    f'{size(free)} ({100 * free // (z.end - z.at)}%) | {size(big)} |')
    head, rest = text.split(ZONES_BEGIN, 1)
    return head + ZONES_BEGIN + '\n' + '\n'.join(rows) + '\n' + ZONES_END + rest.split(ZONES_END, 1)[1]


# ---- the report ----

def size(n):
    return f'{n / 1024:.1f} KB' if n >= 1024 else f'{n} B'


def read_usage(path):
    """CYBERWORLD_FREE_SPACE's file: a line a region, NAME USED_TO"""
    used = {}
    with open(path) as f:
        for line in f:
            parts = line.split()
            if len(parts) == 2:
                used[parts[0]] = int(parts[1], 16)
    return used


def report(zones, used=None):
    for z in zones:
        free = sum(e - a for a, e, _ in gaps(z))
        big = max((e - a for a, e, _ in gaps(z)), default=0)
        print(f'zone {z.key} ({z.space}) {z.at:08x}-{z.end:08x}, {size(z.end - z.at)}: {z.title}')
        print(f'  headroom {size(free)} ({100 * free // (z.end - z.at)}%), the largest gap {size(big)}')
        for r in z.regions:
            indent = '    ' if r.parent else '  '
            use = ''
            if used and r.name in used and not r.children:
                u = used[r.name] - r.at if used[r.name] else 0
                use = f'  used {size(u)} ({100 * u // (r.end - r.at)}%)'
            who = ','.join(r.owners) or 'free'
            print(f'{indent}{r.at:08x}-{r.end:08x} {size(r.end - r.at):>9}  {r.name:<22} {who}{use}')
        for a, e, name in gaps(z):
            if not name:
                print(f'  {a:08x}-{e:08x} {size(e - a):>9}  (free)')
        print()


def fit(zones, need, zone=None, align=0x100):
    out = []
    for z in zones:
        if zone and z.key != zone:
            continue
        for a, e, name in gaps(z):
            start = (a + align - 1) // align * align
            if start + need <= e:
                out.append((z, start, e, name))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--write', action='store_true')
    ap.add_argument('--check', action='store_true')
    ap.add_argument('--fit', type=lambda s: int(s, 0))
    ap.add_argument('--zone')
    ap.add_argument('--align', type=lambda s: int(s, 0), default=0x100)
    ap.add_argument('--usage')
    a = ap.parse_args()
    zones, regions = parse()
    bad = problems(zones, regions)
    for b in bad:
        print(f'  {PLAN}: {b}')
    if bad:
        print(f'free space: the plan is not whole, {len(bad)} problems')
        return 1
    want = {HEADER: header(zones, regions)}
    with open(os.path.join(ROOT, DOC)) as f:
        want[DOC] = doc_tables(zones, f.read())
    with open(os.path.join(ROOT, ZONES_DOC)) as f:
        want[ZONES_DOC] = zones_table(zones, f.read())
    if a.check or a.write:
        stale = []
        for path, text in want.items():
            full = os.path.join(ROOT, path)
            with open(full) as f:
                if f.read() == text:
                    continue
            if a.write:
                with open(full, 'w') as f:
                    f.write(text)
                print(f'free space: wrote {path}')
            else:
                stale.append(path)
        for path in stale:
            print(f'  {path}: not as {PLAN} makes it (python3 tools/free_space.py --write)')
        print(f'free space: {len(regions)} regions in {len(zones)} zones, {len(stale)} files out of step')
        return 1 if stale else 0
    if a.fit:
        found = fit(zones, a.fit, a.zone, a.align)
        for z, start, end, name in found:
            off = f'+0x{start - EMU_FREE:06X}' if z.space == 'rom' else f'0x{start:08X}'
            print(f'zone {z.key}: {off}, {size(end - start)} free there{f" ({name})" if name else ""}')
        if not found:
            print(f'free space: no gap of {size(a.fit)} {"in zone " + a.zone if a.zone else "anywhere"}: '
                  'see docs/engine/emulation/free-space.md, Moving regions')
        return 0 if found else 1
    report(zones, read_usage(a.usage) if a.usage else None)
    return 0


if __name__ == '__main__':
    sys.exit(main())
