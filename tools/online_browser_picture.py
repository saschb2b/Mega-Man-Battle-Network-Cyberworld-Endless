#!/usr/bin/env python3
"""The release picture of the browser online (0.13.0's notes,
docs/screenshots/online-browser.png): the site's player in headless Chrome,
its game in a Net Square beside a game on this machine, the browser's
MegaMan saying Hello! beside Alpha's.

The site is build/site as `build.py serve` makes it, without the visit
counter, served on 127.0.0.1; Chrome is agent-browser's
(.claude/skills/agent-browser, `npm i -g agent-browser`), a session of its
own. The Square is a server of its own on 127.0.0.1 (the example's
configuration, the page's origin allowed), never the official one. Alpha
is the Linux build headless (build/linux), as docs/devtools/net-square.md's two games
at once are; the page's game is the play page's own start, its arguments
and a few more added (the scene, the gate's server and name, the scripted
input), both ROMs chosen in its own file input. Each goes through the gate
in Lan's HP, past the greeter's stamps, and says a phrase. The picture is
the page's player as the viewport shows it, one screenshot a second from
34 to 52 seconds after the ROMs are chosen, the one with the phrase's
bubble kept (its black, which the Square's floor and the names' labels
lack): the games keep the wall clock's pace, so a retake falls a few
frames apart.

    python3 build.py screenshots online-browser   (the Linux build, the server and the browser build made first)
    python3 tools/online_browser_picture.py
"""
import functools
import glob
import hashlib
import http.server
import json
import os
import shutil
import socket
import subprocess
import sys
import threading
import time

from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)
import build  # noqa: E402  (the site, the ROM folder and the ROMs' SHA-1)

WORK = os.path.join(ROOT, '.build', 'online-browser-picture')
OUT = os.path.join(ROOT, 'docs', 'screenshots', 'online-browser.png')
SESSION = 'cw-online-browser'
# (home after act 2 at seed 3, the arrival's words paged, onto the gate
# and Yes, as tools/online_browser.html goes; then the greeter's stamps
# paged, and a phrase from SELECT's strip (its phrases' page, issue #178):
# the two side by side by the arrival, facing each other, the page's
# MegaMan's Hello!, and Alpha's Nice build! a moment later. Their places
# are the plaza's panels (2.25, 1) and (3.25, 0) from its landmark in the
# example server's Square, seed 120: tools/online_pictures.py's floor())
GATE = '450:,4:A,60:,4:A,60:,4:A,60:,4:A,60:,1:place 16 72 3,120:,4:LEFT,30:,4:A,300:,200:,4:A,60:,4:A,60:,4:A,90:,4:A,90:,4:A,60:,'
WORDS = '4:B,50:,' * 4   # (the greeter's last pages, with B: an A with no box open would read a card)
PAGE_INPUT = GATE + WORDS + '1:place -12 116 2,64:,6:SELECT,20:,6:R,10:,6:A,400:'
ALPHA_INPUT = GATE + WORDS + '1:place 20 148 6,173:,6:SELECT,20:,6:R,10:,6:RIGHT,6:,6:A,400:,6000:'
SCENE = ['--scene', 'home', '--run-depth', '7', '--seed', '3']
SECONDS = range(34, 53)


def free_port():
    with socket.socket() as s:
        s.bind(('127.0.0.1', 0))
        return s.getsockname()[1]


def roms():
    """BN6's and BN5's ROMs in the developer's ROM folder, told by their SHA-1."""
    found = {}
    for path in sorted(glob.glob(os.path.join(build.default_rom_dir(), '*.gba'))):
        with open(path, 'rb') as f:
            digest = hashlib.sha1(f.read()).hexdigest()
        for game, sha1 in (('bn6', build.BN6_SHA1), ('bn5', build.BN5_SHA1)):
            if digest == sha1:
                found.setdefault(game, path)
    if len(found) < 2:
        sys.exit('the browser picture needs both ROMs in ' + build.default_rom_dir())
    return found


def browser(*args):
    p = subprocess.run(['agent-browser', '--session', SESSION, *args], capture_output=True, text=True, timeout=180)
    if p.returncode:
        sys.exit(f'agent-browser {args[0]}: {(p.stdout + p.stderr).strip()}')
    return p.stdout.strip()


def page_arguments(url):
    """The play page's start, the scene and the gate's server and name added."""
    more = SCENE + ['--dev', f'quiet,statistics=off,online={url},onlinename=Browser', '--input', PAGE_INPUT]
    return ('(() => { const start = Module.callMain.bind(Module); '
            f'Module.callMain = (args) => start(args.concat({json.dumps(more)})); return true; }})()')


def bubble(im):
    """How much of a phrase's bubble shows: its black pixels."""
    return im.convert('L').histogram()[0]


def main():
    if not shutil.which('agent-browser'):
        sys.exit('the browser picture needs agent-browser (npm i -g agent-browser)')
    for need in ('build/linux/cyberworld', 'build/server/cyberworld-server.pyz', 'build/web/cyberworld.js'):
        if not os.path.exists(os.path.join(ROOT, need)):
            sys.exit(f'the browser picture needs {need}: python3 build.py linux, server and web')
    rom = roms()
    shutil.rmtree(WORK, ignore_errors=True)
    os.makedirs(WORK)
    site = build.site(analytics=False)
    site_port, square_port = free_port(), free_port()
    url = f'ws://127.0.0.1:{square_port}/presence'
    with open(os.path.join(ROOT, 'server', 'server.example.ini')) as f:
        ini = f.read().replace('\norigins =', f'\norigins = http://127.0.0.1:{site_port}')
    with open(os.path.join(WORK, 'server.ini'), 'w') as f:
        f.write(ini)
    server = subprocess.Popen([sys.executable, os.path.join(ROOT, 'build', 'server', 'cyberworld-server.pyz'),
                               '--config', os.path.join(WORK, 'server.ini'), '--host', '127.0.0.1', '--port', str(square_port),
                               '--state-dir', os.path.join(WORK, 'state')],
                              stdout=open(os.path.join(WORK, 'server.log'), 'w'), stderr=subprocess.STDOUT)
    quiet = type('Quiet', (http.server.SimpleHTTPRequestHandler,), {'log_message': lambda *a: None})
    httpd = http.server.ThreadingHTTPServer(('127.0.0.1', site_port), functools.partial(quiet, directory=site))
    threading.Thread(target=httpd.serve_forever, daemon=True).start()
    alpha = None
    try:
        time.sleep(2)
        alpha = subprocess.Popen([os.path.join(ROOT, 'build', 'linux', 'cyberworld'), '--headless', '--rom-dir',
                                  build.default_rom_dir(), '--data-dir', os.path.join(WORK, 'alpha'), *SCENE, '--dev',
                                  f'quiet,statistics=off,online={url},onlinename=Alpha', '--input', ALPHA_INPUT],
                                 stdout=open(os.path.join(WORK, 'alpha.log'), 'w'), stderr=subprocess.STDOUT, cwd=ROOT,
                                 env=dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy'))
        browser('set', 'viewport', '1280', '800')
        browser('open', f'http://127.0.0.1:{site_port}/play/')
        browser('wait', '4000')
        browser('eval', page_arguments(url))
        browser('upload', '#rom', rom['bn6'], rom['bn5'])
        start, shots = time.time(), []
        for t in SECONDS:
            time.sleep(max(0.0, start + t - time.time()))
            shots.append(os.path.join(WORK, f'page-{t}.png'))
            browser('screenshot', shots[-1])
        box = json.loads(browser('eval', "JSON.stringify(document.getElementById('app').getBoundingClientRect())"))
    finally:
        subprocess.run(['agent-browser', '--session', SESSION, 'close'], capture_output=True, timeout=60)
        if alpha:
            alpha.terminate()
        server.terminate()
        httpd.shutdown()
    if isinstance(box, str):
        box = json.loads(box)
    pad = 12
    crop = (max(0, int(box['left']) - pad), max(0, int(box['top']) - pad), int(box['right']) + pad, int(box['bottom']) + pad)
    pictures = [Image.open(shot).convert('RGB').crop(crop) for shot in shots]
    best = max(range(len(pictures)), key=lambda i: (bubble(pictures[i]), -abs(SECONDS[i] - 42)))
    pictures[best].save(OUT, optimize=True)
    print(f'screenshot online-browser at {SECONDS[best]} s', pictures[best].size, 'black', bubble(pictures[best]))
    return 0


if __name__ == '__main__':
    sys.exit(main())
