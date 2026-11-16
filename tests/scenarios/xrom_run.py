"""A run meets the older nets only where their ROMs are: the title says
which were found, NEW GAME's run has their areas among its acts and jacks
in on one, and with BN6's ROM alone, or with the Older nets extra switched
off in the Jack-In Setup, every act is BN6's own."""

# (seed 5: its first act an older net's with either ROM; the title's menu, then Lan's PC, by the autopilot; a first
# NEW GAME is the short net, three acts)
TITLE = 'CYBERWORLD_AUTOPILOT=1 --scene title --seed 5'
NEW_GAME = [
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=new'),
    ('repeat', 'press A; wait 40', 'title=setup|title=leaving|scene=emu'),
    ('repeat', 'press START; wait 60', 'scene=emu'),
    ('expect', 'where=town'),
]


def run(found, acts, first, name):
    steps = [('until', 'foundnote=up', 300), ('expect', f'found={found}')] if found else [('expect', 'found=nothing'), ('expect', 'foundnote=none')]
    return steps + NEW_GAME + [
        ('expect', f'acts={acts}'),
        ('until', 'where=layer', 20000),      # from Lan's room into the net
        ('expect', 'layer=1'),
        ('expect', f'areafrom={first}'),
        ('expect', f'area={name}'),
    ]


MANY = {
    'bn6-alone': dict(
        START=TITLE, GUARDS="BN6's ROM alone: the title has no found note, and every act of the run is BN6's own",
        STEPS=run('', 'bn6 bn6 bn6', 'bn6', 'Seaside Area') + [('expect', 'olderacts=0'), ('expect', 'olderguards=0')]),
    'bn5': dict(
        START=TITLE, NEEDS=('bn5',), GUARDS="BN5's ROM beside BN6's: the title says so, the run's first act is BN5's End Area",
        STEPS=run('BN5 found: its Net joins ours', 'bn5 bn5 bn6', 'bn5', 'End Area') + [('expect', 'olderguards>=1')]),
    'bn4': dict(
        START=TITLE, NEEDS=('bn4',), GUARDS="BN4's ROM beside BN6's: the title says so, the run's first act is BN4's Town Area",
        STEPS=run('BN4 found: its Net joins ours', 'bn4 bn6 bn6', 'bn4', 'Town Area')),
    'both': dict(
        START=TITLE.replace('--seed 5', '--seed 9'), NEEDS=('bn5', 'bn4'),
        GUARDS="BN5's and BN4's ROMs beside BN6's: the title names both, and one run holds acts of all three nets",
        STEPS=run('BN5 + BN4 found: both join ours', 'bn4 bn5 bn6', 'bn4', 'Town Area')),
    'extra-off': dict(
        # (--setup's sixth number: the extras switched off, 32 the Older nets)
        START='--scene emu --seed 5 --setup endless,0,0,0,0,32', NEEDS=('bn5', 'bn4'),
        GUARDS="The Older nets extra switched off in the Jack-In Setup: both ROMs read, every act and guardian BN6's own",
        STEPS=[
            ('expect', 'acts=bn6 bn6 bn6 bn6 bn6 bn6'),
            ('expect', 'olderacts=0'),
            ('expect', 'olderguards=0'),
            ('expect', 'areafrom=bn6'),
            ('expect', 'area=Seaside Area'),
        ]),
    'extra-on': dict(
        START='--scene emu --seed 5 --setup endless,0,0,0,0,0', NEEDS=('bn5', 'bn4'),
        GUARDS="... and switched on, the same seed's run: older nets among its acts (the control of extra-off)",
        STEPS=[('expect', 'olderacts>=1'), ('expect', 'areafrom=bn5')]),
}
