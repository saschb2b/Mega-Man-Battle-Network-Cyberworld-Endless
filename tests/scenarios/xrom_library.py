"""The PET's Library, a page per game: BN5's and BN4's pages beside BN6's
(their Standard, Mega and Giga classes in a ring of nine), there with
their slots whether or not the game's ROM is; and a chip of BN5's or
BN4's found in a run counted on its game's page the next time the Library
is opened (a chip to id 511 used to wait for the run's next save: found
here, fixed with it)."""

# (NEW GAME's Lan's room, Dad's call paged: tools/cw_routes.py's room)
START = '--scene town --seed 2'
# (the chips of a first run's starting folder each game's Library names, held from the start)
HELD = {'bn6': 12, 'bn5': 10, 'bn4': 9}
ROOM = 'wait 40; mash A 460; wait 30'
LIBRARY = [
    ('repeat', 'press START; wait 40', 'screen=pet'),
    ('repeat', 'press DOWN; wait 6; press DOWN; wait 6; press A; wait 60', 'screen=library', 3),   # the PET's third row
    ('expect', 'library~page bn6 standard row 1 of'),
]
CLOSE = [('repeat', 'press B; wait 40', 'screen=town')]


def turn(page):
    return ('repeat', 'press RIGHT; wait 20', f'library~page {page}', 9)


PAGES = [('do', ROOM)] + LIBRARY + [
    turn('bn6 mega'), turn('bn6 giga'),
    turn('bn5 standard row 1 of'),                    # BN5's page, its slots listed
    ('expect', f"library~held {HELD['bn5']} of"),     # the starting folder's chips BN5 names too
    turn('bn5 mega'), turn('bn5 giga'),
    turn('bn4 standard row 1 of'),
    ('expect', f"library~held {HELD['bn4']} of"),
    turn('bn4 mega'), turn('bn4 giga'),
    turn('bn6 standard row 1 of'),                    # round the ring
    ('repeat', 'press LEFT; wait 20', 'library~page bn4 giga', 2),
]


def found(game, name, chip, code):
    other = 'bn5' if game == 'bn6' else 'bn6'
    return dict(
        START=START, NEEDS=() if game == 'bn6' else (game,),
        GUARDS=f"{game.upper()}'s {name} found in a run: counted on {game.upper()}'s page when the Library is opened next",
        STEPS=[('do', ROOM)] + LIBRARY + [
            turn(f'{game} standard row 1 of'),
            ('expect', f'library~held {HELD[game]} of'),
        ] + CLOSE + [
            ('do', f'chip {chip} {code}; wait 10'),   # given as the game gives a chip
        ] + LIBRARY + [
            turn(f'{game} standard row 1 of'),
            ('expect', f'library~held {HELD[game] + 1} of'),
            turn(f'{other} standard row 1 of'),
            ('expect', f'library~held {HELD[other]} of'),   # ... and on no other game's
        ])


MANY = {
    'pages': dict(START=START, NEEDS=('bn5', 'bn4'), GUARDS="The Library's nine pages with all three ROMs: BN6's, BN5's and BN4's, each its three classes",
                  STEPS=PAGES),
    'pages-without-the-roms': dict(START=START, GUARDS="BN6's ROM alone: BN5's and BN4's pages keep their slots and counts", STEPS=PAGES),
    'found/bn6': found('bn6', 'AquaNdl1', 61, 'J'),
    'found/bn5': found('bn5', 'WideSht1', 204, 'L'),
    'found/bn4': found('bn4', 'HeatShot', 588, 'B'),
}
