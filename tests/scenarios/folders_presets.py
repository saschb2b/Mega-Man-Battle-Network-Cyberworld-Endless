"""The starting folders taken from BN6's own presets (issue #865,
docs/game/meta/decisions.md): each locked on a new profile's shelf, opened
by its milestone in play, and a run started with it drawing from its 30
(the folder's first chip and its sum, as the next battle draws)."""

SETUP = '--scene setup'
# (a battle's Custom screen: the first chip picked, OK, and the chip used once the fight begins)
USE_FIRST = 'press A; wait 10; press START; wait 10; press A; wait 120; hold A 4; wait 300'


# (a run of a layer walked by the autopilot at `depth`; fragile: the first hit deletes MegaMan, and the run is over)
def lost(depth):
    return f'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth {depth} --dev fragile'


OVER = [('until', 'title=summary', 12000), ('repeat', 'press A; wait 30', 'title=start', 12)]


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
MANY.update({
    'steady-and-navi-locked-on-a-new-profile': dict(
        GUARDS='Steady and Navi locked on a new profile.',
        START=SETUP, STEPS=[
            ('expect', 'shelf~Steady:locked,Navi:locked'),
        ]),
    'steady-opened-by-act-3': dict(
        GUARDS="A run that reached act 3's first layer (layer 7) and ended: Steady open at the next setup.",
        START=lost(7), STEPS=[*OVER, ('restart', SETUP), ('expect', 'shelf~Steady:open')]),
    'steady-not-at-layer-6': dict(
        GUARDS='... and not one that ended at layer 6.',
        START=lost(6), STEPS=[*OVER, ('restart', SETUP), ('expect', 'shelf~Steady:locked')]),
    'navi-opened-by-a-navi-chip': dict(
        GUARDS='BlastMan used in a battle (a folder of him): Navi open at the next setup, Z-Canon not.',
        START='battle --dev folder=257', STEPS=[
            ('expect', 'folder~first BlastMan'),
            ('do', USE_FIRST),
            ('restart', SETUP),
            ('expect', 'shelf~Navi:open'),
            ('expect', 'shelf~Z-Canon:locked'),
        ]),
    'steady-draws': draws(6, 'Steady', 'first Cannon A sum 2fb2'),
    'navi-draws': draws(7, 'Navi', 'first BlastMan B sum 9a7d'),
})
