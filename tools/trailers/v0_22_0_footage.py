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
"""

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
}

BN6_ONLY = ()   # scenes that must run with BN6's ROM alone: none (each was taken with the whole ROM folder)

# what could not be taken as asked (every scene asked for has a run above)
MISSING = {
    'options-3d (the lighting itself)': "the scene shows the line and its note turning, but no lighting: --shot-range saves the "
                                        "canvas, and Depth lighting is laid under it as the screen is presented "
                                        "(docs/engine/voxel.md). Only --screen-shot shows it (a layer lit at --size 960x640 "
                                        "--depth-lighting on: tried, it does), and that takes 16 frames a run (capture.c's "
                                        "screen_shots[16]), no range. Options' preview beside the line stayed unlit in a "
                                        "--screen-shot too.",
}
