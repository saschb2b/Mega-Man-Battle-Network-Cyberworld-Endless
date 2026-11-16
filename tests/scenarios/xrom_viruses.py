"""The older nets' viruses in BN6's battles: each family moved into the
engine (BN5's 25, BN4's 11) met alone at its first version under its own
name, fought by the autopilot and deleted, MegaMan back on the layer; and
one of BN5's own battle records fought whole, its two CanGards and a
Mettaur."""

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
    ('repeat', 'wait 10', 'enemies=0', 600),  # every one deleted
    ('expect', 'screen=battle'),
    ('until', 'screen=map', 1800),            # its RESULT paid: on the layer again
    ('expect', 'layer=1'),
]


def family(game, first, name):
    return dict(
        START=START, NEEDS=(game,),
        GUARDS=f"{game.upper()}'s {name} in BN6's engine: met under its name, deleted, the battle won",
        STEPS=ARRIVE + [
            ('do', f'foes {first:X}@52'),
            ('until', 'enemies>=1', 1200),
            ('expect', f'enemynames~{name}'),
            ('expect', 'olderfoes>=1'),
        ] + WON)


MANY = {f'bn5/{name.lower()}': family('bn5', 0x200 + 6 * k, name) for k, name in enumerate(BN5)}
MANY.update({f'bn4/{name.lower()}': family('bn4', 0x2C0 + 6 * k, name) for k, name in enumerate(BN4)})
# (BN5's ACDC Area record 0x0811DE10, on BN6's ground: docs/devtools/battle-on-demand.md)
MANY['bn5/record'] = dict(
    START=START, NEEDS=('bn5',),
    GUARDS="One of BN5's battle records fought whole in BN6's engine: two CanGards and a Mettaur, all deleted",
    STEPS=ARRIVE + [
        ('do', 'xrecord 811DE10'),
        ('until', 'enemies>=3', 1200),
        ('expect', 'enemynames=CanGard+CanGard+Mettaur'),
        ('expect', 'olderfoes=2'),
    ] + WON)
