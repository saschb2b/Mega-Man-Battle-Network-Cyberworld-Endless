"""Issue #897: nothing drawn reads the game. With the GBA core on a thread
of its own (the New 3DS, the Vita; here CYBERWORLD_EMU_THREAD=1) the frame
done is drawn while the next one runs, and a read of the game made while
drawing waits for that next frame: the drawing then stands behind the GBA
where it ran beside it, a frame's worth of the system's time lost each
time. 0.24.0 read the game's mode for the PET menu's picture on every frame
drawn, and the 3DS ran slower than before 0.22.0.

The state's `drawwaits` counts those waits since the start. Each course
walks a part of the game with what is drawn over it up (SELECT's map left
open, the PET's menu, a battle, the town) and holds the count at 0
(docs/engine/emulation/thread.md, Nothing drawn reads the game). The PET's
menu stands on the map's picture, kept as START goes down now where it was
copied every other frame: `petfield kept` says it was."""
import cw_routes

THREAD = 'CYBERWORLD_EMU_THREAD=1 '
LAYER = THREAD + '--scene emu --dev quiet --seed 3 --run-depth 2'
TOWN = THREAD + '--scene town --seed 2'

PAGED = ('repeat', 'press A; wait 30', 'held=none', 30)         # a chat's boxes to its end, MegaMan free again
ARRIVE = [('do', 'wait 240'), PAGED, ('do', 'wait 30'), PAGED, ('expect', 'screen=map')]
NONE = ('expect', 'drawwaits=0')

MANY = {
    # a layer walked: the picture, the counters, the dive's band's place
    'layer': dict(START=LAYER, STEPS=ARRIVE + [
        ('do', 'hold RIGHT 40; wait 20; hold LEFT 40; wait 60'),
        NONE,
    ]),
    # SELECT's map left open while MegaMan walks under it, and its way worked out
    'map-open': dict(START=LAYER, STEPS=ARRIVE + [
        ('do', 'press SELECT; wait 60'),
        ('expect', 'map=open'),
        ('do', 'hold RIGHT 40; wait 60'),
        ('expect', 'map=open'),
        NONE,
    ]),
    # the PET's menu of ours over BN6's, opened from the layer and closed again
    'pet-menu': dict(START=LAYER, STEPS=ARRIVE + [
        ('repeat', 'press START; wait 40', 'screen=pet', 6),
        ('expect', 'petmenu=at ChipFolder'),
        ('expect', 'petfield=kept'),                            # the map's picture behind it, kept as START went down
        ('do', 'press DOWN; wait 30; press RIGHT; wait 30'),
        ('repeat', 'press B; wait 40', 'screen=map', 8),
        ('do', 'wait 30'),
        NONE,
    ]),
    # ... and with the core between the frames' drawing, as on most systems
    'pet-menu-field': dict(START=LAYER.replace(THREAD, ''), STEPS=ARRIVE + [
        ('repeat', 'press START; wait 40', 'screen=pet', 6),
        ('expect', 'petfield=kept'),
    ]),
    # a battle from its start
    'battle': dict(START=LAYER, STEPS=ARRIVE + [
        ('do', 'battle'),
        ('until', 'screen=battle', 1200),
        ('do', 'wait 300'),
        NONE,
    ]),
    # the town, Lan walking, and the PET's menu there
    'town': dict(START=TOWN, STEPS=[
        ('do', cw_routes.ROOM),
        ('do', 'hold DOWN 30; wait 60'),
        ('repeat', 'press START; wait 40', 'screen=pet', 6),
        ('do', 'wait 60'),
        NONE,
    ]),
}
