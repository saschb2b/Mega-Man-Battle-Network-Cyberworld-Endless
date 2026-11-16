"""The older nets' Navis as an act's guardian: BN5's six and Nebula's two,
BN4's twelve. The door, his fight in BN6's engine under his own name, his
deletion, his Guardian Data taken (five HPMemory, and the Soul of each of
BN5's six); and each of BN4's seen
facing MegaMan on the map from either side of his arena (0.22.0: BN4
draws four views where BN6 draws eight, and a guardian with MegaMan on his
left showed his back, or nothing at all)."""

# (--guardian N: guardians.h's numbers; BN4's 34 plus his seat)
# (a battle names him as his own game's battles do, eight letters at most)
IN_BATTLE = {'ShadowMan': 'ShadoMan', 'NumberMan': 'NumbrMan', 'TomahawkMan': 'TmhkMan', 'KnightMan': 'KnightMn', 'GutsMan': 'GutsM',
             'WindMan': 'WindM', 'SearchMan': 'SrchM', 'FireMan': 'FireM', 'ThunderMan': 'ThunM', 'JunkMan': 'JunkM', 'AquaMan': 'AquaM',
             'WoodMan': 'WoodM', 'TopMan': 'TopM', 'BurnerMan': 'BurnM', 'SparkMan': 'SprkM'}
BN5 = {24: 'Colonel', 25: 'ShadowMan', 26: 'NumberMan', 27: 'TomahawkMan', 28: 'KnightMan', 29: 'ToadMan'}
NEBULA = {65: 'LarkMan', 66: 'GridMan'}
BN4 = {34: 'Roll', 35: 'GutsMan', 36: 'WindMan', 37: 'SearchMan', 38: 'FireMan', 39: 'ThunderMan', 43: 'JunkMan', 44: 'AquaMan',
       45: 'WoodMan', 46: 'TopMan', 48: 'BurnerMan', 50: 'SparkMan'}


def start(biome, navi, seed, depth=3):
    return f'CYBERWORLD_AUTOPILOT=1 --scene emu --seed {seed} --run-depth {depth} --net-biome {biome} --guardian {navi} --dev onehit,god,quiet'


def fight(game, biome, navi, name, depth=3):
    """An act's last layer walked by the autopilot: his door, his fight, his data."""
    # (BN5's six give their Soul, a bit each from Colonel's; Nebula's two and BN4's have none to give)
    soul = 1 << (navi - 24) if navi in BN5 else 0
    return dict(
        START=start(biome, navi, 3, depth), NEEDS=(game,),
        GUARDS=f"{game.upper()}'s {name} guarding an act: fought in BN6's engine, deleted, his Guardian Data taken",
        STEPS=[
            ('expect', f'guardianfrom={game}'),
            ('expect', 'soulsheld=00'),
            ('until', 'guardianstands>=0', 600),      # the map's people in place: he in a view his sprite draws
            ('expect', 'guardianstands~shown|guardianstands~front'),
            ('until', 'guardian~fighting', 6000),
            ('until', 'enemies>=1', 1200),
            ('expect', f'enemynames~{IN_BATTLE.get(name, name)}'),   # himself, moved in from his own game
            ('expect', 'olderfoes>=1'),
            ('until', 'guardian~beaten', 6000),
            ('expect', f'guardian={name} beaten'),
            ('until', 'guardian~done', 6000),         # his Guardian Data taken
            ('expect', 'hp=200/200'),                 # its five HPMemory, and the full heal
            ('expect', f'soulsheld={soul:02x}'),      # ... and his Soul, where he has one
        ])


def stands(navi, name, side, seed):
    """BN4's on the map, MegaMan arriving on that side of him: a view his sprite draws, his face to the camera."""
    return dict(
        START=start('x15', navi, seed), NEEDS=('bn4',),
        GUARDS=f"BN4's {name} on his arena with MegaMan on his {side}: seen, and not from behind (0.22.0)",
        STEPS=[
            ('expect', 'guardianfrom=bn4'),
            ('until', 'guardianstands>=0', 600),      # the map's people in place
            ('expect', f'guardianstands~front-{side}'),
            ('until', 'talk=director', 6000),         # his door's words: he has logged in
            ('expect', f'guardianstands~front-{side}'),
        ])


MANY = {f'bn5/{name.lower()}': fight('bn5', 'x0', navi, name) for navi, name in BN5.items()}
MANY.update({f'bn5/{name.lower()}': fight('bn5', 'x5', navi, name, 15) for navi, name in NEBULA.items()})
MANY.update({f'bn4/{name.lower()}': fight('bn4', 'x15', navi, name) for navi, name in BN4.items()})
MANY.update({f'bn4-stands/{name.lower()}-{side}': stands(navi, name, side, seed) for navi, name in BN4.items() for side, seed in (('left', 3), ('right', 1))})
