#!/usr/bin/env python3
"""persona_prompt.py N LAST NOTES GOALS: the persona's prompt for session N.

persona.md below its line, with {{ORDINAL}}, {{N}} and {{N-1}} filled in and
the three {{...}} blocks (the last session, the patch notes, the goals) taken
from the files LAST, NOTES and GOALS. Prints the prompt; fails where a
placeholder is left.
"""
import os
import re
import sys


def ordinal(n):
    suffix = 'th' if 10 <= n % 100 <= 20 else {1: 'st', 2: 'nd', 3: 'rd'}.get(n % 10, 'th')
    return f'{n}{suffix}'


def main():
    if len(sys.argv) != 5:
        sys.exit(__doc__.strip().splitlines()[0])
    n = int(sys.argv[1])
    last, notes, goals = (open(p).read().strip() for p in sys.argv[2:5])
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
    print(body.strip())


if __name__ == '__main__':
    main()
