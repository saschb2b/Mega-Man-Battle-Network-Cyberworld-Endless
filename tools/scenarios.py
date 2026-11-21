#!/usr/bin/env python3
"""The scenarios (tests/scenarios/, docs/devtools/scenarios.md): the game
played headless through a scripted course, its state in words checked on
the way. A bug that only a sequence shows (a run's end, then the next run;
a quit, then CONTINUE) is kept here as the play that showed it, so it stays
fixed.

  tools/scenarios.py [NAME ...] [--jobs N] [--list] [--check]
      every scenario, or those named (a name, or the start of one:
      "route" runs every route/...), --jobs at a time (4); one line each,
      and for a failure the step that failed, the state it found, and
      where its picture and log are. Exit 1 where one failed.
      --list names them with what each guards; --check reads them all
      without the game (build.py test does); --stop stops the games a run
      that was cut off left playing (a run ended or signalled stops its own).

A scenario is a file tests/scenarios/NAME.py: its docstring says what it
guards (the issue first), START is the game's start (tools/cw.py's: a
route, the game's options, VAR=VALUE first for its environment), NEEDS the
other games' ROMs it plays with beside BN6's ('bn5', 'bn4'; skipped
without them), KNOWN a bug it shows that is not fixed yet (what is wrong
and its issue: it is played and reported KNOWN, no failure of the run,
and fails once it passes, so the line goes with the fix) and STEPS
its course, a tuple each:

  ('do', COMMANDS)                tools/play.py's commands
  ('until', COND[, FRAMES])       the game runs on until the state says so
                                  (tools/cw.py's until; 6000 frames at most)
  ('repeat', COMMANDS, COND[, TIMES])   COMMANDS again and again until COND
                                  (12 times at most): a button pressed
                                  through screens whose timing is theirs
  ('expect', COND)                the state says so now
  ('keep', WORDS)                 the state's lines of those first words kept
                                  (a do step's `shot`: kept at 2x in the folder
                                  CW_SCENARIO_SHOTS names, for a PR's pictures)
  ('same', WORDS)                 ... and the same now as when kept
  ('moved', WORD, BY)             ... or line WORD's number moved by BY since:
                                  a number (1, -1), or a kept word with a
                                  sign ('-price': down by the price kept)
  ('restart', START)              the game quit as a player quits it
                                  (SIGTERM: the port's close), then started
                                  again on the same saves
  ('server', 'stop')              the scenario's server gone at once
                                  (('server', 'start'): back, as it was)

An online course asks for a server of this machine and other games beside
its first (tools/scenarios_online.py): SERVER = True, GAMES = {'beta':
START}, FILES = {name: text} into each game's data folder, and a step in
another game names it first, ('@beta', 'do', 'press A'). {server} in a
START or a file is the server's address, {nowhere} one nothing answers on;
no scenario names an address of its own, and no game reaches a host but
this machine.

A file may hold many instead: MANY = {name: dict(START=..., STEPS=...,
NEEDS=..., GUARDS=..., KNOWN=..., ROMS=...)} (tests/scenarios/routes.py: a scenario a route).

In a START and in COMMANDS, {dir} is the game's own folder (a second data
folder, a HOME for its Downloads), {roms} the folder of BN6's ROM and its
NEEDS, and {bn6}, {bn5}, {bn4} a ROM's file alone in a folder, {wrong} a
.gba that is none of the games' and {zip} an archive, to drop on the ROMs
screen, {damaged} a .cwsave cut short, to drop on the title, and {update}
the address of a made-up newer version's update.json, served here while
the scenarios play (CYBERWORLD_UPDATE_URL={update}). ROMS names the files
in the game's ROM folder where they are not BN6's and its NEEDS: () none,
('wrong',) the wrong file alone; that folder is then the game's own (the ROMs screen keeps a copy
of what it is given there).

The games run from build/host/cyberworld (built first where a source is
newer; CYBERWORLD_CW_BIN another), each in .build/scenarios/NAME, with
BN6's ROM of ~/.cache/mmbn-ref/roms (CYBERWORLD_ROM_DIR) and those it
NEEDS, no other: the same game whatever else that folder holds. Local,
never CI's.
"""
import atexit
import concurrent.futures
import glob
import hashlib
import http.server
import importlib.util
import json
import os
import shutil
import signal
import sys
import threading
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import cw  # noqa: E402
import cw_pictures as pics  # noqa: E402
import play  # noqa: E402
import scenarios_online as online  # noqa: E402

DIR = os.path.join(ROOT, 'tests', 'scenarios')
WORK = os.path.join(ROOT, '.build', 'scenarios')
UNTIL = 6000     # an until's most frames, where the step names none
REPEAT = 12      # a repeat's most times
KINDS = {'do': (1, 1), 'until': (1, 2), 'repeat': (2, 3), 'expect': (1, 1), 'keep': (1, 1), 'same': (1, 1), 'moved': (2, 2),
         'restart': (1, 1), 'server': (1, 1), 'sleep': (1, 1)}
# (the other games' ROMs a scenario may need, told by their SHA-1: build.py's)
ROMS = {'bn5': '5f472f78d8de2df01d5039e045c043cb40969a39', 'bn4': 'a97e96a7da03abd70f7953328e511b9fa29179f1'}
BN6 = '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6'
# (files that are no ROM of the games', made here: a .gba whose header names another game, an archive)
MADE = {'wrong': ('wrong.gba', bytes(0xAC) + b'ZZZZ' + bytes(0x1000)), 'zip': ('rom.zip', b'PK\x05\x06' + bytes(18)),
        'damaged': ('damaged.cwsave', b'CWSAVE1\n' + bytes(64))}   # (and saves cut short, for SAVES to refuse)
EACH = os.path.join(WORK, '.roms', 'each')   # every file a scenario may name: {bn6}, {wrong}


class Failed(Exception):
    """A step that did not hold: what it found."""


# ---- reading them ----

def load():
    """{name: dict(guards, start, needs, steps)} of every scenario, by name."""
    out = {}
    for path in sorted(glob.glob(os.path.join(DIR, '*.py'))):
        base = os.path.basename(path)[:-3].replace('_', '-')
        spec = importlib.util.spec_from_file_location('scenario_' + base.replace('-', '_'), path)
        m = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(m)
        doc = ' '.join((m.__doc__ or '').split())
        if hasattr(m, 'MANY'):
            for name, s in m.MANY.items():
                out[f'{base}/{name}'] = dict(guards=s.get('GUARDS', doc), start=s['START'], needs=tuple(s.get('NEEDS', ())),
                                             steps=list(s['STEPS']), known=s.get('KNOWN', ''), roms=s.get('ROMS'), **online.read(s))
        else:
            out[base] = dict(guards=doc, start=m.START, needs=tuple(getattr(m, 'NEEDS', ())), steps=list(m.STEPS),
                             known=getattr(m, 'KNOWN', ''), roms=getattr(m, 'ROMS', None), **online.read(m))
    return out


def problems(name, s):
    """What is wrong with a scenario as written, a line each (none: it reads)."""
    out = []
    if not s['guards']:
        out.append(f'{name}: no docstring saying what it guards')
    if not isinstance(s.get('known', ''), str):
        out.append(f'{name}: KNOWN is a line of words: what is wrong, and its issue')
    if any(n not in ROMS for n in s['needs']):
        out.append(f'{name}: NEEDS {s["needs"]}: {", ".join(ROMS)}')
    if s.get('roms') is not None and any(n not in ('bn6', *ROMS, *MADE) for n in s['roms']):
        out.append(f'{name}: ROMS {s["roms"]}: {", ".join(("bn6", *ROMS, *MADE))}')
    if s.get('roms') is not None and set(s['roms']) & set(ROMS) - set(s['needs']):
        out.append(f'{name}: ROMS names a game its NEEDS does not')
    out += online.written(name, s)
    # (a step in another game names it first: the step itself, here)
    steps = [online.split(step)[1] for step in s['steps']]
    starts = [s['start'], *(s.get('games') or {}).values()] + [step[1] for step in steps if step and step[0] == 'restart' and len(step) > 1]
    for start in starts:
        try:
            cw.parse_start(start)
        except SystemExit as e:
            out.append(f'{name}: start {start!r}: {e.code}')
    if not s['steps']:
        out.append(f'{name}: no steps')
    for i, step in enumerate(steps, 1):
        kind = step[0] if step else None
        if kind not in KINDS or not KINDS[kind][0] <= len(step) - 1 <= KINDS[kind][1]:
            out.append(f'{name}: step {i} {step!r}: not a step ({", ".join(KINDS)})')
            continue
        try:
            if kind in ('do', 'repeat'):
                play.steps(step[1])
            if kind in ('until', 'expect'):
                cw.condition(step[1])
            if kind == 'repeat':
                cw.condition(step[2])
        except SystemExit as e:
            out.append(f'{name}: step {i} {step!r}: {e.code}')
        if kind in ('until', 'repeat') and len(step) - 1 == KINDS[kind][1] and not isinstance(step[-1], int):
            out.append(f'{name}: step {i} {step!r}: its last is a number')
    kept = {}
    for i, raw in enumerate(s['steps'], 1):
        game, step = online.split(raw)
        mine = kept.setdefault(game, set())   # (each game keeps its own)
        if step and step[0] == 'keep':
            mine |= set(step[1].split())
        if step and step[0] == 'same' and set(step[1].split()) - mine:
            out.append(f'{name}: step {i}: same {" ".join(sorted(set(step[1].split()) - mine))} never kept')
        if step and step[0] == 'moved' and len(step) == 3:
            by = str(step[2]).lstrip('+-')
            if ({step[1]} | ({by} if not by.isdigit() else set())) - mine:
                out.append(f'{name}: step {i}: moved {step[1]} by {step[2]}: a word never kept')
    return out


def have_roms():
    """The ROMs in the ROM folder, {name: path} ('bn6' and the other games'), told by their SHA-1."""
    have = {}
    for path in sorted(glob.glob(os.path.join(play.ROM_DIR, '*.gba'))):
        with open(path, 'rb') as f:
            sha = hashlib.sha1(f.read()).hexdigest()
        for name, want in {'bn6': BN6, **ROMS}.items():
            if want == sha:
                have.setdefault(name, path)
    return have


def rom_folder(roms, needs):
    """A folder of BN6's ROM and those a scenario needs, no other (links, in
    .build): a scenario plays the same game whatever else the ROM folder
    holds (a run's areas, a dark layer's board and its team change with
    BN5 beside BN6)."""
    names = ['bn6', *sorted(needs)]
    d = os.path.join(WORK, '.roms', '+'.join(names))
    os.makedirs(d, exist_ok=True)
    for name in names:
        link = os.path.join(d, name + '.gba')
        if os.path.realpath(link) != os.path.realpath(roms[name]):
            if os.path.lexists(link):
                os.remove(link)
            os.symlink(os.path.abspath(roms[name]), link)
    return d


def each_file(name):
    """Where a file a scenario may name stands ({bn6}, {wrong}): a folder of
    its own, so that a ROM dropped on the ROMs screen comes alone (one with
    another game's beside it brings that along)."""
    return os.path.join(EACH, name, MADE[name][0] if name in MADE else name + '.gba')


def each_rom(roms):
    """The files a scenario may name: the ROMs there are, linked, and the
    made ones. Made once, before the games."""
    for name, path in roms.items():
        os.makedirs(os.path.dirname(each_file(name)), exist_ok=True)
        link = each_file(name)
        if os.path.realpath(link) != os.path.realpath(path):
            if os.path.lexists(link):
                os.remove(link)
            os.symlink(os.path.abspath(path), link)
    for name, (_, data) in MADE.items():
        os.makedirs(os.path.dirname(each_file(name)), exist_ok=True)
        with open(each_file(name), 'wb') as f:
            f.write(data)


def own_roms(h, names):
    """A game's own ROM folder, h/roms, holding the files `names` name and
    no other: what the ROMs screen is given, it keeps a copy of there."""
    d = os.path.join(h, 'roms')
    os.makedirs(d)
    for name in names:
        os.symlink(each_file(name), os.path.join(d, os.path.basename(each_file(name))))
    return d


SHARED = {}   # (a game's folder: the folder of BN6's ROM and its NEEDS, {roms})
# (a newer version's update.json, made up: {update} is its address on this machine)
UPDATE = json.dumps({'version': '99.0.0', 'files': {},
                     'changes': 'A version made up for a test\n\nIts summary.\n\n## New\n\n### A thing of the test\n\nIts words.\n'})
SERVED = []   # (the address, once a scenario asked for it)


def serve_update():
    """A stand-in for a release's update.json, served on this machine's own
    address while the scenarios play: the address a game is given as
    CYBERWORLD_UPDATE_URL={update} (docs/engine/updates.md), so its look
    finds version 99.0.0 and never asks GitHub."""
    class Handler(http.server.BaseHTTPRequestHandler):
        def do_GET(self):
            body = UPDATE.encode()
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Content-Length', str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def log_message(self, *args):
            pass
    server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), Handler)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    SERVED.append(f'http://127.0.0.1:{server.server_address[1]}/update.json')


def placed(h, text):
    """A START's or a step's words with the game's own places in them:
    {dir} its folder, {roms} the folder of BN6's ROM and its NEEDS, {bn6}
    ... {damaged} a file alone in a folder of its own."""
    out = text.replace('{dir}', h).replace('{roms}', SHARED.get(h, '')).replace('{update}', SERVED[0] if SERVED else '')
    for name in ('bn6', *ROMS, *MADE):
        out = out.replace('{' + name + '}', each_file(name))
    return out


# ---- playing one ----

def words(state, names):
    """The state's lines whose first word is one of `names`, by that word."""
    out = {}
    for line in cw.state_lines(state):
        if cw.key(line) in names:
            out.setdefault(cw.key(line), line)
    return out


def number(line):
    """A state line's number, its first: 'hp 60/100' is 60."""
    for word in line.split()[1:]:
        digits = word.split('/')[0]
        if digits.lstrip('-').isdigit():
            return int(digits)
    raise Failed(f'the line {line!r} has no number')


def moved(step, state, kept):
    """A moved step: line WORD's number now is its kept number plus BY, BY a
    number or a kept word with a sign ('-price': down by the price kept)."""
    word, by = step[1], str(step[2])
    now = words(state, [word])
    if word not in now:
        raise Failed(f'the state has no line {word}', state)
    sign, name = (-1 if by.startswith('-') else 1), by.lstrip('+-')
    amount = sign * (int(name) if name.isdigit() else number(kept[name]))
    was, have = number(kept[word]), number(now[word])
    if have != was + amount:
        raise Failed(f'{word} was {was} and is {have}: not moved by {amount:+d}', state)


LIVE = set()   # the folders of the games this run started
STOPPING = []  # (set once the run is ending: no game starts after it)


def stop_all(*signalled):
    """Every game and server this run started, stopped: a run cut off (a
    terminal closed, a tool's time up) left its games playing on for good,
    a hundred of them after a day of sessions. Called as the run ends and
    at the signals that end it; `--stop` does the same for the games an
    earlier run left (their pid files in .build/scenarios)."""
    STOPPING.append(True)
    # (twice: a scenario's thread may have been starting its game as the first pass ran)
    for _ in range(2):
        for h in list(LIVE):
            play.stop(h)
        online.stop_all()
    if signalled:
        os._exit(128 + signalled[0])


def guard():
    atexit.register(stop_all)
    for sig in (signal.SIGTERM, signal.SIGHUP, signal.SIGINT):
        signal.signal(sig, stop_all)


def stop_left():
    """The games earlier runs left under .build/scenarios, stopped: how many."""
    left = [os.path.dirname(p) for p in glob.glob(os.path.join(WORK, '*', 'pid')) + glob.glob(os.path.join(WORK, '*', '*', 'pid'))]
    for h in left:
        play.stop(h)
    return len(left)


def start(h, spec, fresh, roms):
    """Game h started from START with the ROMs of folder `roms` (cw.begin
    without its print's redirect: several run at once here; the game takes
    the last --rom-dir it is given)."""
    if STOPPING:
        raise SystemExit('the run is ending')
    LIVE.add(h)
    commands, args, env, _ = cw.parse_start(placed(h, spec))
    seed, args = cw.take_seed(args, 1)
    # (every game a scenario or the fuzz starts goes online on this machine alone: tools/scenarios_online.py)
    play.launch(h, seed, fresh, args + ['--rom-dir', roms], {**online.ENV, **env}, cw.BIN, keep_bin=False)
    _, ran, shots, _, state = cw.batch(h, 'wait 1' + (f'; {commands}' if commands else ''))
    for _, p in shots:
        os.remove(p)
    return state, ran


def quit_game(h):
    """The game quit as a player's system quits it (SIGTERM: it saves and
    leaves), waited for; its log kept beside the next start's."""
    try:
        pid = int(open(os.path.join(h, 'pid')).read())
    except (OSError, ValueError):
        raise Failed('the game was not running to quit')
    for i in range(1000):
        # (this process's own child: gone once reaped. The signal again every
        # second: one heard between a batch's answer and the read of the
        # next wakes no read, and the game waited there for good, one quit
        # in some hundreds with the machine busy)
        try:
            if i % 100 == 0:
                os.killpg(pid, signal.SIGTERM)
            if os.waitpid(pid, os.WNOHANG)[0]:
                break
        except OSError:
            break
        time.sleep(0.01)
    else:
        raise Failed('the game did not quit in 10 seconds')
    os.remove(os.path.join(h, 'pid'))
    n = len(glob.glob(os.path.join(h, 'log-*.txt')))
    shutil.copy(os.path.join(h, 'log.txt'), os.path.join(h, f'log-{n + 1}.txt'))


def kept_shot(h, path):
    """A `shot` of a do step kept at 2x where CW_SCENARIO_SHOTS names a folder
    (pictures for a pull request), as GAME-N.png; else let go with the step."""
    where = os.environ.get('CW_SCENARIO_SHOTS')
    if not where:
        return
    os.makedirs(where, exist_ok=True)
    name = os.path.basename(h)
    n = len([f for f in os.listdir(where) if f.startswith(name + '-')])
    pics.scaled(pics.game(pics.load(path)), 2).save(os.path.join(where, f'{name}-{n}.png'))


def step_run(h, step, state, kept, roms=None):
    """One step in game h from `state`: (the state after, frames run)."""
    kind = step[0]
    if kind in ('do', 'repeat'):
        step = (kind, placed(h, step[1]), *step[2:])
    if kind == 'do':
        _, ran, shots, _, state = cw.batch(h, step[1])
        for _, p in shots:
            kept_shot(h, p)
            os.remove(p)
        return state, ran
    if kind == 'until':
        limit = step[2] if len(step) > 2 else UNTIL
        _, ran, _, state, passed = cw.run_until(h, cw.condition(step[1]), limit, 0)
        if not passed:
            raise Failed(f'not reached in {limit} frames', state)
        return state, ran
    if kind == 'repeat':
        test, most, ran = cw.condition(step[2]), step[3] if len(step) > 3 else REPEAT, 0
        for _ in range(most):
            if test(state):
                return state, ran
            _, n, shots, _, state = cw.batch(h, step[1])
            ran += n
            for _, p in shots:
                os.remove(p)
        if not test(state):
            raise Failed(f'not reached in {most} times', state)
        return state, ran
    if kind == 'expect':
        if not cw.condition(step[1])(state):
            raise Failed('the state does not say so', state)
    elif kind == 'keep':
        kept.update(words(state, step[1].split()))
        missing = [w for w in step[1].split() if w not in kept]
        if missing:
            raise Failed(f'the state has no line {" ".join(missing)} to keep', state)
    elif kind == 'same':
        now = words(state, step[1].split())
        differ = [f'{kept[w]!r} then, {now.get(w, "no line")!r} now' for w in step[1].split() if now.get(w) != kept[w]]
        if differ:
            raise Failed('; '.join(differ), state)
    elif kind == 'moved':
        moved(step, state, kept)
    elif kind == 'sleep':
        # (the clock's seconds, the game standing: what a server holds for a time sees it pass, where a dive's
        # layer runs its frames faster than the clock)
        time.sleep(float(step[1]))
    elif kind == 'restart':
        quit_game(h)
        return start(h, step[1], False, roms)
    return state, 0


def failure(h, state):
    """A failure's state and picture kept in the game's folder; their paths."""
    with open(os.path.join(h, 'fail.txt'), 'w') as f:
        f.write(state + '\n')
    out = [os.path.join(h, 'fail.txt')]
    try:
        _, _, shots, _, _ = cw.batch(h, 'shot', record=False)
        pics.scaled(pics.game(pics.load(shots[-1][1])), 2).save(os.path.join(h, 'fail.png'))
        os.remove(shots[-1][1])
        out.append(os.path.join(h, 'fail.png'))
    except (SystemExit, OSError, IndexError):
        pass   # (the game gone: its log says why)
    return out + [os.path.join(h, 'log.txt')]


def run(name, s, roms):
    """One scenario played with the ROMs of folder `roms`: dict(name, ok, seconds, frames, lines)."""
    h = os.path.join(WORK, name.replace('/', '--'))
    # (its server and its other games with it, where it has any: tools/scenarios_online.py; each game's ROM folder is
    # that of BN6's ROM and its NEEDS, {roms}, or its own of the files ROMS names, made once its folder is)
    shared = roms
    cast = online.Cast(h, s, (lambda folder: own_roms(folder, s['roms'])) if s.get('roms') is not None else roms, start, step_run)
    t, frames, state, at, game = time.time(), 0, '', 0, ''
    lines = []
    for folder in cast.h.values():
        SHARED[folder] = shared
    try:
        state, frames = cast.start()
        for at, step in enumerate(s['steps'], 1):
            game = online.split(step)[0]
            game, state, n = cast.step(step)
            frames += n
        ok = True
    except (Failed, SystemExit) as e:
        ok = False
        what = e.args[0] if isinstance(e, Failed) else str(e.code)
        state = e.args[1] if isinstance(e, Failed) and len(e.args) > 1 else cast.state.get(game, state)
        step = s['steps'][at - 1] if at else ('start', s['start'])
        lines.append(f'     step {at} {step!r}: {what}')
        shown = [line for line in cw.state_lines(state) if cw.key(line) not in ('frame', 'gameframe')]
        if shown:
            lines.append('     state: ' + ' · '.join(shown))
        cast.quiet()
        lines.append('     kept: ' + ', '.join(os.path.relpath(p, ROOT) for p in failure(cast.h[game], state)))
    finally:
        cast.close()
    return dict(name=name, ok=ok, seconds=time.time() - t, frames=frames, lines=lines)


def verdict(r, known):
    """A played scenario's word, whether it fails the run, and its lines: a
    scenario with KNOWN shows a bug not fixed yet, so its failure is KNOWN
    and no failure of the run, and its pass is one (the bug is gone: its
    KNOWN is to go)."""
    if not known:
        return ('PASS' if r['ok'] else 'FAIL'), not r['ok'], r['lines']
    if r['ok']:
        return 'FAIL', True, [f'     it passes now: take its KNOWN out ({known})']
    return 'KNOWN', False, [f'     {known}'] + r['lines'][:1]


# ---- all of them ----

def chosen(every, names):
    """The scenarios `names` name (a name, or the start of one); SystemExit where a name finds none."""
    if not names:
        return dict(every)
    out = {}
    for want in names:
        got = {n: s for n, s in every.items() if n == want or n.startswith(want.rstrip('/') + '/') or n.startswith(want)}
        if not got:
            raise SystemExit(f'no scenario {want!r}: {", ".join(every)}')
        out.update(got)
    return out


def check():
    every = load()
    bad = [line for name, s in every.items() for line in problems(name, s)]
    for line in bad:
        print('FAIL', line)
    print(f'{len(every)} scenarios read, {len(bad)} problems')
    return 1 if bad or not every else 0


def main():
    if '--stop' in sys.argv[1:]:
        print(f'{stop_left()} games stopped')
        return
    guard()
    if '--fuzz' in sys.argv[1:]:
        # (a mode of its own: quits at frames a dice names, tools/scenarios_fuzz.py)
        import scenarios_fuzz
        sys.exit(scenarios_fuzz.main([a for a in sys.argv[1:] if a != '--fuzz']))
    rest, values = cw.options(sys.argv[1:], {'--list': 'list', '--check': 'check'}, {'--jobs': 'jobs'})
    if values.get('check'):
        sys.exit(check())
    every = load()
    if values.get('list'):
        for name, s in every.items():
            print(f'{name}: {s["guards"]}' + (f' (needs {", ".join(s["needs"])})' if s['needs'] else '') +
                  (f' (KNOWN: {s["known"]})' if s['known'] else ''))
        return
    bad = [line for name, s in every.items() for line in problems(name, s)]
    if bad:
        sys.exit('\n'.join(bad))
    todo = chosen(every, rest)
    reply = cw.Reply()
    if not cw.ready(reply):
        sys.exit(reply.text)
    roms = have_roms()
    if 'bn6' not in roms:
        sys.exit(f"BN6's ROM is not in {play.ROM_DIR} (CYBERWORLD_ROM_DIR): the scenarios play the game")
    skipped = {n: s for n, s in todo.items() if set(s['needs']) - set(roms)}
    todo = {n: s for n, s in todo.items() if n not in skipped}
    t = time.time()
    failed = known = 0
    width = max((len(n) for n in list(todo) + list(skipped)), default=0)
    # (each set of ROMs' folder made before the games start, once)
    folders = {needs: rom_folder(roms, needs) for needs in {s['needs'] for s in todo.values()}}
    each_rom(roms)
    if any('{update}' in str(s['start']) + str(s['steps']) for s in todo.values()):
        serve_update()
    with concurrent.futures.ThreadPoolExecutor(int(values.get('jobs') or 4)) as pool:
        for r in pool.map(lambda item: run(*item, folders[item[1]['needs']]), todo.items()):
            word, fails, lines = verdict(r, todo[r['name']]['known'])
            failed += fails
            known += word == 'KNOWN'
            print(f'{word:5} {r["name"]:{width}}  {r["seconds"]:5.1f} s  {r["frames"]:6} frames', flush=True)
            for line in lines:
                print(line, flush=True)
    for name, s in skipped.items():
        print(f'SKIP  {name:{width}}  needs {", ".join(sorted(set(s["needs"]) - set(roms)))} in {play.ROM_DIR}')
    print(f'{len(todo)} scenarios: {len(todo) - failed - known} passed, {failed} failed' + (f', {known} known' if known else '') +
          (f', {len(skipped)} skipped' if skipped else '') + f' in {time.time() - t:.0f} s')
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
