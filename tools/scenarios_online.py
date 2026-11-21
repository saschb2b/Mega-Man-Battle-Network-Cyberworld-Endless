"""The scenarios' cast (tools/scenarios.py; docs/devtools/scenarios.md,
A server and other games): what an online course needs beside its one
game. A Net Square server of this machine, started from the checkout
(server/main.py) on a free port of 127.0.0.1 before the games and stopped
after them; other games beside the first, each with a name a step
addresses; files written into each game's data folder before it starts;
and the games not being stepped kept running while they are online, since
a visit plays at the clock's pace and a game that stands still drops off
its server.

A scenario asks for them so:

  SERVER = True                   a server (or a dict: seed=, weeks=, settings=)
  GAMES = {'beta': START}         other games, started after the first
  FILES = {'netpass.ini': '...'}  into every game's data folder
  ('@beta', 'do', 'press A')      a step in game beta (no name: the first)
  ('server', 'stop')              the server gone; ('server', 'start') back,
                                  on the same port with the same state

and names what only the run knows by a place in braces, in START, GAMES
and FILES: {server} the server's address, {nowhere} an address of this
machine nothing answers on, {name} the game's name (the first: alpha) and
{identity} a private identity made of it.

One rule above the rest: no game of a scenario reaches a host but this
machine. A scenario names no address of its own (written(): {server} and
{nowhere} alone), and every game the runner starts runs with
CYBERWORLD_ONLINE_LOCAL, under which the game's socket refuses any host
but 127.0.0.1 (src/online/online_socket.c).
"""
import concurrent.futures
import hashlib
import json
import os
import re
import shutil
import socket
import subprocess
import sys
import threading
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import cw  # noqa: E402
import play  # noqa: E402

FIRST = 'alpha'                   # the first game's name, where a place asks it
ADDRESS = re.compile(r'wss?://', re.I)
PLACE = re.compile(r'\{(\w+)\}')
PLACES = ('server', 'nowhere', 'name', 'identity')
SERVER_KEYS = ('seed', 'weeks', 'settings')
# (the server's weeks: server/cyberworld_server/week.py's, from Monday 5 January 1970)
WEEK, WEEK_EPOCH = 7 * 24 * 3600, 4 * 24 * 3600
PUMP = 5                          # an idle game's batch, in frames: a twelfth of a second of a visit
ENV = {'CYBERWORLD_ONLINE_LOCAL': '1'}   # every game's that scenarios.py's start starts, whatever the scenario


def free_port():
    with socket.socket() as s:
        s.bind(('127.0.0.1', 0))
        return s.getsockname()[1]


def address(port):
    return f'ws://127.0.0.1:{port}/presence'


# ---- reading one ----

def read(m):
    """A scenario's cast as its file (a module, or a MANY entry's dict)
    writes it: dict(server, games, files)."""
    get = m.get if isinstance(m, dict) else lambda k, d=None: getattr(m, k, d)
    return dict(server=get('SERVER'), games=dict(get('GAMES') or {}), files=dict(get('FILES') or {}))


def split(step):
    """A step's game and the step without it: ('@beta', 'do', ...) is
    ('beta', ('do', ...)); no name is the first game's ('')."""
    if step and isinstance(step[0], str) and step[0].startswith('@'):
        return step[0][1:], tuple(step[1:])
    return '', tuple(step)


def written(name, s):
    """What is wrong with a scenario's cast as written, a line each."""
    out = []
    server, games, files = s.get('server'), s.get('games') or {}, s.get('files') or {}
    if server not in (None, False, True) and (not isinstance(server, dict) or set(server) - set(SERVER_KEYS)):
        out.append(f'{name}: SERVER is True or a dict of {", ".join(SERVER_KEYS)}')
    texts = [s['start'], *games.values(), *files.values()]
    texts += [step[1] for step in (split(x)[1] for x in s['steps']) if step and step[0] == 'restart' and len(step) > 1]
    for text in texts:
        if not isinstance(text, str):
            out.append(f'{name}: {text!r}: a start and a file are text')
            continue
        if ADDRESS.search(text):
            out.append(f'{name}: {text!r} names an address: a scenario reaches this machine alone, by {{server}} or {{nowhere}}')
        for place in PLACE.findall(text):
            if place not in PLACES:
                continue   # (another's place: the runner's own)
            if place == 'server' and not server:
                out.append(f'{name}: {{server}} with no SERVER')
        if re.search(r'\bonline=(?!\{(server|nowhere)\})', text):
            out.append(f'{name}: {text!r}: --dev online= takes {{server}} or {{nowhere}}')
    for game in games:
        if not re.fullmatch(r'[a-z][a-z0-9]*', game) or game == FIRST:
            out.append(f'{name}: GAMES {game!r}: a name of small letters, not {FIRST} (the first game)')
    for i, raw in enumerate(s['steps'], 1):
        game, step = split(raw)
        if game and game not in games:
            out.append(f'{name}: step {i} {raw!r}: no game {game!r} in GAMES')
        if step and step[0] == 'server':
            if game or len(step) != 2 or step[1] not in ('stop', 'start'):
                out.append(f'{name}: step {i} {raw!r}: ("server", "stop") or ("server", "start")')
            elif not server:
                out.append(f'{name}: step {i} {raw!r}: no SERVER')
    return out


# ---- the server ----

class Server:
    """The ROM-free server from the checkout on 127.0.0.1, its state in
    `folder`: seed its Square's, weeks the week's Board's posts before it
    starts ((name, depth, won, minutes) each), settings others of
    server.example.ini over these (dict(capacity=1), dict(maintenance=
    'until 14:00 UTC'))."""

    def __init__(self, folder, seed=120, weeks=(), settings=None):
        self.folder, self.port, self.proc, self.log = folder, free_port(), None, None
        shutil.rmtree(folder, ignore_errors=True)
        os.makedirs(os.path.join(folder, 'state'))
        if weeks:
            week = int((time.time() - WEEK_EPOCH) // WEEK)
            with open(os.path.join(folder, 'state', 'weeks.jsonl'), 'w') as f:
                for i, (name, depth, won, minutes, *more) in enumerate(weeks):
                    # (a fifth: the dive pulled back once by its crew, the Board's "*", issue #718)
                    f.write(json.dumps({'week': week, 'seed': 1, 'identity': f'{i + 1:064x}', 'name': name, 'depth': depth,
                                        'act': min((depth - 1) // 3 + 1, 4), 'won': won, 'minutes': minutes, 'hash': 1,
                                        'pulled': bool(more and more[0]), 'time': time.time()},
                                       separators=(',', ':'), sort_keys=True) + '\n')
        self.ini = os.path.join(folder, 'server.ini')
        lines = dict(name='Scenario', motd="A scenario's Square", host='127.0.0.1', port=self.port, seed=seed, capacity=8,
                     player_cap=64, tick_hz=20, state_dir=os.path.join(folder, 'state'), whitelist='', free_text='false',
                     trading='true', arena='true', origins='')
        lines.update(settings or {})
        # (never another's to come in: this machine alone, on the port the scenario was given)
        lines.update(host='127.0.0.1', port=self.port)
        with open(self.ini, 'w') as f:
            f.write('[server]\n' + ''.join(f'{k} = {v}\n' for k, v in lines.items()))
        self.start()

    @property
    def url(self):
        return address(self.port)

    def start(self):
        """Started (again, on the same port and state), waited for until it listens."""
        if self.proc and self.proc.poll() is None:
            return
        self.log = open(os.path.join(self.folder, 'server.log'), 'a')
        if STOPPING:
            raise SystemExit('the run is ending')
        LIVE.append(self)
        self.proc = subprocess.Popen([sys.executable, os.path.join(ROOT, 'server', 'main.py'), '--config', self.ini,
                                      '--event-log', os.path.join(self.folder, 'events.jsonl')],
                                     stdout=self.log, stderr=subprocess.STDOUT, cwd=self.folder)
        for _ in range(200):
            if self.proc.poll() is not None:
                break
            try:
                socket.create_connection(('127.0.0.1', self.port), 0.2).close()
                return
            except OSError:
                time.sleep(0.05)
        raise SystemExit(f'the server did not start: {os.path.relpath(os.path.join(self.folder, "server.log"), ROOT)}')

    def stop(self):
        """Gone at once, as a server that fell over (no goodbye to its games)."""
        if self.proc and self.proc.poll() is None:
            self.proc.kill()
            self.proc.wait(10)
        if self.log:
            self.log.close()
            self.log = None


LIVE = []   # the servers started
STOPPING = []   # (set once the run is ending: no server starts after it)


def stop_all():
    """Every server this run started, gone (tools/scenarios.py's stop_all)."""
    STOPPING.append(True)
    for server in LIVE:
        try:
            server.stop()
        except (OSError, subprocess.SubprocessError):
            pass


class Nowhere:
    """An address of this machine nothing answers on: a port bound and
    never listened on, held while the scenario plays so no other process
    takes it (a game's call there is refused at once)."""

    def __init__(self):
        self.sock = socket.socket()
        self.sock.bind(('127.0.0.1', 0))
        self.url = address(self.sock.getsockname()[1])

    def close(self):
        self.sock.close()


# ---- the games kept running ----

def online(state):
    """Whether a game's state says it is online: in a visit, or on a dive
    with its link (such a game is kept running while another is stepped)."""
    words = {cw.key(line): line.split(' ', 1)[1] if ' ' in line else '' for line in cw.state_lines(state)}
    return words.get('visit', 'home') != 'home' or words.get('divelink', 'off') == 'on'


class Pump:
    """A game kept running between its steps while it is online, PUMP
    frames a batch at the clock's pace: `hold()` before a step of its own
    (the batch under way waited for), `release(state)` after it."""

    def __init__(self, h):
        self.h, self.lock, self.on, self.alive, self.state = h, threading.Lock(), False, True, None
        self.thread = threading.Thread(target=self.loop, daemon=True)
        self.thread.start()

    def loop(self):
        state = os.path.join(self.h, 'pump.txt')
        while self.alive:
            took = 0.0
            with self.lock:
                if self.on and self.alive:
                    t = time.time()
                    try:
                        if not play.talk(self.h, f'{PUMP};state {play.short(state)}', 30).startswith('ok'):
                            self.on = False
                        else:
                            with open(state) as f:
                                self.state = f.read().strip()
                            self.on = online(self.state)
                    except (SystemExit, OSError, ValueError):
                        self.on = False   # (the game gone: its next step says so)
                    took = time.time() - t
            # (a visit paces itself; a dive's layer runs as fast as it can, and is held to the clock's pace here)
            time.sleep(max(0.001, PUMP / 60 - took) if self.on else 0.02)

    def hold(self):
        """The game held for a step: its state where it ran on since its last one (None: it stood)."""
        self.lock.acquire()
        state, self.state = self.state, None
        return state

    def release(self, state):
        self.on = online(state)
        self.lock.release()

    def close(self):
        self.alive = False
        self.thread.join(35)


# ---- a scenario's cast ----

class Cast:
    """A scenario's server and games: its first game in folder h, the
    others in h--NAME. scenarios.py's `start` and `step_run` play each
    (passed in: this file stands under it)."""

    def __init__(self, h, s, roms, start, step_run):
        # (roms: the games' ROM folder, or what makes a game's own once its folder h is there: roms(h))
        self.s, self.roms, self.start_game, self.step_run, self.rom = s, roms, start, step_run, {}
        self.h = {'': h, **{name: f'{h}--{name}' for name in s.get('games') or {}}}
        self.state = dict.fromkeys(self.h, '')
        self.kept = {name: {} for name in self.h}
        self.pumps, self.server, self.nowhere = {}, None, None

    def placed(self, text, game):
        """A start or a file with its places put in (others' left as they are)."""
        name = game or FIRST

        def put(m):
            if m[1] == 'server':
                return self.server.url
            if m[1] == 'nowhere':
                self.nowhere = self.nowhere or Nowhere()
                return self.nowhere.url
            if m[1] == 'name':
                return name.capitalize()
            if m[1] == 'identity':
                return hashlib.sha1(name.encode()).hexdigest()[:32]
            return m[0]
        return PLACE.sub(put, text)

    def begin(self, game, spec, fresh):
        """Game `game` started from `spec`: its files written first where fresh."""
        h = self.h[game]
        if fresh:
            shutil.rmtree(h, ignore_errors=True)
            os.makedirs(os.path.join(h, 'data'))
            for name, text in (self.s.get('files') or {}).items():
                with open(os.path.join(h, 'data', name), 'w') as f:
                    f.write(self.placed(text, game))
            self.rom[game] = self.roms(h) if callable(self.roms) else self.roms
        spec = self.placed(spec, game)
        # (never play.launch's fresh, which would take the files just written with the folder)
        self.state[game], ran = self.start_game(h, spec, False, self.rom[game])
        return ran

    def start(self):
        """The server, then every game: (the first's state, frames run)."""
        server = self.s.get('server')
        if server:
            self.server = Server(self.h[''] + '--server', **(server if isinstance(server, dict) else {}))
        # (side by side: a game with the other games' ROMs takes seconds to start)
        with concurrent.futures.ThreadPoolExecutor(len(self.h)) as pool:
            frames = sum(pool.map(lambda game: self.begin(game, self.s['start'] if not game else self.s['games'][game], True), self.h))
        for game in self.h:
            self.pumps[game] = Pump(self.h[game])
            self.pumps[game].hold()
            self.pumps[game].release(self.state[game])
        return self.state[''], frames

    def step(self, raw):
        """One step: (its game, that game's state after, frames run)."""
        game, step = split(raw)
        if step[0] == 'server':
            (self.server.stop if step[1] == 'stop' else self.server.start)()
            return game, self.state[game], 0
        if step[0] == 'sleep':
            # (the clock's seconds with no game held: each online game runs on at its pace, answering the others)
            time.sleep(float(step[1]))
            return game, self.state[game], 0
        pump, ran, n = self.pumps[game], 0, 0
        # (the game ran on since its last step: its state as its last batch left it)
        self.state[game] = pump.hold() or self.state[game]
        try:
            if step[0] == 'restart':
                step = ('restart', self.placed(step[1], game))
            try:
                self.state[game], n = self.step_run(self.h[game], step, self.state[game], self.kept[game], self.rom[game])
            except Exception as e:
                # (a failure's state is its game's: kept for the next step's pump too)
                if len(e.args) > 1 and isinstance(e.args[1], str):
                    self.state[game] = e.args[1]
                raise
        finally:
            pump.release(self.state[game])
        return game, self.state[game], ran + n

    def quiet(self):
        """Every game left alone: none is run on any more (a failure's
        picture is taken of a game nothing else talks to)."""
        for pump in self.pumps.values():
            pump.close()
        self.pumps = {}

    def close(self):
        self.quiet()
        for h in self.h.values():
            play.stop(h)
        if self.server:
            self.server.stop()
        if self.nowhere:
            self.nowhere.close()


# ---- the courses' common steps ----

# a game's netpass.ini with its stamps seen before, so the greeter's stamp
# moment does not hold MegaMan as he arrives: BN6's ROM alone, and the set
SEEN_SQUARE = 'stamps=1\n'
SEEN_FULL = 'stamps=4\ngames=3\n'
# Lan's HP after act 2, the gate pointed at the scenario's server by --dev
# online (no question asked: a step onto the gate goes)
HOME = '--scene home --run-depth 7 --seed 3 --dev quiet,online={server},onlinename={name}'


def at(game, *steps):
    """Steps as game `game`'s ('' the first)."""
    return [(f'@{game}', *step) if game else step for step in steps]


def paged(game=''):
    """Lan's HP's arrival words paged: MegaMan free at home."""
    return at(game, ('do', 'wait 300'), ('repeat', 'mash A 60; wait 30', 'chat=closed'))


def soon(cond, game='', frames=300):
    """Until COND in a visit, looked at every ten frames: a visit plays at
    the clock's pace, where an `until`'s look once in sixty frames costs a
    second each."""
    return at(game, ('repeat', 'wait 10', cond, max(1, frames // 10)))


def into_square(game=''):
    """From HOME into the Square: the arrival paged, onto the gate, there
    and free (the log-in beam over, no greeter's word)."""
    return paged(game) + at(game, ('do', 'place 16 72 3; wait 10')) + soon('visit=square', game, 900) + soon('held=none', game, 600)


def yes(game=''):
    """A question's Yes taken where it opens on No, as the gate's and the
    way home's do: its options up and its cursor set (BN6 shows the options
    a few frames before it puts the cursor on No), LEFT onto Yes, then A."""
    return at(game, ('repeat', 'wait 4', 'choice=2/2', 90), ('repeat', 'press LEFT; wait 4', 'choice=1/2', 6), ('do', 'press A; wait 6'))


def pick(done, game=''):
    """The option under the cursor taken, A until `done` says its effect:
    a chat box counts its options as it types them out, a moment before it
    takes a key."""
    return at(game, ('repeat', 'press A; wait 20', done, 8))


def home_again(game=''):
    """From the Square home: R's question, Yes, the gate's word paged."""
    return (at(game, ('do', 'press R; wait 6')) + yes(game) + soon('visit=home', game, 900)
            + at(game, ('repeat', 'mash A 60; wait 30', 'chat=closed')))


def course(*parts):
    """A course of steps and of lists of them (the common ones above), flat."""
    out = []
    for part in parts:
        out += part if isinstance(part, list) else [part]
    return out
