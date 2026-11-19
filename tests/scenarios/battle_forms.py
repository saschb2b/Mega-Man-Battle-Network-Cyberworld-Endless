"""MegaMan's forms taken on BN6's own Custom screen and fought in: the
Cross a run's setup brings (HELP's Cross row, each of Gregar's five and
Falzar's five, issue #706), chosen
in CROSSSELECT, and Beast Out (the setup's helper, here --dev powers),
from its button under OK. Each is his form once the fight begins, and a
shot in it ends the battle."""

ARRIVE = [
    ('do', 'wait 150; mash A 300; wait 20'),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
    ('do', 'foes 01@52'),
    ('until', 'phase=custom', 1200),
    ('do', 'wait 60'),
    ('expect', 'form=none'),
]
FIGHT = [
    ('repeat', 'press START; wait 10; press A; wait 60', 'phase=fight'),
    ('repeat', 'wait 10', 'custom>0', 60),    # the form taken as the turn begins
]
WIN = [('repeat', 'mash B 30', 'phase=results', 30), ('expect', 'result=won')]


def cross(n, name):
    """The setup's Cross n: the only row of CROSSSELECT (UP opens it, A takes it)."""
    return dict(
        START=f'--scene emu --seed 1 --run-depth 2 --setup short,0,0,0,{n} --dev onehit,god',
        STEPS=[*ARRIVE, ('do', 'press UP; wait 30; press A; wait 60'), *FIGHT, ('expect', f'form={name}'), *WIN],
    )


MANY = {
    **{name.lower(): cross(n, name) for n, name in enumerate(('HeatCross', 'ElecCross', 'SlashCross', 'EraseCross', 'ChargeCross',
                                                               'SpoutCross', 'TomahawkCross', 'TenguCross', 'GroundCross', 'DustCross'), 1)},
    # (no setup's Cross: CROSSSELECT has none to offer, and the fight is his own)
    'none': dict(START='--scene emu --seed 1 --run-depth 2 --dev onehit,god',
                 STEPS=[*ARRIVE, ('do', 'press UP; wait 30; press A; wait 60'), *FIGHT, ('expect', 'form=none'), *WIN]),
    'beastout': dict(START='--scene emu --seed 1 --run-depth 2 --dev onehit,god,powers',
                     STEPS=[*ARRIVE, ('do', 'press START; wait 10; press DOWN; wait 10; press A; wait 60'), *FIGHT,
                            ('expect', 'form=BeastOut'), *WIN]),
}
