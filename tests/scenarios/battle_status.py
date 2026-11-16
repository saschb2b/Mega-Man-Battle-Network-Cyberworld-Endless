"""A status on the foes, as a hit leaves it (the status step of
docs/devtools/battle-on-demand.md): BN6's engine holds the enemy in it
for its frames, lets it go, and the battle goes on to its end."""


def status(kind, word):
    return dict(
        START='--scene emu --seed 1 --run-depth 2 --dev onehit,god',
        STEPS=[
            ('do', 'wait 150; mash A 300; wait 20'),
            ('repeat', 'mash A 60; wait 30', 'chat=closed'),
            ('until', 'screen=map', 600),
            ('do', 'foes 01@52'),
            ('until', 'phase=custom', 1200),
            ('repeat', 'press START; wait 10; press A; wait 60', 'phase=fight'),
            ('repeat', 'wait 10', 'custom>0', 40),   # BATTLE START! gone: the fight runs
            ('expect', 'foestatus=none'),
            ('do', f'status {kind} 90; wait 10'),
            ('expect', f'foestatus={word}'),
            ('do', 'wait 60'),
            ('expect', f'foestatus={word}'),  # still held
            ('repeat', 'wait 10', 'foestatus=none', 12),  # its 90 frames up: let go
            ('expect', 'down=0'),
            ('repeat', 'mash B 30', 'phase=results'),
            ('expect', 'result=won'),
        ],
    )


MANY = {'paralysis': status('paralysis', 'paralysed'), 'freeze': status('freeze', 'frozen'), 'bubble': status('bubble', 'bubbled')}
