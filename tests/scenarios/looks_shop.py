"""MegaMan's colors at AsterLand's counter (issue #854; docs/game/looks-earning.md,
Earning; docs/world/home/home-shop.md): the clerk offers the home's
palettes the profile lacks beside his orders, a sale takes its price and
grants the palette at once, the got box says so, and the palette is not
offered again."""

# (Lan's room to AsterLand, as home_town.py walks it)
FREE = ('until', 'held=none', 600)
PAGED = ('repeat', 'press A; wait 20', 'chat=closed', 40)
ASTER = [
    ('do', 'place -62 80 3; wait 10; hold DOWN 24'),
    ('until', "place=Lan's House", 600), FREE,
    ('do', 'place -78 55 1; wait 10; hold DOWN 30'),
    ('until', 'place=Central Town', 600), FREE,
    ('do', 'place -156 -40 7; wait 10; hold UP+LEFT 62'),
    ('until', 'place=AsterLand', 600), FREE,
]


def dev(switches):
    return f'room --dev {switches}'

CLERK = ('do', 'place 23 -40 7; wait 10')
# (his greeting paged to his two services, the cursor on the order)
SERVICES = ('repeat', 'press A; wait 40', 'choice=1/2', 20)
CENTRAL = 1 << 7
# (... and down to MegaMan's colors, the list of three: the cursor moves
# once the options have settled)
COLORS = [('do', 'wait 20; hold DOWN 6; wait 20'), ('expect', 'choice=2/2'), ('do', 'press A; wait 40'), ('expect', 'choice=1/3')]

MANY = {
    'sells': dict(
        GUARDS="AsterLand's clerk sells the Central Town colors: 6000 zenny taken, the palette owned and said, the clerk's colors then two.",
        START=dev('zenny=20000'),
        STEPS=ASTER + [
            ('expect', 'looksowned=1'),
            CLERK, SERVICES,
            *COLORS,                                # (Central Town, Seaside, Sky)
            ('do', 'press A; wait 60'),                             # Central Town: his question, on Yes
            ('repeat', 'press A; wait 40', f'looksowned={CENTRAL | 1}', 8),
            PAGED, FREE,
            ('expect', 'zenny=14000'),
            ('expect', f'lookstold={CENTRAL}'),
            # (no word of MegaMan's after: the got box was the sale's own)
            ('do', 'wait 90'),
            ('expect', 'talk=none'),
            # (the clerk again this visit: Central Town his word alone, no second sale)
            CLERK, SERVICES, *COLORS,
            ('do', 'press A; wait 60'),
            PAGED, FREE,
            ('expect', 'zenny=14000'),
            ('expect', f'looksowned={CENTRAL | 1}'),
        ]),
    'broke': dict(
        GUARDS="AsterLand's clerk with too little zenny: no palette, no zenny taken, his word.",
        START=dev('zenny=5000'),
        STEPS=ASTER + [
            CLERK, SERVICES, *COLORS,
            ('do', 'press A; wait 60'),
            ('repeat', 'press A; wait 40', 'chat=closed', 12),
            FREE,
            ('expect', 'looksowned=1'),
            ('expect', 'zenny=5000'),
        ]),
    'all-owned': dict(
        GUARDS="With the home's three palettes owned, the clerk's talk is his orders alone, as before colors.",
        START=dev(f'zenny=20000,looks={(1 << 7) | (1 << 8) | (1 << 9)}'),
        STEPS=ASTER + [
            CLERK,
            ('repeat', 'press A; wait 40', 'screen=shop', 20),
            ('repeat', 'press B; wait 40', 'screen=town', 20),
            PAGED, FREE,
            ('expect', 'zenny=20000'),
        ]),
}
