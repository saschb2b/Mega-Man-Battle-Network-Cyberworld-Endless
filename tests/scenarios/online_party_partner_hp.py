"""A partner's own HP on a party's field (issue #757, docs/online/shared-dive.md,
Who is who on a party's field): three MegaMen against layer 3's guardian
on the party's core (--dev partycore=3, the partners the bot's), seen as
seat 1 sees them (partyview=1), the stood-in partners going in with 60 HP
to the lead's 100 (partyhp=60). His own Custom screen's HP box shows his
60, not his lead's; BN6's results show his HP; and a partner deleted on
the field sees no results of BN6's, whose reward is not his."""

import scenarios_online as online
from scenarios_online import soon

NEEDS = ('bn5', 'bn4')   # (the dive takes the NetPass's DeepNet stamp: the full set)
PORTAL = 'place -44 150 3; wait 10'    # before the week's portal, facing it (seed 120's Square)
MENU = 'hold SELECT 4; hold SELECT+R 4; wait 6; '
WIN = MENU + 'press DOWN; ' * 4 + 'press A; wait 60'          # the dev menu's Win this battle
# (not quiet: the battle step goes through BN6's roll; the field's turns a bot's)
# (partyend=300 on both games: the field's enemies at 1 HP at its 300th iteration, the crew's win quick)
CREW = online.HOME.replace('quiet,', '') + ',partybot=1,partyend=300'


def dive(game=''):
    """The week's portal by the Board spoken to, its Yes, its word paged: the dive's first layer, its arrival words paged."""
    return online.course(
        online.at(game, ('do', PORTAL), ('repeat', 'press A; wait 10', 'choice~/2', 20)),
        online.yes(game),
        online.at(game, ('repeat', 'press A; wait 10', 'dive=on', 30), ('until', 'where=layer', 1200), ('until', 'chat=open', 600),
                  ('repeat', 'press A; wait 10', 'chat=closed', 20)))


FIELD = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --dev quiet,partycore=3,partybot=1,partyview=1,partyhp=60'

MANY = {
    'custom-screen': dict(START=FIELD, STEPS=[
        ('until', 'partyturn~own 60', 3000),      # his own Custom screen opened (the bot's few frames), his 60 on the field
        ('expect', 'partyturn~agree'),            # ... and in its HP box, not his lead's 100
    ]),
    'results': dict(START=FIELD + ',partyend=200', STEPS=[
        ('until', 'partyresults~hp', 3000),       # the guardian down: BN6's results on this game
        ('expect', 'partyresults~agree'),         # ... his HP in them, not his lead's
        ('until', 'partyover~shown', 3000),
    ]),
    'fallen': dict(START=FIELD + ',partyend=200,partydel=2', STEPS=[
        ('until', 'partyover~skipped hp 0', 3000),   # deleted on the field: no results of BN6's for him
    ]),
    # Two games over a server: Beta hit to 90 in a battle of his own, then Alpha's crew battle joined: Beta's own
    # Custom screen and results his 90, not Alpha's 100
    'two-games': dict(START=CREW, NEEDS=NEEDS, SERVER=True, FILES={'netpass.ini': online.SEEN_FULL}, GAMES={'beta': CREW},
                      STEPS=online.course(
        online.into_square(), dive(), online.into_square('beta'), dive('beta'),
        ('sleep', 1.5),
        soon('crew~2', '', 600), soon('crew~2', 'beta', 600),
        ('@beta', 'do', 'foes 01@52'),
        ('@beta', 'until', 'battlestep=custom', 600),
        ('@beta', 'do', 'wait 60; press START; wait 6; press A; wait 30'),
        ('@beta', 'until', 'hp<100', 1500),
        ('@beta', 'do', WIN),
        ('@beta', 'repeat', 'press A; wait 20', 'screen=map', 60),
        ('@beta', 'do', 'wait 120'),
        ('sleep', 1.5),
        soon('crew~2', '', 600),
        ('do', 'battle; wait 10'),                    # Alpha's battle: Beta, near, joins it unasked
        soon('partyfield=2 seat 1', 'beta', 1200),
        soon('partyturn~own 90', 'beta', 1200),       # Beta's own Custom screen: his 90 in its box
        ('@beta', 'expect', 'partyturn~agree'),
        soon('partyresults~agree', 'beta', 3600),     # ... and BN6's results his HP, not Alpha's
        soon('partyover~shown', 'beta', 3600),
    )),
}
