"""A CONTINUE around a guardian. Issue #614: continued at his Guardian
Data, BN4's GutsMan logged in again, unseen, and stood on it, and no A
took it; a guardian deleted before a CONTINUE stays gone, and his data is
taken, the exit opening (BN6's HeatMan too). And 0.22.0's CONTINUE at his
door, which forgot him: after his title card, the act's card read
"Guardian: ???" again; it names him."""

# (the autopilot to his data: a quit as it shows, CONTINUE, and on; the
# game's options again, as a test start's area and guardian are its own)


def data(options):
    return [
        ('until', 'guardian~beaten', 6000),
        ('keep', 'guardian area'),
        ('repeat', 'wait 10', 'data=fresh|data=waiting', 60),
        ('restart', f'CYBERWORLD_AUTOPILOT=1 --scene title {options}'),
        ('repeat', 'press START; wait 40', 'title=menu'),
        ('expect', 'cursor=continue'),
        ('repeat', 'press A; wait 40', 'scene=emu'),
        ('until', 'screen=map', 2400),
        ('same', 'guardian area'),            # gone, and his data there
        ('expect', 'data=fresh|data=waiting'),
        ('expect', 'navis=1'),
        ('until', 'exitpad=open', 3000),      # taken: the way on
        ('expect', 'data=taken'),
        ('expect', 'hp=200/200'),
        ('until', 'where=town', 1200),
    ]


# (a quit in his fight comes back to the arena's door, where the run was
# saved as MegaMan stepped in)
DOOR = [
    ('expect', 'guardian=??? waiting'),
    ('until', 'battle=guardian', 4000),
    ('restart', 'CYBERWORLD_AUTOPILOT=1 --scene title --guardian 1 --dev onehit,quiet'),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=continue'),
    ('repeat', 'press A; wait 40', 'scene=emu'),
    ('repeat', 'wait 10', 'card=area', 120),  # the act's card, before his own
    ('expect', 'cardsays~Guardian: HeatMan /'),
    ('until', 'battle=guardian', 2400),       # and he is met again at once
    ('until', 'guardian=HeatMan beaten', 4000),
]

MANY = {
    'data/heatman': dict(START='CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --guardian 1 --dev onehit,quiet',
                         STEPS=data('--guardian 1 --dev quiet')),
    # (BN4's ACDC Area, which run seed 6 has GutsMan guard, by the run's own
    # coin: --guardian's is a test start's and no CONTINUE's; the weak
    # autopilot ends his fight)
    'data/gutsman': dict(START='CYBERWORLD_AUTOPILOT=weak --scene emu --seed 6 --run-depth 3 --net-biome x15 --dev quiet',
                         STEPS=[('expect', 'area=ACDC Area'), ('until', 'guardian=GutsMan fighting', 4000)] + data('--net-biome x15 --dev quiet'),
                         NEEDS=('bn4',)),
    'door': dict(START='CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --guardian 1 --dev onehit,quiet', STEPS=DOOR),
}
