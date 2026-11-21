"""The week's DeepNet (docs/online/weekly-net.md, docs/online/shared-dive.md):
the portal by the Board and its Yes into the dive's first layer, another
diver seen there, a crew made with him and a battle of one fought by both
unasked (issue #715), the Board's posts of the week,
a dive left by a quit (CONTINUE is the normal run's, as it was, and the
portal offers the dive again), a game without the DeepNet stamp, which
the portal offers nothing, and the server gone on a dive, which goes on
alone.

The week's net is the server's own (its seed from its key and the calendar
week), so the dive's layer is another each run: these courses read nothing
of its layout but that both divers arrive on the same spot."""
import scenarios_online as online
from scenarios_online import soon

NEEDS = ('bn5', 'bn4')   # (the dive takes the NetPass's DeepNet stamp: the full set)
CAST = dict(START=online.HOME, NEEDS=NEEDS, SERVER=True, FILES={'netpass.ini': online.SEEN_FULL})
PORTAL = 'place -44 150 3; wait 10'    # before the week's portal, facing it (seed 120's Square)


def dive(game=''):
    """The week's portal by the Board spoken to, its Yes, its word paged: the dive's first layer, its arrival words paged."""
    return online.course(
        online.at(game, ('do', PORTAL), ('repeat', 'press A; wait 10', 'choice~/2', 20)),
        online.yes(game),
        online.at(game, ('repeat', 'press A; wait 10', 'dive=on', 30), ('until', 'where=layer', 1200), ('until', 'chat=open', 600),
                  ('repeat', 'press A; wait 10', 'chat=closed', 20)))


MANY = {
    'dive-together': dict(CAST, GAMES={'beta': online.HOME}, STEPS=online.course(
        online.into_square(),
        ('expect', 'weekoffer=new'),
        dive(),
        ('expect', 'layer=1'),
        ('expect', 'divelink=on'),
        ('expect', 'divers=0'),
        ('expect', 'visit=home'),                  # no visit: the Square's link kept for the dive
        online.into_square('beta'),
        soon('peers=0', 'beta'),                   # Alpha is in the dive, not the Square
        dive('beta'),
        soon('divers~Beta', '', 600),              # each on the other's layer
        soon('divers~Alpha', 'beta', 600),
        ('expect', 'crew=0'),
        # (A on Beta's MegaMan, who arrived where Alpha stands: "Lan,it's Beta!", "Team up,Lan?" opens on Yes)
        ('repeat', 'press A; wait 10', 'choice~/2', 20),
        ('do', 'wait 20'),
        ('expect', 'choice=1/2'),
        ('do', 'press A; wait 10'),
        soon('chat=open', 'beta', 600),            # "Alpha wants to team up with us!"
        ('@beta', 'repeat', 'press A; wait 10', 'choice~/2', 30),
        ('@beta', 'do', 'wait 20'),
        ('@beta', 'expect', 'choice=1/2'),
        ('@beta', 'do', 'press A; wait 10'),
        soon('crew=2: Alpha Beta', '', 600),
        soon('crew=2: Alpha Beta', 'beta', 600),
    )),
    'dive-kept-apart': dict(CAST, STEPS=online.course(
        online.paged(),
        ('keep', 'hp zenny bugfrags place spares'),
        online.into_square(),
        dive(),
        ('expect', 'dive=on'),
        ('restart', online.HOME.replace('--scene home', '--scene title')),   # the game closed on the dive
        ('repeat', 'press START; wait 40', 'title=menu'),
        ('expect', 'cursor=continue'),
        ('repeat', 'press A; wait 40', 'scene=emu'),
        ('until', 'where=town', 1200),             # CONTINUE is the normal run's, at the gate
        ('repeat', 'mash A 60; wait 30', 'chat=closed'),
        ('expect', 'dive=none'),
        ('same', 'hp zenny bugfrags place spares'),
        ('do', 'place -40 -8 7; wait 20; place 16 72 3; wait 10'),   # off the gate and on: the Square again
        soon('visit=square', '', 900),
        soon('held=none', '', 600),
        ('expect', 'weekoffer=again'),             # the unfinished dive, offered again
    )),
    'board-lists-the-week': dict(CAST, SERVER=dict(weeks=[('Mira', 7, False, 31), ('Dex', 4, False, 12)]), STEPS=online.course(
        online.into_square(),
        soon('weekboard~Mira'),
        ('expect', 'weekboard~Dex'),
        ('expect', 'weekboard~7'),
    )),
    'no-stamp-no-dive': dict(START=online.HOME, SERVER=True, FILES={'netpass.ini': online.SEEN_SQUARE}, NEEDS=(), STEPS=online.course(
        online.into_square(),
        ('expect', 'netpass=square'),
        ('expect', 'weekoffer=stamp'),
        ('do', PORTAL),
        ('repeat', 'press A; wait 10', 'chat=open', 6),
        ('repeat', 'press A; wait 10', 'chat=closed', 20),   # its words, and no question
        ('expect', 'dive=none'),
        ('expect', 'visit=square'),
    )),
    # (the link down: the dive goes on at once, and after twenty seconds of the clock is left solo, its link let go;
    # a layer runs faster than the clock headless, so the wait is counted in looks, not frames)
    'server-gone-on-a-dive': dict(CAST, STEPS=online.course(
        online.into_square(),
        dive(),
        ('expect', 'divelink=on'),
        ('expect', 'link=up'),
        ('keep', 'layer area hp'),
        ('server', 'stop'),
        ('repeat', 'wait 30', 'link=reconnecting', 200),
        ('do', 'hold UP 20; wait 20'),             # MegaMan walks on meanwhile
        ('expect', 'dive=on'),
        ('expect', 'where=layer'),
        ('repeat', 'wait 60', 'divelink=off', 700),
        ('expect', 'dive=on'),
        ('same', 'layer area hp'),
    )),
}

# A crew's battle fought together, unasked (issue #715): Alpha's battle with Beta standing where he does puts both
# on one field with no key pressed on Beta's game. Its games start without --dev quiet, whose hold of the random
# battles the battle step obeys; nobody walks, so BN6 rolls none of its own.
LOUD = online.HOME.replace('quiet,', '')
MANY['crew-battle-unasked'] = dict(CAST, START=LOUD, GAMES={'beta': LOUD}, STEPS=online.course(
    MANY['dive-together']['STEPS'],
    ('expect', 'join=none'),
    ('@beta', 'expect', 'join=none'),
    ('do', 'battle; wait 10'),
    soon('join=fighting', 'beta', 900),
    soon('join=fighting', '', 900),
))
