#!/usr/bin/env python3
"""generated.py [write | check [--strict] | restore [REF]]: the files that
tools write from other files of the tree, kept out of pull requests' way
(docs/project/merging.md).

  docs/re/state.json, STATUS.md, dashboard.html   tools/re_status.py, from docs/re/ledger/
  docs/symbols/*                                  tools/symbols.py, from the headers, rom.c and docs/rom/
  docs/rom/symbols.md's "holdings" part           the same
  docs/voice/catalog.json                         tools/voice_catalog.py, from the words files
  docs/log.md                                     tools/log_fold.py, from the entries' files in docs/log.d/
  each concept's `generated: { by, at }` line     the same, from the entry that names the concept
  tests/layer_make_hash.txt                       tools/layer_make.py, from the unit tests, where a bump was added

Every pull request that names a ROM fact, moves a feature's status or
changes a line changes what these hold, so two of them always conflicted
there. They are therefore written on dev alone, by the workflow
.github/workflows/generated.yml after each push to it; a branch leaves
them as its base has them.

  python3 tools/generated.py write      write them all (the workflow; or to look at
                                        one here: `restore` before committing)
  python3 tools/generated.py check      build.py lint: their sources are sound (each
                                        tool's --lint) and this branch leaves the
                                        files as the commit it left dev at has them.
                                        A branch that changes a tool itself writes
                                        that tool's files, and is held to their
                                        being in step instead.
                                        --strict (and on main, a tag and a pull
                                        request to main): the files in step with
                                        their sources instead
  python3 tools/generated.py restore    put them back as that commit has them. With a
                                        merge of dev under way: take dev's, staged,
                                        which resolves every conflict in them. Lines
                                        the branch had added to docs/log.md are kept,
                                        as an entry's file in docs/log.d/
Standard library only; needs no ROM and nothing outside git (the layers'
make's record needs the build image, and only where a bump was added).
"""
import glob
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOLS = ('re_status.py', 'symbols.py', 'voice_catalog.py', 'log_fold.py', 'layer_make.py')
FILES = ('docs/re/state.json', 'docs/re/STATUS.md', 'docs/re/dashboard.html', 'docs/voice/catalog.json', 'docs/log.md',
         'tests/layer_make_hash.txt')
FOLDERS = ('docs/symbols/',)
# Each tool: its own files (a branch that changes one changes what the tool writes, and so
# writes it), and where what it writes lies.
OWN = {
    're_status.py': (('tools/re_status.py', '.claude/skills/reverse-engineering/assets/dashboard.html'), ('docs/re/',)),
    'symbols.py': (('tools/symbols.py',), ('docs/symbols/', 'docs/rom/symbols.md')),
    'voice_catalog.py': (('tools/voice_catalog.py',), ('docs/voice/',)),
    'log_fold.py': ((), ('docs/log.md',)),   # (it folds entries in and never writes the log anew: no branch's to write)
    'layer_make.py': (('tools/layer_make.py',), ('tests/layer_make_hash.txt',)),
}
LOG = 'docs/log.md'
# A page a person writes with parts a tool writes: the page, the marks around each part.
PARTS = {'docs/rom/symbols.md': (('<!-- tools/symbols.py writes this part: holdings -->',
                                  '<!-- end of what tools/symbols.py writes -->'),)}
DEV = 'origin/dev'


def git(*args, check=True):
    r = subprocess.run(['git', *args], cwd=ROOT, capture_output=True, text=True, encoding='utf-8')
    if check and r.returncode:
        sys.exit(f'generated: git {" ".join(args)}: {r.stderr.strip()}')
    return r.stdout if r.returncode == 0 else None


def at(ref, path):
    """The file as a commit has it; None where it has none."""
    return git('show', f'{ref}:{path}', check=False)


def here(path):
    try:
        with open(os.path.join(ROOT, path), encoding='utf-8') as f:
            return f.read()
    except OSError:
        return None


def put(path, text):
    full = os.path.join(ROOT, path)
    if text is None:
        if os.path.exists(full):
            os.remove(full)
        return
    os.makedirs(os.path.dirname(full), exist_ok=True)
    with open(full, 'w', encoding='utf-8', newline='\n') as f:
        f.write(text)


def whole(ref=None):
    """Every file a tool writes whole: here, and in the commit where one is named."""
    found = set(FILES)
    for folder in FOLDERS:
        found |= {os.path.relpath(p, ROOT).replace(os.sep, '/') for p in glob.glob(os.path.join(ROOT, folder, '*'))}
        if ref:
            found |= set((git('ls-tree', '-r', '--name-only', ref, '--', folder, check=False) or '').split())
    return sorted(found)


def parts(text, marks):
    """What stands between each pair of marks, in order; None where a mark is missing."""
    out, pos = [], 0
    for begin, end in marks:
        a = text.find(begin, pos)
        b = text.find(end, a + len(begin)) if a >= 0 else -1
        if b < 0:
            return None
        out.append(text[a + len(begin):b])
        pos = b
    return out


def with_parts(text, marks, new):
    """The page with its marked parts replaced by `new`."""
    out, pos = '', 0
    for (begin, end), part in zip(marks, new):
        a = text.index(begin, pos) + len(begin)
        b = text.index(end, a)
        out += text[pos:a] + part
        pos = b
    return out + text[pos:]


def base():
    """The commit this branch left dev at (a pull request's merge in CI: dev's
    own tip); None where dev is not known here."""
    if os.environ.get('GITHUB_EVENT_NAME') == 'pull_request':
        return git('rev-parse', '--verify', '--quiet', 'HEAD^1', check=False)   # (the checkout is the merge, two commits deep)
    if os.environ.get('GITHUB_ACTIONS'):
        return None   # a push: dev's own commits, the workflow's among them
    merging = git('rev-parse', '--verify', '--quiet', 'MERGE_HEAD', check=False)
    if merging:   # (a merge of dev under way: its commit is where the branch will have left dev)
        return merging.strip()
    found = git('merge-base', 'HEAD', DEV, check=False)
    return found.strip() if found else None


def reworked(ref):
    """The tools this branch changes: theirs to write, in step."""
    return [tool for tool, (own, _) in OWN.items() if any(here(path) != at(ref, path) for path in own)]


def fold():
    sys.path.insert(0, os.path.join(ROOT, 'tools'))
    import log_fold
    return log_fold


def touched(ref, but=()):
    """The generated files and parts that differ here from the commit's, but those of the tools named."""
    free = tuple(prefix for tool in but for prefix in OWN[tool][1])
    out = [path for path in whole(ref) if here(path) != at(ref, path) and not path.startswith(free)]
    out += [f'{path} (its `generated` line)' for path in fold().stamps(ref) if not path.startswith(free) and path not in out]
    for path, marks in PARTS.items():
        mine, theirs = here(path), at(ref, path)
        if mine is None or theirs is None or path.startswith(free):
            continue
        if parts(mine, marks) != parts(theirs, marks):
            out.append(f'{path} (the part a tool writes)')
    return out


def run(tool, *args):
    sys.stdout.flush()
    return subprocess.call([sys.executable, os.path.join(ROOT, 'tools', tool), *args], cwd=ROOT)


def write():
    return max(run(tool) for tool in TOOLS)


def check(strict):
    env = os.environ
    # main and a tag hold the files in step, and so does a pull request to anything but dev (a release's)
    strict = strict or (env.get('GITHUB_EVENT_NAME') == 'push'
                        and (env.get('GITHUB_REF', '') == 'refs/heads/main' or env.get('GITHUB_REF', '').startswith('refs/tags/')))
    strict = strict or (env.get('GITHUB_EVENT_NAME') == 'pull_request' and env.get('GITHUB_BASE_REF') != 'dev')
    ref = None if strict else base()
    ref = ref.strip() if ref else None
    own = reworked(ref) if ref else []
    bad = max(run(tool, '--check' if strict or tool in own else '--lint') for tool in TOOLS)
    for tool in own:
        print(f'generated: tools/{tool} is changed by this branch, which so writes its files: held to their being in step')
    if strict:
        print('generated: in step with their sources' if not bad else 'generated: out of step: python3 tools/generated.py write')
    elif ref is None:
        print('generated: no branch to check (a push to dev)' if env.get('GITHUB_ACTIONS') else
              f'generated: {DEV} is not known here, so whether this branch changes a generated file was not checked')
    else:
        changed = touched(ref, own)
        for path in changed:
            print(f'generated: {path} differs from dev\'s: dev\'s workflow writes it, a branch leaves it '
                  f'(docs/project/merging.md): python3 tools/generated.py restore')
        print(f'generated: {len(changed)} generated files changed by this branch')
        bad = bad or bool(changed)
    return 1 if bad else 0


MARKER = re.compile(r'^(<<<<<<<|>>>>>>>|\|\|\|\|\|\|\|)( [^\n]*)?$', re.M)
# A conflict whose sides are `generated` lines and nothing else (with the base's between, where git shows it).
STAMP_CONFLICT = re.compile(r'^<<<<<<<[^\n]*\n(?:generated:[^\n]*\n)*(?:\|\|\|\|\|\|\|[^\n]*\n(?:generated:[^\n]*\n)*)?'
                            r'=======\n(?:generated:[^\n]*\n)*>>>>>>>[^\n]*\n', re.M)


def marked(text):
    """Whether a conflict's markers stand in the text."""
    return bool(re.search(r'^<<<<<<<( |$)', text, re.M) and re.search(r'^>>>>>>>( |$)', text, re.M))


def three_way(path, fix=None):
    """A conflicted page's own three-way merge from the merge's three sides,
    each passed through `fix` first (the parts a tool writes set to dev's, so
    that they cannot conflict and what a person wrote merges as git merges
    it): (the text, whether it came out clean). None where the page is not in
    conflict in the index. Nothing is written."""
    sides = []
    for stage in (2, 1, 3):
        text = git('show', f':{stage}:{path}', check=False)
        if text is None:
            return None
        sides.append(fix(text) if fix else text)
    style = (git('config', '--get', 'merge.conflictStyle', check=False) or '').strip()
    names = [os.path.join(ROOT, '.git-generated-' + n) for n in ('ours', 'base', 'theirs')]
    try:
        for name, text in zip(names, sides):
            with open(name, 'w', encoding='utf-8', newline='\n') as f:
                f.write(text)
        cmd = ['git', 'merge-file', '-p', '-L', 'ours', '-L', 'base', '-L', 'dev'] + ([f'--{style}'] if style in ('diff3', 'zdiff3') else [])
        # (as git's own merge compares, where this git's merge-file can)
        r = subprocess.run(cmd + ['--diff-algorithm=histogram'] + names, cwd=ROOT, capture_output=True, text=True, encoding='utf-8')
        if r.returncode < 0 or r.returncode > 127:
            r = subprocess.run(cmd + names, cwd=ROOT, capture_output=True, text=True, encoding='utf-8')
    finally:
        for name in names:
            if os.path.exists(name):
                os.remove(name)
    return r.stdout, r.returncode == 0


def untouched(path):
    """Whether the conflicted page in the working tree is still as git's merge
    wrote it: nobody has resolved any of it by hand. Only then may it be merged
    anew; a page someone has worked on is theirs."""
    mine, again = here(path), three_way(path)
    plain = lambda text: MARKER.sub(lambda m: m.group(1), text)   # noqa: E731  (the markers without their labels)
    return mine is not None and again is not None and plain(mine) == plain(again[0])


def settle(path, fix, conflicted, mend=None):
    """A page a person writes with a part a tool writes, in a merge or out of
    one: the tool's part made dev's and nothing else of the page changed,
    unless nobody has touched its conflict and setting that part aside
    resolves all of it. `fix` makes a side's part dev's; `mend` resolves, in
    the page as it stands, a conflict that is the tool's part alone. Returns
    'done' (the tool resolved or restored it: to stage), 'kept' (a person's
    resolution stands, the part set under it), 'left' (a conflict in what a
    person wrote is still there) or None (nothing to do)."""
    mine = here(path)
    if mine is None:
        return None
    if path in conflicted and untouched(path):
        merged = three_way(path, fix)
        if merged and merged[1]:
            put(path, merged[0])
            return 'done'
    was = marked(mine)
    text = fix(mend(mine) if mend and was else mine)
    if text != mine:
        put(path, text)
    if marked(text):
        return 'left'
    if path in conflicted and not was:
        return 'kept'
    return 'done' if text != mine or (path in conflicted and was) else None


def restore(ref):
    merging = git('rev-parse', '--verify', '--quiet', 'MERGE_HEAD', check=False)
    if ref is None:
        ref = merging.strip() if merging else base()
    if ref is None:
        sys.exit(f'generated: {DEV} is not known here: git fetch origin dev, or name the commit')
    ref = ref.strip()
    done, left, kept = [], [], []
    log_fold = fold()
    # what the branch logged the old way, in the log itself, kept as an entry's file before the log is dev's again
    mine = git('show', f':2:{LOG}', check=False) if merging else None
    for path in log_fold.carry(ref, mine, 'HEAD' if merging else None):
        print(f'generated: {path}: the lines this branch had added to {LOG}')
        if merging:
            git('add', '--', path)
    for path in whole(ref):
        theirs = at(ref, path)
        if theirs is None and here(path) is None:
            continue
        if here(path) != theirs or merging:
            put(path, theirs)
            done.append(path)
    conflicted = set((git('diff', '--name-only', '--diff-filter=U', check=False) or '').split()) if merging else set()
    sorts = {'done': done, 'left': left, 'kept': kept}
    for path, marks in PARTS.items():
        theirs = at(ref, path)
        new = parts(theirs, marks) if theirs is not None else None
        if new is None:
            continue
        fix = lambda text, m=marks, n=new: with_parts(text, m, n) if parts(text, m) is not None else text   # noqa: E731
        how = settle(path, fix, conflicted)
        if how:
            sorts[how].append(path)
    # each concept's `generated` line as dev has it: dev's workflow writes it, from the log's entry
    for path in sorted({p for p in conflicted if p.startswith('docs/')} | set(log_fold.stamps(ref))):
        theirs = at(ref, path)
        line = log_fold.STAMP.search(theirs) if theirs and path.endswith('.md') else None
        if not line:
            continue
        fix = lambda text, to=line.group(0): log_fold.STAMP.sub(lambda _: to, text, count=1)   # noqa: E731
        mend = lambda text, to=line.group(0): STAMP_CONFLICT.sub(lambda _: to + '\n', text)   # noqa: E731
        how = settle(path, fix, conflicted - set(done), mend)   # (a page the parts above already resolved is no longer in conflict)
        if how and path not in sorts[how]:
            for other in sorts.values():
                if path in other:
                    other.remove(path)
            sorts[how].append(path)
    if merging and done:
        git('add', '-A', '--', *done)
    for path in done:
        print(f'generated: {path} as {ref[:9]} has it')
    for path in kept:
        print(f'generated: {path}: resolved by hand, and left so; only its part a tool writes is dev\'s (yours to stage)')
    for path in left:
        print(f'generated: {path}: a conflict is left in what a person wrote; resolve it by hand')
    print(f'generated: {len(done)} files put back' + (', staged for the merge' if merging and done else '')
          + (f'; {len(kept)} resolved by hand kept' if kept else '') + (f'; {len(left)} left to resolve by hand' if left else ''))
    return 0


def main():
    args = sys.argv[1:]
    action = args[0] if args else 'check'
    if action == 'write' and len(args) == 1:
        return write()
    if action == 'check' and set(args[1:]) <= {'--strict'}:
        return check('--strict' in args)
    if action == 'restore' and len(args) <= 2:
        return restore(args[1] if len(args) == 2 else None)
    sys.exit(__doc__)


if __name__ == '__main__':
    sys.exit(main())
