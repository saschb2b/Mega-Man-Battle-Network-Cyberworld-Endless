#!/usr/bin/env python3
"""generated.py [write | check [--strict] | restore [REF]]: the files that
tools write from other files of the tree, kept out of pull requests' way
(docs/project/merging.md).

  docs/re/state.json, STATUS.md, dashboard.html   tools/re_status.py, from docs/re/ledger.json
  docs/symbols/*                                  tools/symbols.py, from the headers, rom.c and docs/rom/
  docs/rom/symbols.md's "holdings" part           the same
  docs/voice/catalog.json                         tools/voice_catalog.py, from the words files

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
                                        --strict (and on main, a tag and a pull
                                        request to main): the files in step with
                                        their sources instead
  python3 tools/generated.py restore    put them back as that commit has them. With a
                                        merge of dev under way: take dev's, staged,
                                        which resolves every conflict in them
Standard library only; needs no ROM and nothing outside git.
"""
import glob
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOLS = ('re_status.py', 'symbols.py', 'voice_catalog.py')
FILES = ('docs/re/state.json', 'docs/re/STATUS.md', 'docs/re/dashboard.html', 'docs/voice/catalog.json')
FOLDERS = ('docs/symbols/',)
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
    found = git('merge-base', 'HEAD', DEV, check=False)
    return found.strip() if found else None


def touched(ref):
    """The generated files and parts that differ here from the commit's."""
    out = [path for path in whole(ref) if here(path) != at(ref, path)]
    for path, marks in PARTS.items():
        mine, theirs = here(path), at(ref, path)
        if mine is None or theirs is None:
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
    bad = max(run(tool, '--check' if strict else '--lint') for tool in TOOLS)
    ref = None if strict else base()
    if strict:
        print('generated: in step with their sources' if not bad else 'generated: out of step: python3 tools/generated.py write')
    elif ref is None:
        print('generated: no branch to check (a push to dev)' if env.get('GITHUB_ACTIONS') else
              f'generated: {DEV} is not known here, so whether this branch changes a generated file was not checked')
    else:
        changed = touched(ref.strip())
        for path in changed:
            print(f'generated: {path} differs from dev\'s: dev\'s workflow writes it, a branch leaves it '
                  f'(docs/project/merging.md): python3 tools/generated.py restore')
        print(f'generated: {len(changed)} generated files changed by this branch')
        bad = bad or bool(changed)
    return 1 if bad else 0


def restore(ref):
    merging = git('rev-parse', '--verify', '--quiet', 'MERGE_HEAD', check=False)
    if ref is None:
        ref = merging.strip() if merging else base()
    if ref is None:
        sys.exit(f'generated: {DEV} is not known here: git fetch origin dev, or name the commit')
    ref = ref.strip()
    done = []
    for path in whole(ref):
        if here(path) != at(ref, path) or merging:
            put(path, at(ref, path))
            done.append(path)
    for path, marks in PARTS.items():
        theirs = at(ref, path)
        new = parts(theirs, marks) if theirs is not None else None
        if new is None:
            continue
        if merging:
            # the page's own three-way merge, each side's generated parts set to dev's first:
            # what a person wrote merges as git merges it, what the tool wrote cannot conflict
            sides = []
            for stage in (2, 1, 3):
                text = git('show', f':{stage}:{path}', check=False)
                if text is None:   # no conflict in it: git merged it already
                    break
                sides.append(with_parts(text, marks, new) if parts(text, marks) is not None else text)
            else:
                names = [os.path.join(ROOT, '.git-generated-' + n) for n in ('ours', 'base', 'theirs')]
                try:
                    for name, text in zip(names, sides):
                        with open(name, 'w', encoding='utf-8', newline='\n') as f:
                            f.write(text)
                    r = subprocess.run(['git', 'merge-file', '-p', '-L', 'ours', '-L', 'base', '-L', 'dev', *names],
                                       cwd=ROOT, capture_output=True, text=True, encoding='utf-8')
                finally:
                    for name in names:
                        if os.path.exists(name):
                            os.remove(name)
                put(path, r.stdout)
                if r.returncode == 0:
                    done.append(path)
                else:
                    print(f'generated: {path}: a conflict is left in what a person wrote; resolve it by hand')
                continue
        mine = here(path)
        if mine is not None and parts(mine, marks) not in (None, new):
            put(path, with_parts(mine, marks, new))
            done.append(path)
    if merging and done:
        git('add', '-A', '--', *done)
    for path in done:
        print(f'generated: {path} as {ref[:9]} has it')
    print(f'generated: {len(done)} files put back' + (', staged for the merge' if merging and done else ''))
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
