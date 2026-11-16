"""The older nets' viruses as visitors in BN6's own areas: where the two
nets meet in one battle (an encounter group of the kind, docs/game/
progression/encounter-groups.md), a virus of BN5's or BN4's stands among
BN6's, the battle is won, and MegaMan says his word on the older Net's
virus, the first a profile meets; with BN6's ROM alone, or the Older nets
extra switched off, the same battle holds BN6's viruses alone and no word
follows."""

# (BN6's Central Area at an act's second layer; every random battle a group of template 11, the nets meeting)
START = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 15 --net-biome 0 --run-depth 2 --dev groups,group=11,quiet,onehit,god'
FIGHT = [
    ('until', 'chat=open', 900),              # the arrival's words, which the autopilot pages
    ('until', 'chat=closed', 1800),
    ('do', 'battle'),
    ('until', 'foes>=2', 1200),
    ('expect', 'areafrom=bn6'),
    ('expect', 'olderrecord=none'),           # a battle of BN6's ground, no record of the older nets'
]
WON = [
    ('repeat', 'wait 10', 'lastbattle=won', 800),   # won, back on the layer (looked at often: his word comes at once)
]


def meets(game):
    return dict(
        START=START, NEEDS=(game,),
        GUARDS=f"A virus of {game.upper()}'s visiting a battle of BN6's ground: among BN6's own, deleted, MegaMan's word on it after",
        STEPS=FIGHT + [('expect', 'olderfoes>=1')] + WON + [
            ('repeat', 'wait 10', 'talk=director', 60),   # his word on the older Net's virus
        ])


def alone(needs, setup, why):
    return dict(
        START=START + setup, NEEDS=needs, GUARDS=why,
        STEPS=FIGHT + [('expect', 'olderfoes=0')] + WON + [
            ('do', 'wait 300'),
            ('expect', 'talk=none'),                  # nothing to say of it
        ])


MANY = {
    'bn5': meets('bn5'),
    'bn4': meets('bn4'),
    'bn6-alone': alone((), '', "BN6's ROM alone: the same battle holds BN6's viruses alone, and MegaMan has no word on a visitor"),
    'extra-off': alone(('bn5', 'bn4'), ' --setup endless,0,0,0,0,32',
                       "The Older nets extra switched off, both ROMs read: no visitor in BN6's battles"),
}
