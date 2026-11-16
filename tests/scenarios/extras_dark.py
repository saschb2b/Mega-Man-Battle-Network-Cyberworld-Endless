"""DarkChips (docs/game/meta/darkchips.md, bn6-darkchips.md): the flame of
darkness on an act's middle layer, its DarkChip taken and refused; a
DarkChip used in a battle with a BugFrag to burn and without one, and its
price of 20 max HP after, once a battle, for the rest of the run; what the
next run keeps of it (nothing but what MegaMan was taught); and no flame
with the setup's DarkChips switched off."""

ARRIVE = [('do', 'wait 150; mash A 300; wait 20'), ('repeat', 'mash A 60; wait 30', 'chat=closed')]
# (the flame's question starts on No, its options a second after it: LEFT goes round to Yes)
YES = 'press LEFT; press A; wait 20'
NO = 'press A; wait 20'
# (the Custom screen, once it has opened: the hand's first chip picked, or its first two, then OK, START on it)
PICK = 'wait 90; press A; wait 20; press START; wait 10; press A'
PICK_TWO = 'wait 90; press A; wait 20; press RIGHT; wait 10; press A; wait 20; press START; wait 10; press A'
# (the dev menu's "Win this battle", its fifth row)
# (A uses the chip in hand: until its BugFrags are burnt, or once where it burns none)
BURN = [('repeat', 'press A; wait 40', 'bugfrags=0', 20), ('do', 'wait 60')]
ONCE = [('do', 'wait 20; press A; wait 100')]
WIN = 'hold SELECT 4; hold SELECT+R 4; wait 6; press DOWN; press DOWN; press DOWN; press DOWN; press A; wait 60'
TITLE_CONTINUE = [
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=continue'),
    ('repeat', 'press A; wait 40', 'scene=emu'),
    ('until', 'screen=map', 2400),
]
# (an act's middle layer, where its flame burns; seed 3's first holds DrkSword)
FLAME = '--scene emu --seed 3 --run-depth 2 --dev quiet'
# (a run holding DrkSword, every chip of its folder one, on a layer with no flame)
HELD = '--scene emu --seed 3 --run-depth 4 --dev quiet,darkchips=0x2,folder=286'


def battle(pick, use):
    """A battle begun, DrkSword picked and used (`use`: the steps that use it), then won: the steps to the map."""
    return [
        ('do', 'battle'),
        ('until', 'turn>=1', 1200),
        ('do', pick),
        ('until', 'custom>0', 600),                           # the fight runs
        *use,
        ('do', WIN),
        ('repeat', 'press A; wait 40', 'doing=map', 30),      # ENEMY DELETED!, the results
    ]


MANY = {
    'flame-taken': dict(
        GUARDS="The flame of darkness's Yes: the run holds its DarkChip, the chip in the Pack, the flame gone, the profile "
               'taught; a quit and CONTINUE on its layer gives nothing twice.',
        START=FLAME + ' --talk dark:800',
        STEPS=[
            *ARRIVE,
            ('expect', 'flame=burning'),
            ('expect', 'flamechip=DrkSword'),
            ('expect', 'flamenet=bn6'),
            ('expect', 'darkchips=0'),
            ('expect', 'darktaught=none'),
            ('until', 'chat=open', 900),
            ('repeat', YES, 'flame=taken', 60),
            ('repeat', 'press A; wait 30', 'chat=closed', 30),
            ('until', 'darkpack=1', 600),
            ('expect', 'darkchips=1'),
            ('expect', 'darktaught=flame'),
            ('expect', 'basehp=100'),                 # holding one costs nothing
            ('restart', '--scene title'),
            *TITLE_CONTINUE,
            ('expect', 'layer=2'),
            ('expect', 'flame=taken'),
            ('expect', 'darkchips=1'),
            ('do', 'wait 300'),
            ('expect', 'darkpack=1'),
        ]),
    'flame-refused': dict(
        GUARDS="The flame's No leaves it burning and the run without its chip; a quit and CONTINUE finds the same flame; "
               'a later talk asks again, and its Yes gives the chip.',
        START=FLAME + ' --talk dark:800',
        STEPS=[
            *ARRIVE,
            ('keep', 'flamechip'),
            ('until', 'chat=open', 900),
            ('repeat', NO, 'chat=closed', 60),
            ('do', 'wait 120'),
            ('expect', 'flame=burning'),
            ('expect', 'darkchips=0'),
            ('expect', 'darkpack=0'),
            ('expect', 'darktaught=none'),
            ('restart', '--scene title --dev quiet --talk dark:300'),
            *TITLE_CONTINUE,
            ('expect', 'flame=burning'),
            ('same', 'flamechip'),
            ('until', 'chat=open', 900),              # talked to again
            ('repeat', YES, 'flame=taken', 60),
            ('repeat', 'press A; wait 30', 'chat=closed', 30),
            ('until', 'darkpack=1', 600),
            ('expect', 'darkchips=1'),
        ]),
    'flame-off': dict(
        GUARDS="The setup's DarkChips switched off: no flame of darkness on an act's middle layer.",
        START=FLAME + ' --setup endless,0,0,0,0,16',
        STEPS=[
            *ARRIVE,
            ('expect', 'layer=2'),
            ('expect', 'flame=none'),
        ]),
    'flame-next-act': dict(
        GUARDS="A later act's flame holds a DarkChip the run lacks, never one it holds.",
        START='--scene emu --seed 3 --run-depth 5 --dev quiet,darkchips=0x2',
        STEPS=[
            *ARRIVE,
            ('expect', 'flame=burning'),
            ('expect', 'flamechip!=DrkSword'),
            ('expect', 'darkchips=1'),
        ]),
    'flame-none-left': dict(
        GUARDS="A run holding all four of BN6's DarkChips meets no flame.",
        START='--scene emu --seed 3 --run-depth 5 --dev quiet,darkchips=0xC6',
        STEPS=[
            *ARRIVE,
            ('expect', 'darkchips=4'),
            ('expect', 'flame=none'),
        ]),
    'used': dict(
        GUARDS="A DarkChip's dark power used in a battle: its BugFrag burnt, and after the battle 20 max HP gone, MegaMan "
               'saying so; a quit and CONTINUE keeps the price.',
        START=HELD + ' --talk bugfrag:600',
        STEPS=[
            *ARRIVE,
            ('until', 'bugfrags=1', 900),
            ('expect', 'hp=100/100'),
            ('expect', 'basehp=100'),
            *battle(PICK, BURN),
            ('expect', 'bugfrags=0'),
            ('expect', 'basehp=80'),
            ('expect', 'hp~/80'),
            ('expect', 'darktaught=price'),
            ('until', 'talk=director', 600),          # what it took, in MegaMan's words
            ('repeat', 'press A; wait 30', 'chat=closed', 30),
            ('restart', '--scene title'),
            *TITLE_CONTINUE,
            ('expect', 'basehp=80'),
            ('expect', 'hp~/80'),
        ]),
    'used-twice': dict(
        GUARDS="Two DarkChips used in one battle: two BugFrags burnt, the price paid once.",
        START=HELD + ' --talk bugfrag:600,bugfrag:660',
        STEPS=[
            *ARRIVE,
            ('until', 'bugfrags=2', 900),
            *battle(PICK_TWO, BURN),
            ('expect', 'bugfrags=0'),
            ('expect', 'basehp=80'),
        ]),
    'used-without-a-bugfrag': dict(
        GUARDS="A DarkChip used with no BugFrag to burn runs as its base chip: no price, and MegaMan says what it was.",
        START=HELD,
        STEPS=[
            *ARRIVE,
            ('expect', 'bugfrags=0'),
            *battle(PICK, ONCE),
            ('expect', 'basehp=100'),
            ('expect', 'hp~/100'),
            ('expect', 'darktaught=none'),
            ('until', 'talk=director', 600),          # "... was only a Sword."
        ]),
    'held-unused': dict(
        GUARDS="A battle won without the DarkChip in the hand used costs nothing.",
        START=HELD + ' --talk bugfrag:600',
        STEPS=[
            *ARRIVE,
            ('until', 'bugfrags=1', 900),
            *battle(PICK, []),
            ('expect', 'bugfrags=1'),
            ('expect', 'basehp=100'),
        ]),
    'next-run': dict(
        GUARDS="The next run remembers a DarkChip's price by what the profile was taught alone: the run that paid it over, "
               'a new one begins with no DarkChip and its max HP whole.',
        START=HELD + ' --talk bugfrag:600',
        STEPS=[
            *ARRIVE,
            ('until', 'bugfrags=1', 900),
            *battle(PICK, BURN),
            ('expect', 'basehp=80'),
            ('until', 'talk=director', 600),
            ('repeat', 'press A; wait 30', 'chat=closed', 30),
            # (the run continued by the autopilot at 1 HP in battle: its next battle ends it)
            ('restart', 'CYBERWORLD_AUTOPILOT=1 --scene title --dev fragile'),
            ('repeat', 'press START; wait 40', 'title=menu'),
            ('repeat', 'press A; wait 40', 'scene=emu'),
            ('until', 'scene=title', 20000),
            ('expect', 'title=summary'),
            ('repeat', 'press A; wait 40', 'title=start'),
            ('repeat', 'press START; wait 40', 'title=menu'),
            ('expect', 'cursor=new'),
            ('repeat', 'press A; wait 40', 'title=setup|title=leaving|scene=emu'),
            ('repeat', 'press START; wait 60', 'scene=emu'),
            ('until', 'where=layer', 20000),
            ('expect', 'layer=1'),
            ('expect', 'darkchips=0'),
            ('expect', 'darkpack=0'),
            ('expect', 'basehp>=100'),
            ('expect', 'darktaught=price'),
        ]),
}
