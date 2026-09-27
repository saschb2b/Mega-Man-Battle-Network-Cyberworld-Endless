#!/usr/bin/env python3
"""follow_arrow.py NAME [STEPS]: walk a play.py session's MegaMan the way
the arrow shows, a step at a time, and print the panel and the way after
each: does following the arrow get there? The session must have been
started with CYBERWORLD_STATE_POS=1 (the state then names the arrow's way);
random battles are held off (event flag 0x1700) while it walks.

  CYBERWORLD_STATE_POS=1 python3 tools/play.py start fa --seed S --fresh -- --scene emu --run-depth 3
  python3 .claude/skills/playtest-loop/scripts/follow_arrow.py fa 150

It stops when the state names no way (a chat, a battle, a title card)."""
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))))
PAD = {0: 'RIGHT', 1: 'DOWN+RIGHT', 2: 'DOWN', 3: 'DOWN+LEFT', 4: 'LEFT', 5: 'UP+LEFT', 6: 'UP', 7: 'UP+RIGHT'}


def do(name, cmd):
    env = dict(os.environ, CYBERWORLD_STATE_POS='1')
    return subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'play.py'), 'do', name, cmd],
                          capture_output=True, text=True, env=env).stdout


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    name, steps = sys.argv[1], int(sys.argv[2]) if len(sys.argv) > 2 else 150
    out = do(name, 'flags 0x1700 0x1700 1; wait 1')
    for i in range(steps):
        way = re.search(r'^way (\d) (.*)$', out, re.M)
        panel = re.search(r'^panel (\d+) (\d+)', out, re.M)
        doing = re.search(r'^doing (.*)$', out, re.M)
        if not way or not panel:
            print(f'{i}: no way ({doing.group(1) if doing else "?"}); picture: {re.search(r"^picture (.*)$", out, re.M).group(1)}')
            return
        print(f'{i}: panel {panel.group(1)} {panel.group(2)}, the arrow {way.group(2)}')
        out = do(name, f'flags 0x1700 0x1700 1; hold {PAD[int(way.group(1))]} 10')


if __name__ == '__main__':
    main()
