#!/usr/bin/env python3
"""tools/generated.py restore in a merge of dev, on a repository of its own
made here: what a person wrote and already resolved is never written over
(batch C's pull request #446 lost a resolved page to it, conflict markers
and all), a page whose only conflict was its `generated` line comes out
resolved, and a real conflict nobody touched is left as git left it.

  python3 tests/test_generated.py     (build.py test runs it; needs git)
"""
import os
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FAILED = []
PAGES = 'docs'   # (the test's pages live in its own repository's docs folder)


def check(ok, what):
    if not ok:
        FAILED.append(what)
        print(f'FAIL {what}')


def page(by, at, lines):
    return (f'---\ntype: Reference\ntitle: "A page"\ndescription: "A page of the test."\ntags: [test]\n'
            f'generated: {{ by: {by}, at: {at} }}\n---\n\n# A page\n\n' + '\n'.join(lines) + '\n')


BODY = ['one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine', 'ten', 'eleven', 'twelve']
BASE = ('claude-code/base', '2026-11-01T00:00:00Z')
DEV = ('claude-code/dev', '2026-11-03T00:00:00Z')
MINE = ('claude-code/mine', '2026-11-02T00:00:00Z')
DEV_LINE = f'generated: {{ by: {DEV[0]}, at: {DEV[1]} }}'


def counted(by, at, count):
    """A page with a part a tool writes between its marks (docs/rom/symbols.md's holdings), holding a count."""
    return page(by, at, ['before', '<!-- tools/symbols.py writes this part: holdings -->', '', f'| All | {count} |', '',
                         '<!-- end of what tools/symbols.py writes -->', 'after'])


def changed(lines, **at):
    """The body with the lines named replaced: changed(BODY, two='TWO')."""
    return [at.get(line, line) for line in lines]


def main():
    tmp = tempfile.mkdtemp(prefix='cw-generated-')
    env = dict(os.environ, GIT_AUTHOR_NAME='t', GIT_AUTHOR_EMAIL='t@t', GIT_COMMITTER_NAME='t', GIT_COMMITTER_EMAIL='t@t',
               GIT_CONFIG_GLOBAL=os.devnull, GIT_CONFIG_SYSTEM=os.devnull)

    def git(*args, ok=(0,)):
        r = subprocess.run(['git', *args], cwd=tmp, env=env, capture_output=True, text=True)
        assert r.returncode in ok, f'git {" ".join(args)}: {r.stdout}{r.stderr}'
        return r.stdout

    def put(path, text):
        full = os.path.join(tmp, path)
        os.makedirs(os.path.dirname(full), exist_ok=True)
        with open(full, 'w', encoding='utf-8') as f:
            f.write(text)

    def get(path):
        with open(os.path.join(tmp, path), encoding='utf-8') as f:
            return f.read()

    try:
        for tool in ('generated.py', 'log_fold.py'):
            os.makedirs(os.path.join(tmp, 'tools'), exist_ok=True)
            shutil.copy(os.path.join(ROOT, 'tools', tool), os.path.join(tmp, 'tools', tool))
        git('init', '-q', '-b', 'dev')
        put('docs/log.md', "# The log\n\n## 2026-11-01\n* **Create**: a.md (the test's pages).\n")
        for name in 'abcde':
            put(f'{PAGES}/{name}.md', page(*BASE, BODY))
        put('docs/rom/symbols.md', counted(*BASE, 100))
        git('add', '-A')
        git('commit', '-q', '-m', 'base')
        git('checkout', '-q', '-b', 'work')
        # the branch: each page restamped; a, c, d and e changed where dev changes them too, b somewhere else
        put(f'{PAGES}/a.md', page(*MINE, changed(BODY, three='three, the branch\'s')))
        put(f'{PAGES}/b.md', page(*MINE, changed(BODY, two='two, the branch\'s')))
        put(f'{PAGES}/c.md', page(*BASE, changed(BODY, three='three, the branch\'s')))
        put(f'{PAGES}/d.md', page(*MINE, changed(BODY, three='three, the branch\'s')))
        put(f'{PAGES}/e.md', page(*MINE, changed(BODY, three='three, the branch\'s', ten='ten, the branch\'s')))
        put('docs/rom/symbols.md', counted(*MINE, 101))
        git('commit', '-q', '-am', 'the branch')
        git('checkout', '-q', 'dev')
        put(f'{PAGES}/a.md', page(*DEV, changed(BODY, three='three, dev\'s')))
        put(f'{PAGES}/b.md', page(*DEV, changed(BODY, eleven='eleven, dev\'s')))
        put(f'{PAGES}/c.md', page(*BASE, changed(BODY, three='three, dev\'s')))
        put(f'{PAGES}/d.md', page(*DEV, changed(BODY, three='three, dev\'s')))
        put(f'{PAGES}/e.md', page(*DEV, changed(BODY, three='three, dev\'s', ten='ten, dev\'s')))
        put('docs/rom/symbols.md', counted(*DEV, 102))
        git('commit', '-q', '-am', 'dev')
        git('checkout', '-q', 'work')
        git('merge', '-q', '--no-edit', 'dev', ok=(1,))
        unmerged = git('diff', '--name-only', '--diff-filter=U').split()
        check(unmerged == [f'{PAGES}/{n}.md' for n in 'abcde'] + ['docs/rom/symbols.md'], f'the merge conflicts in all six pages ({unmerged})')
        as_git_left_c = get(f'{PAGES}/c.md')

        # a: resolved by hand in the working tree, not staged (its `generated` line left the branch's)
        by_hand = page(*MINE, changed(BODY, three='three, both, by hand'))
        put(f'{PAGES}/a.md', by_hand)
        # d: resolved by hand and staged
        put(f'{PAGES}/d.md', by_hand)
        git('add', f'{PAGES}/d.md')
        # e: one of its two conflicts resolved by hand, the other left as git wrote it
        half = get(f'{PAGES}/e.md')
        start, end = half.index('<<<<<<<', half.index('---\n\n')), half.index('>>>>>>>', half.index('---\n\n'))
        end = half.index('\n', end) + 1
        check('three' in half[start:end], 'e\'s first text conflict is the line "three"')
        half = half[:start] + 'three, both, by hand\n' + half[end:]
        put(f'{PAGES}/e.md', half)

        r = subprocess.run([sys.executable, os.path.join(tmp, 'tools', 'generated.py'), 'restore'], cwd=tmp, env=env,
                           capture_output=True, text=True)
        check(r.returncode == 0, f'restore runs ({r.stdout[-300:]}{r.stderr[-300:]})')

        a = get(f'{PAGES}/a.md')
        check('<<<<<<<' not in a and '>>>>>>>' not in a, 'a, resolved by hand and unstaged: no conflict marker written back')
        check('three, both, by hand' in a and 'three, dev\'s' not in a, 'a: the text resolved by hand stands')
        check(DEV_LINE in a and MINE[0] not in a, 'a: its `generated` line is dev\'s')

        b = get(f'{PAGES}/b.md')
        check('<<<<<<<' not in b and DEV_LINE in b and 'two, the branch\'s' in b and 'eleven, dev\'s' in b,
              'b, whose only conflict was the `generated` line: resolved, both sides\' text in it')
        check(f'{PAGES}/b.md' not in git('diff', '--name-only', '--diff-filter=U').split(), 'b: staged')

        check(get(f'{PAGES}/c.md') == as_git_left_c, 'c, a real conflict nobody touched: left exactly as git left it')
        check(f'{PAGES}/c.md' in r.stdout and 'by hand' in r.stdout, 'c: named as left to resolve by hand')

        d = get(f'{PAGES}/d.md')
        check('<<<<<<<' not in d and 'three, both, by hand' in d and DEV_LINE in d, 'd, resolved by hand and staged: kept, its line dev\'s')

        e = get(f'{PAGES}/e.md')
        check('three, both, by hand' in e, 'e: the conflict resolved by hand stands')
        check(e.count('<<<<<<<') == 1 and 'ten, the branch\'s' in e and 'ten, dev\'s' in e, 'e: the conflict not yet resolved is still there, once')
        check(e.count('generated:') == 1 and DEV_LINE in e, 'e: its `generated` line\'s conflict is gone, the line dev\'s')

        counts = get('docs/rom/symbols.md')
        check('<<<<<<<' not in counts and '| All | 102 |' in counts and DEV_LINE in counts,
              'the page with a part a tool writes, nobody touched: resolved, its part and its line dev\'s')
        check('docs/rom/symbols.md' not in git('diff', '--name-only', '--diff-filter=U').split()
              and 'resolved by hand' not in r.stdout.split('docs/rom/symbols.md')[-1].split('\n')[0], 'that page: staged, not taken for a hand\'s work')

        # a second run changes nothing more
        before = {n: get(f'{PAGES}/{n}.md') for n in 'abcde'}
        subprocess.run([sys.executable, os.path.join(tmp, 'tools', 'generated.py'), 'restore'], cwd=tmp, env=env, capture_output=True)
        check(before == {n: get(f'{PAGES}/{n}.md') for n in 'abcde'}, 'restore twice is restore once')
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    print(f'generated: {len(FAILED)} failed')
    return 1 if FAILED else 0


if __name__ == '__main__':
    sys.exit(main())
