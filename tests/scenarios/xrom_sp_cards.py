"""Issue 835: an older net's Navi chip SP in the PET's Folder Edit, its card's
power the one its own game's card shows for a save without a deletion time
(the last of its row: BN5's table 0x0801D27A, BN4's 0x08018B48, each save's
times 0xFFFF until a deletion, BN5 0x080092E0, BN4 0x08007FFA), and the same
power in the hand in a battle. CosmoMn SP's card read 6544: BN6's card reads
a power past 1000 as one of its own Navis' by his time, and the moved SPs'
powers are entries of BN6's grown table past them
(docs/multirom/chips-code.md, Computed powers)."""
import cw_routes

# (Lan's room at a new run, the PET's menu open; ChipFolder its first row)
TOWN = '--scene town --seed 2'

# (chip: its id in BN6's tables, a code its record has, the power its own game's card shows with no time)
BN5 = {'NumbrMnSP': (490, 'N', 40), 'ShadoMnSP': (494, 'S', 45), 'TmhwkMnSP': (497, 'T', 180), 'KnigtMnSP': (500, 'K', 220),
       'ToadManSP': (503, 'T', 120), 'ShadeMnSP': (506, 'S', 140), 'BlizManSP': (509, 'B', 130), 'CloudMnSP': (513, 'C', 60),
       'CosmoMnSP': (516, 'C', 80), 'LarkManSP': (519, 'S', 170), 'GridManSP': (522, 'F', 150)}
BN4 = {'GutsMnSP': (536, 'G', 100)}


def card(chip, code, power):
    """The chip first in the folder, Folder Edit opened on it, its card's power read, and back to the PET's menu."""
    return [
        ('do', f'chip {chip} {code} 1; wait 5; press A; wait 60'),
        ('expect', 'screen=folders'),
        ('repeat', 'press A; wait 40', 'screen=folder-edit'),
        ('until', f'cardchip={chip}', 300),
        ('expect', f'cardpower={power}'),
        ('repeat', 'press B; wait 40', 'screen=pet'),
    ]


def cards(game, chips):
    return dict(
        START=TOWN, NEEDS=(game,),
        GUARDS=f"Issue 835: {game.upper()}'s Navi chips' SP in Folder Edit, each card's power {game.upper()}'s own for a save without a time",
        STEPS=[('do', cw_routes.PET), ('expect', 'screen=pet')] + [s for c in chips.values() for s in card(*c)])


# (the battle: the target and the first layer as tests/scenarios/xrom_chips.py has them)
START = 'layer --seed 15 --net-biome 0 --dev god,quiet,hp=2000'
TARGET = '205@52'


def hand(game, chip, code, power):
    return dict(
        START=START, NEEDS=tuple({game, 'bn5'}),
        GUARDS=f"Issue 835: {game.upper()}'s SP {chip} in a battle's hand: its power {power}, the card's",
        STEPS=[
            ('do', f'chip {chip} {code} 29; foes {TARGET}'),
            ('until', f'drawn~{chip}{code}', 900),
            ('repeat', 'press A; wait 10', 'handpicks=1'),
            ('do', 'press START; wait 10; press A'),
            ('until', 'hand=1', 600),
            ('expect', f'nextchip={chip}'),
            ('expect', f'nextpower={power}'),
        ])


MANY = {'bn5/cards': cards('bn5', BN5), 'bn4/cards': cards('bn4', BN4),
        'bn5/cosmomnsp-hand': hand('bn5', *BN5['CosmoMnSP']), 'bn4/gutsmnsp-hand': hand('bn4', *BN4['GutsMnSP'])}
