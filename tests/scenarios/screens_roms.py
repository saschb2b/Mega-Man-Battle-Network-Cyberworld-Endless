"""The ROMs screen (src/launcher/, issue #97): with no ROM (PLAY locked, a
wrong file and an archive refused with a reason, BN6 dropped in and
played, its copy found by the next start), with BN6 alone, with BN5 or
BN4 beside it or dropped in later, and brought back from the title with
R; and a start with no ROM and no ROMs screen, which says where to put
it."""

# (HOME the game's own folder: with no ROM the screen looks where downloads
# and front ends keep theirs, and would find this machine's)
LAUNCHER = 'HOME={dir} --scene launcher'
TITLE = 'HOME={dir} --scene title'
PLAY = [   # PLAY: the game begins on its title
    ('expect', 'romfocus=play'),
    ('do', 'press A; wait 10'),
    ('until', 'screen=title', 600),
    ('expect', 'title=start|title=summary'),
]


def beside(game):
    """Another game's ROM in the folder beside BN6's: read at the start, its cartridge in."""
    return dict(START=LAUNCHER, NEEDS=(game,), STEPS=[
        ('expect', f'roms~{game}'),
        ('expect', 'roms~bn6'),
        *PLAY,
        ('expect', f'roms~{game}'),
    ])


def dropped(game):
    """... dropped on the screen later: taken, its copy kept, read again by the next start."""
    return dict(START=LAUNCHER, NEEDS=(game,), ROMS=('bn6',), STEPS=[
        ('expect', 'roms=bn6'),
        ('do', 'drop {%s}; wait 30' % game),
        ('until', f'roms~{game}', 600),
        ('expect', 'romnotekind=good'),
        *PLAY,
        ('restart', TITLE),
        ('expect', f'roms~{game}'),
    ])


MANY = {
    'bn6-alone': dict(START=LAUNCHER, STEPS=[
        ('expect', 'screen=launcher'),
        ('expect', 'roms=bn6'),
        ('expect', 'cartridges=bn6 bn5 bn4'),     # the other games' spots stand open
        *PLAY,
        ('expect', 'roms=bn6'),
    ]),
    # START plays from any place on the screen once BN6 is in
    'start-plays': dict(START=LAUNCHER, STEPS=[
        ('do', 'wait 20; press UP; wait 6'),
        ('expect', 'romfocus!=play'),
        ('do', 'press START; wait 10'),
        ('until', 'screen=title', 600),
    ]),
    # the cursor walks the cartridges and the buttons; an empty spot with no chooser says where to put the ROM
    'the-cursor-walks': dict(START=LAUNCHER, STEPS=[
        ('expect', 'romfocus=play'),
        ('do', 'press UP; wait 6'),
        ('expect', 'romfocus=bn4'),               # over PLAY: the last cartridge
        ('do', 'press LEFT; wait 6'),
        ('expect', 'romfocus=bn5'),
        ('do', 'press LEFT; wait 6'),
        ('expect', 'romfocus=bn6'),
        ('do', 'press RIGHT; wait 6; press DOWN; wait 6'),
        ('expect', 'romfocus=play'),
    ]),
    'no-rom': dict(START=LAUNCHER, ROMS=(), STEPS=[
        ('expect', 'screen=launcher'),
        ('expect', 'roms=none'),
        ('expect', 'romfocus=bn6'),               # the cursor on the cartridge that is needed
        ('do', 'press START; wait 20'),           # START plays nothing yet
        ('expect', 'screen=launcher'),
        ('repeat', 'press DOWN; wait 6; press RIGHT; wait 6', 'romfocus=play', 3),
        ('expect', 'romnotekind=bad'),            # PLAY says what it waits for
        ('do', 'press A; wait 20'),
        ('expect', 'screen=launcher'),
        ('expect', 'romnotekind=bad'),
        ('expect', 'roms=none'),
    ]),
    'a-wrong-file-is-refused': dict(START=LAUNCHER, ROMS=(), STEPS=[
        ('do', 'wait 20; drop {wrong}; wait 20'),
        ('expect', 'romnotekind=bad'),
        ('expect', 'romnote~wrong.gba'),
        ('expect', 'roms=none'),
        ('do', 'drop {zip}; wait 20'),            # an archive: unpack it first
        ('expect', 'romnotekind=bad'),
        ('expect', 'romnote~rom.zip'),
        ('expect', 'roms=none'),
        ('expect', 'screen=launcher'),
    ]),
    # ... and in the ROM folder at the start it is no ROM either
    'a-wrong-file-in-the-folder': dict(START=LAUNCHER, ROMS=('wrong',), STEPS=[
        ('expect', 'screen=launcher'),
        ('expect', 'roms=none'),
        ('expect', 'romfocus=bn6'),
    ]),
    # BN6 dropped on the empty screen: its cartridge in, the cursor on PLAY, the game; its copy
    # kept in the ROM folder, where the next start finds it
    'bn6-dropped-in-and-played': dict(START=LAUNCHER, ROMS=(), STEPS=[
        ('expect', 'roms=none'),
        ('do', 'wait 20; drop {bn6}; wait 30'),
        ('until', 'roms=bn6', 600),
        ('expect', 'romnotekind=good'),
        *PLAY,
        ('restart', TITLE),
        ('expect', 'screen=title'),
        ('expect', 'roms=bn6'),
    ]),
    # BN6 chosen with another game's ROM beside it: both come
    'bn6-dropped-in-brings-bn5-beside-it': dict(START=LAUNCHER, NEEDS=('bn5',), ROMS=(), STEPS=[
        ('expect', 'roms=none'),
        ('do', 'wait 20; drop {roms}/bn6.gba; wait 30'),
        ('until', 'roms=bn6 bn5', 600),
        ('expect', 'romnotekind=good'),
        *PLAY,
    ]),
    'bn5-beside-bn6': beside('bn5'),
    'bn4-beside-bn6': beside('bn4'),
    'bn5-dropped-in': dropped('bn5'),
    'bn4-dropped-in': dropped('bn4'),
    # R on the title brings the screen back over the game; B, or PLAY, returns to the title
    'r-on-the-title-brings-it-back': dict(START=TITLE, STEPS=[
        ('repeat', 'press START; wait 40', 'title=menu'),
        ('do', 'press R; wait 20'),
        ('expect', 'screen=launcher'),
        ('expect', 'roms=bn6'),
        ('do', 'press B; wait 20'),
        ('expect', 'screen=title'),
        ('do', 'wait 30; press R; wait 20'),
        ('expect', 'screen=launcher'),
        ('repeat', 'press DOWN; wait 6; press RIGHT; wait 6', 'romfocus=play', 3),
        ('do', 'press A; wait 20'),
        ('expect', 'screen=title'),
    ]),
    # no ROM and no ROMs screen (a handheld's start): the error says so, no game
    'no-rom-and-no-roms-screen': dict(START=TITLE, ROMS=(), STEPS=[
        ('do', 'wait 30'),
        ('expect', 'scene=error'),
        ('expect', 'roms=none'),
    ]),
}
