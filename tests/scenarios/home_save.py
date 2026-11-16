"""The PET's Save and a quit at home (docs/game/pet.md, Save;
docs/world/home/jobs.md): in Lan's room before the first layer there is no
run to continue yet; home after an act, Save keeps the run where Lan
stands, what he bought and the visit's order with it (a playtester's
purchase and request in the town were left unsaved, session 70); a quit
there without it goes back to the last save, whole."""

FREE = ('until', 'held=none', 600)
PAGED = ('repeat', 'press A; wait 20', 'chat=closed', 40)
ARRIVED = [('until', 'chat=open', 900), ('repeat', 'mash A 60', 'chat=closed', 20), FREE]
# (Lan's HP after act 2 with 20000 zenny; out by R's jack-out, down the
# stairs, out of the house and into AsterLand: home_town.py's ways)
HOME = '--scene home --run-depth 7 --seed 3 --dev zenny=20000'
TO_ASTER = [
    ('do', 'press R; wait 40'),
    ('repeat', 'press A; wait 30', 'choice=2/2'),
    ('do', 'press LEFT; wait 20'),
    ('repeat', 'press A; wait 40', "place=Lan's Room", 20), FREE,
    ('do', 'place -62 80 3; wait 10; hold DOWN 24'),
    ('until', "place=Lan's House", 600), FREE,
    ('do', 'place -78 55 1; wait 10; hold DOWN 30'),
    ('until', 'place=Central Town', 600), FREE,
    ('do', 'place -156 -40 7; wait 10; hold UP+LEFT 62'),
    ('until', 'place=AsterLand', 600), FREE,
]
# (the SubChip seller's first row, whatever the act ahead has him stock: its zenny spent)
BUY = [
    ('do', 'place -4 -36 7; wait 10'),
    ('repeat', 'press A; wait 40', 'screen=shop'),
    ('repeat', 'press A; wait 40', 'choice=1/2'),           # "..., right?", on Yes
    ('do', 'press A; wait 40; press A; wait 40'),               # Yes, "I bought ...!"
    ('repeat', 'press B; wait 40', 'choice=1/2'),           # "Want to leave?"
    ('do', 'press A; wait 40'),
    ('until', 'screen=town', 600), FREE,
    ('expect', 'zenny<20000'),
]
# (the PET's last row, Up from its first: Save closes the PET and its box says so)
SAVE = [
    ('do', 'press START; wait 40; press UP; wait 10; press A; wait 40'),
    ('until', 'chat=open', 600),
    PAGED, FREE,
]
CONTINUE = [
    ('restart', '--scene title'),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=continue'),
    ('repeat', 'press A; wait 40', 'scene=emu'),
    ('until', 'where=town', 1200),
    ('repeat', 'mash A 60', 'held=none', 20),
]
KEPT = 'place at zenny subchips unlockers order ahead portals hp folder'


MANY = {
    'room-before-the-first-layer': dict(
        GUARDS="The PET's Save in Lan's room at a run's start closes the PET with a note and no box (saves begin on layer 1), and a quit there leaves no run to continue.",
        START='room',
        STEPS=[
            ('do', 'press START; wait 40; press UP; wait 10; press A; wait 60'),
            ('until', 'screen=town', 600), FREE,
            ('expect', 'chat=closed'),
            ('restart', '--scene title'),
            ('repeat', 'press START; wait 40', 'title=menu'),
            ('expect', 'cursor=new'),
        ]),
    'saved-in-asterland': dict(
        GUARDS="Home after an act, the PET's Save in AsterLand: CONTINUE goes on there, what was bought and its zenny spent kept.",
        START=HOME,
        STEPS=ARRIVED + TO_ASTER + BUY + SAVE + [('keep', KEPT)] + CONTINUE + [
            ('same', KEPT),
            ('expect', 'place=AsterLand'),
            ('expect', 'zenny<20000'),
        ]),
    'quit-without-save': dict(
        GUARDS="Home after an act, a quit in AsterLand with no Save (at home a quit keeps the last save, not where Lan stands): CONTINUE goes on from Lan's HP's arrival whole, what was bought gone and its zenny back, never one without the other.",
        START=HOME,
        STEPS=ARRIVED + [('keep', 'zenny subchips unlockers order chips')] + TO_ASTER + BUY + CONTINUE + [
            ('expect', "place=Lan's HP"),
            ('same', 'zenny subchips unlockers order chips'),
        ]),
    'order-kept': dict(
        GUARDS="The visit's one order stays made through a Save and CONTINUE: the clerk takes no second one after it.",
        START='--scene home --run-depth 7 --seed 3 --dev zenny=20000',
        STEPS=ARRIVED + TO_ASTER + [
            ('do', 'place 23 -40 7; wait 10'),
            ('repeat', 'press A; wait 40', 'screen=shop'),
            ('repeat', 'press A; wait 40', 'choice=1/2'),
            ('repeat', 'press A; wait 40', 'order=made', 20),
            ('repeat', 'press B; wait 40', 'screen=town', 20),
            PAGED, FREE,
        ] + SAVE + [('keep', 'order zenny chips')] + CONTINUE + [
            ('same', 'order zenny chips'),
            ('expect', 'order=made'),
            ('do', 'place 23 -40 7; wait 10; press A; wait 40'),
            ('expect', 'chat=open'),
            PAGED, FREE,
            ('expect', 'screen=town'),
            ('same', 'order zenny chips'),
        ]),
}
