"""0.24.0's trailer, YOUR MEGAMAN, YOUR CREW: the second cut on the
blueprint (.claude/skills/trailer/blueprint.md), the house look (house.py:
0.17.0's frame) with what is this release's own.

The owner's direction (24 November 2026): Dad recommends, in a Battle
Network way, that a dive is better with friends; the picture pulls back
from one player's game to several; they go into the week's net and a crew
forms by itself; each makes his MegaMan his own (the release's showpiece);
then they go on together, one transformation after another, big hits, a
guardian fought as a party and won; the rest of the release short; the
music opening in 0.22.0's tone.

Its own: the accent, the PET's orange cursor (a release about picking:
your colours, your crew); the field, the battle's net with four rings
drifting on it, each in a colour of its own, as a crew's rings under its
divers; the cut, each shot opening out of a ring from its middle (`ring`);
the tune (v0_24_0_music.py); and the mosaic (house.Mosaic, new with this
cut): several games of one take side by side, each whole at 2x, the first
starting alone at 4x and pulling back into its corner. Dad's hello is four
boxes here, the third his advice, under which the picture widens.

The release, read from the pull requests merged since 0.23.0 (the
CHANGELOG's Unreleased section has the looks, the folders' shelf and the
smaller things; the shared dive's polish, the run's forms, BN4's and Team
ProtoMan's Souls in BN6's engine and Falzar's Crosses are merged and
partly missing there), the climax last:
01 DIVE TOGETHER: the Net Square, the week's portal, a crew formed by
   walking together (no prompt), its sign and rings, a crewmate's battle
   joined at once.
02 MAKE HIM YOURS: MegaMan's look (issue 854): the PET's colours screen,
   three channels, twelve palettes, earned in runs, three saved looks,
   shown everywhere, a crew each in his own.
03 ONE FORM AFTER ANOTHER: the run's FORM row; Falzar's five Crosses;
   Team ProtoMan's Souls and BN4's in BN6's engine; Beast Out.
04 ONE FIELD, ONE GUARDIAN: the door waiting for the crew, three
   MegaMen on one field, the guardian's HP times the party, the win.
The recap: the folders' shelf, the map kept open, fast forward's music,
a request's reward at home and its chip chosen.

Its footage: v0_24_0_footage.py's scripted runs of one game and
v0_24_0_online.py's takes of several games on a local server, whole
frames of the game from the developer's ROMs. A scene not taken yet draws
as a card marked FOOTAGE TO COME.

    python3 build.py host && python3 tools/trailer.py 0.24.0 [--keep] [--frames A,B]
"""
import os
import sys

from PIL import ImageDraw

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailers import house  # noqa: E402
from trailers import v0_24_0_music as music  # noqa: E402
from trailers.house import Mosaic, Narration, Segment, Shot, Tile  # noqa: E402

try:
    from trailers import v0_24_0_footage as runs  # noqa: E402
except ImportError:                       # (before the runs are written: every shot a stand-in)
    runs = None
try:
    from trailers import v0_24_0_online as online  # noqa: E402
except ImportError:
    online = None

VERSION = '0.24.0'
NAME = 'YOUR MEGAMAN, YOUR CREW'
BN6_ONLY = tuple(getattr(runs, 'BN6_ONLY', ()))
ACCENT = house.ORANGE
# the four rings' colours, dim on the navy: a crew's rings, each diver's own
RINGS = ((150, 70, 40), (40, 120, 90), (110, 60, 140), (140, 120, 30))


def field(fr):
    """the battle's net, four rings drifting along its lines, each in a
    colour of its own: a crew walking together"""
    im = house.net(fr)
    d = ImageDraw.Draw(im)
    for i, c in enumerate(RINGS):
        x = (fr * (1 + i % 2) // 3 + 97 * i + 40) % (house.LW + 60) - 30
        y = 40 + 52 * i + (fr // (6 + i)) % 16
        d.ellipse([x - 13, y - 5, x + 13, y + 5], outline=c)
        d.ellipse([x - 12, y - 4, x + 12, y + 4], outline=house.lerp(c, house.NET, 0.5))
    return im


def ring(small, old, new, t):
    """the cut: the new shot opening out of a ring from its middle, the
    ring's edge white as it grows"""
    from PIL import Image
    if t >= 1:
        small.paste(new, house.PIC)
        return
    rx, ry = round(150 * t) + 1, round(100 * t) + 1
    m = Image.new('L', (240, 160), 0)
    ImageDraw.Draw(m).ellipse([120 - rx, 80 - ry, 120 + rx, 80 + ry], fill=255)
    small.paste(new, house.PIC, m)
    ImageDraw.Draw(small).ellipse([house.PIC[0] + 120 - rx, house.PIC[1] + 80 - ry, house.PIC[0] + 120 + rx, house.PIC[1] + 80 + ry],
                                  outline=house.WHITE)


# ---- the takes' names (v0_24_0_online.py's kept folders) ----
SQUARE = Mosaic(('square-a', 'square-b', 'square-c', 'square-d'))
WIDEN = Mosaic(('square-a', 'square-b', 'square-c', 'square-d'), widen=24)
DIVE = Mosaic(('dive-a', 'dive-b', 'dive-c', 'dive-d'))


def hello_backdrop():
    """behind Dad's hello: one player's game (his Square, walking alone);
    a new Net on every dive (three areas); his advice, and the picture
    pulls back to four games in one Square; the hand-over: the crew in the
    week's net"""
    return [(0, 'own-square', 0, 1),
            (5, 'look-field', 20, 1), (7, 'form-cross-a', 10, 1), (9, 'look-battle', 30, 1),
            (12, WIDEN, 20, 1),
            (17, DIVE, 40, 1)]


def thanks_backdrop():
    """behind his thanks: the party's fight, then the crew on its way"""
    return [(0, 'party-fight-a', 120, 1), (4, DIVE, 120, 1), (8, SQUARE, 200, 1)]


HELLO = Narration('dad-hello', [(0, "Hi,everyone! It's Lan's dad."),
                                (5, 'Cyberworld Endless is a new Net on every dive.'),
                                (11, "And diving's even better with friends!"),
                                (17, "Now,here's what 0.24.0 brings!")], backdrop=hello_backdrop())
THANKS = Narration('dad-thanks', [(0, "And that's 0.24.0!"),
                                  (4, 'Thanks for diving in,everyone!'),
                                  (8, 'Look out for each other,OK?')], backdrop=thanks_backdrop())


def segments():
    return [
        Segment('DIVE TOGETHER', [
            Shot('THE SQUARE', 'MEET IN THE NET SQUARE', [(0, 'square-a', 60, 1)]),
            Shot("THE WEEK'S NET", "ONE PORTAL: THE WEEK'S NET, TOGETHER", [(0, 'dive-portal', 10, 1)]),
            Shot('A CREW FORMS', 'WALK TOGETHER AND YOU ARE A CREW', [(0, 'dive-a', 60, 1)], count='UP TO 4'),
            Shot('JOIN IN', "A CREWMATE'S BATTLE: YOU'RE IN AT ONCE", [(0, 'crew-battle-a', 20, 1), (2, 'crew-battle-b', 60, 1)]),
        ], icon='icon-follow'),
        Segment('MAKE HIM YOURS', [
            Shot('COLOURS', "PICK MEGAMAN'S LOOK IN THE PET", [(0, 'colours-screen', 10, 1)]),
            Shot('3 CHANNELS', 'ARMOUR, SUIT AND ACCENT, EACH YOURS', [(0, 'colours-channels', 10, 1)]),
            Shot('12 PALETTES', 'PALETTES EARNED IN THE RUN', [(0, 'palette-earned', 10, 1)], count='12'),
            Shot('IN EVERY FIGHT', 'ON THE FIELD, IN BATTLE, IN EVERY CHAT', [(0, 'look-battle', 40, 1), (2, 'look-chat', 10, 1)]),
            Shot('THE CREW', 'EVERY DIVER IN HIS OWN LOOK', [(0, 'square-a', 160, 1)]),
        ], icon='icon-star'),
        Segment('ONE FORM AFTER ANOTHER', [
            Shot('FORM ROW', 'START A RUN AS CROSS, SOUL BN5 OR BN4', [(0, 'form-row', 0, 1)]),
            Shot("FALZAR'S CROSSES", "FALZAR'S CROSSES JOIN GREGAR'S", [(0, 'form-cross-a', 0, 1), (2, 'form-cross-b', 0, 1)],
                 count='+5'),
            Shot('SOULS, IN BN6', "TEAM PROTOMAN'S SOULS IN BN6'S ENGINE", [(0, 'form-soul5-a', 0, 1), (2, 'form-soul5-b', 0, 1)]),
            Shot("BN4'S SOULS", "BN4'S SOULS UNITE TOO", [(0, 'form-soul4-a', 0, 1), (2, 'form-beast', 0, 1)]),
        ], icon='icon-swap'),
        Segment('ONE FIELD, ONE GUARDIAN', [
            Shot('THE DOOR', "THE GUARDIAN'S DOOR WAITS FOR THE CREW", [(0, 'party-door', 40, 1)]),
            Shot('ONE FIELD', 'THE CREW SIDE BY SIDE ON ONE FIELD', [(0, 'party-fight-a', 10, 1)], count='3'),
            Shot('TEAM HP', "EACH PARTNER'S HP IN BN6'S OWN DIGITS", [(0, 'party-fight-b', 80, 1)]),
            Shot('THE WIN', 'HIS HP TIMES THE PARTY. TAKE HIM DOWN!', [(0, 'party-win', 0, 1)]),
        ], icon='icon-full'),
    ]


FIXES = house.changelog(VERSION)[1]
T = house.Trailer(
    version=VERSION, name=NAME, music=music, accent=ACCENT, field=field, cut=ring,
    # (a MegaMan in a look of his own transforming; then the crew's four games at once)
    cold=[(0, 'form-cross-a', 0, 1), (4, SQUARE, 120, 1)],
    hello=HELLO, thanks=THANKS, segments=segments(),
    recap=[Tile('folders-shelf', 20, ('EVERY STARTING FOLDER', 'ON THE SHELF')),
           Tile('map-open', 40, ('THE MAP STAYS OPEN', 'ON SELECT')),
           Tile('fast-forward', 20, ('FAST FORWARD KEEPS', "THE MUSIC'S PACE")),
           Tile('reward-home', 20, ("A REQUEST'S REWARD", "IN LAN'S HP"))],
    recap_caption=('AND FIXES ACROSS THE WHOLE RUN:', str(FIXES)))

PLAY_LABEL = T.play_label
POSTER = T.poster

# YouTube (docs/project/youtube.md): the title's hook, and the thumbnails
# for Studio's Test & compare (tools/trailer_thumbnail.py), a region of a
# whole frame each with two or three words
YOUTUBE_HOOK = 'Your MegaMan, Your Crew'
THUMBNAILS = [
    {'hero': ('party-fight-a', 60), 'region': (0, 60, 160), 'words': ('YOUR', 'CREW!'), 'lit': 1, 'side': 'right'},
    {'hero': ('look-battle', 40), 'region': (0, 25, 240), 'words': ('YOUR', 'MEGAMAN!'), 'lit': 1, 'side': 'right'},
    {'hero': ('form-cross-a', 30), 'region': (0, 12, 240), 'words': ('NEW', 'FORMS!'), 'lit': 1},
]


def scenes():
    """the runs of one game; the takes of several are footage()'s"""
    return T.scenes(dict(getattr(runs, 'SCENES', {})))


def footage(keep):
    """the online takes: several games on a server of their own
    (v0_24_0_online.py's online_footage)"""
    import trailer as kit
    if online is not None:
        online.online_footage(kit.WORK, keep)


def compose(only=None):
    T.compose(only)
