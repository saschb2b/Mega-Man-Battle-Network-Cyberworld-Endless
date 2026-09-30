#!/usr/bin/env python3
"""watch_session.py NAME [--budget N] [--alert]: how a playtest session is
going, from .build/play/NAME: calls and frames so far, what the game shows
now, how long the current battle has run (in calls, frames and minutes), the
pace (frames per call), and how long since the persona's last note. Each run
adds a snapshot to NAME/watch.log, from which the battle's length and the pace
are measured.

With --alert it exits 1 when something wants a look (a battle past 15 minutes
or 40 calls, the call budget spent, 20 minutes without a note while playing,
no call for 10 minutes while the game runs, under 50 frames a call over the
last ten, or past 80 calls more than 45% of them on the map), and 0
otherwise; the loop to run while a persona plays:

  until python3 .claude/skills/playtest-loop/scripts/watch_session.py kai --alert; do sleep 300; done

(and restart it after looking)."""
import glob
import os
import re
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))))


def read(path):
    try:
        with open(path, encoding='utf-8', errors='replace') as f:
            return f.read()
    except OSError:
        return ''


def main():
    args = sys.argv[1:]
    if not args or args[0].startswith('-'):
        sys.exit(__doc__)
    name, budget, alert = args[0], 260, '--alert' in args
    if '--budget' in args:
        budget = int(args[args.index('--budget') + 1])
    h = os.path.join(ROOT, '.build', 'play', name)
    now = time.time()
    lines = [l for l in read(os.path.join(h, 'history.txt')).splitlines() if '# ok ' in l]
    frames = [int(re.search(r'# ok (\d+)', l).group(1)) for l in lines]
    calls, frame = len(lines), frames[-1] if frames else 0
    state = dict(l.split(' ', 1) for l in read(os.path.join(h, 'state.txt')).splitlines() if ' ' in l)
    doing, hp = state.get('doing', '?'), state.get('hp', '?')
    alive = os.path.exists(os.path.join(h, 'pid'))
    hist_age = (now - os.path.getmtime(os.path.join(h, 'history.txt'))) / 60 if os.path.exists(os.path.join(h, 'history.txt')) else 0
    notes = sorted(glob.glob(os.path.join(h, 'notes-s*.md')), key=os.path.getmtime)
    note_age = (now - os.path.getmtime(notes[-1])) / 60 if notes else 0

    # the snapshot, and the current battle's start among the earlier ones:
    # the battles finished so far (the run log's lines) tell one battle
    # from the next, where two snapshots five minutes apart both in a
    # battle had counted a layer's four fights as one long one
    done = sum(' battle ' in l for l in read(os.path.join(h, 'data', 'runlog.txt')).splitlines())
    log = os.path.join(h, 'watch.log')
    snaps = []
    for l in read(log).splitlines():
        p = l.split()
        if len(p) >= 4:
            snaps.append((float(p[0]), int(p[1]), int(p[2]), p[3], int(p[5]) if len(p) >= 6 else -1))
    snaps.append((now, calls, frame, doing, done))
    with open(log, 'a') as f:
        f.write(f'{now:.0f} {calls} {frame} {doing} {hp} {done}\n')
    start = None
    for s in reversed(snaps):
        if s[3] != 'battle' or s[4] != done:
            break
        start = s
    # the calls spent on the map (walking, reading it, talking), as the
    # snapshots saw them: each snapshot's calls go to what it showed
    # (this session's: the log goes on across sessions, whose calls count
    # from 0 again)
    first = max([i for i in range(1, len(snaps)) if snaps[i][1] < snaps[i - 1][1]] or [0])
    walk = sum(snaps[i][1] - snaps[i - 1][1] for i in range(first + 1, len(snaps)) if snaps[i][3] == 'map')
    pace = ''
    if len(frames) > 10:
        per = (frames[-1] - frames[-11]) / 10
        pace = f', {per:.0f} frames a call over the last ten'
    print(f'{name}: {calls} calls (budget {budget}), frame {frame}, doing {doing}, hp {hp}{pace}; '
          f'last call {hist_age:.0f} min ago, last note {note_age:.0f} min ago' + ('' if alive else '; game stopped'))
    if calls:
        print(f'  on the map: about {100 * walk // calls}% of the calls')
    want = []
    if start and doing == 'battle':
        mins, n = (now - start[0]) / 60, calls - start[1]
        print(f'  this battle: {n} calls, {frame - start[2]} frames, {mins:.0f} min so far')
        if mins > 15 or n > 40:
            want.append('a long battle')
    if calls > budget:
        want.append('the call budget is spent')
    if alive and note_age > 20 and hist_age < 5:
        want.append('no note for 20 minutes')
    if alive and hist_age > 10:
        want.append('no call for 10 minutes')
    if len(frames) > 10 and (frames[-1] - frames[-11]) / 10 < 50:
        want.append('a slow pace')
    # (a session spent 110 of 249 calls walking and reading the map in the
    # Aquarium Comp's mazes, and it showed only in the report)
    # (past sessions: 9 to 60%, most 20 to 40)
    if calls >= 80 and walk * 100 > calls * 45:
        want.append('nearly half the calls on the map (lost, or a long walk?)')
    shots = sorted(glob.glob(os.path.join(h, 'shots', '*.png')))
    if shots:
        print(f'  latest picture: {shots[-1]}')
    if want:
        print('  look: ' + ', '.join(want))
    sys.exit(1 if alert and want else 0)


if __name__ == '__main__':
    main()
