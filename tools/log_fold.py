#!/usr/bin/env python3
"""log_fold.py [--lint | --check | carry REF]: the knowledge base's log,
and each concept's `generated` line, written from entries each in a file
of its own (docs/project/merging.md, The log).

docs/log.md takes a line per change at its top, so every two branches
that touched docs/ conflicted there, and GitHub does not read the
`merge=union` that would have joined them. A branch therefore writes its
entry as a file, docs/log.d/DATE-WORDS.txt: the day it is logged under
(YYYY-MM-DD) and a few words that name it, holding who writes and when,
then the log's lines as they are to stand, one line an entry:

  by: claude-code/claude-fable-5-2
  at: 2026-11-12T09:30:00Z
  * **Update**: project/merging.md (what changed), code/tools.md (its row).

dev's workflow (tools/generated.py write) folds every such file into
docs/log.md, under its day's heading, newest day first, and removes it:
the log itself is changed on dev alone.

The same for a concept's frontmatter line `generated: { by, at }`, which
every branch that changed a concept rewrote, so that two branches that
changed one page conflicted in that line whatever else they changed (114
of 235 conflicts in pages of docs/ were that line alone). A branch leaves
the line of a concept that exists as it is; the fold writes it, from the
entry's `by` and `at`, on each concept the entry's lines name: a path
ending .md outside parentheses and code, as the lines open
(`project/merging.md (what changed)`), and drops a `verified` line there,
which a change ends. A new concept is written with its line, as before.

  python3 tools/log_fold.py           fold docs/log.d/ into docs/log.md
  python3 tools/log_fold.py --lint    the entries' files well formed, and the concepts
                                      each will stamp; write nothing
  python3 tools/log_fold.py --check   as --lint, and none left to fold
  python3 tools/log_fold.py carry REF   the lines docs/log.md holds here and REF's does
                                        not, written as an entry's file (a branch that
                                        still logged the old way; tools/generated.py
                                        restore runs it)
Standard library only.
"""
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LOG = 'docs/log.md'
ENTRIES = 'docs/log.d'
NAME = re.compile(r'(\d{4}-\d{2}-\d{2})-[a-z0-9][a-z0-9-]*\.txt$')
LINE = re.compile(r'\* \*\*[A-Z][A-Za-z]+\*\*')
BY = re.compile(r'by:\s*((?:[\w.-]+/[\w.-]+)|(?:human|process):[\w.-]+)\s*$')
AT = re.compile(r'at:\s*(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z)\s*$')
STAMP = re.compile(r'^generated:.*$', re.M)
STAMP_PARTS = re.compile(r'^generated:\s*\{\s*by:\s*([^,}]+?)\s*,\s*at:\s*([^,}\s]+)\s*\}', re.M)
PATH = re.compile(r'(?<![\w./-])([A-Za-z0-9][\w./-]*\.md)\b')
HEADING = re.compile(r'^## (\d{4}-\d{2}-\d{2})\s*$')
KEEP = 'README.txt'   # (the folder's own note, which also keeps the folder when nothing waits in it)


def concepts(lines):
    """The concepts an entry's lines name as changed: each path ending .md
    outside parentheses and code that is a concept of docs/ (not an index or
    the log), in order."""
    out = []
    for line in lines:
        bare, depth = [], 0
        for ch in re.sub(r'`[^`]*`', ' ', line):
            depth += ch == '('
            if depth == 0:
                bare.append(ch)
            depth -= ch == ')' and depth > 0
        for m in PATH.finditer(''.join(bare)):
            path = 'docs/' + m.group(1)
            if (os.path.basename(path) not in ('index.md', 'log.md') and path not in out
                    and os.path.isfile(os.path.join(ROOT, path))):
                out.append(path)
    return out


def stamped(text, by, at):
    """A concept's text with its `generated` line the entry's (its writer;
    its time, or the later one the line holds: a concept's time never goes
    back) and no `verified` line; None where it has no such line to write."""
    m = STAMP_PARTS.search(text)
    if not STAMP.search(text):
        return None
    if m and m.group(2) > at:
        at = m.group(2)
    text = STAMP.sub(f'generated: {{ by: {by}, at: {at} }}', text, count=1)
    return re.sub(r'^verified:.*\n', '', text, count=1, flags=re.M)


def entries():
    """Each entry's file: (name, day, lines, by, at), in the names' order; and what is wrong with any."""
    out, bad = [], []
    folder = os.path.join(ROOT, ENTRIES)
    for name in sorted(os.listdir(folder)) if os.path.isdir(folder) else []:
        if name == KEEP:
            continue
        m = NAME.match(name)
        if not m:
            bad.append(f'{ENTRIES}/{name}: not named DATE-WORDS.txt (2026-11-12-merging-in-parallel.txt)')
            continue
        with open(os.path.join(folder, name), encoding='utf-8') as f:
            lines = [line.rstrip() for line in f.read().splitlines() if line.strip()]
        by = next((BY.match(line).group(1) for line in lines if BY.match(line)), None)
        at = next((AT.match(line).group(1) for line in lines if AT.match(line)), None)
        lines = [line for line in lines if not BY.match(line) and not AT.match(line)]
        if by is None:
            bad.append(f'{ENTRIES}/{name}: no "by: WRITER" line (by: claude-code/MODEL, human:ID, process:ID)')
        if at is None:
            bad.append(f'{ENTRIES}/{name}: no "at: YYYY-MM-DDTHH:MM:SSZ" line')
        if not lines:
            bad.append(f'{ENTRIES}/{name}: no log line')
        for line in lines:
            if not LINE.match(line):
                bad.append(f'{ENTRIES}/{name}: a line that is neither by:, at: nor a log entry (* **Update**: ...): {line[:60]}')
        out.append((name, m.group(1), lines, by, at))
    return out, bad


def folded(text, found):
    """The log with the entries' lines under their days' headings: a day's
    new lines above the lines it has, the days newest first, each under one
    heading (two headings of a day, or days out of order, are what a union
    merge left: their lines are moved together, none reworded or dropped)."""
    lines = text.rstrip('\n').split('\n')
    first = next((i for i, line in enumerate(lines) if HEADING.match(line)), len(lines))
    head, days = lines[:first], {}
    while head and head[-1] == '':
        head.pop()
    day = None
    for line in lines[first:]:
        m = HEADING.match(line)
        if m:
            day = m.group(1)
            days.setdefault(day, [])
        elif line.strip():
            days[day].append(line)
    have = {line for body in days.values() for line in body}
    new = {}
    for _, day, entry, _, _ in found:
        new.setdefault(day, []).extend(line for line in entry if line not in have)   # (folded twice, a line stands once)
    for day, entry in new.items():
        days[day] = entry + days.get(day, [])
    out = head
    for day in sorted(days, reverse=True):
        out += ['', f'## {day}'] + days[day]
    return '\n'.join(out) + '\n'


def git_show(ref, path):
    r = subprocess.run(['git', 'show', f'{ref}:{path}'], cwd=ROOT, capture_output=True, text=True, encoding='utf-8')
    return r.stdout if r.returncode == 0 else None


def here(path):
    try:
        with open(os.path.join(ROOT, path), encoding='utf-8') as f:
            return f.read()
    except OSError:
        return None


def stamps(ref, ours=None):
    """The concepts whose `generated` line differs here (or in the commit
    `ours`) from the commit's, both having one: {path: (the commit's line,
    by, at)}. A concept the commit lacks is new, and its line its writer's."""
    r = subprocess.run(['git', 'diff', '--name-only', ref, *([ours] if ours else []), '--', 'docs'],
                       cwd=ROOT, capture_output=True, text=True)
    out = {}
    for path in r.stdout.split():
        if not path.endswith('.md'):
            continue
        theirs, mine = git_show(ref, path), git_show(ours, path) if ours else here(path)
        a, b = (STAMP.search(t) if t else None for t in (theirs, mine))
        if a and b and a.group(0) != b.group(0):
            m = STAMP_PARTS.search(mine)
            out[path] = (a.group(0), m.group(1) if m else None, m.group(2) if m else None)
    return out


def carry(ref, mine=None, ours=None):
    """The log's lines here that the commit's log lacks, as an entry's file
    a day, written by whoever stamped the concepts this branch changed; the
    files written. `mine`: the log's text to read instead of the file's, and
    `ours` the commit to read the branch's stamps from (a merge's own side)."""
    theirs = git_show(ref, LOG)
    if mine is None:
        with open(os.path.join(ROOT, LOG), encoding='utf-8') as f:
            mine = f.read()
    if theirs is None:
        sys.exit(f'log: {ref} has no {LOG}')
    had = set(theirs.split('\n'))
    day, new = None, {}
    for line in mine.split('\n'):
        m = HEADING.match(line)
        if m:
            day = m.group(1)
        elif day and LINE.match(line) and line not in had:
            new.setdefault(day, []).append(line)
    branch = subprocess.run(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], cwd=ROOT, capture_output=True, text=True).stdout.strip()
    words = re.sub(r'[^a-z0-9]+', '-', branch.lower()).strip('-') or 'branch'
    written = []
    os.makedirs(os.path.join(ROOT, ENTRIES), exist_ok=True)
    mine_stamps = [(by, at) for _, by, at in stamps(ref, ours).values() if by and at]
    writers = [by for by, _ in mine_stamps]
    by = max(set(writers), key=writers.count) if writers else 'claude-code/unrecorded'
    for day, lines in new.items():
        times = [at for _, at in mine_stamps if AT.match('at: ' + at)]
        at = max(times) if times else f'{day}T00:00:00Z'
        lines = [f'by: {by}', f'at: {at}'] + lines
        name, k = f'{day}-{words}.txt', 1
        while os.path.exists(os.path.join(ROOT, ENTRIES, name)):
            k += 1
            name = f'{day}-{words}-{k}.txt'
        with open(os.path.join(ROOT, ENTRIES, name), 'w', encoding='utf-8', newline='\n') as f:
            f.write('\n'.join(lines) + '\n')
        written.append(f'{ENTRIES}/{name}')
    return written


def main():
    args = sys.argv[1:]
    if args[:1] == ['carry'] and len(args) == 2:
        for path in carry(args[1]):
            print(f'log: {path}')
        return 0
    if args not in ([], ['--lint'], ['--check']):
        sys.exit(__doc__)
    found, bad = entries()
    for b in bad:
        print(f'log: {b}')
    if args == ['--lint']:
        for name, _, lines, by, at in found:
            print(f'log: {ENTRIES}/{name} ({by}, {at}) will stamp: {", ".join(p[5:] for p in concepts(lines)) or "no concept"}')
        print(f'log: {len(found)} entries to fold, {len(bad)} problems')
        return 1 if bad else 0
    path = os.path.join(ROOT, LOG)
    with open(path, encoding='utf-8') as f:
        text = f.read()
    new = folded(text, found)
    if args == ['--check']:
        if new != text:
            print(f'log: {len(found)} entries of {ENTRIES}/ not folded into {LOG}, or its days not in order: python3 tools/log_fold.py')
        else:
            print('log: folded')
        return 1 if bad or new != text else 0
    if bad:
        return 1
    if new != text:
        with open(path, 'w', encoding='utf-8', newline='\n') as f:
            f.write(new)
    marked = 0
    for name, _, lines, by, at in sorted(found, key=lambda e: e[4]):   # (two entries on one concept: the later's stands)
        for concept in concepts(lines):
            with open(os.path.join(ROOT, concept), encoding='utf-8') as f:
                was = f.read()
            now = stamped(was, by, at)
            if now is not None and now != was:
                with open(os.path.join(ROOT, concept), 'w', encoding='utf-8', newline='\n') as f:
                    f.write(now)
                marked += 1
    for name, *_ in found:
        os.remove(os.path.join(ROOT, ENTRIES, name))
    print(f'log: {len(found)} entries folded into {LOG}, {marked} concepts stamped')
    return 0


if __name__ == '__main__':
    sys.exit(main())
