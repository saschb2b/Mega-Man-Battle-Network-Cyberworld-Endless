#!/usr/bin/env python3
"""economy_centres.py [--write] [REPORT]: the economy's bands re-centred on what the report measures
(docs/game/economy/guard.md, The report with bands).

`python3 build.py economy` writes .build/economy.txt: both plans on both nets, each act's chips, good share, Mystery
Data, zenny and codes held. src/core/economy.c holds those numbers as the bands' centres (`centre`, `centre_short`),
a fifth either side. A change that moves the drops on purpose (a chip moved into a reward row, the acts' arcs, a
price) sets the centres again in its own commit:

  python3 build.py economy                    # the report as the change left it: note what is outside (!)
  python3 tools/economy_centres.py            # the centres the report gives, beside those in economy.c
  python3 tools/economy_centres.py --write    # ... written into src/core/economy.c
  python3 build.py economy                    # again: 0 outside; its numbers before and after into the commit

It moves every centre to the measured number, so run it only for a change that means to move them: a number that
drifted by accident is what the bands are there to catch. No ROM is read here; the report needs it."""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REPORT = os.path.join(ROOT, '.build', 'economy.txt')
SOURCE = os.path.join(ROOT, 'src', 'core', 'economy.c')
METRICS = ['ECON_CHIPS', 'ECON_GOOD', 'ECON_MD', 'ECON_ZENNY', 'ECON_CODES']
ROW = re.compile(r'^\s*(\d|Nest)\s+' + r'\s+'.join([r'([\d.]+)!?\s*->\s*([\d.]+)!?'] * 5))


def measured(text, head):
    """The table under the line that starts with `head`: [flat, opening], each a metric's list by act."""
    lines = text[text.index(head):].splitlines()[2:]
    plans = [[[] for _ in METRICS], [[] for _ in METRICS]]
    for line in lines:
        m = ROW.match(line)
        if not m and plans[0][0]:
            break
        if not m:
            continue
        for k in range(len(METRICS)):
            plans[0][k].append(float(m.group(2 + 2 * k)))
            plans[1][k].append(float(m.group(3 + 2 * k)))
    return plans


def shown(metric, v):
    if metric == 'ECON_ZENNY':
        return str(int(round(v / 10.0)) * 10)
    return str(int(round(v))) if metric == 'ECON_CODES' else f'{v:.1f}'


def table(plans):
    out = []
    for p, name in enumerate(("the flat plan's", "the opening plan's")):
        out.append(f'\t{{   /* {name} */')
        out += [f'\t\t[{m}] = {{ {", ".join(shown(m, v) for v in plans[p][k])} }},' for k, m in enumerate(METRICS)]
        out.append('\t},')
    return '\n'.join(out) + '\n'


def main():
    args = [a for a in sys.argv[1:] if a != '--write']
    text = open(args[0] if args else REPORT).read()
    tables = {'centre': measured(text, 'By act, before (flat) -> now (opening)'),
              'centre_short': measured(text, 'The short net (a new run')}
    src = open(SOURCE).read()
    for name, plans in tables.items():
        m = re.search(r'(static const double ' + name + r'\[2\]\[ECON_METRICS\]\[\w+\] = \{\n)(.*?)(\};\n)', src, flags=re.S)
        if not m or not plans[0][0]:
            sys.exit(f'economy_centres: {name} not found in economy.c, or its table not in the report')
        new = table(plans)
        if m.group(2) != new:
            print(f'{name}: the report measures otherwise' + ('' if '--write' in sys.argv else ' (--write sets it)'))
            for old, now in zip(m.group(2).splitlines(), new.splitlines()):
                if old != now:
                    print('  ' + old.strip() + '\n  ' + now.strip())
        src = src[:m.start(2)] + new + src[m.end(2):]
    if '--write' in sys.argv:
        with open(SOURCE, 'w') as f:
            f.write(src)
        print('written:', os.path.relpath(SOURCE, ROOT))
    return 0


if __name__ == '__main__':
    sys.exit(main())
