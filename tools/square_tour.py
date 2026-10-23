#!/usr/bin/env python3
"""The Net Square's tour (docs/ONLINE.md, The layout): the Square at a few
seeds, one of each area it wears, on a local server with three games in
it. Mira waits in the Arena and Dex at Trade Central, so the warps' labels
and signs name them; Kei arrives and walks, as a player walks him (the
pad, never a place step on the way), from the arrival to each warp's gem,
through to its part and back, each walk timed in frames. Pictures of the
welcome plaza, each warp's look from it, each part as MegaMan lands, the
way back, a sign and L's Look around go to .build/square-tour/SEED/, and
the walk to the Arena and back as every second frame (a GIF's). The Linux
build and the ROM-free server, never the official one, as
tools/trailer_online.py's scenes.

    python3 build.py linux && python3 build.py server
    python3 tools/square_tour.py [SEED ...]     (120 121 122: Central, Sky, Green)
"""
import os
import re
import shutil
import subprocess
import sys
import threading
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import trailer_online as online  # noqa: E402

WORK = os.path.join(ROOT, '.build', 'square-tour')

# (the plan's places from the plaza's middle, square_plan.c: grid x, y)
ARRIVE = (3, 0)
# each warp: its part, its gem in the plaza, its gem in its part, and the
# waypoints there from the arrival round the landmark (BN6 steps MegaMan
# off a gem as he lands: back onto it is the way back)
WARPS = [
    ('trade', (-6, 0), (-11, 0), [(3, -3), (-3, -3), (-3, 0)]),
    ('arena', (0, -6), (-2, -11), [(3, -3), (0, -3)]),
    ('stage', (0, 6), (-2, 16), [(3, 3), (0, 3)]),
]
# the pad's eight ways as BN6 walks them, in world units
WAYS = {'UP': (1, -1), 'UP+RIGHT': (1, 0), 'RIGHT': (1, 1), 'DOWN+RIGHT': (0, 1), 'DOWN': (-1, 1),
        'DOWN+LEFT': (-1, 0), 'LEFT': (-1, -1), 'UP+LEFT': (0, -1)}


class Game:
    """A headless game in remote play, its steps sent and answered one
    batch at a time (tools/play.py's pipes)."""

    def __init__(self, dir_, name, url, roms):
        shutil.rmtree(dir_, ignore_errors=True)
        self.dir = dir_
        data = os.path.join(dir_, 'data')
        os.makedirs(data)
        with open(os.path.join(data, 'netpass.ini'), 'w') as f:
            f.write('stamps=4\n')
        self.log = os.path.join(dir_, 'log.txt')
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy', CYBERWORLD_EMU_DEBUG='1')
        args = [online.BINARY, '--headless', '--remote', dir_, '--rom-dir', roms[0], '--data-dir', data, *online.HOME,
                '--dev', ','.join(['quiet', f'online={url}', f'onlinename={name}'])]
        self.proc = subprocess.Popen(args, stdout=open(self.log, 'w'), stderr=subprocess.STDOUT, env=env, cwd=ROOT)
        for _ in range(500):
            if os.path.exists(os.path.join(dir_, 'out')):
                break
            time.sleep(0.01)
        self.frame = 0

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
        m = re.search(rb'(\d+)', got)
        if m:
            self.frame = int(m[1])

    def text(self):
        with open(self.log, errors='replace') as f:
            return f.read()

    def pos(self):
        path = os.path.join(self.dir, 'state.txt')
        self.batch(f'state {path}')
        with open(path) as f:
            m = re.search(r'^pos (-?\d+) (-?\d+)', f.read(), re.M)
        return (int(m[1]), int(m[2])) if m else (0, 0)

    def quit(self):
        if self.proc.poll() is None:
            try:
                self.batch('quit')
                self.proc.wait(20)
            except (OSError, subprocess.TimeoutExpired):
                self.proc.kill()


def wait_log(g, words, most=120):
    t0 = time.time()
    while words not in g.text():
        if time.time() - t0 > most or g.proc.poll() is not None:
            sys.exit(f'{g.dir}: no "{words}" (log.txt)')
        g.batch('30')


class Square:
    """The plan's grid in the game's world units, from the landmark's world
    place in the log (netmap_world: grid y is world -X, grid x world +Y,
    32 units a panel)"""

    def __init__(self, text):
        m = re.search(r'online: square area (\d+), arrival at (-?\d+),(-?\d+), the landmark at (-?\d+),(-?\d+)', text)
        self.area, self.cx, self.cy = int(m[1]), int(m[4]), int(m[5])

    def world(self, p):
        return self.cx - 32 * p[1], self.cy + 32 * p[0]


def steer(g, to, shots=None, until=None, most=900):
    """Kei run by the pad towards world `to` (B held, two frames a step, as
    a player holds it), until within a few units or `until(pos)`: the
    frames it took; every second frame into `shots` (a folder) where given"""
    start = g.frame
    while g.frame - start < most:
        x, y = g.pos()
        if until and until((x, y)):
            return g.frame - start
        dx, dy = to[0] - x, to[1] - y
        if not until and abs(dx) <= 8 and abs(dy) <= 8:
            return g.frame - start
        way = max(WAYS, key=lambda k: (WAYS[k][0] * dx + WAYS[k][1] * dy) / (WAYS[k][0] ** 2 + WAYS[k][1] ** 2) ** 0.5)
        if shots:
            g.batch(f'2 {way}+B;shot {os.path.join(shots, "f%05d.bmp" % g.frame)}')
        else:
            g.batch(f'2 {way}+B')
    return None


def near(sq, p, reach=20):
    w = sq.world(p)
    return lambda pos: abs(pos[0] - w[0]) <= reach and abs(pos[1] - w[1]) <= reach


def tour(seed, roms):
    out = os.path.join(WORK, str(seed))
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    server = online.Server(os.path.join(WORK, f'server-{seed}'), seed)
    games = []
    try:
        for name in ('Mira', 'Dex', 'Kei'):
            games.append(Game(os.path.join(WORK, f'{seed}-{name.lower()}'), name, server.url, roms))
        mira, dex, kei = games
        for g in games:
            # (the arrival's words in Lan's HP paged, then onto the gate: --dev online goes at once)
            g.batch('600;' + '6 A;94;' * 6 + 'place 16 72 3;60')
        for g in games:
            wait_log(g, 'online: square area')
        sq = Square(kei.text())
        # (Mira in the Arena's stands, Dex at Trade Central's desk; then they wait)
        mira.batch('place %d %d 1;60' % sq.world((1, -11)))
        dex.batch('place %d %d 7;60' % sq.world((-13, 0)))
        idle = threading.Event()

        def idling(g):
            while not idle.is_set() and g.proc.poll() is None:
                g.batch('30')
        threads = [threading.Thread(target=idling, args=(g,), daemon=True) for g in (mira, dex)]
        for t in threads:
            t.start()
        kei.batch('240;shot ' + os.path.join(out, 'plaza.bmp'))
        times = {}
        for part, hub, gem, there in WARPS:
            kei.batch('place %d %d 7;30' % sq.world(ARRIVE))
            frames = 0
            shots = None
            if part == 'arena':
                shots = os.path.join(out, 'walk')
                os.makedirs(shots, exist_ok=True)
            for w in there:
                frames += steer(kei, sq.world(w), shots)
            # (beside its pad: the warp's look from the plaza, its label, its part across the void)
            kei.batch('shot ' + os.path.join(out, f'{part}-warp.bmp'))
            took = steer(kei, sq.world(hub), shots, until=near(sq, gem, 40))
            if took is None:
                sys.exit(f'seed {seed}: no warp to the {part} ({kei.log})')
            frames += took
            # (the warp's beam and the camera's ride, BN6's step off the gem: until MegaMan is free in the part)
            t0 = kei.frame
            kei.batch('140')
            frames += kei.frame - t0
            kei.batch('shot ' + os.path.join(out, f'{part}.bmp'))
            if shots:
                for _ in range(30):
                    kei.batch(f'2;shot {os.path.join(shots, "f%05d.bmp" % kei.frame)}')
            way = 0
            took = steer(kei, sq.world(gem), shots, until=near(sq, hub, 40))
            if took is None:
                sys.exit(f'seed {seed}: no warp back from the {part} ({kei.log})')
            t0 = kei.frame
            kei.batch('140')
            way += took + kei.frame - t0
            kei.batch('shot ' + os.path.join(out, f'{part}-back.bmp'))
            if shots:
                for _ in range(30):
                    kei.batch(f'2;shot {os.path.join(shots, "f%05d.bmp" % kei.frame)}')
            times[part] = (frames, way)
            print(f'seed {seed} (area {sq.area}): the {part}, {frames} frames there from the arrival, {way} back', flush=True)
        # (the Arena's sign, read; then L's Look around)
        kei.batch('place %d %d 1;20;6 A;40;shot %s' % (*sq.world((-2, -4)), os.path.join(out, 'sign.bmp')))
        kei.batch('6 A;30;6 A;40;shot ' + os.path.join(out, 'sign-who.bmp'))
        for _ in range(4):
            kei.batch('6 B;20')
        kei.batch('6 L;40;6 A;40;shot ' + os.path.join(out, 'look.bmp'))
        for k in range(4):
            kei.batch(f'6 A;50;shot {os.path.join(out, f"look-{k}.bmp")}')
        idle.set()
        for t in threads:
            t.join(10)
        return times
    finally:
        for g in games:
            g.quit()
        server.stop()


def main(argv):
    if not os.path.exists(online.BINARY) or not os.path.exists(online.SERVER):
        sys.exit('the tour needs build/linux/cyberworld and build/server: python3 build.py linux && python3 build.py server')
    os.makedirs(WORK, exist_ok=True)
    roms = online.rom_dirs(WORK)
    for seed in [int(a) for a in argv] or [120, 121, 122]:
        tour(seed, roms)


if __name__ == '__main__':
    main(sys.argv[1:])
