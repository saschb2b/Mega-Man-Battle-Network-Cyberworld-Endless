"""The starting folders taken from BN6's own presets (issue #865,
docs/game/meta/decisions.md): each locked on a new profile's shelf, opened
by its milestone in play, and a run started with it drawing from its 30
(the folder's first chip and its sum, as the next battle draws)."""

SETUP = '--scene setup'
# (a battle's Custom screen: the first chip picked, OK, and the chip used once the fight begins)
USE_FIRST = 'press A; wait 10; press START; wait 10; press A; wait 120; hold A 4; wait 300'


def draws(folder, name, first):
    """A run begun with `folder` (its number on --setup) draws from it."""
    return dict(GUARDS=f"A run begun with {name} draws from its 30: the next battle's folder is {name}'s.",
                START=f'battle --setup short,{folder},0,0', STEPS=[
                    ('expect', f'startfolder={name}'),
                    ('expect', f'folder={first}'),
                ])


MANY = {
    'zcanon-locked-on-a-new-profile': dict(
        GUARDS='Z-Canon locked on a new profile.',
        START=SETUP, STEPS=[
            ('expect', 'shelf~Z-Canon:locked'),
        ]),
    'zcanon-opened-by-a-program-advance': dict(
        GUARDS='GigaCan1 used in a battle (a folder of it): Z-Canon open at the next setup.',
        START='battle --dev folder=320', STEPS=[
            ('expect', 'folder~first GigaCan1'),
            ('do', USE_FIRST),
            ('restart', SETUP),
            ('expect', 'shelf~Z-Canon:open'),
        ]),
    'zcanon-draws': draws(3, 'Z-Canon', 'first Cannon A sum e410'),
}
