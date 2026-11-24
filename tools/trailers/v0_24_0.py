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
03 ONE FORM AFTER ANOTHER: the PET's Forms; Falzar's five Crosses; Team
   ProtoMan's Souls in BN6's engine (BN5's ROM beside); Beast Out. Not the
   setup's FORM row nor BN4's Souls: no player reaches them in 0.24.0
   (the row shows only with --dev forms).
04 ONE FIELD, ONE GUARDIAN: the door waiting for the crew, three
   MegaMen on one field, the guardian's HP times the party, the win.
The recap: the folders' shelf, the map kept open, fast forward's music,
a request's reward at home and its chip chosen.

What the build does not do yet, and the nearest that was filmed instead
(24 November 2026, v0_24_0_online.py's MISSING): crewmates in different
looks part before a shared field's first frame, so the crew's battle and
the party's guardian are filmed in BN6's blue, and a look is seen on peers
in the Net Square alone (on a dive's layer each game draws every MegaMan
in its own); the party's bots never finish a guardian in a take, so the
climax ends on one MegaMan in his look deleting layer 3's guardian. The
colours screen was filmed on dev with PR 862's palettes merged in
locally (`--dev looks=4095`), and retaken from dev once 862 was there.

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


# ---- the colours screen (issue 854: PR 858's screen, PR 862's palettes earned) ----
# Lan's room (seed 2), the arrival's words paged, the PET's start menu, Colours; every palette owned
# (`--dev looks=4095`, the profile a player has after earning them) and a look worn to begin with (ProtoMan's
# red armour, Nebula's violet suit, Tournament's gold). In the screen: the armour's grid, the cursor walked
# (he takes each colour as it moves), the palettes paged with R, a pick; the suit's and the accent's the same;
# then out to the room in the new look.


def frames(script):
    return sum(int(step.split(':')[0]) for step in script.split(',') if step)


def _press(key, then):
    return f'6:{key},{then}:,'


COLOURS_OPEN = ('40:,' + '4:A,6:,' * 46 + '30:,' + _press('START', 40) + _press('DOWN', 6) * 5 + '4:,' + _press('A', 60))
COLOURS = (COLOURS_OPEN + '30:,'
           + _press('DOWN', 24) + _press('A', 24) + _press('RIGHT', 16) * 2 + _press('DOWN', 16)
           + _press('R', 28) * 4 + _press('RIGHT', 16) + _press('A', 36)
           + _press('DOWN', 24) + _press('A', 24) + _press('R', 28) * 3 + _press('LEFT', 16) + _press('DOWN', 16) + _press('A', 36)
           + _press('DOWN', 24) + _press('A', 24) + _press('R', 28) * 2 + _press('UP', 16) + _press('A', 36)
           + '20:,' + _press('B', 50) + _press('B', 60) + '60:')
# the nearest the build comes to the party's win (a party's bots do not finish a guardian in a take, and crewmates
# in different looks part before the first frame: the cut's docstring): layer 3's guardian (seed 3, DiveMan)
# deleted by one MegaMan in his look, the autopilot playing (won at about frame 10500)
GUARDIAN_WIN = (['--scene', 'emu', '--run-depth', '3', '--seed', '3', '--dev', 'quiet,god,look=11.5,4.4,11.7'],
                {'CYBERWORLD_AUTOPILOT': '1'}, None, 10040, 10760)
# the PET's Forms (issue 774): a run holding all ten Crosses (their flags) and Team ProtoMan's Souls (BN5's ROM
# beside), the arrival's words paged; START, Forms; ElecCross swapped out for a spare; R to the Souls' page
FORMS_OPEN = ('150:,' + '4:A,6:,' * 30 + '20:,' + '4:A,6:,' * 6 + '30:,' + '4:A,6:,' * 6 + '60:,'
              + '0:flags 0xE3 0xEC 1,4:,' + _press('START', 60) + _press('DOWN', 6) * 4 + _press('A', 50))
FORMS_SCREEN = (FORMS_OPEN + _press('DOWN', 16) * 2 + _press('A', 16) + _press('RIGHT', 16) + _press('DOWN', 16)
                + _press('A', 40) + _press('R', 50) + _press('DOWN', 16) * 2 + '60:')
OWN = {
    'forms-screen': (['--scene', 'emu', '--seed', '1', '--run-depth', '2', '--setup', 'short,0,0,0,8',
                      '--dev', 'quiet,god,souls=0xFC0,look=3.2,4.4,5.1'], {}, FORMS_SCREEN, frames(FORMS_OPEN) - 120,
                     frames(FORMS_SCREEN)),
    'guardian-win': GUARDIAN_WIN,
    'colours-screen': (['--scene', 'town', '--seed', '2', '--dev', 'looks=4095,look=3.2,4.4,5.1'], {}, COLOURS,
                       frames(COLOURS_OPEN) - 40, frames(COLOURS)),
}


# ---- the takes' names (v0_24_0_online.py's kept folders) ----
SQUARE = Mosaic(('square-a', 'square-b', 'square-c', 'square-d'))
WIDEN = Mosaic(('square-a', 'square-b', 'square-c', 'square-d'), widen=24)
DIVE = Mosaic(('dive-a', 'dive-b', 'dive-c', 'dive-d'))
# (four runs, four forms at their moment: TenguCross, NapalmSoul, MagnetSoul, Beast Out)
FORMS = Mosaic(('form-cross-tengu', 'form-soul5-napalm', 'form-soul5-magnet', 'form-beast'), offsets=(160, 140, 146, 166))


def hello_backdrop():
    """behind Dad's hello: one player's game (his Square, walking alone);
    a new Net on every dive (three areas); his advice, and the picture
    pulls back to four games in one Square; the hand-over: the crew in the
    week's net"""
    return [(0, 'own-sol', 0, 1),
            (5, 'look-field', 20, 1), (7, 'look-field', 230, 1), (9, 'look-battle', 166, 1),
            (12, WIDEN, 0, 1),
            (17, DIVE, 400, 1)]


def thanks_backdrop():
    """behind his thanks: the party's fight, then the crew on its way"""
    return [(0, 'party-a', 1060, 1), (4, DIVE, 470, 1), (8, SQUARE, 100, 1)]


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
            # (Sol's game: Kai, Nova and Rin round the week's portal, each in his own look)
            Shot('THE SQUARE', 'MEET UP IN THE NET SQUARE', [(0, 'square-a', 40, 1)]),
            # (the portal's page of the week, then layer 1's act card)
            Shot("THE WEEK'S NET", "ONE PORTAL: THE WEEK'S NET, TOGETHER", [(0, 'dive-portal', 40, 1), (2, 'dive-portal', 184, 1)]),
            # (Nova and Rin walk up to Sol and Kai: dotted rings, then the crew of four, a ring under each)
            Shot('A CREW FORMS', "WALK TOGETHER AND YOU'RE A CREW", [(0, 'dive-a', 398, 1)], count='UP TO 4'),
            # (Sol's battle, joined unasked by both crewmates; Kai's own view of it)
            Shot('JOIN IN', "A CREWMATE'S BATTLE: YOU'RE IN AT ONCE", [(0, 'crew-battle-a', 196, 1), (2, 'crew-battle-b', 200, 1)]),
        ], icon='icon-follow'),
        Segment('MAKE HIM YOURS', [
            # (the screen opening on the look worn, into the armour's grid, the cursor walked: he takes each colour)
            Shot('COLOURS', "PICK MEGAMAN'S LOOK IN THE PET", [(0, 'colours-screen', 4, 1), (2, 'colours-screen', 40, 1)]),
            # (the suit's grid, a colour picked; the accent's)
            Shot('3 CHANNELS', 'ARMOUR, SUIT AND ACCENT, EACH YOURS', [(0, 'colours-screen', 200, 1), (2, 'colours-screen', 296, 1)]),
            # (R through ProtoMan, Nebula, Tournament, Hazard and Central Town, each one's way to earn it in the box)
            Shot('12 PALETTES', 'PALETTES EARNED ACROSS YOUR RUNS', [(0, 'colours-screen', 96, 1)], count='12'),
            Shot('IN EVERY FIGHT', 'ON THE FIELD, IN BATTLE, IN EVERY CHAT', [(0, 'look-battle', 170, 1), (2, 'look-chat', 2, 1)]),
            # (Sol's game in the Square: the other three in theirs, Nova's "Haha!")
            Shot('THE CREW', 'IN THE SQUARE, EACH IN HIS OWN LOOK', [(0, 'square-a', 88, 1)]),
        ], icon='icon-star'),
        Segment('ONE FORM AFTER ANOTHER', [
            # (the PET's Forms: CROSSSELECT's five and the spares, EraseCross swapped for DustCross; R to the Souls)
            Shot('FORMS', 'PICK FIVE CROSSES AND SEE YOUR SOULS', [(0, 'forms-screen', 40, 1), (2.5, 'forms-screen', 140, 1)]),
            # (each from MegaMan in his look: the dark, the flash, the Navi beside him, the form; TenguCross, then
            # GroundCross)
            Shot("FALZAR'S CROSSES", "FALZAR'S FIVE CROSSES JOIN GREGAR'S", [(0, 'form-cross-tengu', 184, 1), (2, 'form-cross-ground', 190, 1)],
                 count='+5'),
            # (NapalmSoul, then ProtoSoul)
            Shot('SOULS IN BN6', "TEAM PROTOMAN'S SOULS, IN BN6'S ENGINE", [(0, 'form-soul5-napalm', 164, 1), (2, 'form-soul5-proto', 166, 1)]),
            # (GyroSoul, then Beast Out's flare)
            Shot('BEAST OUT', 'MORE SOULS TO UNITE. AND BEAST OUT!', [(0, 'form-soul5-gyro', 164, 1), (2, 'form-beast', 184, 1)]),
            # (DustCross's scrap, 79 to 19; GroundCross's drill)
            Shot('BIG HITS', 'EVERY FORM HITS IN ITS OWN WAY', [(0, 'form-cross-dust', 392, 1), (2, 'form-cross-ground', 418, 1)]),
        ], icon='icon-swap'),
        Segment('ONE FIELD, ONE GUARDIAN', [
            # (the door waits for the crew, counting down; PARTY OF 3!)
            Shot('THE DOOR', "THE GUARDIAN'S DOOR WAITS FOR THE CREW", [(0, 'party-a', 224, 1), (2, 'party-a', 276, 1)]),
            # (three MegaMen against DiveMan at three times his HP)
            Shot('ONE FIELD', 'HIS HP TIMES THE PARTY, 3 ON ONE FIELD', [(0, 'party-a', 1000, 1)]),
            # (Kai's own Custom screen, then his view of the same fight)
            Shot('OWN SCREENS', 'EACH ON HIS OWN SCREEN, HIS OWN CHIPS', [(0, 'party-b', 0, 1), (2, 'party-b', 300, 1)]),
            # (a guardian deleted: one MegaMan in his look, the nearest the build comes to the party's win)
            Shot('DELETED!', 'ENEMY DELETED! ON TO THE NEXT LAYER', [(0, 'guardian-win', 108, 1), (2.5, 'guardian-win', 164, 1)]),
        ], icon='icon-full'),
    ]


FIXES = house.changelog(VERSION)[1]
T = house.Trailer(
    version=VERSION, name=NAME, music=music, accent=ACCENT, field=field, cut=ring,
    # (a MegaMan in a look of his own transforming; then the crew's four games at once)
    cold=[(0, 'form-cross-dust', 150, 1), (4, FORMS, 0, 1)],
    hello=HELLO, thanks=THANKS, segments=segments(),
    # (the shelf as 0.24.0 ships it, issue 865: ten folders, Steady, Navi, Ember and Tide new, Z-Canon rebuilt;
    # Ember locked, its hint; the map open, more of it drawn; fast forward's mark on a layer; BN6's Chip
    # Trader screen for a Bring request)
    recap=[Tile('folders-shelf', 330, ('TEN STARTING FOLDERS,', 'FOUR NEW, ON THE SHELF')),
           Tile('map-open', 220, ('THE MAP STAYS OPEN', 'ON SELECT')),
           Tile('fast-forward', 190, ('FAST FORWARD KEEPS', "THE MUSIC'S PACE")),
           Tile('chip-choice', 260, ('PICK THE CHIP A REQUEST TAKES,', "ITS REWARD IN LAN'S HP"))],
    recap_caption=('AND FIXES ACROSS THE WHOLE RUN:', str(FIXES)))

PLAY_LABEL = T.play_label
POSTER = T.poster

# YouTube (docs/project/youtube.md): the title's hook, and the thumbnails
# for Studio's Test & compare (tools/trailer_thumbnail.py), a region of a
# whole frame each with two or three words
YOUTUBE_HOOK = 'Your MegaMan, Your Crew'
THUMBNAILS = [
    # (three MegaMen against DiveMan on one field; the Square with four players each in his own look; Beast Out's
    # beast before the Mettaurs)
    {'hero': ('party-a', 1100), 'region': (0, 25, 240), 'words': ('YOUR', 'CREW!'), 'lit': 1, 'side': 'right'},
    {'hero': ('square-a', 96), 'region': (0, 20, 240), 'words': ('YOUR', 'COLOURS!'), 'lit': 1, 'side': 'right'},
    {'hero': ('form-beast', 212), 'region': (0, 25, 240), 'words': ('NEW', 'FORMS!'), 'lit': 1, 'side': 'right'},
]


def scenes():
    """the runs of one game; the takes of several are footage()'s"""
    return T.scenes(dict(getattr(runs, 'SCENES', {}), **OWN))


def footage(keep):
    """the online takes: several games on a server of their own
    (v0_24_0_online.py's online_footage)"""
    import trailer as kit
    if online is not None:
        online.online_footage(kit.WORK, keep)


def compose(only=None):
    T.compose(only)
