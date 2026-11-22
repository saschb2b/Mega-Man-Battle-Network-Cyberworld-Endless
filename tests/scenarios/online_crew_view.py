"""The crew's display (issue #717, docs/online/shared-dive.md, The crew's
display): two divers on the week's net, a crew by standing together, each
shown the other's plate under BN6's HP box, his HP as it changes (a
battle of his own, hit) and his layer once he goes on (the dev menu's
Next layer); a crew split, or the link down, and no plates; a crewmate
gone home between acts drawn on no layer, his plate saying Home (issue
#756)."""
import scenarios_online as online
from scenarios_online import soon

NEEDS = ('bn5', 'bn4')   # (the dive takes the NetPass's DeepNet stamp: the full set)
PORTAL = 'place -44 150 3; wait 10'    # before the week's portal, facing it (seed 120's Square)
MENU = 'hold SELECT 4; hold SELECT+R 4; wait 6; '
WIN = MENU + 'press DOWN; ' * 4 + 'press A; wait 60'          # the dev menu's Win this battle
# ... and Next layer, three below (the menu keeps its cursor); SELECT and R at once, as on a dive's layer SELECT
# alone is the emote strip's
NEXT_LAYER = 'hold SELECT+R 4; wait 6; ' + 'press DOWN; ' * 3 + 'press A; wait 30'
CAST = dict(START=online.HOME, NEEDS=NEEDS, SERVER=True, FILES={'netpass.ini': online.SEEN_FULL}, GAMES={'beta': online.HOME})


def dive(game=''):
    """The week's portal by the Board spoken to, its Yes, its word paged: the dive's first layer, its arrival words paged."""
    return online.course(
        online.at(game, ('do', PORTAL), ('repeat', 'press A; wait 10', 'choice~/2', 20)),
        online.yes(game),
        online.at(game, ('repeat', 'press A; wait 10', 'dive=on', 30), ('until', 'where=layer', 1200), ('until', 'chat=open', 600),
                  ('repeat', 'press A; wait 10', 'chat=closed', 20)))


def crewed():
    """Alpha and Beta on the dive's first layer, on one spot: a crew, each shown the other's plate."""
    return online.course(
        online.into_square(), dive(), online.into_square('beta'), dive('beta'),
        # (a dive's layer runs faster than the clock headless: the server's second before a crew, waited out as such)
        ('sleep', 1.5),
        soon('crew~2', '', 600), soon('crew~2', 'beta', 600),
        soon('crewview=1', '', 300), soon('crewview=1', 'beta', 300))


MANY = {
    'hp-and-layer': dict(CAST, STEPS=online.course(
        crewed(),
        ('expect', 'divers~Beta'),
        ('sleep', 1.0),                                      # (the server tells a crew's HP half a second apart at most)
        soon('mate0=Beta hp 100/100 layer here', '', 300),   # his HP as his game said it, on Alpha's layer
        soon('mate0=Alpha hp 100/100 layer here', 'beta', 300),
        ('@beta', 'do', 'foes 01@52'),                       # a battle of Beta's own: a Mettaur, Beta standing still
        ('@beta', 'until', 'battlestep=custom', 600),
        ('@beta', 'do', 'wait 60; press START; wait 6; press A; wait 30'),   # (the Custom screen's OK, no chip)
        ('@beta', 'until', 'hp<100', 1500),
        ('sleep', 1.0),
        soon('mate0~Beta hp 90/100', '', 300),               # Alpha sees it fall in the battle
        ('@beta', 'do', WIN),
        ('@beta', 'repeat', 'press A; wait 20', 'screen=map', 60),   # (BN6's results paged)
        ('@beta', 'do', 'wait 120'),
        ('sleep', 1.5),                                      # (a crew again, standing together after his battle)
        soon('crew~2', '', 600),
        ('@beta', 'do', NEXT_LAYER),
        ('@beta', 'expect', 'screen!=devmenu'),
        ('@beta', 'until', 'layer=2', 900),
        ('sleep', 1.0),
        soon('mate0~layer 2', '', 300),                      # ... and his layer once he goes on
        ('sleep', 11),                                       # (crew_away_ms, ten seconds of the clock)
        soon('crewview=0', '', 600),                         # out of the crew once he stays away: no plates
    )),
    'link-down': dict(CAST, STEPS=online.course(
        crewed(),
        ('server', 'stop'),
        ('repeat', 'wait 30', 'crewview=0', 200),            # the link down: no plates
    )),
    # (issue #756: three divers crewed on one spot; Beta sent home between acts as an act's last exit sends a run (the dev
    # step `home`): within a second the others draw him nowhere (no diver, no tag) and their plates say Home, the crew
    # kept as it stood)
    'gone-home': dict(CAST, GAMES={'beta': online.HOME, 'gamma': online.HOME}, STEPS=online.course(
        crewed(),
        online.into_square('gamma'), dive('gamma'),
        ('sleep', 1.5),
        soon('crew=3: Alpha Beta Gamma', '', 600),
        soon('crew=3: Alpha Beta Gamma', 'gamma', 600),
        ('expect', 'divers~Beta'),
        ('@beta', 'do', 'home; wait 10'),
        ('@beta', 'until', 'where=town', 1200),
        ('sleep', 1.0),
        soon('divers=1: Gamma', '', 300),                  # Beta drawn nowhere on Alpha's layer
        ('expect', 'tags=1: Gamma*'),
        soon('mate0=Beta hp 100/100 layer home', '', 300),  # his plate: home
        ('expect', 'crew=3: Alpha Beta Gamma'),
        soon('divers=1: Alpha', 'gamma', 300),
        soon('mate0=Alpha hp 100/100 layer here', 'gamma', 300),
        ('@gamma', 'expect', 'mate1=Beta hp 100/100 layer home'),
        ('sleep', 11),                                      # (past crew_away_ms: a crewmate at home stays in his crew)
        soon('crew=3: Alpha Beta Gamma', '', 300),
    )),
}
