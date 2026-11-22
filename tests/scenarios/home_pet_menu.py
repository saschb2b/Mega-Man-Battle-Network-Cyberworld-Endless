"""Issue #793: the PET's start menu of ours over BN6's, BN6's list folded in
two columns. Each entry opens from its place (ChipFolder, SubChip, Library,
MegaMan, Forms down the left; E-Mail, KeyItem, Options, Save down the right)
as BN6 opens it, and B gives the menu back with the cursor on that entry;
Up from the top wraps to Save as BN6's does, Right goes across and on to
EXIT, and EXIT goes back to where the cursor came from
(docs/game/pet-menu.md)."""
import cw_routes

START = '--scene town --seed 2'
OPEN = [
    ('do', cw_routes.ROOM),
    ('repeat', 'press START; wait 40', 'screen=pet', 6),
    ('expect', 'petmenu=at ChipFolder'),
]


def entry(path, screen, name):
    """The entry `path` reaches from ChipFolder, its screen, and the menu given back on it."""
    return dict(START=START, STEPS=[
        *OPEN,
        ('do', ''.join(f'press {k}; wait 6; ' for k in path) + 'wait 10'),
        ('expect', f'petmenu=at {name}'),
        ('do', 'press A; wait 60'),
        ('until', f'screen={screen}', 300),
        ('repeat', 'press B; wait 40', 'screen=pet', 8),
        ('expect', f'petmenu=at {name}'),
    ])


MANY = {
    'chipfolder': entry([], 'folders', 'ChipFolder'),
    'subchip': entry(['DOWN'], 'subchip', 'SubChip'),
    'library': entry(['DOWN'] * 2, 'library', 'Library'),
    'megaman': entry(['DOWN'] * 3, 'status', 'MegaMan'),
    'forms': entry(['DOWN'] * 4, 'forms', 'Forms'),
    'email': entry(['RIGHT'], 'mail', 'E-Mail'),
    'keyitem': entry(['RIGHT', 'DOWN'], 'keyitem', 'KeyItem'),
    'options': entry(['UP', 'UP'], 'options-display', 'Options'),
    # (the folded list's ends and EXIT, which the menu closes on)
    'moves': dict(START=START, STEPS=[
        *OPEN,
        ('do', 'press UP; wait 6'), ('expect', 'petmenu=at Save'),               # Up from the top: Save, as BN6's
        ('do', 'press DOWN; wait 6'), ('expect', 'petmenu=at ChipFolder'),       # and back round
        ('do', 'press UP; wait 6; ' * 4), ('expect', 'petmenu=at E-Mail'),       # BN6's Up count to E-Mail
        ('do', 'press UP; wait 6'), ('expect', 'petmenu=at Forms'),              # across the fold
        ('do', 'press RIGHT; wait 6'), ('expect', 'petmenu=at EXIT'),            # nothing beside Forms: EXIT
        ('do', 'press LEFT; wait 6'), ('expect', 'petmenu=at Forms'),            # back where it came from
        ('do', 'press LEFT; wait 6'), ('expect', 'petmenu=at Forms'),            # Left from the left column: nothing
        ('do', 'press UP; wait 6; press UP; wait 6; press RIGHT; wait 6'), ('expect', 'petmenu=at Options'),   # Library's row: Options
        ('do', 'press RIGHT; wait 6'), ('expect', 'petmenu=at EXIT'),
        ('do', 'press A; wait 60'),
        ('until', 'screen=town', 300),                                            # A on EXIT closes it
        ('repeat', 'press START; wait 40', 'screen=pet', 6),
        ('expect', 'petmenu=at ChipFolder'),                                      # it opens on ChipFolder, as BN6's
    ]),
}
