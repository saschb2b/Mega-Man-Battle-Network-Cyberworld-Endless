"""The Net Square (docs/online/square/): two players arrive in the welcome
plaza, each sees the other where he stands and as he moves, a wave from
his card and a phrase from the strip are heard by the other, and one who
heads home is gone from the other's Square; the plaza walked to a warp and
each part reached through its gem; the greeter's stamp moment on a first
visit; a full channel and the keeper's way to the next; and the server
gone in the middle, B giving up for home or the Square going on once it
is back."""
import scenarios_online as online
from scenarios_online import soon

MANY = {
    'two-meet': dict(START=online.HOME, SERVER=True, GAMES={'beta': online.HOME}, FILES={'netpass.ini': online.SEEN_SQUARE}, STEPS=online.course(
        online.into_square(),
        ('expect', 'peers=0'),
        ('expect', 'part=plaza'),
        online.into_square('beta'),
        soon('peers~Alpha', 'beta'),
        soon('peers~Beta'),
        ('@beta', 'do', 'place 116 -52 5; wait 10'),   # Beta walks off: seen where he stands
        soon('peer~Beta at 116 -52 in plaza'),
        # (A facing him from within reach: his card paged to its options, Wave the first)
        ('do', 'place 92 -52 1; wait 10'),
        ('repeat', 'press A; wait 10', 'choice=1/3', 20),
        online.pick('sentlast~emote 0'),
        soon('heardlast~emote 0 from Alpha', 'beta'),
        ('@beta', 'do', 'press SELECT; wait 20; press R; wait 10; press A; wait 10'),   # the strip's phrases: "Hello!"
        soon('sentlast~phrase 0', 'beta'),
        soon('heardlast~phrase 0 from Beta'),
        online.home_again('beta'),
        ('@beta', 'expect', 'visitended=left'),
        soon('peers=0', '', 600),
    )),
}

SQUARE = dict(START=online.HOME, SERVER=True, FILES={'netpass.ini': online.SEEN_SQUARE})

# (seed 120's Square, square_plan.c's places in the game's world: MegaMan arrives at -4 120, the landmark at 20 44; the
# warps' gems in the plaza: Trade Central's 20 -148, the Arena's 212 44, the stage's -172 44; the pad's ways: UP+RIGHT is
# +x, UP+LEFT is -y, DOWN+RIGHT is +y, DOWN+LEFT is -x)
MANY['walk-to-the-arena'] = dict(SQUARE, STEPS=online.course(
    online.into_square(),
    ('expect', 'part=plaza'),
    # (walked as a player walks it, round the landmark to the Arena's gem and through it)
    ('repeat', 'hold DOWN+RIGHT+B 4', 'squarey>=140', 40),
    ('repeat', 'hold UP+RIGHT+B 6', 'squarex>=116', 60),
    ('repeat', 'hold UP+LEFT+B 6', 'squarey<=44', 60),
    ('repeat', 'hold UP+RIGHT+B 6', 'part=arena', 90),
    soon('held=none'),
    ('expect', 'part=arena'),
    # (back through its gem there, BN6 having stepped MegaMan off it)
    ('do', 'place 372 -20 5; wait 10'),
    soon('part=plaza'),
    soon('held=none'),
    online.home_again(),
    ('expect', 'visitended=left'),
))
# (Trade Central's and the stage's gems stepped onto, and back)
MANY['parts-by-their-gems'] = dict(SQUARE, STEPS=online.course(
    online.into_square(),
    ('do', 'place 20 -148 7; wait 10'),
    soon('part=trade'),
    soon('held=none'),
    ('do', 'place 20 -308 3; wait 10'),
    soon('part=plaza'),
    soon('held=none'),
    ('do', 'place -172 44 5; wait 10'),
    soon('part=stage'),
    soon('held=none'),
    ('do', 'place -492 -20 1; wait 10'),
    soon('part=plaza'),
))
# (the full set and no netpass.ini: a full pass whose stamps were never seen)
MANY['first-visit-stamp'] = dict(START=online.HOME, SERVER=True, NEEDS=('bn5', 'bn4'), STEPS=online.course(
    online.paged(),
    ('do', 'place 16 72 3; wait 10'),
    soon('visit=square', '', 900),
    soon('chat=open', '', 600),                    # the greeter's word on the stamp
    ('repeat', 'press A; wait 10', 'screen=netpass', 20),   # the pass up, its stamp landing
    ('do', 'wait 120'),
    ('repeat', 'press A; wait 10', 'screen=square', 20),
    soon('held=none'),
    online.home_again(),
    # (the second visit: no moment)
    ('do', 'place -40 -8 7; wait 20; place 16 72 3; wait 10'),
    soon('visit=square', '', 900),
    soon('held=none', '', 600),
    ('do', 'wait 60'),
    ('expect', 'chat=closed'),
    ('expect', 'screen=square'),
    ('expect', 'visits=2'),
))
# (channels of two: the third player lands in the next, unseen, until the keeper moves MegaMan there)
MANY['channels'] = dict(SQUARE, SERVER=dict(settings=dict(capacity=2)), GAMES={'beta': online.HOME, 'gamma': online.HOME}, STEPS=online.course(
    online.into_square(),
    ('expect', 'channel=1/32'),
    online.into_square('beta'),
    online.into_square('gamma'),
    ('@gamma', 'expect', 'channel=2/32'),
    ('@gamma', 'expect', 'peers=0'),
    soon('peers=1: Beta'),
    # (the channel keeper by the way in: his words paged to his options, the next channel)
    ('do', 'place 84 150 3; wait 10'),
    ('repeat', 'press A; wait 10', 'choice~/2', 20),
    online.pick('channel=2/32|chat=closed'),
    ('repeat', 'press A; wait 10', 'channel=2/32', 20),
    soon('peers=1: Gamma'),
    soon('peers=1: Alpha', 'gamma'),
    soon('peers=0', 'beta'),
))
MANY['server-gone'] = dict(SQUARE, STEPS=online.course(
    online.paged(),
    ('keep', 'hp zenny bugfrags place spares'),
    online.into_square(),
    ('server', 'stop'),
    soon('visit=waiting', '', 600),                # the game held, the wait drawn over the Square
    ('expect', 'link=reconnecting'),
    ('repeat', 'press B; wait 10', 'visit=home', 6),   # B gives up: home at once
    ('expect', 'visitended=lost'),
    ('expect', 'where=town'),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),   # the keeper's word on it
    ('same', 'hp zenny bugfrags place spares'),
))
MANY['server-back'] = dict(SQUARE, STEPS=online.course(
    online.into_square(),
    ('do', 'place 116 -52 5; wait 10'),
    ('server', 'stop'),
    soon('visit=waiting', '', 600),
    ('server', 'start'),
    soon('visit=square', '', 900),                 # the link back: the Square goes on where he stood
    ('expect', 'link=up'),
    ('expect', 'resumes=1'),
    ('expect', 'squarex=116'),
    ('expect', 'part=plaza'),
))
