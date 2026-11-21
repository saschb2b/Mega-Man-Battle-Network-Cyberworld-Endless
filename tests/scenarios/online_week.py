"""The week's DeepNet (docs/online/weekly-net.md, docs/online/shared-dive.md):
the portal by the Board and its Yes into the dive's first layer, another
diver seen there, a crew by standing together (issue #714), formed and
split by walking together and apart, four at most, and a battle of one
fought by both unasked (issue #715) and through a dropped link (issue
#180), a crew formed again after one diver's link came back (#180), the Board's posts of the week,
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
        soon('tags~Beta', '', 600),                # ... his name over his MegaMan
        soon('tags~Alpha', 'beta', 600),
        # (no asking: Beta arrived where Alpha stands, and the server made them a crew within a second, issue #714;
        # a dive's layer runs faster than the clock, so the clock's seconds are waited out as such)
        ('sleep', 1.5),
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
    # (a dive pulled back once by its crew marked "*" after its layer, and the mark said; issue #718)
    'board-marks-a-pull-back': dict(CAST, SERVER=dict(weeks=[('Mira', 7, False, 31, True), ('Dex', 4, False, 12)]),
                                    STEPS=online.course(
        online.into_square(),
        soon('weekboard~Mira:7*'),
        ('expect', 'weekboard~Dex:4 '),
        ('expect', 'weekboard~*:pulled back once by the crew.'),
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

# A crew's battle through a dropped link (issue #180): Beta's own link cut for three seconds mid-fight; the server
# holds his seat, both fields wait on his keys and go on together once he is back, the hashes agreeing after it
MANY['crew-battle-through-a-drop'] = dict(CAST, START=LOUD, GAMES={'beta': LOUD}, STEPS=online.course(
    MANY['crew-battle-unasked']['STEPS'],
    soon('party~fight', '', 900),
    soon('party~fight', 'beta', 900),
    soon('partyagreed>=2', '', 900),
    ('@beta', 'do', 'reconnect cut 3'),
    soon('partylink=down', 'beta', 120),
    soon('partylink=up', 'beta', 900),              # back: his seat taken back
    ('expect', 'partylink=up'),                     # Alpha's field waited on him, never let him go
    soon('partyagreed>=7', '', 1500),               # the fields together again, every seat's hash agreeing
    soon('partyagreed>=7', 'beta', 600),
    ('@beta', 'expect', 'partylink=up'),
))

# A diver's link back after a drop (issue #180): Beta's own link cut for three seconds on the layer; the server
# took him out of his room and his crew as it dropped, his game enters the room again once back: each sees the other
# again, the crew forms again, and a battle of Alpha's is joined by both as before the drop
MANY['crew-again-after-a-drop'] = dict(CAST, START=LOUD, GAMES={'beta': LOUD}, STEPS=online.course(
    MANY['dive-together']['STEPS'],
    ('@beta', 'do', 'reconnect cut 3'),
    soon('link=reconnecting', 'beta', 120),
    soon('crew=0', '', 600),                        # Alpha's crew split at the drop
    # (the cut and the transport's tries at 0.5, 1 and 2 seconds are the clock's: a dive's layer outruns it)
    ('sleep', 4.0),
    soon('link=up', 'beta', 1800),                  # back: his place taken back
    soon('divers~Alpha', 'beta', 600),              # the room entered again: each sees the other
    soon('divers~Beta', '', 600),
    ('sleep', 1.5),
    soon('crew=2: Alpha Beta', 'beta', 600),        # the crew formed again by nearness
    soon('crew=2: Alpha Beta', '', 600),
    ('do', 'battle; wait 10'),
    soon('join=fighting', 'beta', 900),             # and a battle of Alpha's joined by both
    soon('join=fighting', '', 900),
))

# Crews by nearness (issue #714): together where they stand, apart once one walks off, together again as he comes
# back; between crew_near (128) and crew_far (192) a crew stays as it was, looked at again and again. MegaMan is put
# down by world units, wherever the week's layer has its floor: only the distance between the two counts here.
EDGE = [step for _ in range(6) for step in (('sleep', 0.5), ('do', 'wait 10'), ('expect', 'crew=2: Alpha Beta'),
                                            ('@beta', 'do', 'wait 10'), ('@beta', 'expect', 'crew=2: Alpha Beta'))]
MANY['crew-by-nearness'] = dict(CAST, GAMES={'beta': online.HOME}, STEPS=online.course(
    MANY['dive-together']['STEPS'],
    ('do', 'place 0 0 3; wait 5'),
    ('@beta', 'do', 'place 160 0 3; wait 5'),         # past crew_near, within crew_far: still a crew
    *EDGE,
    ('@beta', 'do', 'place 600 0 3; wait 5'),         # walked off: none, on both, past crew_split_ms
    ('sleep', 2.5),
    soon('crew=0', 'beta', 600),
    soon('crew=0', '', 600),
    ('@beta', 'do', 'place 160 0 3; wait 5'),         # within crew_far but not crew_near: none yet
    ('sleep', 2),
    ('do', 'wait 10'),
    ('expect', 'crew=0'),
    ('@beta', 'do', 'place 100 0 3; wait 5'),         # within crew_near: a crew again
    ('sleep', 1.5),
    soon('crew=2: Alpha Beta', '', 600),
    soon('crew=2: Alpha Beta', 'beta', 600),
))

# Five divers on one spot: four in a crew, the first there, and the fifth outside it; one of the four walks off and
# the fifth, standing there, takes his place.
FIVE = {name: online.HOME for name in ('beta', 'gamma', 'delta', 'echo')}
MANY['crew-of-four-and-a-fifth'] = dict(CAST, GAMES=FIVE, STEPS=online.course(
    online.into_square(),
    dive(),
    *[part for name in ('beta', 'gamma', 'delta') for part in online.course(online.into_square(name), dive(name))],
    ('sleep', 1.5),
    soon('crew=4: Alpha Beta Gamma Delta', '', 900),
    online.into_square('echo'),
    dive('echo'),
    soon('divers~Echo', '', 600),
    ('sleep', 2),
    ('@echo', 'do', 'wait 10'),
    ('@echo', 'expect', 'crew=0'),
    ('expect', 'crew=4: Alpha Beta Gamma Delta'),
    ('@delta', 'do', 'place 2000 0 3; wait 5'),
    ('sleep', 3.5),                                   # out after crew_split_ms, Echo in a second after
    soon('crew=0', 'delta', 600),
    soon('crew=4: Alpha Beta Gamma Echo', '', 900),
    soon('crew=4: Alpha Beta Gamma Echo', 'echo', 600),
))

# A crewmate busy (BN6's PET open on his map: not free to go in) says so at once (CREW busy, issue #715), so Alpha's
# battle begins as his own without the window's three seconds: a second of the clock later it is under way, nobody
# called. Before the busy answer Alpha stood "calling" through the window.
MANY['crew-battle-busy'] = dict(CAST, START=LOUD, GAMES={'beta': LOUD}, STEPS=online.course(
    MANY['dive-together']['STEPS'],
    ('@beta', 'do', 'press START; wait 40'),
    ('do', 'battle; wait 10'),
    ('sleep', 1),                                 # (both games run on at the clock's pace meanwhile)
    ('expect', 'join=none'),
    soon('screen=battle', '', 600),
    ('@beta', 'expect', 'join=none'),
))

# Far apart, no crew: each battle his own, nobody held.
MANY['crew-battles-apart'] = dict(CAST, START=LOUD, GAMES={'beta': LOUD}, STEPS=online.course(
    MANY['dive-together']['STEPS'],
    ('@beta', 'do', 'place 600 0 3; wait 5'),
    ('do', 'place 0 0 3; wait 5'),
    ('sleep', 2.5),
    soon('crew=0', '', 600),
    ('do', 'battle; wait 10'),
    ('expect', 'join=none'),
    soon('screen=battle', '', 600),
    ('@beta', 'expect', 'join=none'),
    ('@beta', 'expect', 'screen=map'),
))

# The crew's sign (issue #716): a crewmate's name in the crew's colour (`tags`' NAME*) and a ring under his feet
# (`rings`), on both games; gone as he walks off and the crew splits; a dotted ring as he comes back within
# crew_near, the server's second not yet passed (`NAME(forming)`), and the crew's again after it.
MANY['crew-sign'] = dict(CAST, GAMES={'beta': online.HOME}, STEPS=online.course(
    MANY['dive-together']['STEPS'],
    ('do', 'place 0 0 3; wait 5'),
    ('@beta', 'do', 'place 48 0 3; wait 5'),
    ('sleep', 1.5),
    soon('rings=1: Beta', '', 600),
    ('expect', 'tags=1: Beta*'),
    ('expect', 'crew=2: Alpha Beta'),
    soon('rings=1: Alpha', 'beta', 600),
    ('@beta', 'expect', 'tags=1: Alpha*'),
    ('@beta', 'do', 'place 600 0 3; wait 5'),         # walked off: the crew split past crew_split_ms
    ('sleep', 2.5),
    soon('crew=0', '', 600),
    ('expect', 'rings=0'),
    soon('crew=0', 'beta', 600),
    ('@beta', 'expect', 'rings=0'),
    ('@beta', 'do', 'place 64 0 3; wait 5'),          # back within crew_near: about to be a crew
    soon('rings=1: Beta(forming)', '', 300),
    ('expect', 'tags=1: Beta'),
    ('sleep', 1.5),
    soon('rings=1: Beta', '', 600),
    ('expect', 'tags=1: Beta*'),
))

# A crewmate pulled back (issue #718; docs/online/shared-dive.md, A deletion in a crew): on both games of a crew's
# field the MegaMen of a seat mask are deleted at the battle loop's 20th iteration and its enemies 60 later (partyend,
# partydel: the same on each, as the field must be), each game's pad the Arena's bot (partybot).
def pulled(mask):
    return LOUD.replace('onlinename={name}', 'onlinename={name},partyend=20,partybot=1,partydel=%d' % mask)


def crew_fight(start):
    """Both divers crewed, Alpha's battle joined by Beta, fought to its end on the field."""
    return dict(CAST, START=start, GAMES={'beta': start}, STEPS=online.course(
        MANY['dive-together']['STEPS'],
        ('do', 'battle; wait 10'),
        soon('join=fighting', 'beta', 900),   # (the battle short: Alpha's may be at its results already)
        # (BN6's results on each game's own pad, A through them)
        ('@beta', 'repeat', 'press A; wait 20', 'crewend!=none|dive=none', 120),
        ('repeat', 'press A; wait 20', 'crewend!=none|dive=none', 120),
    ))


BETA = pulled(2)   # (seat 1, Beta's: the battle's owner, Alpha, is seat 0)
# Beta deleted, the crew's win: Beta back on the layer with Alpha's quarter, Alpha's HP the less for it, Beta's
# pull-back spent and his band saying so; Alpha's free.
MANY['crew-pulls-back'] = dict(crew_fight(BETA), STEPS=online.course(
    crew_fight(BETA)['STEPS'],
    ('@beta', 'expect', 'crewend~won'),
    ('@beta', 'expect', 'crewend~pulled'),
    ('expect', 'crewend~gave'),
    ('@beta', 'expect', 'pullback=spent'),
    ('expect', 'pullback=free'),
    ('@beta', 'expect', 'dive=on'),
    soon('where=layer', 'beta', 600),
    ('@beta', 'expect', 'hp>0'),
    soon('band~pulled us back', 'beta', 600),
))
# ... and a second deletion: spent, the dive over as alone; Alpha gives nothing.
MANY['crew-pulls-back-once'] = dict(crew_fight(BETA), STEPS=online.course(
    MANY['crew-pulls-back']['STEPS'],
    soon('join=none', '', 600),
    soon('join=none', 'beta', 600),
    ('do', 'battle; wait 10'),
    soon('join=fighting', 'beta', 900),
    ('@beta', 'repeat', 'press A; wait 20', 'dive=none', 120),
    ('repeat', 'press A; wait 20', 'join=none', 120),
    ('expect', 'crewend~won'),
    ('expect', 'pullback=free'),
))
# The field lost, both deleted: each dive over, as before.
MANY['crew-lost-ends-both'] = dict(crew_fight(pulled(3)), STEPS=online.course(
    crew_fight(pulled(3))['STEPS'],
    soon('dive=none', 'beta', 600),
    soon('dive=none', '', 600),
))
# A quit after the pull-back, and the dive taken up again from the portal: still spent.
MANY['crew-pull-back-kept'] = dict(crew_fight(BETA), STEPS=online.course(
    MANY['crew-pulls-back']['STEPS'],
    ('@beta', 'restart', BETA),               # (the game closed on the dive, back at Lan's HP)
    online.into_square('beta'),
    soon('weekoffer=again', 'beta', 600),
    online.at('beta', ('do', PORTAL), ('repeat', 'press A; wait 10', 'choice~/', 20)),
    ('@beta', 'repeat', 'wait 4', 'choice=1/3', 90),   # ("Keep diving", "Start over", "Not now": the first)
    ('@beta', 'do', 'press A; wait 6'),
    online.at('beta', ('repeat', 'press A; wait 10', 'dive=on', 30), ('until', 'where=layer', 1200)),
    ('@beta', 'expect', 'pullback=spent'),
))
