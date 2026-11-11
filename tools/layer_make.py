#!/usr/bin/env python3
"""layer_make.py [--lint | --check | --stale]: the layers' make and the
record of its hash (src/net/layer_make.h; docs/project/merging.md,
Counters).

LAYER_MAKE is 100 and a file a bump: src/net/layer_make.d/NNN-WORDS.txt,
the number that bump makes it and a few words, holding a clause on what
changed in what a layer seed makes. The build counts the files, so two
branches that each bump add two files and merge without meeting; before,
both rewrote one line of layer_make.h, the number and its history, and
set the hash beside it, which after a merge was neither's.

tests/layer_make_hash.txt holds "MAKE HASH": the make dev was at and
layer_generate's hash over the unit tests' seeds there
(tests/test_core.c, test_layer_make). The tests fail where the hash
differs and no bump was added. Only dev writes the record, by its
workflow (tools/generated.py write) after a merge that added a bump: it
builds the unit tests and asks them (build/host/test_core
--layer-make-hash).

  python3 tools/layer_make.py           write the record where the make moved (builds
                                        the unit tests in the build image)
  python3 tools/layer_make.py --lint    the bumps' files well formed; write nothing
  python3 tools/layer_make.py --check   as --lint, and the record at this make
  python3 tools/layer_make.py --stale   exit 1 where the record is not at this make
                                        (the workflow: whether to fetch the build image)
Standard library only.
"""
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUMPS = 'src/net/layer_make.d'
RECORD = 'tests/layer_make_hash.txt'
BASE = 100   # layer_make.h's LAYER_MAKE_BASE
NAME = re.compile(r'(\d{3})-[a-z0-9][a-z0-9-]*\.txt$')


def bumps():
    """The bumps' files in order, and what is wrong with any."""
    bad, names = [], sorted(os.listdir(os.path.join(ROOT, BUMPS)))
    for name in names:
        if not NAME.match(name):
            bad.append(f'{BUMPS}/{name}: not named NNN-WORDS.txt (121-a-vendors-list-in-bugfrags.txt)')
            continue
        with open(os.path.join(ROOT, BUMPS, name), encoding='utf-8') as f:
            if not f.read().strip():
                bad.append(f'{BUMPS}/{name}: empty: a clause on what changed in what a seed makes')
    return names, bad


def record():
    with open(os.path.join(ROOT, RECORD), encoding='utf-8') as f:
        made, value = f.read().split()
    return int(made), value


def measured():
    """The make and the hash, from this tree's own unit tests."""
    sys.path.insert(0, ROOT)
    import build
    build.ensure_image()
    if build.docker('make', 'build/host/test_core', '-j4'):
        sys.exit('layer_make: the unit tests do not build')
    out = subprocess.run(build.docker_run() + ['--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src',
                                               build.IMAGE, 'build/host/test_core', '--layer-make-hash'],
                         cwd=ROOT, capture_output=True, text=True)
    m = re.fullmatch(r'(\d+) (0x[0-9a-f]{8})\s*', out.stdout)
    if out.returncode or not m:
        sys.exit(f'layer_make: build/host/test_core --layer-make-hash: {out.stdout[-200:]}{out.stderr[-200:]}')
    return int(m.group(1)), m.group(2)


def main():
    args = sys.argv[1:]
    if args not in ([], ['--lint'], ['--check'], ['--stale']):
        sys.exit(__doc__)
    names, bad = bumps()
    made, (recorded, value) = BASE + len(names), record()
    for b in bad:
        print(f'layer_make: {b}')
    if args == ['--stale']:
        return 1 if made != recorded else 0
    if args == ['--lint']:
        print(f'layer_make: make {made} in {len(names)} bumps, {len(bad)} problems'
              + (f'; the record is at {recorded}, dev\'s workflow\'s to move' if made != recorded else ''))
        return 1 if bad else 0
    if args == ['--check']:
        if made != recorded:
            print(f'layer_make: {RECORD} is at make {recorded}, the tree at {made}: python3 tools/layer_make.py')
        else:
            print(f'layer_make: make {made}, recorded')
        return 1 if bad or made != recorded else 0
    if bad:
        return 1
    if made == recorded:
        print(f'layer_make: make {made}, recorded')
        return 0
    now, value = measured()
    if now != made:
        sys.exit(f'layer_make: the build counts make {now}, the files {made}')
    with open(os.path.join(ROOT, RECORD), 'w', encoding='utf-8', newline='\n') as f:
        f.write(f'{made} {value}\n')
    print(f'layer_make: {RECORD}: make {made}, hash {value}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
