"""Team ProtoMan's partners as an act's guardian by the run's own seed, no
--guardian (issue #774, docs/game/team-protoman.md, Where they stand):
from act 2, BN5's area's guardian is his Team Colonel Navi's partner in
half the runs. Seed 23's act 2 is Oran Area's, its guardian GyroMan where
ShadowMan stood before; seed 16's ACDC Area's, MagnetMan where KnightMan
did. From act 5 the deep's pool holds them too, Colonel's partner
DarkProto (BN5's ProtoMan, every version named DarkProt as BN5 names his
V2) among them: seed 4's act 5. Each fought in BN6's engine and deleted,
his Guardian Data taken gives his own Soul, BN5's Soul 1, 2 or 5, and none
of Team Colonel's."""

# (seed, his name as the game shows him and as BN5 names him in battle, BN5's Soul of his in souls_held_bn5's bits, the
# act's guardian layer: act 2's 6, act 5's 15)
# (DarkProto's with BN4's ROM beside, as the deep's pool by which seed 4 picks him holds BN4's Navis too, and MegaMan's max HP
# 600, more than his one slash takes, which god mode's refill after the frame cannot answer)
PARTNERS = {'gyroman': (23, 'GyroMan', 'GyroMan', 0x002, 6), 'magnetman': (16, 'MagnetMan', 'MagnetM', 0x010, 6),
            'darkproto': (4, 'DarkProto', 'DarkProt', 0x001, 15, ('bn5', 'bn4'), ',hp=600')}


def fight(seed, name, shown, soul, depth, needs=('bn5',), dev=''):
    return dict(
        START=f'CYBERWORLD_AUTOPILOT=1 --scene emu --seed {seed} --run-depth {depth} --dev onehit,god,quiet{dev}', NEEDS=needs,
        GUARDS=f"{name}, his partner's act {(depth - 1) // 3 + 1} guardian by seed {seed}: fought, deleted, his Guardian Data his own Soul",
        STEPS=[
            ('expect', 'guardianfrom=bn5'),
            ('expect', 'soulsbn5=000 000'),
            ('until', 'guardian~fighting', 6000),
            ('until', 'foes>=1', 1200),
            ('expect', f'enemynames~{shown}'),       # himself, moved in from BN5
            ('until', 'guardian~beaten', 6000),
            ('expect', f'guardian={name} beaten'),
            ('until', 'guardian~done', 6000),        # his Guardian Data taken
            ('expect', 'data=taken'),
            ('expect', f'soulsbn5~{soul:03x} '),     # ... his own Soul
            ('expect', 'soulsheld=00'),              # ... and none of Team Colonel's
        ])


MANY = {k: fight(*v) for k, v in PARTNERS.items()}
