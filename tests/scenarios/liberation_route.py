"""A special route kept from a dark layer (docs/game/liberation/special-routes.md)
turns the run's next fork: at home it is a third way, taken or left, and
before the Nest a second way to its second guardian. (--dev route: a route
found on the act before's board; CYBERWORLD_AUTOPILOT_ROUTE: the autopilot
takes its portal and answers its question. Its finding on a board, and its
keeping as the board is left, are liberation-board/route's.)"""

HOME = 'CYBERWORLD_STATE_POS=1 --scene home --seed 7 --dev route --run-depth '
WALKS = 'CYBERWORLD_AUTOPILOT=1 '
TAKES = 'CYBERWORLD_AUTOPILOT=1 CYBERWORLD_AUTOPILOT_ROUTE=1 '

MANY = {
    # before act 4, the route from act 3's board: taken, the act comes by it
    'taken': dict(GUARDS="Before act 4 with the route from act 3's board kept: its way taken, the act comes by it, to the route's own area.", START=TAKES + HOME + '10', STEPS=[
        ('expect', 'where=town'), ('expect', 'route=kept from act 3'),
        ('until', 'where=layer', 8000),
        ('expect', 'layer=10'), ('expect', 'route=taken to act 4'), ('expect', 'area=Mr. Weather Comp'),
    ]),
    # ... left for the act's own way: the fork passed, the route gone with it
    'left': dict(GUARDS="The kept route left for the act's own way: the fork passed, the act's own area.", START=WALKS + HOME + '10', STEPS=[
        ('expect', 'route=kept from act 3'),
        ('until', 'where=layer', 8000),
        # (the act's own area, where the route's way led to Mr. Weather Comp)
        ('expect', 'layer=10'), ('expect', 'area=ACDC HP'),
    ]),
    # the deep's acts have no fork: the route from act 4's board is kept
    # through them, for the Nest's
    'kept-through-the-deep': dict(GUARDS="The deep's acts have no fork: the route from act 4's board is kept through them, for the Nest's.", START=TAKES + HOME + '13', STEPS=[
        ('expect', 'route=kept from act 4'),
        ('until', 'where=layer', 8000),
        ('expect', 'layer=13'), ('expect', 'route=kept from act 4'),
    ]),
    # before the endless net's Nest: the second way, its second guardian
    # there and not the Cybeast
    'nest': dict(GUARDS="Before the endless net's Nest by the kept route: the second way, its second guardian there and not the Cybeast.", START=TAKES + HOME + '19', STEPS=[
        ('until', 'where=layer', 8000),
        ('expect', 'layer=19'), ('expect', 'route~taken'), ('expect', 'guardian~waiting'), ('expect', 'guardian!=Gregar waiting'),
    ]),
    # ... and without the route's way taken, the Cybeast
    'nest/left': dict(GUARDS="Before the endless net's Nest, the route's way not taken: the Cybeast.", START=WALKS + HOME + '19', STEPS=[
        ('until', 'where=layer', 8000),
        ('expect', 'layer=19'), ('expect', 'guardian=Gregar waiting'),
    ]),
    # the short net's Nest by the route (act 3's board's): its second guardian
    'short-nest': dict(GUARDS="The short net's Nest by the route kept from act 3's board: its second guardian.", START=TAKES + HOME + '10 --setup short,0,0,0,0,0', STEPS=[
        ('expect', 'route=kept from act 3'),
        ('until', 'where=layer', 8000),
        ('expect', 'layer=10'), ('expect', 'route=taken to act 4'), ('expect', 'guardian=SlashMan waiting'),
    ]),
    'short-nest/left': dict(GUARDS="The short net's Nest, the route's way not taken: its own guardian.", START=WALKS + HOME + '10 --setup short,0,0,0,0,0', STEPS=[
        ('until', 'where=layer', 8000),
        ('expect', 'layer=10'), ('expect', 'guardian=TomahawkMan waiting'),
    ]),
}
