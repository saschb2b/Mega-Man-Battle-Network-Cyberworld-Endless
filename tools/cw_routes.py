"""Named routes to the screens agents reach most (tools/cw.py's goto): each
a start (the game's options) and play.py's commands from there, checked to
arrive where its name says by the state's `screen` line. Arguments given
with a route follow its own, so `--seed 7 --net-biome 3` picks a layer.

Each route's input waits out the fades before its presses: a press during
one is lost (docs/DEVTOOLS.md, Agents' tools).
"""

# (Options from the title: SELECT; its hub's lines Controls, Display,
# Audio, Statistics)
TITLE = ['--scene', 'title']
OPTIONS = 'wait 60; press SELECT; wait 40'
# (Lan's room at a new run: Dad's call paged, Lan free from about frame 530)
TOWN = ['--scene', 'town', '--seed', '2']
ROOM = 'wait 40; mash A 460; wait 30'
# (the PET's menu: Folder, SubChip, Library, MegaMan, E-Mail, KeyItem, then
# Comm's row, our Options in a run; Save says nothing in Lan's room)
PET = ROOM + '; press START; wait 40'
PET_ROWS = ('folder', 'subchip', 'library', 'megaman', 'mail', 'keyitem', 'options')
# (a layer: the arrival's words paged; quiet keeps random battles off)
NET = ['--scene', 'emu', '--dev', 'quiet']
LAYER = 'wait 150; mash A 300; wait 20'
# (Lan's HP after act 2, its arrival's words paged)
HP = ['--scene', 'home', '--run-depth', '7', '--seed', '3']


def pet_entry(row):
    return PET + ''.join('; press DOWN' for _ in range(row)) + '; press A; wait 60'


ROUTES = {
    # name: (start, commands, the screen it arrives on)
    'title': (TITLE, 'wait 80', 'title'),
    'options': (TITLE, OPTIONS, 'options'),
    'controls': (TITLE, OPTIONS + '; press A; wait 40', 'controls'),
    'options/display': (TITLE, OPTIONS + '; press DOWN; wait 20; press A; wait 40', 'options-display'),
    'options/audio': (TITLE, OPTIONS + '; press DOWN; wait 20; press DOWN; wait 20; press A; wait 40', 'options-audio'),
    'setup': (['--scene', 'setup'], 'wait 60', 'title'),   # (the title's own: its scene's name)
    'saves': (['--scene', 'saves'], 'wait 60', 'saves'),
    'launcher': (['--scene', 'launcher'], 'wait 60', 'launcher'),
    'room': (TOWN, ROOM, 'town'),
    'pet': (TOWN, PET, 'pet'),
    **{f'pet/{name}': (TOWN, pet_entry(row), screen) for row, (name, screen) in
       enumerate(zip(PET_ROWS, ('folders', 'subchip', 'library', 'status', 'mail', 'keyitem', 'options')))},
    'jukebox': (TOWN, pet_entry(5) + '; press DOWN; press RIGHT; wait 14; press A; wait 40', 'jukebox'),
    'layer': (NET, LAYER, 'map'),
    'battle': (NET, LAYER + '; battle; wait 240', 'battle'),
    'hp': (HP, 'wait 600' + '; press A; wait 88' * 6, 'town'),
}


def route(name):
    """(start, commands, screen) of a route; SystemExit naming them where none."""
    if name not in ROUTES:
        raise SystemExit(f'no route {name!r}: {", ".join(ROUTES)}')
    start, commands, screen = ROUTES[name]
    return list(start), commands, screen
