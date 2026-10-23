#!/usr/bin/env python3
"""The agents' tools for the game (docs/DEVTOOLS.md, Agents' tools): build,
play, capture and compare it in one call each, with short answers. The MCP
server (tools/cw_mcp.py, the repository's .mcp.json) serves these as its
tools; the same from a shell:

  tools/cw.py shot [START] [--do CMDS] [--every N] [--until COND] [--ref REF]
                   [--scale S] [--full] [--no-picture] [--state full|none]
      a game of one batch: START (a route: tools/cw_routes.py, or the
      game's options, or both: "jukebox --seed 4"; VAR=VALUE first sets
      an environment variable), then CMDS (tools/play.py's commands), its
      state in words and one picture, a sheet with --every; --ref REF:
      the same by REF's build beside this one's, what differs framed
  tools/cw.py play NAME [--start START] [--fresh] [--do CMDS] [--until COND] ...
      the game NAME (tools/play.py's, in .build/play/NAME), started where
      --start says or still running: CMDS run, then what changed in its
      state and a picture; --do stop ends it
  tools/cw.py build [host|linux|test|lint]
      a build or a check, answered in a line, or its errors alone
  tools/cw.py routes

--until COND runs on (a batch of 60 frames at a time) until the state
says so, at most --limit frames (20000): COND is a state line's first word,
an operator and a value, "layer>=2", "screen=battle", "doing!=walking";
several apart by | are any of them. Every start builds the host binary
first where a source is newer than it, quietly.

The game runs from build/host/cyberworld on this machine (CYBERWORLD_CW_BIN
another), the ROMs read from ~/.cache/mmbn-ref/roms; nothing here keeps
state but the files in .build/: the server can restart at any time.
"""
import contextlib
import glob
import io
import os
import re
import shlex
import shutil
import subprocess
import sys
import threading
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import cw_pictures as pics  # noqa: E402
import cw_routes  # noqa: E402
import play  # noqa: E402

BIN = os.environ.get('CYBERWORLD_CW_BIN', os.path.join(ROOT, 'build', 'host', 'cyberworld'))
WORK = os.path.join(ROOT, '.build', 'cw')
CHUNK = 60           # an until's batch, in frames
LIMIT = 20000        # an until's most frames
SOURCES = ('src', 'shared', 'Makefile')
# a compiler's or make's complaint (never a command line's -Werror or scene_error.o)
DIAG = re.compile(r':\d+(:\d+)?: (fatal error|error|warning|note):|undefined reference|\*\*\*|^FAIL|Error \d+$')
COMPILED = re.compile(r'^\S*(gcc|cc|clang)\s.*\s-c\s+-o\s+\S+')


class Reply:
    """A tool's answer: lines of text and pictures (PIL images, with the
    paths they were saved at)."""

    def __init__(self, text='', error=False):
        self.lines = [text] if text else []
        self.images = []
        self.error = error

    def say(self, line):
        self.lines.append(line)

    def picture(self, im, path):
        os.makedirs(os.path.dirname(path), exist_ok=True)
        im.save(path)
        self.images.append(im)
        self.say(f'picture {os.path.relpath(path, ROOT)} ({im.width}x{im.height})')

    @property
    def text(self):
        return '\n'.join(self.lines)


# ---- builds ----

def build(what='host'):
    """build.py's build or check, its output kept to its complaints."""
    if what not in ('host', 'linux', 'test', 'lint'):
        return Reply(f'no build {what!r}: host, linux, test or lint', error=True)
    t = time.time()
    p = subprocess.run([sys.executable, os.path.join(ROOT, 'build.py'), what], cwd=ROOT, capture_output=True, text=True)
    lines = (p.stdout + p.stderr).splitlines()
    compiled = sum(1 for line in lines if COMPILED.match(line))
    diag = [line[:300] for line in lines if DIAG.search(line)]
    took = f'{time.time() - t:.1f} s'
    if p.returncode:
        reply = Reply(f'{what} FAILED in {took} (exit {p.returncode})', error=True)
        for line in (diag or [line[:300] for line in lines[-25:]])[:40]:
            reply.say(line)
        return reply
    reply = Reply(f'{what} ok in {took}' + (f', {compiled} files compiled' if compiled else ''))
    for line in diag[:20]:
        reply.say(line)
    if what in ('test', 'lint'):
        for line in [line for line in lines if line.strip()][-3:]:
            reply.say(line[:300])
    return reply


def stale(binary=BIN):
    """Whether a source is newer than the host binary."""
    if not os.path.exists(binary):
        return True
    built = os.path.getmtime(binary)
    for top in SOURCES:
        path = os.path.join(ROOT, top)
        if os.path.isfile(path):
            if os.path.getmtime(path) > built:
                return True
            continue
        for d, _, files in os.walk(path):
            if any(os.path.getmtime(os.path.join(d, f)) > built for f in files if f.endswith(('.c', '.h'))):
                return True
    return False


def ready(reply):
    """The host binary built where it is stale: False (the reply says why)
    where the build failed."""
    if BIN != os.path.join(ROOT, 'build', 'host', 'cyberworld') or not stale():
        return True
    b = build('host')
    reply.lines += b.lines
    reply.error = b.error
    return not b.error


# ---- games ----

def parse_start(spec):
    """START's words: (route commands, the game's options, environment,
    the screen the route arrives on)."""
    words = shlex.split(spec or '')
    env = {}
    while words and re.match(r'^[A-Z_][A-Z0-9_]*=', words[0]):
        k, v = words.pop(0).split('=', 1)
        env[k] = v
    commands, screen, args = '', None, []
    if words and not words[0].startswith('-'):
        args, commands, screen = cw_routes.route(words.pop(0))
    return commands, args + words, env, screen


def take_seed(args, default=0):
    """--seed out of the game's options (the last one given), for play.py's launch."""
    seed = default
    while '--seed' in args:
        i = args.index('--seed')
        seed = int(args[i + 1], 0)
        del args[i:i + 2]
    return seed, args


def state_lines(text):
    return [line for line in text.splitlines() if line.strip()]


def key(line):
    return line.split(' ', 1)[0]


def changes(old, new):
    """What changed between two states, a line each: the new line, with
    what it was where one line of that word was there before."""
    was = {}
    for line in state_lines(old):
        was.setdefault(key(line), []).append(line)
    now = state_lines(new)
    out = []
    for line in now:
        if key(line) in ('frame', 'screen') or line in was.get(key(line), ()):
            continue
        before = was.get(key(line), [])
        out.append(line + (f' (was {before[0].split(" ", 1)[1]})' if len(before) == 1 and ' ' in before[0] else ''))
    keys = {key(line) for line in now}
    out += [f'{k} gone' for k in was if k not in keys]
    return out


def headline(state, frames=None):
    words = dict(line.split(' ', 1) for line in state_lines(state) if ' ' in line)
    parts = [f'frame {words.get("frame", "?")}', f'screen {words.get("screen", words.get("scene", "?"))}']
    if frames is not None:
        parts.append(f'{frames} frames run')
    return ' · '.join(parts)


CONDITION = re.compile(r'^\s*([\w-]+)\s*(>=|<=|!=|=|>|<|~)\s*(.*?)\s*$')


def condition(spec):
    """An until's test: a function of the state's text."""
    tests = []
    for part in spec.split('|'):
        m = CONDITION.match(part)
        if not m:
            raise SystemExit(f'bad until {part.strip()!r}: WORD OP VALUE, OP one of = != > >= < <= ~ (contains)')
        tests.append(m.groups())

    def number(s):
        m = re.search(r'-?\d+', s)
        return int(m.group()) if m else None

    def holds(state):
        words = {}
        for line in state_lines(state):
            words.setdefault(key(line), line.split(' ', 1)[1] if ' ' in line else '')
        for word, op, value in tests:
            if word not in words:
                continue
            have = words[word]
            if op == '=' and have == value or op == '!=' and have != value or op == '~' and value in have:
                return True
            a, b = number(have), number(value)
            if a is not None and b is not None and op in ('>', '>=', '<', '<='):
                if {'>': a > b, '>=': a >= b, '<': a < b, '<=': a <= b}[op]:
                    return True
        return False
    return holds


def batch(h, commands, every=0, record=True):
    """Runs play.py's commands in game h: (its answer's frame, frames run,
    [(frame, picture path)], second screen paths, state)."""
    seq, every = play.prepare(commands, every)
    line = commands + (f'  # every {every}' if every else '')
    answer, shots, seconds, state = play.send(h, seq, line if record else None)
    if not answer.startswith('ok'):
        raise SystemExit(f'the game answered {answer!r}')
    end = int(answer.split()[1])
    run = sum(s[0] for s in seq if s != 'shot' and s[0] not in play.DEV_STEPS)
    return end, run, [(end - run + at, p) for at, p in shots], seconds, state


def run_until(h, test, limit, every):
    """Batches of CHUNK frames until the state passes `test`: (frame, frames
    run, pictures, state, whether it passed); one history line for all."""
    shots, ran, end, state = [], 0, 0, ''
    passed = False
    while ran < limit:
        end, n, got, _, state = batch(h, f'wait {min(CHUNK, limit - ran)}', every, record=False)
        ran += n
        shots += got if every else []
        for _, p in got if not every else ():
            os.remove(p)
        if test(state):
            passed = True
            break
    with open(os.path.join(h, 'history.txt'), 'a') as f:
        f.write(f'wait {ran}  # until  # ok {end}\n')
    return end, ran, shots, state, passed


def thin(shots, most=play.MAX_SHOTS):
    """At most `most` pictures, spread evenly; the rest deleted."""
    if len(shots) <= most:
        return shots
    keep = {round(i * (len(shots) - 1) / (most - 1)) for i in range(most)}
    for i, (_, p) in enumerate(shots):
        if i not in keep:
            os.remove(p)
    return [s for i, s in enumerate(shots) if i in keep]


def pictures(reply, shots, seconds, out, scale=None, full=False, picture=True):
    """The batch's pictures in the reply (one, or a sheet), saved as out;
    the BMPs removed."""
    if picture and shots:
        frames = [pics.game(pics.load(p), full) for _, p in shots]
        if len(frames) == 1:
            im = pics.scaled(frames[0], scale or 2)
        else:
            im = pics.sheet(frames, [f for f, _ in shots], scale or 1)
        reply.picture(im, out)
    for i, p in enumerate(seconds):
        if picture and os.path.exists(p):
            reply.picture(pics.load(p), out.replace('.png', f'-second{i}.png'))
    for p in [p for _, p in shots] + seconds:
        if os.path.exists(p):
            os.remove(p)


def turn(h, do, until=None, limit=LIMIT, every=0, picture=True, state='changes', scale=None, full=False, before=''):
    """One call's input in game h: the commands, then the until's run, the
    state (its changes, or whole) and the pictures."""
    reply = Reply()
    shots, seconds, ran, passed = [], [], 0, None
    end, now = 0, before
    if do:
        end, ran, shots, seconds, now = batch(h, do, every)
    if until:
        end, n, more, now, passed = run_until(h, condition(until), limit, every)
        ran += n
        shots = [s for s in shots if os.path.exists(s[1])] + more
    if picture and (not shots or shots[-1][0] != end):
        # (the end's picture where the input's last was earlier: an until's run)
        _, _, last, _, now = batch(h, 'shot', record=False)
        shots.append(last[-1])
    shots = thin(shots)
    reply.say(headline(now, ran))
    if until:
        reply.say(f'until {until}: ' + ('reached' if passed else f'not reached in {limit} frames'))
    if state == 'changes':
        reply.lines += changes(before, now)
    elif state == 'full':
        reply.lines += [line for line in state_lines(now) if key(line) not in ('frame', 'screen')]
    with open(os.path.join(h, 'cw_state.txt'), 'w') as f:
        f.write(now)
    n = len(glob.glob(os.path.join(h, 'shots', '*.png')))
    pictures(reply, shots if picture else [], seconds, os.path.join(h, 'shots', f'{n:04d}.png'), scale, full, picture)
    if not picture:
        for _, p in shots:
            if os.path.exists(p):
                os.remove(p)
    return reply


def begin(h, spec, fresh=False, seed=0, binary=None, keep_bin=True):
    """Game h started from START: (its route's commands, the screen it should reach)."""
    commands, args, env, screen = parse_start(spec)
    seed, args = take_seed(args, seed)
    with contextlib.redirect_stdout(io.StringIO()):
        play.launch(h, seed, fresh, args, env, binary or BIN, keep_bin)
    batch(h, 'wait 1')   # (as play.py's start: its replay skips it)
    return commands, screen


def arrived(reply, screen, state):
    """A route's screen against the state's: a line where it missed."""
    words = dict(line.split(' ', 1) for line in state_lines(state) if ' ' in line)
    if screen and words.get('screen') not in (None, screen):
        reply.say(f'(the route meant to reach {screen}: input may have been lost to a fade)')


def guarded(fn):
    """A call's SystemExit (play.py's words for a bad command) as an error reply."""
    def call(*a, **k):
        try:
            return fn(*a, **k)
        except SystemExit as e:
            return Reply(str(e.code), error=True)
    call.__name__ = fn.__name__
    call.__doc__ = fn.__doc__
    return call


@guarded
def play_game(name, do='', start=None, fresh=False, until=None, limit=LIMIT, every=0, picture=True, state='changes',
              scale=None, full=False):
    """tools/play.py's game NAME: started where START says (or as it runs),
    the commands, then what changed and a picture."""
    h = play.home(name)
    if do.strip() == 'stop':
        play.stop(h)
        return Reply(f'stopped {name}')
    reply = Reply()
    route, screen, before = '', None, ''
    if start is not None or fresh or not os.path.exists(os.path.join(h, 'pid')):
        if not ready(reply):
            return reply
        route, screen = begin(h, start or 'title', fresh)
        reply.say(f'started {name} from {start or "the title"}' + (' (fresh)' if fresh else ''))
    elif os.path.exists(os.path.join(h, 'cw_state.txt')):
        before = open(os.path.join(h, 'cw_state.txt')).read()
    if route and do:
        # (the route a batch of its own: a sheet is the input's frames)
        _, _, shots, _, _ = batch(h, route)
        for _, p in shots:
            os.remove(p)
    r = turn(h, do or route, until, limit, every, picture, state, scale, full, before)
    reply.lines += r.lines
    reply.images += r.images
    if screen and not do and not until:
        arrived(reply, screen, open(os.path.join(h, 'state.txt')).read())
    return reply


SHOTS = [0]


def one_shot(spec, do, until, limit, every, binary, seed):
    """A game of one batch in a folder of its own, stopped after: a dict of
    its folder, frame, frames run, pictures, second screens, state, the
    route's screen and whether the until was reached."""
    SHOTS[0] += 1
    h = os.path.join(WORK, 'g', f'{os.getpid() % 100000}-{SHOTS[0]}{threading.get_ident() % 100}')
    shutil.rmtree(h, ignore_errors=True)
    os.makedirs(h)
    try:
        route, screen = begin(h, spec, True, seed, binary, keep_bin=False)
        ran = 0
        if route and do:
            # (the route a batch of its own: a sheet is the input's frames)
            _, ran, shots, _, _ = batch(h, route, record=False)
            for _, p in shots:
                os.remove(p)
        end, n, shots, seconds, state = batch(h, (do or route) or 'wait 1', every, record=False)
        ran += n
        passed = None
        if until:
            end, n, more, state, passed = run_until(h, condition(until), limit, every)
            ran += n
            if every:
                shots += more
            else:
                for _, p in shots:
                    os.remove(p)
                _, _, shots, _, state = batch(h, 'shot', record=False)
        return dict(h=h, end=end, ran=ran, shots=thin(shots), seconds=seconds, state=state, screen=screen, passed=passed)
    finally:
        play.stop(h)


@guarded
def shot(start='title', do='', until=None, limit=LIMIT, every=0, picture=True, state='full', scale=None, full=False,
         ref=None):
    """A game of one batch from START: its state and picture (a sheet with
    every); with ref, REF's build beside this one's."""
    reply = Reply()
    if not ready(reply):
        return reply
    commands, args, env, _ = parse_start(start)
    seed, _ = take_seed(args, 1)
    if ref:
        return compare(reply, start, do, until, limit, every, scale, full, ref, seed)
    g = one_shot(start, do, until, limit, every, BIN, seed)
    reply.say(headline(g['state'], g['ran']))
    if until:
        reply.say(f'until {until}: ' + ('reached' if g['passed'] else f'not reached in {limit} frames'))
    if state == 'full':
        reply.lines += [line for line in state_lines(g['state']) if key(line) not in ('frame', 'screen')]
    if not do and not until:
        arrived(reply, g['screen'], g['state'])
    n = len(glob.glob(os.path.join(WORK, 'pictures', '*.png')))
    pictures(reply, g['shots'], g['seconds'], os.path.join(WORK, 'pictures', f'{n:04d}-shot.png'), scale, full, picture)
    shutil.rmtree(g['h'], ignore_errors=True)
    return reply


# ---- another commit's build beside this one's ----

def ref_binary(ref):
    """REF's host binary, built once in a worktree of its own under
    .build/cw/ref (kept for the next compare): (path, short hash) or
    SystemExit with the build's complaints."""
    p = subprocess.run(['git', 'rev-parse', '--verify', '--quiet', f'{ref}^{{commit}}'], cwd=ROOT, capture_output=True, text=True)
    if p.returncode:
        raise SystemExit(f'no commit {ref!r}')
    sha = p.stdout.strip()[:12]
    d = os.path.join(WORK, 'ref', sha)
    binary = os.path.join(d, 'build', 'host', 'cyberworld')
    if os.path.exists(binary):
        return binary, sha
    if not os.path.isdir(d):
        subprocess.run(['git', 'worktree', 'add', '--detach', '--quiet', d, sha], cwd=ROOT, check=True, capture_output=True)
    b = subprocess.run([sys.executable, os.path.join(d, 'build.py'), 'host'], cwd=d, capture_output=True, text=True)
    if b.returncode:
        diag = [line[:300] for line in (b.stdout + b.stderr).splitlines() if DIAG.search(line)][:20]
        raise SystemExit('\n'.join([f'{ref} ({sha}) did not build'] + diag))
    return binary, sha


def compare(reply, start, do, until, limit, every, scale, full, ref, seed):
    old_bin, sha = ref_binary(ref)
    results = {}

    def go(which, binary):
        try:
            results[which] = one_shot(start, do, until, limit, every, binary, seed)
        except SystemExit as e:
            results[which] = e
    threads = [threading.Thread(target=go, args=(w, b)) for w, b in (('old', old_bin), ('new', BIN))]
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    for which in ('old', 'new'):
        if isinstance(results[which], SystemExit):
            reply.say(f'{which}: {results[which].code}')
            reply.error = True
            return reply
    old, new = results['old'], results['new']
    reply.say(f'{ref} ({sha}) beside this build: ' + headline(new['state'], new['ran']))
    pairs, labels = [], []
    for (fa, pa), (fb, pb) in zip(old['shots'], new['shots']):
        a, b = pics.game(pics.load(pa), full), pics.game(pics.load(pb), full)
        n, box = pics.difference(a, b)
        reply.say(f'frame {fb}: ' + (f'{n} pixels differ in x {box[0]}-{box[2] - 1}, y {box[1]}-{box[3] - 1}' if n else 'the same'))
        pairs.append((a, b))
        labels.append(fb)
    for line in changes(old['state'], new['state']):
        reply.say(f'state: {line}')
    if pairs:
        n = len(glob.glob(os.path.join(WORK, 'pictures', '*.png')))
        reply.picture(pics.side_by_side(pairs, labels, scale or (2 if len(pairs) == 1 else 1)),
                      os.path.join(WORK, 'pictures', f'{n:04d}-compare.png'))
    for r in (old, new):
        shutil.rmtree(r['h'], ignore_errors=True)
    return reply


def prune():
    """The cached builds of other commits removed (their worktrees too)."""
    for d in glob.glob(os.path.join(WORK, 'ref', '*')):
        subprocess.run(['git', 'worktree', 'remove', '--force', d], cwd=ROOT, capture_output=True)
        shutil.rmtree(d, ignore_errors=True)
    return Reply('removed the cached builds')


def routes():
    return Reply('\n'.join(f'{n}: {" ".join(a)} → {s}' for n, (a, _, s) in cw_routes.ROUTES.items()))


# ---- the command line ----

def options(rest, flags, values):
    got = {}
    i = 0
    pos = []
    while i < len(rest):
        a = rest[i]
        if a in flags:
            got[flags[a]] = True
        elif a in values:
            got[values[a]] = rest[i + 1]
            i += 1
        else:
            pos.append(a)
        i += 1
    return pos, got


def main():
    if len(sys.argv) < 2 or sys.argv[1] not in ('shot', 'play', 'build', 'routes', 'prune'):
        sys.exit(__doc__)
    op, rest = sys.argv[1], sys.argv[2:]
    common = {'--do': 'do', '--until': 'until', '--limit': 'limit', '--every': 'every', '--scale': 'scale',
              '--state': 'state', '--ref': 'ref', '--start': 'start'}
    pos, got = options(rest, {'--full': 'full', '--no-picture': 'nopic', '--fresh': 'fresh'}, common)
    k = dict(do=got.get('do', ''), until=got.get('until'), limit=int(got.get('limit', LIMIT)), every=int(got.get('every', 0)),
             picture=not got.get('nopic'), scale=int(got['scale']) if 'scale' in got else None, full=bool(got.get('full')))
    if op == 'shot':
        reply = shot(' '.join(pos) or 'title', state=got.get('state', 'full'), ref=got.get('ref'), **k)
    elif op == 'play':
        if not pos:
            sys.exit('play NAME')
        reply = play_game(pos[0], start=got.get('start'), fresh=bool(got.get('fresh')), state=got.get('state', 'changes'), **k)
    elif op == 'build':
        reply = build(pos[0] if pos else 'host')
    elif op == 'prune':
        reply = prune()
    else:
        reply = routes()
    print(reply.text)
    sys.exit(1 if reply.error else 0)


if __name__ == '__main__':
    main()
