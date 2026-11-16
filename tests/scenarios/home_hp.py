"""Lan's HP after an act (docs/world/home/lans-hp.md, going-back.md): the
arrival and what it shows, the ways on (the pink pad unasked, a link after
its question), a way back's question and its trip, the Net's clock a notch
on and kept, the courier heard, the jack-out to Lan's room and back, and a
CONTINUE that finds home as it was."""

# (home before layer 7, act 2 won: the route `hp`'s start)
HOME = '--scene home --run-depth 7 --seed 3'
FREE = ('until', 'held=none', 600)
PAGED = ('repeat', 'press A; wait 20', 'chat=closed', 40)
ARRIVED = [('until', 'chat=open', 900), ('repeat', 'mash A 60', 'chat=closed', 20), FREE]
# (the portals' spots, BN6's map's own: the pink pad at the corridor's end,
# the link up top a way on, the link on the left a way back)
# (a link asks where MegaMan steps onto its art, as a walk meets it: its front, 32 before its cells' middle)
LINK_ON = 'place 68 -40 7; wait 30'
LINK_BACK = 'place -27 -70 1; wait 30'
# (a box's words take up to 40 frames to print, and its question's options come last)
ASKED = ('repeat', 'press A; wait 40', 'choice=1/2|choice=2/2', 30)
# (the options show a moment before BN6 sets the cursor on No)
NO = ('until', 'choice=2/2', 120)
ON_LAYER = [('until', 'where=layer', 900), ('repeat', 'mash A 60', 'held=none', 30)]


MANY = {
    'arrival': dict(
        GUARDS="Lan's HP after act 2: MegaMan's words paged, two ways on and two back lit, layer 7 ahead, the courier waiting, the visit's order open, MegaMan whole.",
        START=HOME,
        STEPS=ARRIVED + [
            ('expect', "place=Lan's HP"),
            ('expect', 'portals=on 2 back 2'),
            ('expect', 'ahead=layer 7'),
            ('expect', 'courier=waiting'),
            ('expect', 'order=open'),
            ('expect', 'hour=afternoon'),
            ('expect', 'hp=100/100'),
            ('expect', 'lab=none'),
        ]),
    'pink-pad-goes-on': dict(
        GUARDS="The pink pad, the act's own way, unasked: the next act's first layer, no trip and no notch on the clock.",
        START=HOME,
        STEPS=ARRIVED + [('do', 'place 171 0 1; wait 10; hold UP+RIGHT 25')] + ON_LAYER + [
            ('expect', 'layer=7'),
        ]),
    'link-on-asks': dict(
        GUARDS="A way on through a link asks first, its cursor on No (a playtester's act was chosen unasked, session 73): No stays home; asked again, Yes goes to layer 7.",
        START=HOME,
        STEPS=ARRIVED + [
            ('do', LINK_ON), ASKED, NO,
            ('do', 'press A; wait 60'),
            FREE,
            ('expect', "place=Lan's HP"),
            ('do', 'place 100 -20 1; wait 30; ' + LINK_ON), ASKED, NO,  # off it and on again
            ('do', 'press LEFT; wait 20'),
            ('expect', 'choice=1/2'),
            ('do', 'press A; wait 40'),
        ] + ON_LAYER + [
            ('expect', 'layer=7'),
        ]),
    'link-on-no-holds': dict(
        GUARDS="A way on's link answered No takes MegaMan nowhere, though he walks on across its square.",
        KNOWN="after No on a way on's link, a step further onto its cells warps MegaMan to the next act unasked: BN6's link marker, "
              "shown while the link's story flag is set, keeps its warp on whatever the engine's warp-off flag says (lanhp_lit)",
        START=HOME,
        STEPS=ARRIVED + [
            ('do', LINK_ON), ASKED, NO,
            ('do', 'press A; wait 60'),
            FREE,
            ('do', 'hold UP+LEFT 30; wait 120'),                        # on up the square, onto its cells
            ('expect', "place=Lan's HP"),
        ]),
    'way-back-asks': dict(
        GUARDS="A way back asks with its cursor on No: No stays home, the clock unmoved and both ways back still lit; Yes is a trip to a layer won, a notch on the Net's clock.",
        START=HOME,
        STEPS=ARRIVED + [
            ('do', LINK_BACK), ASKED, NO,
            ('do', 'press A; wait 60'),
            FREE,
            ('expect', "place=Lan's HP"),
            ('expect', 'portals=on 2 back 2'),
            ('do', 'place -27 -20 1; wait 30; ' + LINK_BACK), ASKED, NO,
            ('do', 'press LEFT; wait 20'),
            ('expect', 'choice=1/2'),
            ('do', 'press A; wait 40'),
        ] + ON_LAYER + [
            ('expect', 'back=from layer 7'),
            ('expect', 'clock=1'),
            ('expect', 'layer<7'),
        ]),
    'trip-back-and-the-clock': dict(
        GUARDS="A trip back by the autopilot: a layer of an act won, the Net's clock at one notch; its exit pad home again, that way back dark for the visit and the clock kept; then the way on to layer 7, the clock still one.",
        START='CYBERWORLD_AUTOPILOT=1 CYBERWORLD_AUTOPILOT_BACK=1 ' + HOME + ' --dev god,onehit',
        STEPS=[
            ('until', 'back=from layer 7', 6000),
            ('expect', 'clock=1'),
            ('until', 'where=town', 12000),
            ('expect', 'clock=1'),
            ('expect', 'portals=on 2 back 1'),
            ('expect', 'ahead=layer 7'),
            ('until', 'layer=7', 6000),
            ('expect', 'clock=1'),
        ]),
    'clock-kept-by-continue': dict(
        GUARDS="The Net's clock is the run's: two notches at home are two after a quit and CONTINUE.",
        START=HOME + ' --dev clock=2',
        STEPS=ARRIVED + [
            ('expect', 'clock=2'),
            ('keep', 'clock portals ahead zenny hp folder'),
            ('restart', '--scene title'),
            ('repeat', 'press START; wait 40', 'title=menu'),
            ('expect', 'cursor=continue'),
            ('repeat', 'press A; wait 40', 'scene=emu'),
            ('until', "place=Lan's HP", 1200),
            ('repeat', 'mash A 60', 'held=none', 20),
            ('same', 'clock portals ahead zenny hp folder'),
        ]),
    'portal-checked': dict(
        GUARDS="A facing the pink pad from its edge: MegaMan's words on what he reads through it, and he stays home until he steps on it.",
        START=HOME,
        STEPS=ARRIVED + [
            ('do', 'place 165 -15 1; wait 30; press A; wait 40'),
            ('expect', 'talk=director'),
            PAGED, FREE,
            ('expect', "place=Lan's HP"),
        ]),
    'courier-heard': dict(
        GUARDS="The courier in Lan's HP: talked to, he names what waits in town, and his burst goes once he is heard.",
        START=HOME,
        STEPS=ARRIVED + [
            ('expect', 'courier=waiting'),
            ('do', 'place -84 54 7; wait 10; press A; wait 30'),
            ('expect', 'chat=open'),
            PAGED, FREE,
            ('expect', 'courier=heard'),
        ]),
    'jack-out-and-in': dict(
        GUARDS="R in Lan's HP: BN6's \"MegaMan, jack out?\" on No stays; Yes sets Lan at his PC, R there jacks in again, and home is as it was.",
        START=HOME,
        STEPS=ARRIVED + [
            ('keep', 'portals ahead order hp zenny'),
            ('do', 'press R; wait 40'),
            ('repeat', 'press A; wait 30', 'choice=2/2'),
            ('do', 'press A; wait 60'),                                 # No
            FREE,
            ('expect', "place=Lan's HP"),
            ('do', 'press R; wait 40'),
            ('repeat', 'press A; wait 30', 'choice=2/2'),
            ('do', 'press LEFT; wait 20'),
            ('repeat', 'press A; wait 40', "place=Lan's Room", 20), FREE,
            ('do', 'press R; wait 40'),                                 # he stands at his PC
            ('until', "place=Lan's HP", 900),
            ('repeat', 'mash A 60', 'held=none', 20),
            ('same', 'portals ahead order hp zenny'),
        ]),
}
