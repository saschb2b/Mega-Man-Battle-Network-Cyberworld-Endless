#!/usr/bin/env python3
"""A trailer's online footage: a local Net Square server and several
headless games in it at once, each with its own data folder, operator name
and scripted input, their frames kept as tools/trailer.py's capture keeps a
scene's (every second frame, 30 fps, NNNN.png in WORK/SCENE-GAME).

The release pictures' runs (build.py screenshots) are single games in the
build image; a visit to the Square needs a server and other players, so
these run the Linux build (build/linux/cyberworld, `build.py linux`) on
this machine against the ROM-free server (build/server, `build.py
server`), as docs/DEVTOOLS.md's two games at once do. Never the official
server: each scene starts its own on 127.0.0.1, its port its own, its
state in WORK (the week's Board seeded there where a scene asks).

A scene is a dict: `seed` (the Square's), optional `weeks` (the Board's
entries this week: (name, layer, won, minutes)), and `games`, each a dict:
`name` (the folder's and the operator's), `script` (--input), `args`
(more of the game's), `first` and `last` (the frames kept; none where
the game is only there to be seen), `bn5` (False: BN6's ROM alone),
`delay` (seconds after the first game starts). A cut lists its scenes and
calls capture(); --keep in tools/trailer.py keeps what is there.

A scene whose course waits on what its games do (a NetBattle's end, a
team's match, a card to answer) is a function instead, `run(work, scene,
port, roms)`, its games `Remote`s: played by remote play (--remote) a
batch of steps at a time as their logs say, every frame kept as it comes
(0.14.0's NetBattles, relay and two a side). Those run all at once, after
the scenes by the clock.

    python3 tools/trailer_online.py VERSION [SCENE ...]   (a cut's online scenes, again)
"""
import glob
import hashlib
import json
import os
import shutil
import subprocess
import sys
import threading
import time

from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import build  # noqa: E402  (the ROM folder and the ROMs' SHA-1)

BINARY = os.path.join(ROOT, 'build', 'linux', 'cyberworld')
SERVER = os.path.join(ROOT, 'build', 'server', 'cyberworld-server.pyz')
WEEK, WEEK_EPOCH = 604800, 345600       # (server/cyberworld_server/week.py, shared/online's spec)

# (Lan's HP after act 2, seed 3: the arrival's words paged, MegaMan free
# from frame 1200, as build.py's HP_7 pictures; then on the gate, and
# Yes to MegaMan's question)
HOME = ['--scene', 'home', '--run-depth', '7', '--seed', '3']
PAGED = '600:,' + '6:A,94:,' * 6
GATE = PAGED + '0:place 16 72 3,40:,6:LEFT,20:,6:A,'


def rom_dirs(work):
    """The ROM folder with both games, and one with BN6's alone (a link
    to it in WORK), for the players who have only BN6."""
    full = build.default_rom_dir()
    bn6 = os.path.join(work, 'data-roms-bn6')
    if not os.path.isdir(bn6):
        os.makedirs(bn6)
        for path in sorted(glob.glob(os.path.join(full, '*.gba'))):
            with open(path, 'rb') as f:
                if hashlib.sha1(f.read()).hexdigest() == build.BN6_SHA1:
                    os.symlink(path, os.path.join(bn6, 'bn6.gba'))
                    break
    return full, bn6


def free_port():
    import socket
    with socket.socket() as s:
        s.bind(('127.0.0.1', 0))
        return s.getsockname()[1]


class Server:
    """The ROM-free server on 127.0.0.1, its state in `folder`; the week's
    Board seeded with `weeks` before it starts."""

    def __init__(self, folder, seed, weeks=()):
        self.folder, self.port = folder, free_port()
        shutil.rmtree(folder, ignore_errors=True)
        os.makedirs(os.path.join(folder, 'state'))
        if weeks:
            week = int((time.time() - WEEK_EPOCH) // WEEK)
            with open(os.path.join(folder, 'state', 'weeks.jsonl'), 'w') as f:
                for i, (name, depth, won, minutes) in enumerate(weeks):
                    f.write(json.dumps({'week': week, 'seed': 1, 'identity': f'{i + 1:064x}', 'name': name,
                                        'depth': depth, 'act': min((depth - 1) // 3 + 1, 4), 'won': won,
                                        'minutes': minutes, 'hash': 1, 'time': time.time()},
                                       separators=(',', ':'), sort_keys=True) + '\n')
        ini = os.path.join(folder, 'server.ini')
        with open(ini, 'w') as f:
            f.write('[server]\nname = Cyberworld Endless\nmotd = Welcome to the Net Square\nhost = 127.0.0.1\n'
                    f'port = {self.port}\nseed = {seed}\ncapacity = 8\nplayer_cap = 64\ntick_hz = 20\n'
                    f'state_dir = {os.path.join(folder, "state")}\nwhitelist =\nfree_text = false\n'
                    'trading = true\narena = true\norigins =\n')
        self.log = open(os.path.join(folder, 'server.log'), 'w')
        self.proc = subprocess.Popen([sys.executable, SERVER, '--config', ini, '--event-log',
                                      os.path.join(folder, 'events.jsonl')], stdout=self.log, stderr=subprocess.STDOUT)
        time.sleep(1.0)
        if self.proc.poll() is not None:
            sys.exit(f'the server did not start: {os.path.join(folder, "server.log")}')

    @property
    def url(self):
        return f'ws://127.0.0.1:{self.port}/presence'

    def stop(self):
        self.proc.terminate()
        try:
            self.proc.wait(10)
        except subprocess.TimeoutExpired:
            self.proc.kill()
        self.log.close()


def run_game(work, scene, g, url, roms):
    """One game of a scene, its frames first to last kept as every second
    one in WORK/SCENE-NAME/NNNN.png (the canvas's middle 240 x 160)."""
    name = g['name']
    data = os.path.join(work, f'data-{scene}-{name}')
    shutil.rmtree(data, ignore_errors=True)
    os.makedirs(data)
    out = os.path.join(work, f'{scene}-{name}')
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    operator = g.get('operator', name.capitalize())
    for fname, text in g.get('files', {}).items():
        with open(os.path.join(data, fname), 'w') as f:
            f.write(text)
    if g.get('offline'):
        dev = ','.join(['quiet'] + g.get('dev', []))   # (a screen of the game's own, no visit)
    elif g.get('ask'):
        # (the gate as a player's: network.ini names the server, the keeper's
        # first questions answered, so MegaMan asks "To the Net Square,Lan?")
        with open(os.path.join(data, 'network.ini'), 'w') as f:
            f.write(f'identity={hashlib.sha1(name.encode()).hexdigest()[:32]}\nname={operator}\nnamed=1\nselected=1\n'
                    f'consent=2\nserver=wss://cyberworld.saschb2b.com/presence\nserver={url}\n')
        dev = ','.join(['quiet'] + g.get('dev', []))
    else:
        dev = ','.join(['quiet', f'online={url}', f'onlinename={operator}'] + g.get('dev', []))
    args = [BINARY, '--headless', '--rom-dir', roms[0] if g.get('bn5', True) else roms[1], '--data-dir', data,
            *g.get('args', HOME), '--dev', dev, '--input', g['script'], '--frames', str(g.get('frames') or g['last'] + 1)]
    if g.get('first') is not None:
        args += ['--shot-range', f'{g["first"]}:{g["last"]}:{out}/f']
    with open(os.path.join(data, 'log.txt'), 'w') as log:
        code = subprocess.call(args, stdout=log, stderr=subprocess.STDOUT, cwd=ROOT,
                               env=dict(os.environ, CYBERWORLD_EMU_DEBUG='1'))
    if code:
        print(f'{scene}-{name}: the game ended with {code} ({data}/log.txt)')
    if g.get('first') is None:
        os.rmdir(out)
        return
    n = 0
    for f in range(g['first'], g['last'] + 1):
        p = os.path.join(out, f'f{f:05d}.bmp')
        if not os.path.exists(p):
            break
        if (f - g['first']) % 2 == 0:
            im = Image.open(p).convert('RGB')
            w, h = im.size
            im.crop(((w - 240) // 2, (h - 160) // 2, (w + 240) // 2, (h + 160) // 2)).save(os.path.join(out, f'{n:04d}.png'))
            n += 1
        os.remove(p)


def capture_scene(work, scene, spec):
    """A scene: its server, then its games at once (each after its
    `delay`), until every one has ended."""
    roms = rom_dirs(work)
    server = Server(os.path.join(work, f'data-server-{scene}'), spec.get('seed', 120), spec.get('weeks', ()))
    try:
        threads = []
        t0 = time.time()
        for g in spec['games']:
            wait = g.get('delay', 0) - (time.time() - t0)
            if wait > 0:
                time.sleep(wait)
            t = threading.Thread(target=run_game, args=(work, scene, g, server.url, roms))
            t.start()
            threads.append(t)
        for t in threads:
            t.join()
    finally:
        server.stop()
    print('captured', scene)


def kept(work, scene, spec):
    return all(os.path.isdir(os.path.join(work, f'{scene}-{g["name"]}'))
               and os.listdir(os.path.join(work, f'{scene}-{g["name"]}'))
               for g in spec['games'] if g.get('first') is not None)


# ---- games driven as they go: remote play, the course reacting to each log ----

# (through the gate as a player goes: the arrival's words paged, on the
# gate, LEFT to MegaMan's Yes; remote play's steps, as tools/play.py's)
REMOTE_GATE = '450;4 A;60;4 A;60;4 A;60;4 A;60;place 16 72 3;120;4 LEFT;30;4 A;300'


class Remote:
    """One game of a scene played by remote play (--remote): its steps sent
    as the scene's course reacts to its log (the Arena's and the teams'
    lines), and while none wait, a second at a time with A pressed every
    third where `page` (a chat box's words or a card). Every frame from
    `first` on is saved, and a thread keeps every second one as
    WORK/SCENE-NAME/NNNN.png (the canvas's middle 240 x 160) as they come,
    so a scene of minutes needs no more room than its pictures."""

    def __init__(self, work, scene, name, port, roms, dev, args=HOME, files=None, first=1000, bn5=True):
        self.name, self.first = name, first
        self.dir = os.path.join(work, f'data-{scene}-{name}')
        shutil.rmtree(self.dir, ignore_errors=True)
        data = os.path.join(self.dir, 'data')
        self.raw = os.path.join(self.dir, 'raw')
        os.makedirs(data)
        os.makedirs(self.raw)
        self.out = os.path.join(work, f'{scene}-{name}')
        shutil.rmtree(self.out, ignore_errors=True)
        os.makedirs(self.out)
        operator = name.capitalize()
        with open(os.path.join(data, 'network.ini'), 'w') as f:
            f.write(f'identity={hashlib.sha1(name.encode()).hexdigest()[:32]}\nname={operator}\nnamed=1\nselected=0\n'
                    f'consent=1\nserver=ws://127.0.0.1:{port}/presence\n')
        with open(os.path.join(data, 'netpass.ini'), 'w') as f:
            f.write('stamps=4\n')
        for fname, text in (files or {}).items():
            with open(os.path.join(data, fname), 'w') as f:
                f.write(text)
        self.log = os.path.join(self.dir, 'log.txt')
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy', CYBERWORLD_EMU_DEBUG='1')
        args = [BINARY, '--headless', '--remote', self.dir, '--rom-dir', roms[0] if bn5 else roms[1], '--data-dir', data,
                *args, '--dev', ','.join(['quiet'] + list(dev)), '--shot-range', f'{first}:99999999:{self.raw}/f']
        self.proc = subprocess.Popen(args, stdout=open(self.log, 'w'), stderr=subprocess.STDOUT, env=env, cwd=ROOT)
        for _ in range(500):
            if os.path.exists(os.path.join(self.dir, 'out')):
                break
            time.sleep(0.01)
        self.todo = []
        self.lock = threading.Lock()
        self.frame, self.page, self.n, self.alive, self.busy = 0, False, 0, True, False
        self.answered = time.time()
        self.threads = [threading.Thread(target=self.loop, daemon=True), threading.Thread(target=self.keep, daemon=True)]
        for t in self.threads:
            t.start()

    def batch(self, line):
        with open(os.path.join(self.dir, 'in'), 'w') as f:
            f.write(line + '\n')
        fd = os.open(os.path.join(self.dir, 'out'), os.O_RDONLY)
        got = b''
        while not got.endswith(b'\n'):
            c = os.read(fd, 1)
            if not c:
                break
            got += c
        os.close(fd)
        try:
            self.frame = int(got.split()[1])
        except (IndexError, ValueError):
            pass

    def loop(self):
        while self.alive and self.proc.poll() is None:
            with self.lock:
                line = self.todo.pop(0) if self.todo else None
                self.busy = line is not None
            if line is None:
                self.n += 1
                line = '4 A;56' if self.page and self.n % 3 == 0 else '60'
            self.batch(line)
            self.answered = time.time()
            with self.lock:
                self.busy = False

    def do(self, line):
        with self.lock:
            self.todo.append(line)

    def idle(self):
        """whether every step sent has run"""
        with self.lock:
            return not self.todo and not self.busy

    def keep(self):
        """every second frame kept as its picture, as the game saves them"""
        while True:
            ended = self.proc.poll() is not None
            names = sorted(f for f in os.listdir(self.raw) if f.endswith('.bmp'))
            for f in names if ended else names[:-1]:
                n = int(f[1:-4])
                p = os.path.join(self.raw, f)
                if (n - self.first) % 2 == 0:
                    try:
                        im = Image.open(p).convert('RGB')
                    except OSError:
                        continue
                    w, h = im.size
                    im.crop(((w - 240) // 2, (h - 160) // 2, (w + 240) // 2, (h + 160) // 2)).save(
                        os.path.join(self.out, f'{(n - self.first) // 2:04d}.png'), compress_level=1)
                os.remove(p)
            if ended:
                return
            time.sleep(0.2)

    def text(self):
        try:
            with open(self.log, errors='replace') as f:
                return f.read()
        except FileNotFoundError:
            return ''

    def has(self, words, after=None):
        t = self.text()
        if after is not None:
            t = t.split(after, 1)[1] if after in t else ''
        return words in t

    def count(self, words):
        return self.text().count(words)

    def sample(self):
        """the footage's sample now (30 fps, from `first`): the last batch's
        end, and the clock since (a visit plays at the clock's pace)"""
        return max(0, round((self.frame + (time.time() - self.answered) * 60 - self.first) / 2))

    def quit(self):
        if self.proc.poll() is None:
            self.do('quit')
            try:
                self.proc.wait(20)
            except subprocess.TimeoutExpired:
                self.proc.kill()
        self.alive = False
        self.threads[1].join()


def wait(test, what, most=180, t0=None):
    """until `test()`, at most `most` seconds: whether it came"""
    start = time.time()
    while not test():
        if time.time() - start > most:
            print('  timed out:', what, flush=True)
            return False
        time.sleep(0.1)
    print(f'  {time.time() - (t0 or start):6.1f}s {what}', flush=True)
    return True


def reactive(work, scene, run):
    """A scene that reacts to its games' logs: its server, then its own
    course (`run(work, scene, port, roms)`, its games Remote's)."""
    server = Server(os.path.join(work, f'data-server-{scene}'), 120)
    try:
        run(work, scene, server.port, rom_dirs(work))
    finally:
        server.stop()
    open(os.path.join(work, f'{scene}.done'), 'w').close()
    print('captured', scene, flush=True)


def capture(work, scenes, keep=False, only=None):
    """Every online scene of a cut: those by the clock one at a time (each
    runs its games at the clock's pace; two scenes at once would share the
    machine's cores with up to sixteen games), then those that react to
    their games' logs (a function each) all at once, which wait for what
    each game says rather than the clock."""
    if not os.path.exists(BINARY) or not os.path.exists(SERVER):
        sys.exit('the online footage needs build/linux/cyberworld and build/server: '
                 'python3 build.py linux && python3 build.py server')
    rom_dirs(work)
    threads = []
    for scene, spec in scenes.items():
        if only and scene not in only:
            continue
        if callable(spec):
            if not (keep and os.path.exists(os.path.join(work, f'{scene}.done'))):
                threads.append(threading.Thread(target=reactive, args=(work, scene, spec)))
            continue
        if keep and kept(work, scene, spec):
            continue
        capture_scene(work, scene, spec)
    for t in threads:
        t.start()
    for t in threads:
        t.join()


if __name__ == '__main__':
    import trailers
    cut = trailers.module(sys.argv[1] if len(sys.argv) > 1 else None)
    work = os.path.join(ROOT, '.build', 'trailer-' + cut.VERSION)
    os.makedirs(work, exist_ok=True)
    capture(work, cut.online_scenes(), only=set(sys.argv[2:]) or None)
