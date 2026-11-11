#!/usr/bin/env python3
"""shards.py [check | absorb [--base REF] | split FILE]: the registries
every branch adds to, each kept as a folder with a file an entry and read as
one (docs/project/merging.md, Registries).

  docs/re/ledger/           the research ledger (tools/re_status.py)
  tests/xdiff/mutations/    the differential's mutations, a file a spec (tools/xdiff.py --mutations)
  tests/xdiff/legit/        what may differ between the games (tools/xdiff.py, tools/xdiff_bn4.py)

One JSON file for all of a registry made any two branches that each added
an entry conflict at its end. A new entry is now a new file, so two
branches' entries merge without either knowing of the other; an entry two
branches change is still a conflict, in that entry's file alone.

How a folder is read (LAYOUTS): a `dict` folder gives a key per file
(NAME.json) and per folder in it, and where it has a `rest` file, that
file's keys too; a `list` folder gives its files' values in the order of
their names; a `groups` folder gives its files' lists one after another
under one key, each file holding the entries whose field (a mutation's spec)
is its name, beside its `rest` file's keys.

  python3 tools/shards.py check     build.py lint: every registry reads whole, and
                                    none of the old single files is back
  python3 tools/shards.py absorb    for a branch that still changed an old single
                                    file: in a merge of dev (the file's conflict
                                    "deleted by them"), what the branch added or
                                    changed there carried into the folder, entry
                                    by entry, the old file removed, both staged.
                                    --base REF: outside a merge, the old file
                                    here against REF's
  python3 tools/shards.py split FILE   an old single file written out as its folder
                                    (how the folders were made)
Standard library only.
"""
import hashlib
import json
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def slug(text, most=48):
    """A file name's words from an entry's: lower case, the rest as dashes."""
    words = re.sub(r"['’]", '', str(text).lower())
    words = re.sub(r'[^a-z0-9]+', '-', words).strip('-')
    return words[:most].rstrip('-') or 'entry'


def digest(value):
    return hashlib.sha1(canon(value).encode()).hexdigest()[:8]


def numbered(entry, number):
    return f'{number:03d}-{slug(entry.get("name") or entry.get("what") or digest(entry))}'


def dated(*fields):
    """A file named by its entry's date and its words: no number to take, so two branches never want the same."""
    def name(entry, number):
        return f'{entry.get("date") or "undated"}-{slug(" ".join(str(entry[f]) for f in fields if entry.get(f)))}'
    return name


# Each registry: the old single file, its folder, the indent its JSON is written
# with, and how the folder is laid out. A dict's `children` are its keys kept
# as folders; every other key is a file of its own, or a key of its `rest` file.
LAYOUTS = {
    'ledger': dict(
        old='docs/re/ledger.json', folder='docs/re/ledger', indent=2,
        drop=('updated', 'previous'),   # (dates tools/re_status.py reads off the entries now)
        layout=dict(kind='dict', rest='ledger.json', children={
            'features': dict(kind='list', name=numbered),
            'changes': dict(kind='list', name=dated('what')),
            'experiments': dict(kind='list', name=dated('feature', 'name')),
        })),
    'mutations': dict(
        old='tests/xdiff/mutations.json', folder='tests/xdiff/mutations', indent=1,
        layout=dict(kind='groups', by='spec', key='mutations', rest='about.json')),
    'legit': dict(
        old='tests/xdiff/legit.json', folder='tests/xdiff/legit', indent=1,
        layout=dict(kind='dict', children={
            'maps': dict(kind='dict'),
            'layouts': dict(kind='dict'),
            'masks': dict(kind='list', name=numbered),
            'bn4': dict(kind='dict', children={
                'allowed': dict(kind='list', name=numbered),
                'sounds': dict(kind='list', name=numbered),
            }),
        })),
}


class ShardError(ValueError):
    pass


def identity(entry):
    """What makes an entry of a list the same entry after its words changed: its name, within its spec or feature."""
    if isinstance(entry, dict) and isinstance(entry.get('name'), str):
        return ', '.join(str(entry[k]) for k in ('spec', 'feature', 'name') if entry.get(k) is not None)
    return None


def canon(value):
    return json.dumps(value, sort_keys=True, ensure_ascii=False)


def read_json(path):
    try:
        with open(path, encoding='utf-8') as f:
            return json.load(f)
    except (OSError, json.JSONDecodeError) as error:
        raise ShardError(f'{os.path.relpath(path, ROOT)}: {error}') from None


def names(folder):
    """The folder's JSON files' names without .json, and its folders', each in order."""
    if not os.path.isdir(folder):
        raise ShardError(f'{os.path.relpath(folder, ROOT)}: no such folder')
    entries = sorted(os.listdir(folder))
    return ([n[:-5] for n in entries if n.endswith('.json') and os.path.isfile(os.path.join(folder, n))],
            [n for n in entries if os.path.isdir(os.path.join(folder, n))])


def read(folder, layout):
    """The folder as the one value its old file held."""
    files, folders = names(folder)
    kind = layout['kind']
    if kind == 'list':
        return [read_json(os.path.join(folder, n + '.json')) for n in files]
    if kind == 'groups':
        out, rest = [], layout['rest']
        for n in files:
            if n + '.json' == rest:
                continue
            part = read_json(os.path.join(folder, n + '.json'))
            if not isinstance(part, list):
                raise ShardError(f'{os.path.relpath(folder, ROOT)}/{n}.json: not a list')
            for entry in part:
                if not isinstance(entry, dict) or entry.get(layout['by']) != n:
                    raise ShardError(f'{os.path.relpath(folder, ROOT)}/{n}.json: an entry whose "{layout["by"]}" is not {n}')
            out += part
        return dict(read_json(os.path.join(folder, rest)), **{layout['key']: out})
    out, rest = {}, layout.get('rest')
    children = layout.get('children', {})
    if rest:
        out.update(read_json(os.path.join(folder, rest)))
    for n in files:
        if n + '.json' != rest:
            out[n] = read_json(os.path.join(folder, n + '.json'))
    for n in folders:
        if n not in children:
            raise ShardError(f'{os.path.relpath(folder, ROOT)}/{n}: a folder the layout does not know (tools/shards.py, LAYOUTS)')
        out[n] = read(os.path.join(folder, n), children[n])
    return out


TEXTS = {}   # a registry's whole JSON, read once a process


def load(registry, fresh=False):
    """A registry whole: what its single file held (tools read it through
    this). A value of the caller's own each time, as parsing the single file
    gave; the files are read once a process, or again where `fresh`."""
    if fresh or registry not in TEXTS:
        spec = LAYOUTS[registry]
        TEXTS[registry] = json.dumps(read(os.path.join(ROOT, spec['folder']), spec['layout']), ensure_ascii=False)
    return json.loads(TEXTS[registry])


def text_of(value, indent):
    return json.dumps(value, indent=indent, ensure_ascii=False) + '\n'


def put(path, value, indent, changed):
    text = text_of(value, indent)
    try:
        with open(path, encoding='utf-8') as f:
            if f.read() == text:
                return
    except OSError:
        pass
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(text)
    changed.append(os.path.relpath(path, ROOT))


def drop(path, changed):
    os.remove(path)
    changed.append(os.path.relpath(path, ROOT))


def write(folder, layout, value, indent, changed):
    """The value into its folder. An entry a file already holds keeps that
    file, so writing a registry back as read changes nothing; a new entry
    takes a new file, and a file whose entry is gone goes."""
    os.makedirs(folder, exist_ok=True)
    files, _ = names(folder)
    kind = layout['kind']
    if kind == 'list':
        held, named = {}, {}
        for n in files:
            was = read_json(os.path.join(folder, n + '.json'))
            held.setdefault(canon(was), []).append(n)
            if identity(was):
                named.setdefault(identity(was), []).append(n)
        same = {canon(e) for e in value}
        taken = set(files)
        number = max([int(m.group(1)) for m in (re.match(r'(\d+)-', n) for n in files) if m] or [0])
        kept = set()
        for entry in value:
            have = held.get(canon(entry))
            if have:
                kept.add(have.pop(0))
                continue
            # an entry whose words changed keeps its file (and so its place): the one of its name, whose old words are gone
            was = [n for n in named.get(identity(entry), []) if n not in kept
                   and canon(read_json(os.path.join(folder, n + '.json'))) not in same]
            if identity(entry) and was:
                kept.add(was[0])
                put(os.path.join(folder, was[0] + '.json'), entry, indent, changed)
                continue
            number += 1
            stem = base = layout['name'](entry, number)
            k = 1
            while stem in taken:
                k += 1
                stem = f'{base}-{k}'
            taken.add(stem)
            kept.add(stem)
            put(os.path.join(folder, stem + '.json'), entry, indent, changed)
        for n in files:
            if n not in kept:
                drop(os.path.join(folder, n + '.json'), changed)
        return
    if kind == 'groups':
        groups = {layout['rest'][:-5]: {k: v for k, v in value.items() if k != layout['key']}}
        for entry in value[layout['key']]:
            groups.setdefault(entry[layout['by']], []).append(entry)
        for n, part in groups.items():
            put(os.path.join(folder, n + '.json'), part, indent, changed)
        for n in files:
            if n not in groups:
                drop(os.path.join(folder, n + '.json'), changed)
        return
    rest, children = layout.get('rest'), layout.get('children', {})
    loose = {k: v for k, v in value.items() if k not in children}
    if rest:
        put(os.path.join(folder, rest), loose, indent, changed)
    else:
        for k, v in loose.items():
            put(os.path.join(folder, k + '.json'), v, indent, changed)
    for n in files:
        if n + '.json' != rest and (rest or n not in loose):
            drop(os.path.join(folder, n + '.json'), changed)
    for k, child in children.items():
        if k in value:
            write(os.path.join(folder, k), child, value[k], indent, changed)


def store(registry, value):
    """The whole value written into the registry's folder; the files it changed."""
    spec, changed = LAYOUTS[registry], []
    write(os.path.join(ROOT, spec['folder']), spec['layout'], value, spec['indent'], changed)
    TEXTS.pop(registry, None)
    return changed


# ---- a branch's changes to an old single file, carried into the folder ----

def keyed(items):
    """A list as a dict by identity, where every entry has one of its own; else None."""
    ids = [identity(e) for e in items]
    return dict(zip(ids, items)) if None not in ids and len(set(ids)) == len(ids) else None


def merge3(base, ours, theirs, path, clashes):
    """Three-way, by JSON's own structure: what the branch (ours) changed
    since base, put onto dev's (theirs). A place both changed otherwise keeps
    dev's and is reported with the branch's value."""
    if canon(ours) == canon(base) or canon(ours) == canon(theirs):
        return theirs
    if canon(theirs) == canon(base):
        return ours
    if all(isinstance(v, dict) for v in (ours, theirs)) and isinstance(base, (dict, type(None))):
        base, out = base or {}, {}
        for k in list(theirs) + [k for k in ours if k not in theirs]:
            if k in ours and k in theirs:
                out[k] = merge3(base.get(k), ours[k], theirs[k], f'{path}/{k}', clashes)
            elif k in theirs:   # the branch has none: it removed it, or dev added it
                if k in base and canon(theirs[k]) != canon(base[k]):
                    clashes.append((f'{path}/{k}', 'removed by the branch, changed on dev: kept', None))
                if k not in base or canon(theirs[k]) != canon(base[k]):
                    out[k] = theirs[k]
            elif k not in base:   # the branch's own addition
                out[k] = ours[k]
            elif canon(ours[k]) != canon(base[k]):
                clashes.append((f'{path}/{k}', 'changed by the branch, removed on dev: left out', ours[k]))
        return out
    if all(isinstance(v, list) for v in (ours, theirs)) and isinstance(base, (list, type(None))):
        base = base or []
        k_base, k_ours, k_theirs = keyed(base), keyed(ours), keyed(theirs)
        if None not in (k_base, k_ours, k_theirs):
            merged = merge3(k_base, k_ours, k_theirs, path, clashes)
            return list(merged.values())
        if all(isinstance(e, dict) for e in ours + theirs + base):
            had, mine = {canon(e) for e in base}, {canon(e) for e in ours}
            out = [e for e in theirs if canon(e) not in had or canon(e) in mine]   # (what the branch removed goes)
            have = {canon(e) for e in theirs}
            return out + [e for e in ours if canon(e) not in had and canon(e) not in have]
    clashes.append((path, 'changed by the branch and on dev: dev\'s kept', ours))
    return theirs


def git(*args, check=True):
    r = subprocess.run(['git', *args], cwd=ROOT, capture_output=True, text=True, encoding='utf-8')
    if check and r.returncode:
        sys.exit(f'shards: git {" ".join(args)}: {r.stderr.strip()}')
    return r.stdout if r.returncode == 0 else None


def absorb(base_ref):
    done = 0
    for registry, spec in LAYOUTS.items():
        old = spec['old']
        if base_ref:
            base_text = git('show', f'{base_ref}:{old}', check=False)
            path = os.path.join(ROOT, old)
            ours_text = open(path, encoding='utf-8').read() if os.path.exists(path) else None
        else:
            base_text, ours_text = (git('show', f':{stage}:{old}', check=False) for stage in (1, 2))
        if ours_text is None:
            continue   # the branch never changed it, or it is gone already
        if base_text is None:
            sys.exit(f'shards: {old}: no base to read the branch\'s changes against (--base REF)')
        base, ours = json.loads(base_text), json.loads(ours_text)
        for key in spec.get('drop', ()):
            base.pop(key, None)
            ours.pop(key, None)
        clashes = []
        merged = merge3(base, ours, load(registry, fresh=True), registry, clashes)
        changed = store(registry, merged)
        git('rm', '-q', '-f', '--ignore-unmatch', '--', old)
        if os.path.exists(os.path.join(ROOT, old)):
            os.remove(os.path.join(ROOT, old))
        git('add', '-A', '--', spec['folder'])
        for path in changed:
            print(f'shards: {path}')
        for where, what, value in clashes:
            print(f'shards: BY HAND {where}: {what}' + (f'\n        the branch\'s: {canon(value)[:400]}' if value is not None else ''))
        print(f'shards: {old}: {len(changed)} files of {spec["folder"]}/ written from the branch\'s changes, '
              f'{len(clashes)} places left to merge by hand; the old file removed')
        done += 1
    if not done:
        print('shards: no old single file changed by this branch: nothing to carry over')
    return 0


def check():
    bad = 0
    for registry, spec in LAYOUTS.items():
        try:
            value = load(registry)
            # (every file where writing the registry back would put it: no stray file, no entry in the wrong one)
            stray = dry(registry, value)
            if stray:
                raise ShardError(f'{spec["folder"]}: not as tools/shards.py writes it: {", ".join(stray[:5])}')
        except ShardError as error:
            print(f'shards: {error}')
            bad += 1
            continue
        if os.path.exists(os.path.join(ROOT, spec['old'])):
            print(f'shards: {spec["old"]} is back: its entries live in {spec["folder"]}/ now (python3 tools/shards.py absorb)')
            bad += 1
        print(f'shards: {spec["folder"]}/ reads whole')
    return 1 if bad else 0


def dry(registry, value):
    """The files writing the value back would change, nothing written: the folder's own text, to its last space."""
    spec = LAYOUTS[registry]
    stray = []

    def walk(folder, layout, val):
        files, _ = names(folder)
        kind = layout['kind']
        want = {}
        if kind == 'list':
            for n in files:   # (a list's files keep their names: only their text is held)
                want[n] = read_json(os.path.join(folder, n + '.json'))
        elif kind == 'groups':
            want[layout['rest'][:-5]] = {k: v for k, v in val.items() if k != layout['key']}
            for entry in val[layout['key']]:
                want.setdefault(entry[layout['by']], []).append(entry)
        else:
            rest, children = layout.get('rest'), layout.get('children', {})
            loose = {k: v for k, v in val.items() if k not in children}
            if rest:
                want[rest[:-5]] = loose
            else:
                want.update(loose)
            for k, child in children.items():
                if k in val:
                    walk(os.path.join(folder, k), child, val[k])
        for n in sorted(set(files) | set(want)):
            path = os.path.join(folder, n + '.json')
            text = open(path, encoding='utf-8').read() if n in files else None
            if n not in want or text != text_of(want[n], spec['indent']):
                stray.append(os.path.relpath(path, ROOT))

    walk(os.path.join(ROOT, spec['folder']), spec['layout'], value)
    return stray


def split(path):
    rel = os.path.relpath(os.path.abspath(path), ROOT).replace(os.sep, '/')
    for registry, spec in LAYOUTS.items():
        if spec['old'] == rel:
            value = read_json(os.path.join(ROOT, rel))
            for key in spec.get('drop', ()):
                value.pop(key, None)
            changed = store(registry, value)
            print(f'shards: {rel} as {len(changed)} files of {spec["folder"]}/')
            return 0
    sys.exit(f'shards: {rel} is no registry\'s old file ({", ".join(s["old"] for s in LAYOUTS.values())})')


def main():
    args = sys.argv[1:]
    try:
        if args in ([], ['check']):
            return check()
        if args[:1] == ['absorb'] and (len(args) == 1 or (len(args) == 3 and args[1] == '--base')):
            return absorb(args[2] if len(args) == 3 else None)
        if args[:1] == ['split'] and len(args) == 2:
            return split(args[1])
    except ShardError as error:
        sys.exit(f'shards: {error}')
    sys.exit(__doc__)


if __name__ == '__main__':
    sys.exit(main())
