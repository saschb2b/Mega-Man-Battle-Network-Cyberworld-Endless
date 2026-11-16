"""A guardian from his arena's door to the way on, by the autopilot: the
run saved at the door, his title card naming him, his words, the fight,
his last word and log-out, his Guardian Data (five HPMemory, MegaMan's HP
restored, the program draft), the exit pad opening only once it is taken,
and the pad home to Lan's HP after the act."""

# (onehit: a short fight; quiet: no random battle on the way to his arena)
START = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --guardian 1 --dev onehit,quiet'

STEPS = [
    ('expect', 'guardian=??? waiting'),       # a first profile: a strong signal, no name
    ('expect', 'known=no'),
    ('expect', 'exitpad=shut'),
    ('until', 'card=title', 3000),            # in his arena: his card
    ('expect', 'cardsays~HeatMan'),
    ('expect', 'guardian=HeatMan waiting'),
    ('until', 'chat=open', 600),              # his words before the battle
    ('until', 'battle=guardian', 1200),
    ('expect', 'guardian=HeatMan fighting'),
    ('until', 'guardian=HeatMan beaten', 4000),
    ('expect', 'navis=1'),
    ('expect', 'viruses=0'),                  # a Navi is no virus of the tally
    ('expect', 'known=yes'),                  # battle data on him, from here on
    ('expect', 'data=hidden'),
    ('until', 'chat=open', 600),              # his last word
    ('until', 'data=fresh|data=waiting', 600),
    ('expect', 'exitpad=shut'),               # not before his data is taken
    ('until', 'screen=reward-pick', 2000),
    ('expect', 'pick=draft'),
    ('expect', 'hp=200/200'),                 # five HPMemory, and his HP restored
    ('until', 'data=taken', 1200),
    ('until', 'exitpad=open', 120),
    ('expect', 'guardian=HeatMan done'),
    ('until', 'where=town', 1200),            # the act's end: home
    ('until', "place=Lan's HP", 600),
    ('expect', 'hp=200/200'),
]
