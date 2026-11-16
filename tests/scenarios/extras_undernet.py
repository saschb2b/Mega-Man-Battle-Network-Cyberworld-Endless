"""The Undernet's door and what lies behind it (docs/GUIDE.md, Places to
find; docs/game/bosses/super-bosses.md): a dark warp on a layer taken and
refused, the Undernet's layer and its exit a layer deeper on the run's
own; its golden gate shut without three ScrtData, opened and refused with
them; the Secret Area behind it, its guardian beaten, and its exit back
to the run's own layers."""

ARRIVE = [('do', 'wait 150; mash A 300; wait 20'), ('repeat', 'mash A 60; wait 30', 'chat=closed')]
# (the dark warp's question starts on No: LEFT goes round to Yes)
YES = 'press LEFT; press A; wait 20'
NO = 'press A; wait 20'
# (the golden gate's starts on Yes while no super boss is known behind it, and on No once Bass waits there)
OTHER = 'press LEFT; press A; wait 20'
FIRST = 'press A; wait 20'
# (the run quit and continued with the autopilot, which walks to the exit pad; quiet: no random battles)
CONTINUE = [
    ('restart', 'CYBERWORLD_AUTOPILOT=1 --scene title --dev quiet'),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=continue'),
    ('repeat', 'press A; wait 40', 'scene=emu'),
    ('until', 'screen=map', 2400),
]
# (seed 9's fourth layer holds a dark warp by its seed)
WARP = '--scene emu --seed 9 --run-depth 4 --dev quiet'
# (a run begun on the Undernet's layer, its golden gate there)
UNDERNET = '--scene emu --seed 9 --run-depth 4 --dev quiet,side=1'

MANY = {
    'warp-taken': dict(
        GUARDS="A dark warp's Yes: the Undernet's layer at the same depth, its golden gate there; a quit and CONTINUE keeps "
               "it; its exit leads a layer deeper, on the run's own.",
        START=WARP + ' --talk undernet:800',
        STEPS=[
            *ARRIVE,
            ('expect', 'darkwarp=here'),
            ('expect', 'sidelayer=none'),
            ('expect', 'layer=4'),
            ('expect', 'area!=the Undernet'),
            ('until', 'chat=open', 900),
            ('repeat', YES, 'sidelayer=undernet', 30),
            ('until', 'chat=open', 1200),             # the Undernet's arrival
            ('repeat', 'mash A 60; wait 30', 'chat=closed'),
            ('until', 'screen=map', 600),
            ('expect', 'layer=4'),
            ('expect', 'area=the Undernet'),
            ('expect', 'goldgate=here'),
            *CONTINUE,
            ('expect', 'sidelayer=undernet'),
            ('expect', 'area=the Undernet'),
            ('until', 'layer=5', 20000),              # the autopilot to its exit
            ('expect', 'sidelayer=none'),
            ('expect', 'area!=the Undernet'),
        ]),
    'warp-refused': dict(
        GUARDS="A dark warp's No, and a later talk's: MegaMan stays on the run's layer, whose own exit leads on.",
        START=WARP + ' --talk undernet:800,undernet:1400',
        STEPS=[
            *ARRIVE,
            ('until', 'chat=open', 900),
            ('repeat', NO, 'chat=closed', 30),
            ('expect', 'sidelayer=none'),
            ('until', 'chat=open', 900),
            ('repeat', NO, 'chat=closed', 30),
            ('do', 'wait 120'),
            ('expect', 'sidelayer=none'),
            ('expect', 'darkwarp=here'),
            ('expect', 'screen=map'),
            *CONTINUE,
            ('until', 'layer=5', 20000),
            ('expect', 'sidelayer=none'),
        ]),
    'gate-shut': dict(
        GUARDS="The golden gate without three ScrtData: its words, no way in.",
        START=UNDERNET + ' --talk gate:800',
        STEPS=[
            *ARRIVE,
            ('expect', 'goldgate=here'),
            ('expect', 'scrtdata=0'),
            ('until', 'chat=open', 900),
            ('repeat', FIRST, 'chat=closed', 30),     # no question: its words alone
            ('do', 'wait 120'),
            ('expect', 'sidelayer=undernet'),
            ('expect', 'screen=map'),
        ]),
    'gate-refused': dict(
        GUARDS="The golden gate's No with three ScrtData held: they are kept, MegaMan stays in the Undernet.",
        START=UNDERNET + ' --talk scrtdata:600,gate:800',
        STEPS=[
            *ARRIVE,
            ('until', 'scrtdata=3', 900),
            ('until', 'chat=open', 900),
            ('repeat', OTHER, 'chat=closed', 30),     # No
            ('do', 'wait 120'),
            ('expect', 'sidelayer=undernet'),
            ('expect', 'scrtdata=3'),
        ]),
    'gate-bass-refused': dict(
        GUARDS="With the title's S held Bass waits behind the golden gate, and its question starts on No: an A paged "
               'through its words opens nothing.',
        START=UNDERNET + ' --marks 08 --talk scrtdata:600,gate:800',
        STEPS=[
            *ARRIVE,
            ('until', 'scrtdata=3', 900),
            ('until', 'chat=open', 900),
            ('repeat', FIRST, 'chat=closed', 30),     # No, the cursor's own
            ('do', 'wait 120'),
            ('expect', 'sidelayer=undernet'),
            ('expect', 'scrtdata=3'),
        ]),
    'gate-opened': dict(
        GUARDS="The golden gate's Yes with three ScrtData: the Secret Area at the same depth, a guardian waiting in it.",
        START=UNDERNET + ' --talk scrtdata:600,gate:800',
        STEPS=[
            *ARRIVE,
            ('until', 'scrtdata=3', 900),
            ('until', 'chat=open', 900),
            ('repeat', FIRST, 'sidelayer=secret', 30),   # Yes, the cursor's own
            ('until', 'chat=open', 1200),             # the Secret Area's arrival
            ('repeat', 'mash A 60; wait 30', 'chat=closed'),
            ('until', 'screen=map', 600),
            ('expect', 'layer=4'),
            ('expect', 'area~Secret'),
            ('expect', 'guardian~waiting'),
            ('expect', 'goldgate=none'),
        ]),
    'secret-area-cleared': dict(
        GUARDS="The Secret Area's guardian beaten and its Guardian Data taken, by the autopilot: its exit leads a layer "
               "deeper, back on the run's own layers.",
        START='CYBERWORLD_AUTOPILOT=1 --scene emu --seed 9 --run-depth 4 --dev quiet,god,onehit,hp=1000,side=2',
        STEPS=[
            ('expect', 'sidelayer=secret'),
            ('until', 'guardian~fighting', 12000),
            ('until', 'guardian~done', 12000),
            ('until', 'layer=5', 12000),
            ('expect', 'sidelayer=none'),
            ('expect', 'area!=the Secret Area'),
        ]),
}
