#!/usr/bin/env python3
"""claims.py [--lint]: what each branch in flight may take of the numbers
several branches take from (docs/project/claims.md).

Chip ids, enemy ids, AI numbers, the dispatch's types, sprite lists'
numbers, song slots and spans of the free space are taken by hand, and
two branches that take the same one find out when the second merges, or
not at all: git reports no conflict for two tables that each use 0x2C6.
So the session that starts others writes each one's claim first, a file a
branch, docs/project/claims/BRANCH.json:

  {
    "branch": "bn5-magnetman",
    "what": "MagnetMan moved, Team ProtoMan's fifth",
    "takes": { "enemy_ids": [["0x3A8", "0x3AB"]], "song_slots": [["0x45", "0x45"]],
               "free_rom": [["0x141000", "0x14FFFF"]] },
    "units": ["MagnetMan"],
    "run": false
  }

`takes` names a space and the ranges taken in it, first and last, both
taken (numbers, or strings as C writes them); the spaces are whatever the
claims call them, and two claims meet only in a space of one name
(docs/project/claims.md lists the names in use). `units` are pieces of work
by name (a chip, a Navi, a page). `run` says the branch changes `Run`
(src/core/run.h), which only one branch at a time may.

  python3 tools/claims.py          the claims, a line each
  python3 tools/claims.py --lint   build.py lint: every file well formed, no two
                                   claims meeting in a space or a unit, at most
                                   one changing Run
Standard library only.
"""
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CLAIMS = 'docs/project/claims'


def number(value, where):
    try:
        return int(value, 0) if isinstance(value, str) else int(value) if not isinstance(value, bool) else int('x')
    except (TypeError, ValueError):
        raise ValueError(f'{where}: {value!r} is not a number') from None


def read():
    """Every claim: {branch: (takes as {space: [(first, last)]}, units, run, what)}; and what is wrong."""
    out, bad = {}, []
    folder = os.path.join(ROOT, CLAIMS)
    for name in sorted(os.listdir(folder)) if os.path.isdir(folder) else []:
        if not name.endswith('.json'):
            continue
        path = f'{CLAIMS}/{name}'
        try:
            with open(os.path.join(folder, name), encoding='utf-8') as f:
                claim = json.load(f)
            if not isinstance(claim, dict) or claim.get('branch') != name[:-5]:
                raise ValueError(f'{path}: "branch" is not the file\'s name, {name[:-5]}')
            if not isinstance(claim.get('what'), str) or not claim['what'].strip():
                raise ValueError(f'{path}: "what" is missing: a clause on the work')
            takes = {}
            for space, ranges in (claim.get('takes') or {}).items():
                takes[space] = []
                for r in ranges:
                    if not isinstance(r, list) or len(r) != 2:
                        raise ValueError(f'{path}: {space}: {r!r} is not [first, last]')
                    first, last = (number(v, f'{path}: {space}') for v in r)
                    if first > last:
                        raise ValueError(f'{path}: {space}: {r!r} ends before it starts')
                    takes[space].append((first, last))
            units = claim.get('units') or []
            if not isinstance(units, list) or not all(isinstance(u, str) for u in units):
                raise ValueError(f'{path}: "units" is not a list of names')
            if not isinstance(claim.get('run', False), bool):
                raise ValueError(f'{path}: "run" is not true or false')
            unknown = set(claim) - {'branch', 'what', 'takes', 'units', 'run', 'by'}
            if unknown:
                raise ValueError(f'{path}: {", ".join(sorted(unknown))}: not a claim\'s key')
            out[name[:-5]] = (takes, units, claim.get('run', False), claim['what'])
        except (ValueError, json.JSONDecodeError) as error:
            bad.append(str(error) if str(error).startswith(CLAIMS) else f'{path}: {error}')
    return out, bad


def meetings(claims):
    """Where two claims take the same thing."""
    out, names = [], sorted(claims)
    for i, a in enumerate(names):
        for b in names[i + 1:]:
            for space, mine in claims[a][0].items():
                for first, last in mine:
                    for f2, l2 in claims[b][0].get(space, []):
                        if first <= l2 and f2 <= last:
                            out.append(f'{a} and {b} both take {space} {max(first, f2):#x} to {min(last, l2):#x}')
            for unit in sorted(set(claims[a][1]) & set(claims[b][1])):
                out.append(f'{a} and {b} both take the unit {unit}')
    runs = [n for n in names if claims[n][2]]
    if len(runs) > 1:
        out.append(f'{", ".join(runs)} all change Run: one at a time (its size and save_legacy.c\'s reading of the last)')
    return out


def main():
    args = sys.argv[1:]
    if args not in ([], ['--lint']):
        sys.exit(__doc__)
    claims, bad = read()
    met = meetings(claims)
    if not args:
        for name, (takes, units, run, what) in claims.items():
            spans = '; '.join(f'{space} ' + ', '.join(f'{a:#x}-{b:#x}' for a, b in ranges) for space, ranges in takes.items())
            print(f'{name}: {what}' + (f' | {spans}' if spans else '') + (f' | {", ".join(units)}' if units else '')
                  + (' | changes Run' if run else ''))
    for line in bad + met:
        print(f'claims: {line}')
    print(f'claims: {len(claims)} in flight, {len(bad)} malformed, {len(met)} meeting')
    return 1 if bad or met else 0


if __name__ == '__main__':
    sys.exit(main())
