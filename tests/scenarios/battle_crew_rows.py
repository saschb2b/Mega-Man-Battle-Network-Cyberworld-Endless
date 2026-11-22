"""A crew stood side by side (issue #777, docs/online/shared-dive.md,
Viruses' aim in a crew's battle): a virus finds its target by the panels in
its line, so a crew's MegaMen each stand in a row of their own, the lead in
BN6's own place (column 2, row 2), the partners above and below him. Three
MegaMen standing still on the run's own battle (--dev party=3,partyaim=2,
the spike's): a Mettaur in row 1 strikes the partner there first, the lead
untouched; a Gunner in each row strikes every MegaMan while the lead
stands. With the partners behind him (column 1) every virus went for the
lead, in the front column, till he fell."""

ARRIVE = [
    ('do', 'wait 150; mash A 300; wait 20'),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
]


def fight(foes, until, lead):
    return dict(START='--scene emu --run-depth 2 --seed 3 --dev quiet,party=3,partyaim=2', STEPS=ARRIVE + [
        ('do', f'foes {foes}'),
        ('until', 'phase=custom', 1200),
        ('do', 'wait 60; press START; wait 6; press A'),
        ('until', until, 1500),
        ('expect', lead),
    ])


MANY = {
    # (a Mettaur in row 1: the partner beside the lead there is struck first, the lead untouched; stood behind him, the
    # Mettaur walked to the lead's row)
    'mettaur-row-1': fight('01@51', 'spike~hit 1', 'spike~lead whole'),
    # (a Gunner in each row: every MegaMan struck while the lead stands; behind him, the crew was not till he fell)
    'gunners': fight('53@51/53@52/53@53', 'spike~hit 3', 'spike~lead hit'),
}
