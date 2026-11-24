"""MegaMan's look (issue #854, docs/game/looks.md): set by the dev switch
into the profile's worn slot, his palettes coloured where the frame draws
them (the state's `lookseen`: a chat box's mugshot as he answers on arrival, the field,
the Custom screen's face, a battle's body). HeatCross, the setup's Cross,
keeps BN6's own colours, and the look is back in the next battle without
it. A quit and CONTINUE, with no switch, wear it still."""

LOOK = '3.2,4.4,5.1'   # (ProtoMan's red armour, Nebula's violet suit, Tournament's gold)

START = f'--scene emu --seed 1 --run-depth 1 --setup short,0,0,0,1 --dev onehit,god,quiet,look={LOOK}'

BATTLE = [
    ('do', 'foes 01@52'),
    ('until', 'phase=custom', 1200),
    ('do', 'wait 60'),
    ('expect', 'lookseen~face'),         # the Custom screen's emotion window
]
FIGHT = [
    ('repeat', 'press START; wait 10; press A; wait 60', 'phase=fight'),
    ('repeat', 'wait 10', 'custom>0', 60),
]
WIN = [
    ('repeat', 'mash B 30', 'phase=results', 30),
    ('expect', 'result=won'),
    ('repeat', 'mash A 60; wait 30', 'screen=map', 20),
]

STEPS = [
    ('expect', f'look={LOOK} slot 0'),
    ('do', 'wait 150'),
    ('repeat', 'press A; wait 20', 'lookseen~mugshot', 40),   # MegaMan's words on arrival
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
    ('expect', 'lookseen~field'),
    # (HeatCross: his own colours, none of the look's)
    *BATTLE,
    ('do', 'press UP; wait 30; press A; wait 60'),
    *FIGHT,
    ('expect', 'form=HeatCross'),
    ('expect', 'lookseen=none'),
    *WIN,
    ('expect', 'lookseen~field'),
    # (his plain body again: the look's)
    *BATTLE,
    *FIGHT,
    ('expect', 'form=none'),
    ('expect', 'lookseen~battle'),
    *WIN,
    ('keep', 'look'),
    ('restart', '--scene title'),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=continue'),
    ('repeat', 'press A; wait 40', 'scene=emu'),
    ('until', 'screen=map', 1200),
    ('same', 'look'),
    ('expect', 'lookseen~field'),
]
