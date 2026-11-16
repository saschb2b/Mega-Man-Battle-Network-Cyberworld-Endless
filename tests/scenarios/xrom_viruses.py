"""The older nets' foes in BN6's battles: each virus family moved into the
engine (BN5's 25, BN4's 11) met alone at its first version under its own
name, fought by the autopilot and deleted, MegaMan back on the layer; one
of BN5's own battle records fought whole, its two CanGards and a Mettaur;
and BN5's four Darkloids at V1, each deleted."""

# (a family's first enemy id, six a family from 0x200: tests/xdiff/families.json's order, then BN4's from 0x2C0)
BN5 = ['CanGard', 'Handi', 'Powie', 'Bugtank', 'Dominerd', 'MetFire', 'Shakey', 'NinJoy', 'Lark', 'BomBoy', 'Zomon', 'Drixol', 'Batty',
       'Skarab', 'Draggin', 'Whirly', 'Marina', 'Flashy', 'Eleogre', 'Cactikil', 'WuNote', 'Appley', 'BigBrute', 'TinHawk', 'Bladia']
BN4 = ['Billy', 'Spikey', 'Shrimpy', 'Walla', 'Elmperor', 'CirKill', 'Mettaur', 'Melody', 'VolGear', 'Kilby', 'MagTect']

# (BN6's own first area whatever the ROMs, seed 15; no random battle but the step's)
START = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 15 --net-biome 0 --dev onehit,quiet'
ARRIVE = [
    ('until', 'chat=open', 900),              # the arrival's words, which the autopilot pages
    ('until', 'chat=closed', 1800),
]
WON = [
    ('until', 'lastbattle=won', 8000),        # every one deleted, its RESULT paid: on the layer again
    ('expect', 'layer=1'),
]


def family(game, first, name):
    return dict(
        START=START, NEEDS=(game,),
        GUARDS=f"{game.upper()}'s {name} in BN6's engine: met under its name, deleted, the battle won",
        STEPS=ARRIVE + [
            ('do', f'foes {first:X}@52'),
            ('until', 'foes>=1', 1200),
            ('expect', f'enemynames~{name}'),
            ('expect', 'olderfoes>=1'),
        ] + WON)


MANY = {f'bn5/{name.lower()}': family('bn5', 0x200 + 6 * k, name) for k, name in enumerate(BN5)}
MANY.update({f'bn4/{name.lower()}': family('bn4', 0x2C0 + 6 * k, name) for k, name in enumerate(BN4)})
# (the Darkloids' moved ids at V1: tools/cw_routes.py's, docs/multirom/navis.md)
DARKLOIDS = {'ShadeMan': 0x1E0, 'BlizzardMan': 0x1E4, 'CloudMan': 0x1E8, 'CosmoMan': 0x1EC}
IN_BATTLE = {'BlizzardMan': 'BlizMan'}
for _name, _id in DARKLOIDS.items():
    MANY[f'bn5/darkloid-{_name.lower()}'] = family('bn5', _id, IN_BATTLE.get(_name, _name))
    # (a Navi's blows: MegaMan kept whole through them, and 2000 HP against one that would take his 100 at once)
    MANY[f'bn5/darkloid-{_name.lower()}']['START'] = START.replace('onehit,quiet', 'onehit,quiet,god,hp=2000')
    MANY[f'bn5/darkloid-{_name.lower()}']['GUARDS'] = f"BN5's Darkloid {_name} at V1 in BN6's engine: met under his name, deleted, the battle won"
# (BN5's ACDC Area record 0x0811DE10, on BN6's ground: docs/devtools/battle-on-demand.md)
MANY['bn5/record'] = dict(
    START=START, NEEDS=('bn5',),
    GUARDS="One of BN5's battle records fought whole in BN6's engine: two CanGards and a Mettaur, all deleted",
    STEPS=ARRIVE + [
        ('do', 'xrecord 811DE10'),
        ('until', 'foes>=3', 1200),
        ('expect', 'enemynames=CanGard+CanGard+Mettaur'),
        ('expect', 'olderfoes=2'),
    ] + WON)
