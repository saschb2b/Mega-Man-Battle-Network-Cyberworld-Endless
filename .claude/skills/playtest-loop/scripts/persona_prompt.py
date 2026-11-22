#!/usr/bin/env python3
"""persona_prompt.py N LAST NOTES GOALS [NAME] [--running]: the persona's prompt for session N.

persona.md below its line, with {{ORDINAL}}, {{N}} and {{N-1}} filled in and
the three {{...}} blocks (the last session, the patch notes, the goals) taken
from the files LAST, NOTES and GOALS. Prints the prompt; fails where a
placeholder is left. NAME (kai2, kai3, ...) runs another instance beside
kai: its play.py session and its folder in .build/play/ are NAME's.
--running: the developers started NAME's game already (a fresh profile,
or a test start with `-- --scene emu --run-depth D`): the prompt says
never to pass `start` or `fresh`, but to stop and start the game itself
for a quit-and-CONTINUE test (session 76's kai7 skipped those, told
never to start it).
"""
import os
import re
import sys


def ordinal(n):
    suffix = 'th' if 10 <= n % 100 <= 20 else {1: 'st', 2: 'nd', 3: 'rd'}.get(n % 10, 'th')
    return f'{n}{suffix}'


def running_prompt(body, name):
    """The prompt for a game the developers started: no start or fresh on
    the first call, no memory to read on a fresh profile, and how to quit
    and CONTINUE all the same."""
    body = re.sub(r'First read your memory:.*?do NOT use --fresh\. ', '', body, flags=re.S)
    body = body.replace(
        'Your first call starts the game at the title with your persistent profile (no `fresh`); then `do` takes',
        'Your game is already running (the developers started it): never pass `start` or `fresh` while it runs; every call is a `do`, which takes')
    body = re.sub(r'- Without it: `python3 tools/play\.py start ' + name + r'`:[^\n]*',
                  f'- Without it: your game is already running; `python3 tools/play.py do {name} ...` plays it.', body)
    return body + (f'\n\nTo test quitting and CONTINUE: `do` "stop" (or `python3 tools/play.py stop {name}`), then '
                   f'`python3 tools/play.py start {name}` (never `--fresh`) starts it again at the title with your profile.')


def main():
    args = [a for a in sys.argv[1:] if a != '--running']
    running = len(args) != len(sys.argv) - 1
    if len(args) not in (4, 5):
        sys.exit(__doc__.strip().splitlines()[0])
    n = int(args[0])
    last, notes, goals = (open(p).read().strip() for p in args[1:4])
    name = args[4] if len(args) == 5 else 'kai'
    if not re.fullmatch(r'kai\d*', name):
        sys.exit('NAME is kai, kai2, kai3, ...')
    here = os.path.dirname(os.path.abspath(__file__))
    persona = open(os.path.join(here, '..', 'persona.md')).read()
    body = persona.split('\n---\n', 1)[1]
    body = body.replace('{{ORDINAL}}', ordinal(n)).replace('{{N-1}}', str(n - 1)).replace('{{N}}', str(n))
    for key, text in (('LAST SESSION', last), ('PATCH NOTES', notes), ('GOALS', goals)):
        body, k = re.subn(r'\{\{' + key + r':[^}]*\}\}', lambda _m, t=text: t, body)
        if k != 1:
            sys.exit(f'persona.md has {k} {{{{{key}: ...}}}} blocks, not one')
    left = re.findall(r'\{\{[^}]*\}\}', body)
    if left:
        sys.exit(f'left unfilled: {left}')
    if name != 'kai':
        body = body.replace('.build/play/kai/', f'.build/play/{name}/')
        body = re.sub(r'(play\.py (?:start|do|stop|replay)) kai\b', lambda m: f'{m.group(1)} {name}', body)
        body = body.replace('`name` "kai"', f'`name` "{name}"')
    if running:
        body = running_prompt(body, name)
    print(body.strip())


if __name__ == '__main__':
    main()
