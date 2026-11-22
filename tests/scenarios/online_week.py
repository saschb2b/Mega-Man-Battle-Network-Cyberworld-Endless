"""The week's DeepNet (docs/online/weekly-net.md, docs/online/shared-dive.md):
the portal by the Board and its Yes into the dive's first layer, another
diver seen there and his dot on the map (issue #728), a crew by standing
together (issue #714), formed and
split by walking together and apart, four at most, and a battle of one
fought by both unasked (issue #715) and through a dropped link (issue
#180), a crew formed again after one diver's link came back (#180) and
however long it was away (#751), the Board's posts of the week,
a dive left by a quit (CONTINUE is the normal run's, as it was, and the
portal offers the dive again), a game without the DeepNet stamp, which
the portal offers nothing, and the server gone on a dive, which goes on
alone and links again once it is back (#751).

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
        # (past the transport's 30 seconds the dive goes on alone and starts its link again now and then, for as long
        # as it lasts (issue #751): never let go for the rest of the dive; the clock's seconds, a layer outruns it)
        ('sleep', 33.0),
        soon('divestate=away', '', 900),
        ('expect', 'divelink=on'),
        ('expect', 'dive=on'),
        ('same', 'layer area hp'),
        # (the server back: the dive's link with it within its next start, 15 seconds at most)
        ('server', 'start'),
        ('sleep', 16.0),
        soon('divestate=up', '', 1800),
        ('expect', 'link=up'),
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

# On screen only (issue #750, the owner's call of 22 November 2026): a crew holds no wider than the picture. Beta past
# the far reach is still in Alpha's picture (`insight`) and still in the crew for crew_split_ms, and Alpha's battle
# then offers him nothing; the crew is gone while each still sees the other. The joiner's band names whose battle
# he is taken into ("Alpha's battle!"), long enough to read.
MANY['crew-on-screen-only'] = dict(CAST, START=LOUD, GAMES={'beta': LOUD}, STEPS=online.course(
    MANY['dive-together']['STEPS'],
    ('do', 'place 0 0 3; wait 5'),
    ('@beta', 'do', 'place 80 0 3; wait 5'),          # within the far reach: a crew, each in the other's picture
    ('sleep', 0.5),
    ('do', 'wait 10'),
    ('expect', 'crew=2: Alpha Beta'),
    ('expect', 'insight~Beta'),
    ('@beta', 'expect', 'insight~Alpha'),
    ('@beta', 'do', 'place 96 0 3; wait 5'),          # past it: still seen, still a crew for crew_split_ms
    ('sleep', 0.5),
    soon('diver~Beta at 96 ', '', 900),               # (the server has his new place: Alpha's snapshot says it)
    ('expect', 'insight~Beta'),
    ('expect', 'crew=2: Alpha Beta'),
    ('do', 'battle; wait 10'),                         # Alpha's battle: Beta, at the picture's edge, offered none
    soon('screen=battle', '', 600),
    ('@beta', 'expect', 'join=none'),
    ('@beta', 'expect', 'screen=map'),
    ('sleep', 2.5),
    soon('crew=0', 'beta', 600),
    ('@beta', 'expect', 'insight~Alpha'),             # the crew gone before he is out of sight
    ('@beta', 'do', 'place 0 32 3; wait 5'),          # back by Alpha's side as Alpha's battle ends: no crew yet
))

# The joiner's band names whose battle (issue #750): Beta taken into Alpha's, "Alpha's battle!" over its start.
MANY['crew-battle-named'] = dict(CAST, START=LOUD, GAMES={'beta': LOUD}, STEPS=online.course(
    MANY['dive-together']['STEPS'],
    ('do', 'battle; wait 10'),
    soon("band=Alpha's battle!", 'beta', 600),
    ('@beta', 'do', 'wait 30; shot'),
    ('@beta', 'expect', "band=Alpha's battle!"),
    soon('join=fighting', 'beta', 900),
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



def link_down(seconds):
    """Beta's own link cut `seconds` on the layer (issue #751): the band says it is down at once (and that he dives
    alone, past the transport's 30 seconds), then back; the room entered again, each sees the other, the crew again.
    Pictures of Beta's game kept (shot): down, away where it goes so long, back."""
    away = seconds > 30
    return online.course(
        ('@beta', 'do', f'reconnect cut {seconds}'),
        soon('divestate=down', 'beta', 120),
        ('sleep', 1.5),
        ('@beta', 'do', 'wait 1; shot'),
        # (the cut and the tries are the clock's: a dive's layer outruns it)
        *([('sleep', 34.0), soon('divestate=away', 'beta', 600), ('@beta', 'do', 'wait 1; shot')] if away else []),
        # (past the transport's 30 seconds the dive starts the link again every 15 seconds at most: one more such wait)
        ('sleep', max(1.0, seconds - (35.5 if away else 1.5)) + (16.0 if away else 0.0)),
        soon('divestate=up', 'beta', 3600),
        ('@beta', 'do', 'wait 20; shot'),
        soon('divers~Alpha', 'beta', 600),
        soon('divers~Beta', '', 600),
        ('sleep', 1.5),
        soon('crew=2: Alpha Beta', 'beta', 600),
        soon('crew=2: Alpha Beta', '', 600),
    )


# A dropped link on a layer (issue #751): 5, 30 and 120 seconds; the dive taken up again however long it was away,
# a crewmate's battle joined after it as before
for cut in (5, 30, 120):
    MANY[f'link-down-{cut}s'] = dict(CAST, START=LOUD, GAMES={'beta': LOUD}, STEPS=online.course(
        MANY['dive-together']['STEPS'],
        link_down(cut),
        ('do', 'battle; wait 10'),
        soon('join=fighting', 'beta', 900),
        soon('join=fighting', '', 900),
    ))

# A link cut during a join (issue #751): nobody held for an answer the drop has made impossible. Beta answers his
# offers two seconds late (joinwait=120, dev), a window for the cut. The joiner's cut: he is back on the map at once,
# and Alpha, his only crewmate offered it, fights the battle alone as soon as the server knows (not the offer's 3
# seconds and 3 more). The owner's cut: Alpha fights his battle alone at once, and Beta is let go as the server tells
# him it went on without him.
SLOW = LOUD + ',joinwait=120'
MANY['join-cut-joiner'] = dict(CAST, START=LOUD, GAMES={'beta': SLOW}, STEPS=online.course(
    MANY['dive-together']['STEPS'],
    ('do', 'battle; wait 10'),
    soon('join=joined', 'beta', 120),
    ('@beta', 'do', 'reconnect cut 4'),
    soon('join=none', 'beta', 30),                  # back on the map at once
    ('@beta', 'expect', 'doing=map'),
    soon('join=none', '', 120),                     # Alpha alone within the server's notice
    soon('doing=battle', '', 300),
))
MANY['join-cut-owner'] = dict(CAST, START=LOUD, GAMES={'beta': SLOW}, STEPS=online.course(
    MANY['dive-together']['STEPS'],
    ('do', 'battle; wait 2; reconnect cut 4'),
    soon('join=none', '', 30),                      # Alpha fights it alone at once
    soon('doing=battle', '', 300),
    soon('join=none', 'beta', 120),                 # Beta let go as the server tells him
    ('@beta', 'expect', 'doing=map'),
))

# A link cut as the portal is taken (issue #751): the dive begins with its link down, kept, and takes it up again
# once back, not solo for the rest of the dive
MANY['portal-cut'] = dict(CAST, STEPS=online.course(
    online.into_square(),
    ('do', PORTAL),
    ('repeat', 'press A; wait 10', 'choice~/2', 20),
    online.yes(),
    ('do', 'reconnect cut 3'),
    ('repeat', 'press A; wait 10', 'dive=on', 60),
    ('until', 'where=layer', 1200),
    ('expect', 'divelink=on'),
    ('sleep', 4.5),                                     # the cut and a try again: the clock's
    soon('divestate=up', '', 1800),
    ('expect', 'link=up'),
))

# Crews by nearness (issue #714): together where they stand, apart once one walks off, together again as he comes
# back; between the near reach and the far (along x, 64 and 88 world units: protocol.json's crew_near_* and crew_far_*,
# issue #750) a crew stays as it was, looked at again and again. MegaMan is put
# down by world units, wherever the week's layer has its floor: only the distance between the two counts here.
EDGE = [step for _ in range(6) for step in (('sleep', 0.5), ('do', 'wait 10'), ('expect', 'crew=2: Alpha Beta'),
                                            ('@beta', 'do', 'wait 10'), ('@beta', 'expect', 'crew=2: Alpha Beta'))]
MANY['crew-by-nearness'] = dict(CAST, GAMES={'beta': online.HOME}, STEPS=online.course(
    MANY['dive-together']['STEPS'],
    ('do', 'place 0 0 3; wait 5'),
    ('@beta', 'do', 'place 80 0 3; wait 5'),          # past the near reach, within the far: still a crew
    *EDGE,
    ('@beta', 'do', 'place 600 0 3; wait 5'),         # walked off: none, on both, past crew_split_ms
    ('sleep', 2.5),
    soon('crew=0', 'beta', 600),
    soon('crew=0', '', 600),
    ('@beta', 'do', 'place 80 0 3; wait 5'),          # within the far reach but not the near: none yet
    ('sleep', 2),
    ('do', 'wait 10'),
    ('expect', 'crew=0'),
    ('@beta', 'do', 'place 48 0 3; wait 5'),          # within the near reach: a crew again
    ('sleep', 1.5),
    soon('crew=2: Alpha Beta', '', 600),
    soon('crew=2: Alpha Beta', 'beta', 600),
))

# Every diver on the layer a dot on the map (issue #728): each game sees the other's, ringed in the crew's colour
# while the two are a crew, moving as he walks, and dimmer once he has walked out of the crew. The dots are told by
# their panel from MegaMan's, so the layer's layout plays no part; SELECT held shows the map for its picture (`shot`),
# the second screen's too (`second`).
MAP = 'hold SELECT 20; shot; second; hold SELECT 4'
MANY['divers-on-the-map'] = dict(CAST, GAMES={'beta': online.HOME}, STEPS=online.course(
    MANY['dive-together']['STEPS'],
    ('do', 'place 0 0 3; wait 5'),
    ('@beta', 'do', 'place 64 0 3; wait 5'),          # two panels off, in the crew
    soon('mapdots=1: Beta(0,-2)*', '', 600),
    soon('mapdots=1: Alpha(0,2)*', 'beta', 600),
    ('do', MAP),
    ('@beta', 'do', MAP),
    ('@beta', 'do', 'place 576 0 3; wait 5'),         # eighteen panels off: his dot goes with him
    soon('mapdots~Beta(0,-18)', '', 600),
    ('sleep', 2.5),                                    # out of the crew past crew_split_ms: dimmer, no ring
    soon('mapdots=1: Beta(0,-18)', '', 600),
    soon('mapdots=1: Alpha(0,18)', 'beta', 600),
    ('do', MAP),
    ('@beta', 'do', 'place 16 160 3; wait 5'),        # and across: five panels the other way, still apart
    soon('mapdots~Beta(5,', '', 600),
    ('expect', 'crew=0'),
    ('do', MAP),
))

# A crew of four where the layer has little room (issue #755): a guardian's layer whose own NPCs leave two of BN6's
# sixteen for the others (diveroom=2, as layer 3 of week 2967 left), all four at its door in a crew. Each game gives
# its two MegaMen to crewmates (never to a stranger before them), and the third crewmate, with no MegaMen left, still
# has his name and his ring where he stands, the state counting him undrawn.
NEXT_GUARDIAN = 'hold SELECT 2; hold SELECT+R 4; wait 10; ' + 'press DOWN; wait 4; ' * 8 + 'press A; wait 30'
ROOM2 = online.HOME + ',diveroom=2'
CREW4 = ('beta', 'gamma', 'delta')


def to_the_door(game=''):
    """The dev menu's Next guardian, its arrival paged, then by the arena's way in."""
    return online.at(game, ('do', NEXT_GUARDIAN), ('until', 'where=layer', 1200), ('repeat', 'press A; wait 15', 'chat=closed', 20),
                     ('expect', 'layer=3'), ('do', 'by arena; wait 10'), ('repeat', 'press A; wait 15', 'chat=closed', 20))


MANY['crew-drawn-with-little-room'] = dict(CAST, START=ROOM2, GAMES={name: ROOM2 for name in CREW4}, STEPS=online.course(
    online.into_square(),
    dive(),
    *[part for name in CREW4 for part in online.course(online.into_square(name), dive(name))],
    # (first on layer 1, each on his own spot a panel or two apart, for the picture: one crewmate with no MegaMan)
    ('do', 'place 0 0 3; wait 5'),
    ('@beta', 'do', 'place 48 0 3; wait 5'),
    ('@gamma', 'do', 'place 0 48 3; wait 5'),
    ('@delta', 'do', 'place 48 48 3; wait 5'),
    ('sleep', 1.5),
    soon('crew=4: Alpha Beta Gamma Delta', '', 900),
    soon('rings>=3', '', 300),
    ('expect', 'undrawn~1: '),
    ('do', 'shot'),
    to_the_door(),
    *[to_the_door(name) for name in CREW4],
    ('sleep', 1.5),
    soon('crew=4: Alpha Beta Gamma Delta', '', 900),
    soon('rings>=3', '', 300),                         # a ring under all three, the one with no MegaMan too
    ('expect', 'undrawn~1: '),
    ('do', 'shot'),
    *[step for name in CREW4 for step in (soon('rings>=3', name, 300), ('@' + name, 'expect', 'undrawn~1: '))],
    ('@delta', 'do', 'shot'),
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
    ('@echo', 'expect', 'rings=0'),                   # (issue #758: no "about to be" ring where the crew is full)
    ('@echo', 'expect', "comings=Alpha's crew is full"),
    ('expect', 'crew=4: Alpha Beta Gamma Delta'),
    ('@delta', 'do', 'place 2000 0 3; wait 5'),
    ('sleep', 3.5),                                   # out after crew_split_ms, Echo in a second after
    soon('crew=0', 'delta', 600),
    ('expect', 'comings=none'),                       # (walked off: the plates' and the ring's to show, no line)
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
# the near reach, the server's second not yet passed (`NAME(forming)`), and the crew's again after it.
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
    ('@beta', 'do', 'place 48 0 3; wait 5'),          # back within the near reach: about to be a crew
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
    ('@beta', 'restart', BETA.replace('--scene home', '--scene title')),   # (the game closed on the dive)
    ('@beta', 'repeat', 'press START; wait 40', 'title=menu'),
    ('@beta', 'repeat', 'press A; wait 40', 'scene=emu'),
    ('@beta', 'until', 'where=layer', 1200),
    ('@beta', 'expect', 'pullback=spent'),
))

# Comings and goings (issue #758): three crewed; Beta's PET open, his plate busy on the others'; Gamma's game quit, his
# signal gone on Alpha's band; Gamma's game started again and his dive kept on, on the layer again, his signal back.
MANY['comings-and-goings'] = dict(CAST, GAMES={'beta': online.HOME, 'gamma': online.HOME}, STEPS=online.course(
    MANY['dive-together']['STEPS'],
    online.into_square('gamma'), dive('gamma'),
    ('sleep', 1.5),
    soon('crew=3: Alpha Beta Gamma', '', 600),
    ('@beta', 'do', 'press START; wait 40'),
    ('sleep', 1.5),
    soon('mate0=Beta hp 100/100 layer busy', '', 300),
    soon('mate1=Beta hp 100/100 layer busy', 'gamma', 300),
    ('@beta', 'repeat', 'press B; wait 20', 'screen=map', 10),
    ('@gamma', 'restart', online.HOME),               # (Gamma's game quit, as a player's system quits it)
    ('sleep', 1.5),
    soon("comings=Gamma's signal is gone...", '', 600),
    soon("comings=Gamma's signal is gone...", 'beta', 600),
    # (his game started again, through the gate and the portal's "Keep diving", on the layer again)
    online.into_square('gamma'),
    soon('weekoffer=again', 'gamma', 600),
    online.at('gamma', ('do', PORTAL), ('repeat', 'press A; wait 10', 'choice~/', 20)),
    ('@gamma', 'repeat', 'wait 4', 'choice=1/3', 90),
    ('@gamma', 'do', 'press A; wait 6'),
    online.at('gamma', ('repeat', 'press A; wait 10', 'dive=on', 30), ('until', 'where=layer', 1200)),
    ('sleep', 1.5),
    soon("comings=Gamma's signal is back!", '', 600),
    soon('crew=3: Alpha Beta Gamma', '', 600),
))

# A full guardian's door (issue #753): five divers on the guardian's layer, four into the door's queue and the
# fifth after them. The four see the party by name on their plates, the leader first; the fifth is told the party
# is full and keeps his keys (START opens his PET); once one of the four goes alone, a place is open and the fifth,
# asking again, is in. MegaMan's words on the way in are paged before the door is taken, where A is not yet the
# leader's "Go in" (#752).
DOOR = ('alpha', 'beta', 'gamma', 'delta', 'echo')


def to_layer(game):
    """On to the guardian's layer (the dev menu's Next guardian), its words paged."""
    return online.at(game,
                     ('do', 'guardian; wait 60'),
                     ('until', 'screen=map', 900),
                     ('repeat', 'press B; wait 15', 'chat=closed', 30))


def to_door(game):
    """In at the arena's door: his words on the way in paged with A, before the door is taken (B is "not yet")."""
    return online.at(game,
                     ('do', 'by arena; wait 60'),
                     ('repeat', 'press A; wait 30', 'chat=closed', 30),
                     ('do', 'by arena; wait 5; hold AHEAD 90; wait 30'))


def at_a_full_door():
    names = {g: g.capitalize() for g in DOOR}
    steps = online.course(online.into_square(), dive())
    for g in DOOR[1:]:
        steps += online.course(online.into_square(g), dive(g))
    for g in DOOR:
        steps += to_layer('' if g == 'alpha' else g)
    steps += online.course(soon('divers~Echo', '', 600), to_door(''), soon('door=queued: Alpha', '', 300))
    for k, g in enumerate(DOOR[1:4], 2):
        steps += online.course(to_door(g), soon('door=queued: ' + ' '.join(names[d] for d in DOOR[:k]), g, 300))
    steps += online.course(
        soon('door=queued: Alpha Beta Gamma Delta', '', 300),
        ('expect', 'crewview=3 party'),                   # the leader's plates: the other three, by name
        ('expect', 'mate0~Beta'),
        soon('crewview=3 party', 'beta', 300),
        ('@beta', 'expect', 'mate0~Alpha'),               # ... and a member's: the leader first
        to_door('echo'),
        soon('door=full', 'echo', 300))
    return steps


GAMES5 = {g: online.HOME for g in DOOR[1:]}
MANY['door-full'] = dict(CAST, GAMES=GAMES5, STEPS=online.course(
    at_a_full_door(),
    ('@echo', 'do', 'press START; wait 40'),              # his keys his own: START opens the PET
    ('@echo', 'expect', 'screen=pet'),
    ('@echo', 'repeat', 'press B; wait 20', 'screen=map', 10),
    ('@echo', 'expect', 'door=full'),
    ('@delta', 'do', 'press B; wait 10'),                 # Delta goes alone: a place opens
    ('sleep', 3.5),
    soon('door=queued: Alpha Beta Gamma Echo', 'echo', 600),
))
MANY['door-full-alone'] = dict(CAST, GAMES=GAMES5, STEPS=online.course(
    at_a_full_door(),
    ('@echo', 'do', 'press A; wait 10'),                  # A: in alone, the arena closing round him
    soon('card=title', 'echo', 600),
    ('expect', 'door=queued: Alpha Beta Gamma Delta'),    # the four's party as it was
))

# The Board without a visit (issue #759): Alpha deleted on the dive's first layer (fragile: the first hit deletes),
# his game never back in the Square; Beta, in the Square, reads him on the Board, the server's post from his end.
FRAGILE = online.HOME.replace('quiet,', 'fragile,')
MANY['dive-end-on-the-board'] = dict(CAST, START=FRAGILE, GAMES={'beta': online.HOME}, STEPS=online.course(
    online.into_square(),
    dive(),
    ('expect', 'divelink=on'),
    online.into_square('beta'),
    ('do', 'foes 01@42 mm@22; wait 10'),               # (a Mettaur in MegaMan's row: its wave the first hit)
    ('repeat', 'press START; wait 10; press A; wait 90', 'dive=none', 60),   # (the Custom screen's OK, no chip: the first hit deletes)
    soon('weekboard~Alpha:1', 'beta', 600),
))


# CONTINUE on an unfinished dive goes straight back in (issue #759, the owner's call of 22 November 2026): the game
# closed on the dive's first layer, the title's CONTINUE says the dive and takes it up at its checkpoint, the link
# taken up with it; deleted there, the dive over, CONTINUE is the normal run's again, as it was before the dive.
TITLE = online.HOME.replace('--scene home', '--scene title')


def back_in(game=''):
    """The game closed and started at its title, CONTINUE: on the dive's layer again."""
    return online.at(game, ('restart', TITLE), ('repeat', 'press START; wait 40', 'title=menu'), ('expect', 'cursor=continue'),
                     ('expect', 'divesaved=1'), ('repeat', 'press A; wait 40', 'scene=emu'), ('until', 'where=layer', 1200))


MANY['dive-kept-apart'] = dict(CAST, STEPS=online.course(
    online.paged(),
    ('keep', 'hp zenny bugfrags place spares'),
    online.into_square(),
    dive(),
    ('expect', 'dive=on'),
    back_in(),
    ('expect', 'dive=on'),
    ('expect', 'layer=1'),
    soon('divelink=on', '', 900),
    soon('divestate=up', '', 900),             # (the Square answered: the dive's room entered again)
    ('restart', FRAGILE.replace('--scene home', '--scene title')),   # (closed again; fragile: the next hit deletes)
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('repeat', 'press A; wait 40', 'scene=emu'),
    ('until', 'where=layer', 1200),
    ('do', 'foes 01@42 mm@22; wait 10'),               # (a Mettaur in MegaMan's row: its wave the first hit)
    ('repeat', 'press START; wait 10; press A; wait 90', 'dive=none', 60),
    ('restart', TITLE),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'divesaved=0'),
    ('expect', 'cursor=continue'),
    ('repeat', 'press A; wait 40', 'scene=emu'),
    ('until', 'where=town', 1200),             # the normal run's CONTINUE, at the gate
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('same', 'hp zenny bugfrags place spares'),
))

# ... with a crew: Beta's game closed on the layer, CONTINUE puts him back there, Alpha sees him again, the crew forms.
MANY['dive-continue-crew'] = dict(CAST, GAMES={'beta': online.HOME}, STEPS=online.course(
    MANY['dive-together']['STEPS'],
    back_in('beta'),
    soon('divelink=on', 'beta', 900),
    ('sleep', 2),                                     # (the server's crew_form_ms, on its clock)
    soon('divers~Beta', '', 900),
    soon('crew=2: Alpha Beta', 'beta', 900),
    soon('crew=2: Alpha Beta', '', 600),
))

# The door waits for the crew (issue #752, the owner's call of 22 November 2026): Alpha at the guardian's door with
# Beta, his crewmate, still on the layer. Alpha mashes A as a player pages through the arena's words: the door waits
# (a press is no "Go in" while a crewmate is on the way) and Beta's band says Alpha waits at the door. Beta comes,
# mashing A too; the wait is over, Alpha's A takes both in, and they fight the guardian on one field.
MANY['door-waits-crew'] = dict(CAST, GAMES={'beta': online.HOME}, STEPS=online.course(
    MANY['dive-together']['STEPS'],
    soon('crew=2: Alpha Beta', '', 600),
    to_layer(''),
    to_layer('beta'),
    soon('divers~Beta', '', 600),
    ('sleep', 1.5),                                       # the crew told where Beta is (crew_vitals_ms)
    to_door(''),
    soon('door=queued: Alpha', '', 300),
    ('expect', 'doorwait~for 1'),                         # waiting for one crewmate
    ('do', 'mash A 60; wait 10'),                         # A mashed: no "Go in" while he is waited for
    ('expect', 'door=queued: Alpha'),
    soon('band~Alpha waits at the door', 'beta', 600),   # Beta told, on the same layer
    to_door('beta'),
    ('@beta', 'do', 'mash A 60'),
    soon('door=queued: Alpha Beta', '', 300),
    ('expect', 'doorwait=0 for 0'),                       # all here: the wait over
    ('do', 'wait 30; press A; wait 10'),                  # the leader's A
    *[step for _ in range(30) for step in (('do', 'press A; wait 20'), ('@beta', 'do', 'press A; wait 20'))],
    soon('partyfield~seat', '', 3000),                    # the guardian's words paged on both, the fight on
    soon('partyfield~seat', 'beta', 3000),
    ('expect', 'partyfield~2 seat'),                      # two MegaMen on one field
))

# A queued diver's link cut at the door (issue #751): the server took him out of its queue as it dropped; back, his
# game joins it again (else, past 10 seconds down, he goes in alone): Beta, come to the door after, finds both in it
MANY['door-cut'] = dict(CAST, GAMES={'beta': online.HOME}, STEPS=online.course(
    MANY['dive-together']['STEPS'],
    soon('crew=2: Alpha Beta', '', 600),
    to_layer(''),
    to_layer('beta'),
    soon('divers~Beta', '', 600),
    ('sleep', 1.5),
    to_door(''),
    soon('door=queued: Alpha', '', 300),
    ('do', 'reconnect cut 3'),
    ('sleep', 4.5),                                       # the cut and a try again: the clock's
    soon('divestate=up', '', 1200),
    soon('door=queued: Alpha', '', 300),                  # joined again
    to_door('beta'),
    ('@beta', 'do', 'mash A 60'),
    soon('door=queued: Alpha Beta', 'beta', 300),         # the server's queue knows him again
))

# The door waits for the crew as it was (issue #778): Alpha, Beta and Gamma a crew on the guardian's layer. Alpha goes
# on ahead, out of their sight, and the crew splits (#770: a crew lives on screen); at the door it still waits for the
# two who were his crew ("Waiting for crew 2"). They come one after the other; each is told of the door once, and
# Gamma, in the queue, reads his own line, not a flare.
THREE = {'beta': online.HOME, 'gamma': online.HOME}
MANY['door-waits-who'] = dict(CAST, GAMES=THREE, STEPS=online.course(
    online.into_square(), dive(),
    online.into_square('beta'), dive('beta'),
    online.into_square('gamma'), dive('gamma'),
    ('sleep', 1.5),
    soon('crew=3: Alpha Beta Gamma', '', 900),
    to_layer(''), to_layer('beta'), to_layer('gamma'),
    ('sleep', 1.5),
    soon('crew~3: Alpha', '', 900),
    ('do', 'by arena; wait 60'),                          # Alpha ahead, out of their sight,
    ('repeat', 'press A; wait 30', 'chat=closed', 30),
    ('sleep', 3),                                         # ... the crew split (crew_split_ms)
    soon('crew=0', '', 600),
    ('do', 'by arena; wait 5; hold AHEAD 90; wait 30'),   # and in at the door: it waits for the two
    soon('door=queued: Alpha', '', 300),
    ('expect', 'doorwait~for 2'),
    ('expect', 'band~Waiting for crew'),
    soon('band~Alpha waits at the door', 'beta', 600),
    to_door('beta'),
    soon('door=queued: Alpha Beta', '', 300),
    ('expect', 'doorwait~for 1'),
    to_door('gamma'),
    soon('door=queued: Alpha Beta Gamma', '', 300),
    ('expect', 'doorwait=0 for 0'),                       # all here: the wait over
    ('@gamma', 'do', 'wait 30'),
    ('@gamma', 'expect', 'band=Alpha leads 3  B:Go alone'),   # his own line: Beta's join told him nothing again
))

# An HP reward on a crew's field reaches the run once (issue #769): Alpha, the battle's owner, on 20 HP of 100, so BN6's
# results pick from the rows of a MegaMan low on HP; whatever they give, the run's HP after it is the field's as its
# fight ended and the reward once (the field's read stops before the results' recovery, BN6_PHASE_LEAVING).
# (partyreward=32818: the results' reward 0x8032, HP+50, on every game of the field; partydel=8, a seat nobody holds, so
# that the enemies fall 60 iterations after partyend whatever they are)
LOW = LOUD.replace('onlinename={name}', 'onlinename={name},hp=100/20,partyend=20,partybot=1,partyreward=32818,partydel=8')
MANY['crew-reward-once'] = dict(crew_fight(LOW), STEPS=online.course(
    crew_fight(LOW)['STEPS'],
    ('expect', 'crewend~won'),
    ('expect', 'crewend~reward 8032'),
    ('expect', 'hp=70/100'),                      # (the field's own MegaMan: 20 and the reward once; 100 before, its cap)
    ('@beta', 'expect', 'hp=70/100'),             # (a partner: BN6's results gave him nothing on the field)
))

# A dive's end screen of its own (issue #769): Beta's second deletion on a crew's field ends his dive; his title opens
# the dive's RESULT, the crew's battles fought and won, and his place on the week's Board as the server answers it.
MANY['dive-end-screen'] = dict(crew_fight(BETA), STEPS=online.course(
    MANY['crew-pulls-back']['STEPS'],
    soon('join=none', '', 600),
    soon('join=none', 'beta', 600),
    ('do', 'battle; wait 10'),
    soon('join=fighting', 'beta', 900),
    ('@beta', 'repeat', 'press A; wait 20', 'page=result', 120),   # (BN6's results paged; the dive's RESULT on the title)
    ('@beta', 'expect', 'diveend~crew 2/2'),
    soon('diveend~place 1', 'beta', 600),
))
