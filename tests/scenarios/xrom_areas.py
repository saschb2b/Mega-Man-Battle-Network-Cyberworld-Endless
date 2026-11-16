"""Each area of the older nets as a run's layer (BN5's six, BN4's five):
arrived on under its own name, a random battle fought and won there (on
BN5's ground one of BN5's own records, in BN6's engine), and its exit
found and taken by the autopilot."""

# (the area as --net-biome names it: its name on the layer, its game)
BN5 = {'x0': 'ACDC Area', 'x1': 'SciLab Area', 'x2': 'End Area', 'x3': 'Nebula Area', 'x4': 'Oran Area', 'x5': 'the Undernet'}
BN4 = {'x15': 'ACDC Area', 'x16': 'Town Area', 'x17': 'Park Area', 'x18': 'Net countries', 'x19': 'the Undernet'}


def course(biome, name, game):
    """One area's: the layer, a battle of its ground, the way on."""
    return dict(
        START=f'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --net-biome {biome} --dev onehit,quiet',
        NEEDS=(game,),
        GUARDS=f"{game.upper()}'s {name} ({biome}) as a layer: its name, a battle won on its ground, its exit taken",
        STEPS=[
            ('expect', 'layer=1'),
            ('expect', f'area={name}'),
            ('expect', f'areafrom={game}'),
            ('until', 'chat=open', 900),              # the arrival's words, which the autopilot pages
            ('until', 'chat=closed', 1800),
            ('do', 'battle'),                         # the layer's next random battle, quiet or not
            ('until', 'enemies>=1', 1200),            # its foes on the field
            # (BN5's ground fights BN5's own records; BN4's has none of its own yet: BN6's)
            ('expect', 'olderrecord!=none' if game == 'bn5' else 'olderrecord=none'),
            ('repeat', 'wait 10', 'enemies=0', 400),  # every one deleted (the RESULT is short: looked at often)
            ('expect', 'screen=battle'),
            ('until', 'screen=map', 1800),            # its RESULT paid, back on the layer
            ('expect', f'area={name}'),
            ('until', 'layer>=2', 20000),             # walked to its exit
            ('until', 'screen=map', 1200),
            ('expect', f'areafrom={game}'),           # the act's next layer, the same ground
        ])


MANY = {f'{game}/{biome}': course(biome, name, game) for game, areas in (('bn5', BN5), ('bn4', BN4)) for biome, name in areas.items()}
