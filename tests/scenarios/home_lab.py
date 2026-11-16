"""The Battle Lab in Lan's HP (docs/game/battle-lab.md): its keeper opens
the builder, a foe is set on a panel and BATTLE! fights it inside a
snapshot of the game, so a test battle won, left or lost changes nothing
of the run (HP, zenny, BugFrags, the folder, the pack, the request), and
the run goes on."""

# (home before layer 7, the keeper there whatever the profile's battle
# data: --dev lab; one-hit enemies make the win short)
HOME = '--scene home --run-depth 7 --seed 3 --dev lab'
FREE = ('until', 'held=none', 600)
ARRIVED = [('until', 'chat=open', 900), ('repeat', 'mash A 60', 'chat=closed', 20), FREE]
RUN = 'hp zenny bugfrags folder chips unlockers subchips mail request portals ahead'
# (the keeper, from the cell before him: his first meeting's words, his
# question on Yes, and the builder over the game)
KEEPER = [
    ('do', 'place -76 -30 7; wait 10'),
    ('repeat', 'press A; wait 40', 'lab~builder', 30),
]
# (FOES' first family, Mettaur, lifted and set on the panel the cursor goes to; START is BATTLE!)
SET = [
    ('expect', 'lab=builder page foes foes 0 last none'),
    ('do', 'wait 30; press A; wait 20; press A; wait 60'),
    ('expect', 'lab=builder page foes foes 1 last none'),
    ('do', 'press START; wait 60'),
    ('until', 'screen=battle', 600),
    ('expect', 'lab=fighting'),
]
# (B goes back a level, and at a page's own place asks "Leave the Lab?", on Yes)
LEAVE = [
    ('do', 'wait 30'),
    ('repeat', 'press B; wait 30', 'lab~leaving', 6),
    ('do', 'press A; wait 60'),
    ('until', 'lab=keeper', 600), FREE,
    ('expect', 'screen=town'),
    ('expect', "place=Lan's HP"),
]


MANY = {
    'no-keeper-without-battle-data': dict(
        GUARDS="A first profile's Lan's HP has no Battle Lab: nobody stands at its place.",
        START='--scene home --run-depth 7 --seed 3',
        STEPS=ARRIVED + [('expect', 'lab=none')],
    ),
    'won-changes-nothing': dict(
        GUARDS="A test battle set up and won in the Battle Lab: the builder is back with its result, and after leaving the run is what it was, MegaMan free in Lan's HP.",
        START=HOME + ',onehit',
        STEPS=ARRIVED + [('expect', 'lab=keeper'), ('keep', RUN)] + KEEPER + SET + [
            ('until', 'turn=1', 600),                                   # its first Custom screen
            ('do', 'press A; wait 10; press START; wait 10; press A; wait 60'),    # a chip, OK
            ('do', 'mash B 120'),                                       # the buster: the Mettaur stands in his row
            ('repeat', 'press A; wait 10', 'lab~builder', 80),          # the RESULT paged
            ('expect', 'lab=builder page foes foes 1 last won'),
        ] + LEAVE + [('same', RUN)],
    ),
    'left-changes-nothing': dict(
        GUARDS="A test battle left with L on its Custom screen (in a test the way out never fails): the builder says so, and the run is what it was.",
        START=HOME,
        STEPS=ARRIVED + [('keep', RUN)] + KEEPER + SET + [
            ('until', 'turn=1', 600),
            ('do', 'wait 60; press L; wait 60'),
            ('repeat', 'press A; wait 40', 'lab~builder', 30),          # "Lan,should we run?" Yes, "We got away!"
            ('expect', 'lab=builder page foes foes 1 last left'),
        ] + LEAVE + [('same', RUN)],
    ),
    'deleted-costs-nothing': dict(
        GUARDS="MegaMan deleted in a test battle (one hit, --dev fragile): the builder is back, the run is not over, and his HP and all else are what they were.",
        START=HOME + ',fragile',
        STEPS=ARRIVED + [('keep', RUN)] + KEEPER + SET + [
            ('until', 'turn=1', 600),
            ('do', 'press START; wait 10; press A; wait 60'),           # OK with no chip, and he stands in the Mettaur's row
            ('until', 'lab~builder', 3000),
            ('expect', 'lab=builder page foes foes 1 last deleted'),
            ('expect', 'scene=emu'),
        ] + LEAVE + [('same', RUN), ('expect', 'hp=100/100')],
    ),
    'left-without-a-fight': dict(
        GUARDS="The builder opened and left with no battle fought: MegaMan free in Lan's HP, the run what it was, and the way on still his to take.",
        START=HOME,
        STEPS=ARRIVED + [('keep', RUN)] + KEEPER + [
            ('do', 'wait 30; press R; wait 30'),
            ('expect', 'lab~page field'),
            ('do', 'press L; wait 30'),
            ('expect', 'lab~page foes'),
        ] + LEAVE + [
            ('same', RUN),
            ('do', 'place 171 0 1; wait 10; hold UP+RIGHT 25'),
            ('until', 'layer=7', 900),
        ]),
}
