"""The PET's entries in a run, each by what the player ends up with
(docs/game/pet.md): a chip swapped into the folder, the Library's pages
turned, a NaviCust program placed and run, a mail read, a song played on
the Jukebox and its mode kept, Options left back to the PET."""
import cw_routes

# (the PET's menu in Lan's room, a new run, and on its first layer)
PET = cw_routes.PET
ROW = {name: row for row, name in enumerate(cw_routes.PET_ROWS)}


def entry(name):
    return ''.join('press DOWN; ' for _ in range(ROW[name])) + 'press A; wait 60'


def town(dev=''):
    return '--scene town --seed 2' + (f' --dev {dev}' if dev else '')


MANY = {
    # Folder: EDIT, the first chip held, the Pack's second put in its place
    'folder-swap': dict(
        GUARDS="The PET's Folder: a chip of the Pack swapped in is the folder MegaMan holds, back on the map and after the PET is opened again.",
        START=town('pack=12'),
        STEPS=[
            ('do', PET),
            ('expect', 'screen=pet'),
            ('expect', 'folder~first Cannon A'),
            ('keep', 'folder'),
            ('do', entry('folder')),
            ('expect', 'screen=folders'),
            ('repeat', 'press A; wait 40', 'screen=folder-edit'),
            ('do', 'press A; wait 30; press RIGHT; wait 30; press DOWN; press A; wait 30'),
            ('repeat', 'press B; wait 40', 'screen=town'),          # (BN6 writes the folder as its editor is left)
            ('expect', 'folder~first HiCannon L'),
            ('keep', 'folder'),
            ('do', 'press START; wait 40; press B; wait 40'),       # the PET again: nothing of it puts the old folder back
            ('until', 'screen=town', 300),
            ('same', 'folder'),
        ]),
    'folder-into-the-net': dict(
        GUARDS="A chip swapped into the folder in Lan's room is in the folder on the run's first layer: the jack-in, Lan's HP and the layer's start leave it as edited.",
        START=town('pack=12'),
        STEPS=[
            ('do', PET + '; ' + entry('folder')),
            ('repeat', 'press A; wait 40', 'screen=folder-edit'),
            ('do', 'press A; wait 30; press RIGHT; wait 30; press DOWN; press A; wait 30'),
            ('repeat', 'press B; wait 40', 'screen=town'),
            ('expect', 'folder~first HiCannon L'),
            ('keep', 'folder'),
            ('do', 'place -40 -8 7; wait 10; press R; wait 40'),       # his PC: Lan's HP
            ('until', "place=Lan's HP", 900),
            ('repeat', 'mash A 60', 'held=none', 20),
            ('do', 'place 171 0 1; wait 10; hold UP+RIGHT 25'),         # its pink pad: the first layer
            ('until', 'layer=1', 900),
            ('repeat', 'mash A 60', 'held=none', 30),
            ('same', 'folder'),
        ]),
    'subchip': dict(
        GUARDS="The PET's SubChip: its screen opens and B goes back to the PET's menu, then to the room, Lan free.",
        START=town(),
        STEPS=[
            ('do', PET + '; ' + entry('subchip')),
            ('expect', 'screen=subchip'),
            ('repeat', 'press B; wait 40', 'screen=pet'),
            ('repeat', 'press B; wait 40', 'screen=town'),
        ]),
    'library-pages': dict(
        GUARDS="The PET's Library: BN6's page first, Right and Left turn its classes, a full Library lists every chip, B goes back to the PET.",
        START=town('library=1'),
        STEPS=[
            ('do', PET + '; ' + entry('library')),
            ('expect', 'screen=library'),
            ('expect', 'library~page bn6 standard row 1'),
            ('expect', 'library~held 200 of 200'),
            ('do', 'press RIGHT; wait 20'),
            ('expect', 'library~page bn6 mega row 1'),
            ('do', 'press RIGHT; wait 20'),
            ('expect', 'library~page bn6 giga'),
            ('do', 'press LEFT; wait 20; press LEFT; wait 20'),
            ('expect', 'library~page bn6 standard'),
            ('do', 'press DOWN; wait 10; press DOWN; wait 10'),
            ('expect', 'library~row 3 of'),
            ('do', 'press B; wait 40'),
            ('expect', 'screen=pet'),
            ('do', 'wait 60'),                                      # ... and stays there: B fell through no further
            ('expect', 'screen=pet'),
        ]),
    'library-new-profile': dict(
        GUARDS="The PET's Library in a first run: the chips of the starting folder alone, the Mega page empty.",
        START=town(),
        STEPS=[
            ('do', PET + '; ' + entry('library')),
            ('expect', 'screen=library'),
            ('expect', 'library~of 200'),
            ('expect', 'library<200'),
            ('do', 'press RIGHT; wait 20'),
            ('expect', 'library~page bn6 mega row 0 of 0'),
        ]),
    'navicust-run': dict(
        GUARDS="The PET's MegaMan: his status, then the NaviCust, where HP+50 placed and RUN makes his HP 150 of 150, on the map too.",
        START=town('program=41'),
        STEPS=[
            ('do', PET),
            ('expect', 'hp=100/100'),
            ('do', entry('megaman')),
            ('expect', 'screen=status'),
            ('repeat', 'press A; wait 60', 'screen=navicust'),
            ('repeat', 'press A; wait 40', 'hp=150/150'),           # the program picked, set down, RUN
            ('repeat', 'press A; wait 40', 'screen=status', 20),    # "RUN complete!", and his status again
            ('expect', 'hp=150/150'),
            ('repeat', 'press B; wait 40', 'screen=town'),
            ('expect', 'hp=150/150'),
        ]),
    'mail-read': dict(
        GUARDS="The PET's E-Mail on a run's first layer: the lab's three mails, one read is unread no more.",
        START='layer',
        STEPS=[
            ('expect', 'mail=3 unread 3'),
            ('do', 'press START; wait 40; ' + entry('mail')),
            ('expect', 'screen=mail'),
            ('repeat', 'press A; wait 30', 'mail=3 unread 2', 40),
            ('repeat', 'press B; wait 40', 'screen=map', 20),
            ('expect', 'mail=3 unread 2'),
        ]),
    'keyitem': dict(
        GUARDS="The PET's KeyItem: its screen opens and B goes back to the PET's menu.",
        START=town(),
        STEPS=[
            ('do', PET + '; ' + entry('keyitem')),
            ('expect', 'screen=keyitem'),
            ('repeat', 'press B; wait 40', 'screen=pet'),
        ]),
    'jukebox-plays': dict(
        GUARDS="The Jukebox, KeyItem's: A plays the song under the cursor, another row another song, START turns the mode, B closes it back to KeyItem, and it opens again on the song and the mode it was left with.",
        START='jukebox',
        STEPS=[
            ('expect', 'screen=jukebox'),
            ('expect', 'jukebox~playing none mode repeat'),
            ('do', 'press A; wait 30'),
            ('expect', 'jukebox~row 1 of 8 playing bn6:03'),
            ('do', 'press DOWN; wait 10; press A; wait 30'),
            ('expect', 'jukebox~row 2 of 8 playing bn6:'),
            ('keep', 'jukebox'),
            ('do', 'press START; wait 20'),
            ('expect', 'jukebox~mode order'),
            ('do', 'press B; wait 40'),
            ('expect', 'screen=keyitem'),
            ('do', 'wait 60'),
            ('expect', 'screen=keyitem'),
            ('do', 'press A; wait 40'),                             # opened again: the row and the mode kept, nothing playing
            ('expect', 'jukebox~row 2 of 8 playing none mode order'),
        ]),
    'options-back': dict(
        GUARDS="The PET's Options in a run: Options' Display page, and B goes back to the PET's menu, not through it to the room.",
        START=town(),
        STEPS=[
            ('do', PET + '; ' + entry('options')),
            ('expect', 'screen=options-display'),
            ('do', 'press B; wait 40'),
            ('expect', 'screen=pet'),
            ('do', 'wait 60'),
            ('expect', 'screen=pet'),
        ]),
}
