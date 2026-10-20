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


def capture(work, scenes, keep=False, only=None):
    """Every online scene of a cut, one at a time (each runs its games at
    the clock's pace; two scenes at once would share the machine's cores
    with up to sixteen games)."""
    if not os.path.exists(BINARY) or not os.path.exists(SERVER):
        sys.exit('the online footage needs build/linux/cyberworld and build/server: '
                 'python3 build.py linux && python3 build.py server')
    for scene, spec in scenes.items():
        if only and scene not in only:
            continue
        if keep and kept(work, scene, spec):
            continue
        capture_scene(work, scene, spec)


if __name__ == '__main__':
    import trailers
    cut = trailers.module(sys.argv[1] if len(sys.argv) > 1 else None)
    work = os.path.join(ROOT, '.build', 'trailer-' + cut.VERSION)
    os.makedirs(work, exist_ok=True)
    capture(work, cut.online_scenes(), only=set(sys.argv[2:]) or None)
