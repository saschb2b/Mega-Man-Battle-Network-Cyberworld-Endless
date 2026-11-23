"""Team ProtoMan's partners as an act's guardian by the run's own seed, no
--guardian (issue #774, docs/game/team-protoman.md, Where they stand):
from act 2, BN5's area's guardian is his Team Colonel Navi's partner in
half the runs. Seed 23's act 2 is Oran Area's, its guardian GyroMan where
ShadowMan stood before; seed 16's ACDC Area's, MagnetMan where KnightMan
did. Each fought in BN6's engine and deleted, his Guardian Data taken
gives his own Soul, BN5's Soul 2 or 5, and none of Team Colonel's."""

# (seed, his name as the game shows him and as BN5 names him in battle, BN5's Soul of his in souls_held_bn5's bits)
PARTNERS = {'gyroman': (23, 'GyroMan', 'GyroMan', 0x002), 'magnetman': (16, 'MagnetMan', 'MagnetM', 0x010)}


def fight(seed, name, shown, soul):
    return dict(
        START=f'CYBERWORLD_AUTOPILOT=1 --scene emu --seed {seed} --run-depth 6 --dev onehit,god,quiet', NEEDS=('bn5',),
        GUARDS=f"{name}, his partner's area's act 2 guardian by seed {seed}: fought, deleted, his Guardian Data his own Soul",
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
