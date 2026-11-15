"""0.22.0's trailer footage: each scene a scripted headless run (tools/trailer.py's capture()).

Every scene here was run as capture() runs it (the whole ROM folder mounted, a fresh data folder) and its
frames looked at, 15 November 2026. A sample is every second game frame from `first`:
sample = (frame - first) // 2.

SAVES with a run on the device: capture() starts each scene on an empty data folder, so the saves scenes
play a run's arrival first (layer 5 of seed 7, its checkpoint written), soft-reset to the title
(A+B+SELECT+START) and open SAVES with L, all in one run. `--size 240x160` there is the screen SAVES lays
itself out for (as build.py's `saves` pictures); at the default size capture()'s crop cuts its band off.

options-update reads a made-up update.json, served to it over HTTP before capture_all (UPDATE_ENV names it):

    python3 tools/update_manifest.py v0.22.0 FOLDER > FOLDER/update.json     # the notes of docs/releases/v0.22.0.md
    (cd FOLDER && python3 -m http.server 18723 --bind 0.0.0.0)                # UPDATE_PORT

(no file of the release in FOLDER, so the copy "updates from the download page": A: Download. A headless run
names the keyboard's keys on the box's buttons, J and K. Once the notes hold pictures the manifest names
update-pictures.bin at the release's address on GitHub; point that entry's url at this server to show them.)

NEEDS_SERVER's scenes talk to a Net Square server, which the cut starts before capture_all and stops
after. A container reaches this machine at Docker's bridge address, so the server listens on 0.0.0.0:

    python3 server/main.py --config INI      # INI as SERVER_INI.format(port=SERVER_PORT, state=FOLDER)

(from source; build/server's .pyz does the same). Their marks move from run to run, as the server answers
on the clock and a headless game runs as fast as it can: of two runs here the cloud's "Synced" came at
sample 220 and at 300, the code at 185 and 200; the marks name where both had arrived. Each run's code is
another, and "Synced" carries the run's own date.

The second round (what merged after 0.21.2): the shared dive, the layers' heights, the Nest's crystals, Robot
Control Comp's figures and wires, L's detours, a Glitch layer's flips. Those of one game are in SCENES like
the rest. The dive's scenes with several games are ONLINE_SCENES, taken by online_footage(work, keep): two
takes (`dive`: the portal, the divers, the crew, the join-in battle; `dive-pack`: four divers on a layer;
`dive-door`: the door's queue, the guardian, the party's fight), each a Net Square server of its own from source and two to four host builds
in the build image at once, a game's frames kept in several ranges, each range a folder named as a scene
(dive-portal, dive-divers, dive-crew-a/-b, dive-joinin-a/-b, dive-pack, dive-door, dive-door-b, dive-guardian,
dive-party-a/-b: -a Alpha's game, -b Beta's) with NNNN.png as capture() leaves them. It needs the host
build alone (no build.py linux, no build.py server), Docker's bridge at 172.17.0.1 and about four minutes
(the games run at the clock's pace: 68, 61 and 105 seconds). The comment over ONLINE_SCENES says how.
"""

import os

AUTO = {'CYBERWORLD_AUTOPILOT': '1'}
WEAK = {'CYBERWORLD_AUTOPILOT': 'weak'}


def frames(script):
    """the frames an --input script takes (its steps are durations, held in sequence)"""
    return sum(int(step.split(':')[0]) for step in script.split(',') if step)


# a layer's arrival paged (MegaMan free at 470), and the same for a run to save
ARRIVE = '150:,' + '6:A,4:,' * 30 + '20:,'
PAGED = '100:,' + '4:A,6:,' * 30 + '100:,'
# the soft reset to the title, then L: SAVES over a title with a run saved (772 frames)
TO_SAVES = PAGED + '6:A+B+SELECT+START,200:,' + '6:L,60:,'
SAVED = ['--scene', 'emu', '--run-depth', '5', '--seed', '7', '--dev', 'quiet']

# the Net Square server the saves' cloud and code scenes reach (see the docstring)
SERVER_PORT = 18722
SERVER_URL = f'ws://172.17.0.1:{SERVER_PORT}/presence'      # Docker's bridge: this machine from a container
SERVER_INI = ('[server]\nname = Cyberworld Endless\nmotd = Welcome\nhost = 0.0.0.0\nport = {port}\nseed = 120\n'
              'capacity = 8\nplayer_cap = 64\ntick_hz = 20\nstate_dir = {state}\nwhitelist =\nfree_text = false\n'
              'trading = true\narena = true\norigins =\n')
NEEDS_SERVER = ('saves-cloud', 'saves-code')
ONLINE = ['--scene', 'emu', '--run-depth', '5', '--seed', '7', '--dev', f'quiet,online={SERVER_URL}', '--size', '240x160']

PAGES = '70:,6:R,74:,6:DOWN,40:,6:R,74:,6:DOWN,30:,6:DOWN,30:,6:DOWN,40:,6:L,50:,6:L,60:'
CLOUD = TO_SAVES + '60:,6:A,90:,6:LEFT,40:,6:A,420:'       # Turn on, its question, Yes, the sync
CODE = TO_SAVES + '60:,6:R,60:,6:A,420:'                    # CODE, Send, the code
# seed 4's second layer (RoboDog Comp): a step back from the Security Cube beside the arrival, the walk to it, its four boxes
LOCK = ARRIVE + '24:UP+RIGHT,20:,60:DOWN+LEFT,10:,' + '6:A,116:,' * 4 + '6:A,60:'
# Options from the title (SELECT), Up to Display's last line, Depth lighting: On, Off, On
OPTIONS_3D = '60:,4:SELECT,100:,4:UP,6:,90:,4:RIGHT,6:,170:,4:LEFT,6:,130:,4:RIGHT,6:,170:'
# Lan's room, the PET, KeyItem's Jukebox (tools/cw_routes.py's route)
JUKEBOX = '40:,' + '6:A,4:,' * 46 + '30:,6:START,6:,40:,' + '6:DOWN,6:,' * 5 + '6:A,6:,60:,6:DOWN,6:,6:RIGHT,6:,14:,6:A,6:,40:,'
# L from BN6's Towns is BN4's Unused (three songs); six more, back through BN4's sections, BN5's (seven)
UNUSED = JUKEBOX + '40:,6:L,60:,6:DOWN,40:,6:DOWN,60:,' + '6:L,10:,' * 6 + '60:,' + '6:DOWN,40:,' * 5 + '40:'
# the Game page with the touch controls shown (--touch: the line is listed only where there is touch), Auto to Off
OPTIONS_TOUCH = '60:,4:SELECT,100:,4:L,6:,60:,4:DOWN,6:,110:,4:RIGHT,6:,170:'
# the update box as the title's menu first shows (START), four pages on, Later; SELECT, Options' Game on its
# Update line; A, the box again, a page, Later
UPDATE = '240:,6:START,180:,' + '6:RIGHT,80:,' * 4 + '6:B,60:,6:SELECT,160:,6:A,120:,6:RIGHT,90:,6:B,60:'
UPDATE_PORT = 18723
UPDATE_ENV = {'CYBERWORLD_UPDATE_URL': f'http://172.17.0.1:{UPDATE_PORT}/update.json', 'CYBERWORLD_UPDATE_FROM': '0.21.2'}
# Beast Out (--dev powers) before BN5's viruses: the arrival paged, the foes step, on the Custom screen five
# Right and Down to the star, A, START to OK, A; then the buster held and a chip now and then
BEAST_RUN = ['--scene', 'emu', '--run-depth', '4', '--seed', '4', '--net-biome', 'x1', '--dev', 'quiet,god,powers']
BEAST = ('150:,' + '6:A,4:,' * 60 + '30:,0:foes 254@52/20C@41/260@63,260:,' + '6:RIGHT,6:,' * 5
         + '6:DOWN,6:,10:,6:A,6:,90:,6:START,6:,20:,6:A,6:,240:,'
         + '60:B,6:,4:A,30:,6:UP,10:,60:B,6:,4:A,30:,6:DOWN,10:,' * 4)
MAP_KEY = ARRIVE + '30:,240:SELECT,40:'
TITLE_DEPTH = PAGED + '6:A+B+SELECT+START,200:,' + '60:,6:START,150:'

# the second round (merged since 0.21.2): the party's field, the net's new looks
# the party's fight on one game: --dev partycore=N stages layer 3's guardian (seed 3: BlastMan) on the party's
# core with N MegaMen, partybot=1 the Arena's bot as every one's pad; the autopilot walks to his arena (the
# field from 1700). With four the field stands still from frame 1936 (MISSING), with three it fights on
PARTY_CORE = ['--scene', 'emu', '--run-depth', '3', '--seed', '3', '--dev']
# Green's layer 4 of seed 2: from the arrival up the yellow stair to the terrace (z 0 to 32) and back down
CLIMB_A = ARRIVE + '100:UP+RIGHT,30:,100:DOWN+LEFT,30:'
# CopyBot Comp's layer 2 of seed 2: put on the walkway under its railed ladder (the stair's corner 252,-236 in
# CYBERWORLD_EMU_DEBUG's "stair pasted" line), up it onto the raised room (z 0 to 32)
CLIMB_B = ARRIVE + '0:place 268 -124 7,20:,150:UP+LEFT,40:'
# Robot Control Comp's first layer of seed 9: the arrival paged, a walk to the pad's upper left edge, the robot
# figure in the void behind it
ROBOT = '110:,' + '6:A,24:,' * 8 + '50:UP+LEFT,150:'
# seed 4's second layer: L, its three boxes (ProtoMan, a dark flame, the way)
L_WAY = ARRIVE + '20:,6:L,144:,' + '6:A,144:,' * 3 + '60:'


def fight(biome, foes, first, last, dev='quiet,god', depth=None):
    """the foes step's battle on seed 4's layer, the autopilot fighting in god mode"""
    run = ['--run-depth', str(depth)] if depth else []
    return (['--scene', 'emu', '--net-biome', biome, *run, '--seed', '4', '--dev', dev], AUTO,
            ('460:,' if depth else '100:,') + f'0:foes {foes}', first, last)


def walk(biome, seed, first, last):
    """an area's first layer walked by the weak autopilot (build.py's AREA_WALK)"""
    return (['--scene', 'emu', '--run-depth', '1', '--dev', 'quiet', '--seed', str(seed), '--net-biome', biome], WEAK, None, first, last)


SCENES = {
    # name: (argv, env, script, first, last)   # as tools/trailer.py's capture() takes them
    'saves-pages': (SAVED + ['--size', '240x160'], {}, TO_SAVES + PAGES, 740, frames(TO_SAVES + PAGES)),
    'saves-cloud': (ONLINE, {}, CLOUD, frames(TO_SAVES), frames(CLOUD)),
    'saves-code': (ONLINE, {}, CODE, frames(TO_SAVES), frames(CODE)),
    'lock': (['--scene', 'emu', '--run-depth', '2', '--seed', '4', '--dev', 'quiet'], {}, LOCK, 470, frames(LOCK)),
    'walkway': walk('8', 1, 600, 1000),                       # RoboDog Comp: its room, then the green two-wide walkways
    'bn4-acdc': walk('x15', 4, 300, 900),
    'bn5-oran': walk('x4', 3, 300, 900),
    'options-3d': (['--scene', 'title'], {}, OPTIONS_3D, 150, frames(OPTIONS_3D)),
    'options-touch': (['--scene', 'title', '--touch'], {}, OPTIONS_TOUCH, 200, frames(OPTIONS_TOUCH)),
    'options-update': (['--scene', 'title'], UPDATE_ENV, UPDATE, 296, frames(UPDATE)),
    'jukebox-unused': (['--scene', 'town', '--seed', '2'], {}, UNUSED, frames(JUKEBOX) + 20, frames(UNUSED)),
    'map-key': (['--scene', 'emu', '--run-depth', '2', '--seed', '2', '--dev', 'quiet,mapall'], {}, MAP_KEY, 470, frames(MAP_KEY)),
    'title-depth': (SAVED, {}, TITLE_DEPTH, 600, frames(TITLE_DEPTH)),
    # the cold open: Beast Out before BN5's Draggin, Powie and Marina on a layer of BN5's SciLab (the dark, the
    # flare, the white, the beast), and the same run's fight after its BATTLE START! (the banner is 1452-1516).
    # Spares: BN4's VolGear, Kilby and MagTect there (0.21.0's vir-bn4), its fire columns; BN4's Meteors1 on
    # three Mettaur3 in Central (build.py's bn4-meteors); BN4's BurnerMan
    'cold-a': (BEAST_RUN, {}, BEAST, 1316, 1448),
    'cold-b': (BEAST_RUN, {}, BEAST, 1620, 1830),
    'cold-c': fight('x1', '2F0@42/2F6@53/2FC@63', 880, 1050),
    'cold-d': fight('0', '03@52/03@41/03@63,', 920, 1180, dev='quiet,god,folder=674', depth=4),
    'cold-e': fight('x15', '3B8@52', 700, 1040),
    # the shared dive on one game: three MegaMen against BlastMan on the party's core, and four until it stands still
    'dive-party': (PARTY_CORE + ['quiet,partycore=3,partybot=1'], AUTO, None, 1700, 2900),
    'dive-party4': (PARTY_CORE + ['quiet,partycore=4,partybot=1'], AUTO, None, 1700, 1930),
    # the net's new looks
    'climb-a': (['--scene', 'emu', '--run-depth', '4', '--seed', '2', '--net-biome', '3', '--dev', 'quiet'], {}, CLIMB_A, 460, frames(CLIMB_A)),
    'climb-b': (['--scene', 'emu', '--run-depth', '2', '--seed', '2', '--net-biome', '15', '--dev', 'quiet'], {}, CLIMB_B, 470, frames(CLIMB_B)),
    'nest-crystals': (['--scene', 'emu', '--run-depth', '2', '--seed', '4', '--net-biome', '7', '--dev', 'quiet,god'], WEAK, None, 660, 1080),
    'robot-figures': (['--scene', 'emu', '--run-depth', '1', '--seed', '9', '--net-biome', '11', '--dev', 'quiet'], {}, ROBOT, 330, frames(ROBOT)),
    'robot-wires': (['--scene', 'emu', '--run-depth', '1', '--seed', '9', '--net-biome', '11', '--dev', 'quiet,god'], WEAK, None, 440, 800),
    'l-detours': (['--scene', 'emu', '--run-depth', '2', '--seed', '4', '--dev', 'quiet'], {}, L_WAY, 470, frames(L_WAY)),
    # a Glitch layer held at stage 3 (glitchstage=4), the run begun on one (side=4): Green's floor flipping
    'glitch-flip': (['--scene', 'emu', '--run-depth', '4', '--seed', '4', '--dev', 'quiet,god,glitch,glitchstage=4,side=4'], WEAK, None, 480, 1240),
}


# ---- the shared dive: several games on one Net Square server of their own (ONLINE_SCENES) ----
#
# A scene: {'games': [game, ...]}, a game a dict: `name` (its data folder's), `operator` (its name in the
# net), `script` (--input; the game runs to its end), `keeps` ({NAME: (first, last)}: the frames kept of
# its one run, each range as WORK/NAME/NNNN.png; a game without is only there to be met), optional `args`
# (HOME), `dev` (more --dev switches), `env`, `delay` (seconds after the scene's first game). online_footage(work, keep) takes them all, a scene at a time:
# each scene a server of its own from source (server/main.py, a free port on 0.0.0.0, its state fresh) and
# its games at once, each the host build in the build image as capture() runs a scene, reaching the server
# at Docker's bridge address. Nothing to build but the host build.
#
# A headless game keeps the clock's pace only in the Square and on a party's field (src/core/main.c), and
# two games meet on a dive's layer only where both do: every game here runs with CYBERWORLD_AUDIO_DUMP
# (to /dev/null), which holds the whole run to 60 frames a second, so a script's frame F is F / 60 seconds
# after the game's start on every game, and the scripts below are written by that clock.

HOST = 'build/host/cyberworld'
# Lan's HP after act 2 (seed 3), the arrival's words paged, MegaMan on the gate: in the Square at 1232
# (tools/trailer_online.py's HOME and GATE)
HOME = ['--scene', 'home', '--run-depth', '7', '--seed', '3']
GATE = '600:,' + '6:A,94:,' * 6 + '0:place 16 72 3,'
# the week's net is the server's: its seed from the server's key and the week's number (server/cyberworld_server/
# week.py). Both are pinned, so every take dives the same net: the key written into the state folder, and
# the server's clock (time.time alone, patched before its main runs) started in week 2966, 9 to 15 November 2026
SERVER_KEY = bytes.fromhex('506cdb77859d9c279fa901fd0e0a45b5e412de7aa5a2e4c988e1a2705b818c1b')
SERVER_TIME = 345600 + 2966 * 604800 + 302400
STAMPED = {'netpass.ini': 'stamps=4\ngames=3\n'}     # the full pass, BN4 in its set: the week's net asks for it


class Clock:
    """a game's script by its frames: at(F, steps) waits until frame F, then plays the steps"""

    def __init__(self, script=GATE):
        self.s, self.t = script, frames(script)

    def at(self, frame, steps=''):
        if frame > self.t:
            self.s += f'{frame - self.t}:,'
            self.t = frame
        if steps:
            self.s += steps.rstrip(',') + ','
            self.t += frames(steps)
        return self


def online_game(work, scene, g, url, binary=HOST, logs=False):
    """one game of an online scene, in the build image; its kept frames as capture() leaves a scene's"""
    import shutil
    import subprocess
    import sys
    sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
    import build
    from PIL import Image
    data = os.path.join(work, f'data-{scene}-{g["name"]}')
    shutil.rmtree(data, ignore_errors=True)
    os.makedirs(data)
    for fname, text in g.get('files', STAMPED).items():
        with open(os.path.join(data, fname), 'w', encoding='utf-8') as f:
            f.write(text)
    rel = os.path.relpath(work, build.ROOT)
    dev = ','.join(['quiet', f'online={url}', f'onlinename={g["operator"]}'] + list(g.get('dev', [])))
    last = frames(g['script'])
    cmd = ['docker'] + (['--context', build.CONTEXT] if build.CONTEXT else []) + [
        'run', '--name', f'cw-trailer22-{os.getpid()}-{scene}-{g["name"]}', '--rm', '-u', f'{os.getuid()}:{os.getgid()}',
        '-v', f'{build.ROOT}:/src', '-w', '/src', '-v', f'{build.default_rom_dir()}:/rom:ro',
        '-e', 'CYBERWORLD_AUDIO_DUMP=/dev/null']
    for k, v in sorted(g.get('env', {}).items()):
        cmd += ['-e', f'{k}={v}']
    if logs:
        cmd += ['-e', 'CYBERWORLD_EMU_DEBUG=1']
    cmd += [build.IMAGE, binary, '--headless', '--rom-dir', '/rom', '--data-dir', f'/src/{rel}/data-{scene}-{g["name"]}',
            *g.get('args', HOME), '--dev', dev, '--input', g['script'], '--frames', str(last + 1)]
    keeps = g.get('keeps', {})
    lo = min((a for a, _ in keeps.values()), default=0)
    hi = max((b for _, b in keeps.values()), default=0)
    raw = os.path.join(work, f'raw-{scene}-{g["name"]}')
    if keeps:
        shutil.rmtree(raw, ignore_errors=True)
        os.makedirs(raw)
        cmd += ['--shot-range', f'{lo}:{hi}:/src/{rel}/raw-{scene}-{g["name"]}/f']
    if logs:
        with open(os.path.join(work, f'log-{scene}-{g["name"]}.txt'), 'w', encoding='utf-8') as log:
            code = subprocess.call(cmd, stdout=log, stderr=subprocess.STDOUT)
    else:
        code = subprocess.call(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if code:
        print(f'{scene}: game {g["name"]} ended with {code}')
    for keep, (first, end) in keeps.items():
        out = os.path.join(work, keep)
        shutil.rmtree(out, ignore_errors=True)
        os.makedirs(out)
        for n, f in enumerate(range(first, end + 1, 2)):
            p = os.path.join(raw, f'f{f:05d}.bmp')
            if not os.path.exists(p):
                break
            im = Image.open(p).convert('RGB')
            w, h = im.size
            im.crop(((w - 240) // 2, (h - 160) // 2, (w + 240) // 2, (h + 160) // 2)).save(os.path.join(out, f'{n:04d}.png'))
    shutil.rmtree(raw, ignore_errors=True)
    shutil.rmtree(data, ignore_errors=True)


def online_scene(work, scene, spec, binary=HOST, logs=False):
    """a scene: its server (from source, its state fresh), then its games at once, until each has ended"""
    import shutil
    import socket
    import subprocess
    import sys
    import threading
    import time
    root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    state = os.path.join(work, f'data-server-{scene}')
    shutil.rmtree(state, ignore_errors=True)
    os.makedirs(state)
    with socket.socket() as s:
        s.bind(('0.0.0.0', 0))
        port = s.getsockname()[1]
    ini = os.path.join(state, 'server.ini')
    with open(ini, 'w', encoding='utf-8') as f:
        f.write(SERVER_INI.format(port=port, state=state))
    with open(os.path.join(state, 'identity.key'), 'wb') as f:
        f.write(SERVER_KEY)
    pinned = ('import runpy, sys, time\nreal, t0 = time.time, time.time()\n'
              f'time.time = lambda: {SERVER_TIME} + (real() - t0)\n'
              f'sys.argv = [{os.path.join(root, "server", "main.py")!r}, "--config", {ini!r}]\n'
              'sys.path.insert(0, sys.argv[0].rsplit("/", 1)[0])\n'
              'runpy.run_path(sys.argv[0], run_name="__main__")\n')
    with open(os.path.join(state, 'server.log'), 'w', encoding='utf-8') as log:
        server = subprocess.Popen([sys.executable, '-c', pinned], stdout=log, stderr=subprocess.STDOUT)
        try:
            time.sleep(2)
            if server.poll() is not None:
                sys.exit(f'the server did not start: {state}/server.log')
            url = f'ws://172.17.0.1:{port}/presence'
            threads, t0 = [], time.time()
            for g in spec['games']:
                wait = g.get('delay', 0) - (time.time() - t0)
                if wait > 0:
                    time.sleep(wait)
                t = threading.Thread(target=online_game, args=(work, scene, g, url, binary, logs))
                t.start()
                threads.append(t)
            for t in threads:
                t.join()
        finally:
            server.terminate()
            server.wait()
    if not logs:
        shutil.rmtree(state, ignore_errors=True)
    print('captured', scene)


def online_footage(work, keep=False):
    """every scene of ONLINE_SCENES, one at a time, each kept game's frames in work/KEEP/NNNN.png (240 x 160,
    every second game frame from its `first`, numbered from 0000: as capture() leaves a scene); with `keep`
    the scenes whose folders are there already are left"""
    for scene, spec in ONLINE_SCENES.items():
        kept = [k for g in spec['games'] for k in g.get('keeps', {})]
        if keep and all(os.path.isdir(os.path.join(work, k)) and os.listdir(os.path.join(work, k)) for k in kept):
            continue
        online_scene(work, scene, spec)


# The dive's takes, each game's frames by the clock (the Square at 1232):
#
# `dive` (Alpha and Beta): each speaks to the week's portal from a side of its own (1320; Alpha from its
# right, Beta from above: two MegaMen on one panel speak to each other instead), pages its post (1460,
# 1580), answers Yes (LEFT, A: the cursor waits on No) and A again on "THE PORTAL OPENS" (1850): layer 1 of
# the pinned week's net at 1900, its act card to 2130, the arrival's three boxes paged (free at 2450). Beta
# walks a square beside Alpha (2480 to 2640), turns to him and presses A (2680: "Lan,it's Alpha!", "Team
# up,Lan?", Yes at 2900); Alpha is asked (2920) and answers (3040, 3150): the band's "ALRIGHT! WE'RE A TEAM
# WITH BETA!". Alpha then switches random battles on again in the dev menu (3300 to 3396, between the kept
# ranges: `quiet` holds BN6's roll, which a crew's battle comes through) and takes the `battle` step,
# walking a small square until BN6's roll gives it (about 3405): "BATTLE! CALLING THE CREW" on his band,
# "ALPHA'S IN A BATTLE! A:JUMP IN" on Beta's, who has stepped aside and turned away (an A on a crewmate asks
# "Split up,Lan?") and presses A; both on one field against a Gunner from about 3560, the Arena's bot each
# one's pad (--dev partybot=1).
#
# `dive-door` (Alpha, Beta and Gamma, Gamma ten seconds behind the others so that he speaks to the portal
# after Alpha has gone, from the same side): each takes the dev menu's Next guardian on layer 1 (2480:
# layer 3, before DiveMan's arena) and pages its arrival with B (an A would ask the diver beside to team up),
# then is put into the arena (3300, 3360, and Gamma at the clock's 3920): Alpha's band says "WAITING FOR A
# PARTY", "PARTY OF 2!", "PARTY OF 3! A:GO IN B:ALONE", the others' "ALPHA LEADS 3". Alpha's A (4100) takes
# the party in: DiveMan's card and words (paged with A from 4400), each one's own Custom screen (about 4900)
# and three MegaMen on one field against DiveMan at 4500 HP (two versions up, three times his HP), the bots
# playing, from about 5020 until he has deleted all three (5620, "MEGAMAN DELETED").
#
# A take's frames fall up to ten samples apart from the last's (three takes of 15 November): the games keep
# the clock's pace and the server answers on it; the marks name samples every take had reached.
_SIDE_A, _SIDE_B = '0:place -18 172 5,', '0:place -44 146 3,'


def _enter(side):
    """through the gate, the portal's post and Yes, layer 1's card and the arrival's boxes: free at 2450"""
    return (Clock().at(1300, side + '20:,6:A,').at(1460, '6:A,').at(1580, '6:A,').at(1700, '6:LEFT,20:,6:A,').at(1850, '6:A,')
            .at(2230, '6:A,').at(2330, '6:A,').at(2430, '6:A,'))


_SQUARE = '24:UP+RIGHT,10:,24:DOWN+RIGHT,10:,24:UP+LEFT,10:,24:DOWN+LEFT,10:,24:UP+RIGHT,4:,3:DOWN+LEFT,'
_BATTLES_ON = '4:SELECT,6:SELECT+R,20:,6:DOWN,6:,6:DOWN,6:,6:A,10:,6:B,20:,'
_LOOP = '16:DOWN+LEFT+B,16:UP+LEFT+B,16:UP+RIGHT+B,16:DOWN+RIGHT+B,'
_NEXT_GUARDIAN = '4:SELECT,6:SELECT+R,20:,' + '6:DOWN,6:,' * 8 + '6:A,'
_ARRIVED = '6:B,94:,' * 5
_WORDS = '6:A,74:,' * 8
_DIVE_A = _enter(_SIDE_A).at(3040, '6:A,').at(3150, '6:A,').at(3300, _BATTLES_ON).at(3400, '0:battle,' + _LOOP * 8).at(4100).s
_DIVE_B = (_enter(_SIDE_B).at(2480, _SQUARE).at(2680, '6:A,').at(2790, '6:A,').at(2900, '6:A,').at(3370, '8:UP+RIGHT,')
           .at(3420, '6:A,34:,' * 6).at(4100).s)
_DOOR_A = (_enter(_SIDE_A).at(2480, _NEXT_GUARDIAN).at(2800, _ARRIVED).at(3300, '0:place 276 28 2,').at(4100, '6:A,')
           .at(4400, _WORDS).at(5700).s)
_DOOR_B = _enter(_SIDE_B).at(2500, _NEXT_GUARDIAN).at(2800, _ARRIVED).at(3360, '0:place 268 20 2,').at(4400, _WORDS).at(5700).s
_DOOR_C = _enter(_SIDE_A).at(2480, _NEXT_GUARDIAN).at(2800, _ARRIVED).at(3320, '0:place 284 36 2,').at(3800, _WORDS).at(5100).s
_BOT = ['partybot=1']
# `dive-pack` (four games): Alpha stands on layer 1's pad and watches; Beta walks to and fro beside him from
# 3120, and Gamma and Delta, ten seconds behind (in the dive at the clock's 2500, drawn once their arrival's
# boxes are paged, 3050), walk a line and a square round him: four MegaMen on one layer, the room it leaves
_PACK_A = _enter(_SIDE_A).at(3640).s
_PACK_B = _enter(_SIDE_B).at(3120, '30:UP+RIGHT,8:,30:DOWN+LEFT,8:,' * 7).at(3640).s
_PACK_C = _enter(_SIDE_A).at(2520, '30:DOWN+RIGHT,8:,30:UP+LEFT,8:,' * 7).at(3060).s
_PACK_D = _enter(_SIDE_B).at(2530, '24:UP+RIGHT,6:,24:DOWN+RIGHT,6:,24:DOWN+LEFT,6:,24:UP+LEFT,6:,' * 4).at(3060).s

ONLINE_SCENES = {
    'dive': {'games': [
        dict(name='alpha', operator='Alpha', script=_DIVE_A, dev=_BOT,
             keeps={'dive-portal': (1290, 2140), 'dive-divers': (2460, 2680), 'dive-crew-a': (2900, 3290), 'dive-joinin-a': (3400, 4100)}),
        dict(name='beta', operator='Beta', script=_DIVE_B, dev=_BOT,
             keeps={'dive-crew-b': (2660, 3290), 'dive-joinin-b': (3390, 4100)}),
    ]},
    'dive-pack': {'games': [
        dict(name='alpha', operator='Alpha', script=_PACK_A, keeps={'dive-pack': (3040, 3600)}),
        dict(name='beta', operator='Beta', script=_PACK_B),
        dict(name='gamma', operator='Gamma', script=_PACK_C, delay=10),
        dict(name='delta', operator='Delta', script=_PACK_D, delay=10),
    ]},
    'dive-door': {'games': [
        dict(name='alpha', operator='Alpha', script=_DOOR_A, dev=_BOT, keeps={'dive-door': (3300, 4104), 'dive-guardian': (4140, 4860), 'dive-party-a': (4860, 5680)}),
        dict(name='beta', operator='Beta', script=_DOOR_B, dev=_BOT, keeps={'dive-door-b': (3340, 4104), 'dive-party-b': (4860, 5680)}),
        dict(name='gamma', operator='Gamma', script=_DOOR_C, dev=_BOT, delay=10),
    ]},
}

# name: {what a sample shows: sample index}
MARKS = {
    'saves-pages': {'cloud page (Turn on)': 0, 'code page (Send)': 55, 'code: Receive': 95, 'file page (Export)': 118,
                    'file: Import': 158, 'file: Auto-copy': 176, 'file: Export again': 194, 'code page again': 217,
                    'cloud page again': 245},
    'saves-cloud': {'cloud page (Turn on)': 0, 'the question, No chosen': 30, 'Yes chosen': 60,
                    'on: Add a device, Sending 0%': 112, 'Sending 100% (130 to 220)': 220, 'Synced, the date (from 220 to 300)': 300},
    'saves-code': {'cloud page': 0, 'code page (Send)': 30, 'SEND: Sending...': 70, 'Sending: 100%': 120,
                   'the ten-digit code (from 185 to 200)': 205},
    'lock': {'a step back': 0, 'the walk to the cube': 22, 'at the cube': 55, 'A Security Cube, Lan.': 90,
             'It wants a P-Code...': 150, 'Someone here must know it.': 210, "Let's ask around!": 265, 'the box closed': 310},
    'walkway': {'the room': 0, 'onto the walkway': 30, 'the two-wide walkway': 70, 'walkways crossing': 130, 'on, to a room': 180},
    'bn4-acdc': {'the arrival pad': 0, 'the walkway': 60, 'into the room': 130, 'the room': 180, 'out again': 270},
    'bn5-oran': {'the arrival pad': 0, 'the floor': 45, 'across': 150, 'a platform ahead': 240},
    'options-3d': {'Display page': 0, 'Depth lighting, Off': 15, 'On': 60, 'Off': 150, 'On again': 220},
    'options-touch': {'Game page, Controls': 0, 'Touch controls, Auto': 25, 'Off': 85},
    'options-update': {"the box: WHAT'S NEW IN V0.22.0, its cover": 5, 'Cloud saves, like Steam Cloud': 70,
                       'Send your saves with a code': 113, 'SAVES in three pages': 160, 'The PSP': 202,
                       'Later: the title, its Update hint': 245, "Options' Game: Update to 0.22.0": 285,
                       'A: the box again': 362, 'its second page': 422, 'back on Options': 472},
    'jukebox-unused': {"BN6's Towns": 0, "BN4's Unused, Song 10": 10, 'BN4 Song 11': 50, 'BN4 Song 20': 70,
                       "back through BN4's sections": 100, "BN5's Unused, Song 07": 140, 'BN5 Song 08': 180,
                       'BN5 Song 09': 200, 'BN5 Song 11': 230, 'BN5 Song 12': 250, 'BN5 Song 20': 270},
    'map-key': {'the layer': 0, 'the map, its key in two rows': 20, 'the map closing': 140},
    'title-depth': {'the title, BEST: LAYER 5 bottom right': 20, 'CONTINUE Layer 5, the key hints bottom left': 115},
    'cold-a': {'the field dims': 4, 'the dark, MegaMan alone': 10, 'the flare': 20, 'the flare wide': 30, 'the white': 44,
               'the beast in the dark': 50, 'the field coming back': 60},
    'cold-b': {'fire rings rise': 4, 'the buster through the rings': 20, 'Powie lands': 40, 'the viruses close in': 60,
               "MegaMan's panel lit, his shot": 72, 'rings again, a hit': 88},
    'cold-c': {'the three on the field': 0, 'fire columns': 10, 'three columns': 40, 'MegaMan steps clear': 70},
    'cold-d': {'a meteor falls': 0, 'COUNTER HIT!, the blast': 30, 'the next meteors': 60, 'the big blast': 80, 'its ring': 100},
    'cold-e': {'his flame pillar': 0, 'BurnerMan across the field': 60, 'COUNTER HIT!': 75, 'struck': 90, 'the fight goes on': 140},
    # the second round, one game each
    'dive-party': {'three MegaMen and BlastMan (400) on the red field': 0, 'BATTLE START!': 40, 'his first strike, a white column': 60,
                   'all three firing, 398': 100, "a bomb's white blast on the front panel": 200, 'its smoke over the lead': 210,
                   'BlastMan across the field': 240, 'a flame column where he lands': 280, 'a blast on his own row': 350,
                   'a blast on the middle MegaMan': 360, 'two MegaMen struck (pink)': 410, 'BlastMan aflame, the middle row lit': 480,
                   'his fire rings down the whole row (490 to 520)': 500, 'the row struck': 530, 'the fight goes on': 600},
    'dive-party4': {'four MegaMen and BlastMan (400)': 0, 'BATTLE START!': 40, 'his first strike, a white column': 60, 'hit, 397': 70,
                    'the partners struck': 90, 'BlastMan in the air, 395': 100, 'the last moving sample (it stands still from 118)': 115},
    'climb-a': {'Green, layer 4: at the foot of the yellow stair': 0, 'onto the stair': 10, 'halfway up': 20, 'the top step': 30,
                'on the orange terrace, the green floor below': 45, 'standing on it': 75, 'back to the stair': 90, 'down it': 100,
                'on the ground again': 120},
    'climb-b': {'CopyBot Comp, layer 2: the arrival': 0, 'put on the walkway': 8, 'to the railed ladder': 16, 'on its rungs': 32,
                'the top rung': 40, 'onto the raised room, a Navi on it': 50, 'standing on the raised room': 70},
    'nest-crystals': {"the Nest's floor, a stump under its lower edge": 0, 'along the edge': 40, 'stumps and rubble in the void, left and right': 60,
                      'two stumps close': 100, 'MegaMan beside a stump and its rubble': 130, 'stumps behind the upper edge': 150,
                      'on, a Navi ahead': 170, 'the last stumps': 200},
    'robot-figures': {'the arrival pad, Mr.Prog': 0, 'MegaMan walks to its upper left edge': 16, 'the robot figure rises into the picture': 24,
                      'the figure in the void behind the pad, its colours turning (to the end)': 48, 'the figure whole': 88},
    'robot-wires': {'the white floor, a red light under its edge': 10, 'the light close': 30, 'a second light': 70, 'along the edge': 100,
                    'the light above MegaMan': 130, 'the last light': 160},
    'l-detours': {'layer 2 (RoboDog Comp), the Security Cube': 0, "L: ProtoMan's waiting to the upper left,far off!": 72,
                  'A flame of darkness burns to the upper left,a ways off!': 150, "Let's head to the upper left! It's a long way yet.": 222,
                  'the box closed, the arrow': 245},
    'glitch-flip': {'Green, "Layer 4?"': 0, 'a flip: the floor in another area\'s blue (a few samples)': 30, 'Green again': 40,
                    'a tear across the floor': 140, 'the mosaic into a flip': 210, 'flipped to a white area': 220, 'Green again, later': 230,
                    'the blue again': 270, 'blocks of the picture slipped': 330, 'the exit pad': 360},
    # the shared dive's takes (ONLINE_SCENES; a take's samples fall a few apart from these)
    'dive-portal': {'the plaza, its signs, Beta by the portal': 0, 'the portal beside MegaMan': 10, "THE NET SHIFTED! THIS WEEK'S DEEP NET IS OPEN!": 70,
                    'NOV 9-NOV 15. EVERY NAVI DIVES THE SAME NET!': 140, 'DIVE IN NOW? the cursor on No': 180, 'Yes': 210,
                    "THE PORTAL OPENS INTO THIS WEEK'S DEEP NET!": 265, 'the warp, layer 1 rising out of the dark': 290,
                    'layer 1, both divers on its pad': 300, "the act's card: RoboDog Comp": 330, 'the card gone': 410},
    'dive-divers': {'Alpha stands, Beta under him': 0, 'Beta walks out': 20, 'Beta past him, to the right': 40, 'round him': 70, 'beside him again': 110},
    'dive-pack': {'Alpha on the pad, the others under him or not yet drawn (they came at 5 in one take, 35 in the next)': 0,
                  'four MegaMen apart on the layer': 65, 'the pack, crossing': 115, 'spread again': 175, 'round the pad': 235, 'the last': 275},
    'dive-crew-a': {'the two on the pad': 0, 'Beta wants to team up with us!': 45, 'Team up,Lan? Yes': 100, 'the box closed': 125,
                    "the band: ALRIGHT! WE'RE A TEAM WITH BETA! (to the end)": 135},
    'dive-crew-b': {'Beta beside Alpha': 0, "A on him: Lan,it's Alpha!": 35, 'Team up,Lan? Yes': 95, "the band: LET'S SEE IF ALPHA TEAMS UP WITH US!": 135,
                    "the band: ALRIGHT! WE'RE A TEAM WITH ALPHA!": 250},
    'dive-joinin-a': {'Alpha walks, his crewmate beside': 0, 'the band: BATTLE! CALLING THE CREW...': 10, 'his Custom screen, a Gunner': 25,
                      'CHIP DATA TRANSMISSION': 60, 'both MegaMen on one field': 75, 'BATTLE START!': 110, 'the Gunner aims (the green sight)': 150,
                      'both firing': 190, 'the fight goes on': 300},
    'dive-joinin-b': {'Beta stands aside': 0, "the band: ALPHA'S IN A BATTLE! A:JUMP IN": 10, 'the field slides in': 20, 'his own Custom screen': 30,
                      'CHIP DATA TRANSMISSION': 70, 'both MegaMen on one field': 85, 'BATTLE START!': 115, 'the fight': 150, 'the fight goes on': 300},
    'dive-door': {'layer 3, before the arena': 0, 'in the arena, Run saved, the band: WAITING FOR A PARTY A:GO IN': 12,
                  'Beta steps in': 30, 'PARTY OF 2! A:GO IN B:ALONE': 40, 'PARTY OF 3!': 320, 'Gamma beside them, three at the door': 335,
                  'the last sample before the leader goes in': 400},
    'dive-door-b': {'layer 3, before the arena': 0, 'beside Alpha, the band: ALPHA LEADS 2 B:GO ALONE': 20, 'ALPHA LEADS 3': 300,
                    'Gamma beside them': 310},
    'dive-guardian': {"the screen's bars, the three at the door": 0, 'DiveMan beams in': 10, 'his card: Guardian of RoboDog Comp, DiveMan, Terror of the Deep': 40,
                      "That's DiveMan!": 130, 'Or a copy the Nest made of him...': 170, 'Awooga! Awooga! Intruder!': 205, 'Dive! Dive! Torpedoes ready!': 245,
                      'Battle routine,set!': 275, 'Execute!!': 310, 'the box closing': 345},
    'dive-party-a': {'in the arena, the box closing': 0, 'the Custom screen, DiveMan 4500': 30, 'the field: three MegaMen and DiveMan': 80,
                     'BATTLE START!': 112, 'the buster, DiveMan dives': 140, 'his wave': 160, 'torpedoes': 170, 'a MegaMan struck (his HP 50)': 190,
                     'torpedoes down two rows': 230, 'a MegaMan deleted (white)': 250, 'two left': 270, 'DiveMan up again': 300, 'the last MegaMan': 330,
                     'MEGAMAN DELETED': 382},
    'dive-party-b': {"Beta's game: Execute!!": 0, 'his own Custom screen': 30, 'the field: three MegaMen and DiveMan': 80, 'BATTLE START!': 112,
                     'torpedoes': 170, 'two MegaMen struck (pink)': 180, 'torpedoes down two rows': 230, 'a MegaMan deleted (white)': 250,
                     'the last MegaMan': 330, 'MEGAMAN DELETED': 382},
}

BN6_ONLY = ()   # scenes that must run with BN6's ROM alone: none (each was taken with the whole ROM folder)

# what could not be taken as asked (every scene asked for has a run above)
MISSING = {
    'dive-party (four MegaMen)': "four on one field stand still: --dev partycore=4,partybot=1 at seed 3's layer-3 guardian shows four "
                                 "MegaMen and BlastMan from frame 1700, and from 1936 every sample looked at is the same picture (to "
                                 "frame 4000; at 20000 the state still said the guardian was being fought; with and without god), "
                                 "BlastMan in the air at 395. --dev party=4 on a foes battle does the same under the autopilot "
                                 "(HeatMan 101@52 from frame 686, a Gunner 85@52 from about 1290, to their runs' ends). Three "
                                 "(partycore=3) and two fight on, so dive-party is three and dive-party4 the four's 3.8 seconds. "
                                 "The takes over the server are three too: layer 3 leaves room for two other divers' MegaMen.",
    'dive-divers (name tags, the feed)': "a diver on a layer is drawn without the name tag the Square draws over a visitor, and no feed "
                                         "line came while two dived (the band shows the crew's and the door's lines: dive-crew, dive-door)",
    'dive-pack (running divers)': "the pack walks: with B held (a run) Gamma and Delta were drawn on Alpha's layer for about a second "
                                  "and then not again, though their own games ran their lines to the end (one take; walking, all three "
                                  "stay drawn)",
    'climb (a ramp rising 64)': "not looked for: the weak autopilot met no stair in CopyBot Comp (seeds 1 to 4), the Graveyard (2, 3) "
                                "or Mr. Weather Comp (2) within 7000 frames, so both climbs are walked by script: Green's stair and "
                                "CopyBot Comp's ladder, each rising 32",

    'options-3d (the lighting itself)': "the scene shows the line and its note turning, but no lighting: --shot-range saves the "
                                        "canvas, and Depth lighting is laid under it as the screen is presented "
                                        "(docs/engine/voxel.md). Only --screen-shot shows it (a layer lit at --size 960x640 "
                                        "--depth-lighting on: tried, it does), and that takes 16 frames a run (capture.c's "
                                        "screen_shots[16]), no range. Options' preview beside the line stayed unlit in a "
                                        "--screen-shot too.",
}
