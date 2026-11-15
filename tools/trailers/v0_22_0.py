"""0.22.0's trailer, YOUR RUN, ON EVERY DEVICE (the release's notes'
own line): the first cut on the blueprint
(.claude/skills/trailer/blueprint.md), the house look (house.py: 0.17.0's
frame, kept from here on) with what is this release's own.

Its own: the accent, the green of the PET's RUN! button (a save in step);
the field, the battle's net with packets climbing its lines (saves on
their way up); the cut, each shot drawn in from the left under a bright
line, as a transfer fills; and the tune (v0_22_0_music.py). The narrator
is Dad, in BN6's own chat box (house.Narration): his hello after the
title, his thanks before the stinger, his box over footage that fits
each line (a dive, a new Net three times over, the release's shots to
come; the owner: Dad speaking while MegaMan stands there is boring).

The release, read from the pull requests merged since 0.21.2 and not
from the CHANGELOG alone (the first cut was laid from its Unreleased
section, which had neither the shared dive nor the layers' heights yet;
the owner: "go over the merged prs you've most likely missed
something"), the climax last:
01 SAVES THAT FOLLOW YOU: cloud saves, sealed and with no account, a
   code that sends them, SAVES in three pages.
02 THE NET, RELAID: layers that climb (stairs, raised rooms, terraces),
   the areas' own props (the Nest's crystals, Robot Control Comp's
   figures), BN6's locks met every act, the comps' two-wide ways, BN4's
   ACDC Area and BN5's Oran Area drawn whole.
03 THE LITTLE THINGS: the map's key, L's two detours, the Jukebox's
   Unused section.
04 MORE WAYS TO PLAY: the PSP, 3D on the New 3DS at full speed, Android
   and the Switch updating themselves.
05 DIVE TOGETHER: the shared dive (docs/online/shared-dive.md), the
   week's portal, divers seen live, a crew, a crewmate's battle joined,
   a party on one field against a guardian.
The recap: the net built faster, the music playing on under the menus,
the game telling of a new version, the count of its fixes (counted from
the CHANGELOG as the cut is drawn).

Its footage is v0_22_0_footage.py's scripted runs, whole frames of the
game from the developer's ROMs. A scene not taken yet draws as a card
marked FOOTAGE TO COME: the layer generators were still being polished
when the cut was laid, and a scene is retaken by deleting its folder.

    python3 build.py host && python3 tools/trailer.py 0.22.0 [--keep] [--frames A,B]
"""
import os
import sys

from PIL import ImageDraw

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailers import house  # noqa: E402
from trailers import v0_22_0_music as music  # noqa: E402
from trailers.house import Narration, Segment, Shot, Tile  # noqa: E402

try:
    from trailers import v0_22_0_footage as runs  # noqa: E402
except ImportError:                       # (before the runs are written: every shot a stand-in)
    runs = None

VERSION = '0.22.0'
NAME = 'YOUR RUN, ON EVERY DEVICE'
BN6_ONLY = tuple(getattr(runs, 'BN6_ONLY', ()))
GREEN = house.BUTTON


def field(fr):
    """the battle's net, packets climbing its lines: saves on their way"""
    im = house.net(fr)
    d = ImageDraw.Draw(im)
    o = (fr // 2) % 16
    for i in range(18):
        x = -16 + o + 16 * ((i * 7) % 27)
        y = (house.LH + 24 - (fr * (2 + i % 3) + i * 53) % (house.LH + 24)) - 12
        d.line([(x, y), (x, y + 5)], fill=house.NET_DOT)
        d.point((x, y), fill=house.FRAME_HI)
    return im


# (every sample below is one of v0_22_0_footage.py's MARKS, named beside it)

def hello_backdrop():
    """behind Dad's hello: a dive (a layer walked), then a new Net three
    times over, then the release's own shots a beat each, the segments
    to come"""
    return [(0, 'walkway', 30, 1),                                    # onto the walkway
            (5, 'bn4-acdc', 130, 1), (7, 'bn5-oran', 45, 1), (9, 'cold-c', 10, 1),   # a room, a floor, fire columns
            (11, 'saves-cloud', 110, 1), (12, 'climb-a', 14, 1), (13, 'map-key', 30, 1),
            (14, 'options-update', 70, 1), (15, 'dive-party', 198, 1)]


def thanks_backdrop():
    """behind his thanks: the release's battles again, then four divers
    on one layer"""
    # (BN4's BurnerMan across the field, the party's fight, the pack walking)
    return [(0, 'cold-e', 55, 1), (4, 'dive-party', 300, 1), (8, 'dive-pack', 66, 1)]


HELLO = Narration('dad-hello', [(0, "Hi,everyone! It's Lan's dad."),
                                (5, 'Cyberworld Endless is a new Net on every dive.'),
                                (11, "Now,here's what 0.22.0 brings!")], backdrop=hello_backdrop())
THANKS = Narration('dad-thanks', [(0, "And that's 0.22.0!"),
                                  (4, 'Thanks for diving in,everyone!'),
                                  (8, 'Be careful down there,OK?')], backdrop=thanks_backdrop())


def segments():
    return [
        Segment('SAVES THAT FOLLOW YOU', [
            # (Turn on's question answered Yes; the first sending, then Synced with its date)
            Shot('CLOUD SAVES', 'CLOUD SAVES KEEP EVERY DEVICE IN STEP', [(0, 'saves-cloud', 44, 1)]),
            Shot('NO ACCOUNT', 'SEALED ON YOUR DEVICE. NO ACCOUNT.', [(0, 'saves-cloud', 196, 1), (2, 'saves-cloud', 282, 1)]),
            # (SEND sending, then its code)
            Shot('SEND BY CODE', 'SEND SAVES BY A CODE OF', [(0, 'saves-code', 76, 1), (1.5, 'saves-code', 196, 1)],
                 count='10 DIGITS'),
            # (CLOUD, CODE and FILE, turned with R)
            Shot('THREE PAGES', 'SAVES IN THREE PAGES: CLOUD, CODE, FILE',
                 [(0, 'saves-pages', 30, 1), (1.5, 'saves-pages', 55, 1), (3, 'saves-pages', 118, 1)]),
        ], icon='icon-put'),
        Segment('THE NET, RELAID', [
            Shot('LAYERS CLIMB', 'LAYERS CLIMB: STAIRS, TERRACES, RAISED ROOMS', [(0, 'climb-a', 6, 1), (2, 'climb-b', 26, 1)]),
            Shot("AREAS' OWN PROPS", "THE NEST'S CRYSTALS, THE COMP'S ROBOTS", [(0, 'nest-crystals', 60, 1), (2, 'robot-figures', 20, 1)]),
            # (the walk to seed 4's Security Cube, then MegaMan's word on it)
            Shot('LOCKS', "BN6'S LOCKS, MET EVERY ACT", [(0, 'lock', 24, 1), (2.5, 'lock', 92, 1)]),
            Shot('WIDE WAYS', 'THE COMPS WALK TWO PANELS WIDE', [(0, 'walkway', 70, 1)]),
            Shot('BN4 AND BN5', "BN4'S ACDC AND BN5'S ORAN, DRAWN WHOLE", [(0, 'bn4-acdc', 150, 1), (2, 'bn5-oran', 150, 1)]),
        ], icon='icon-layers'),
        Segment('THE LITTLE THINGS', [
            Shot('MAP KEY', "THE MAP'S KEY NAMES EVERYTHING", [(0, 'map-key', 14, 1)]),
            Shot('L: THE WAY', 'L NAMES THE WAY AND TWO DETOURS', [(0, 'l-detours', 68, 1), (2.5, 'l-detours', 222, 1)]),
            # (BN4's Unused section, then BN5's)
            Shot('UNUSED SONGS', "THE JUKEBOX'S UNUSED SECTION", [(0, 'jukebox-unused', 10, 1), (2, 'jukebox-unused', 150, 1)]),
        ], icon='icon-star'),
        Segment('MORE WAYS TO PLAY', [
            # (the PSP holds BN6's net alone: a BN6 comp walked)
            Shot('PSP', 'ON THE PSP: 2000, 3000, GO, STREET', [(0, 'walkway', 130, 1)]),
            # (Depth lighting, the 3DS's 3D depth, turned on at the half bar)
            Shot('3D ON NEW 3DS', '3D ON THE NEW 3DS AT', [(0, 'options-3d', 36, 1)], count='59 FPS'),
            Shot('SELF-UPDATES', 'ANDROID AND THE SWITCH UPDATE THEMSELVES', [(0, 'options-update', 0, 1)]),
        ], icon='icon-handheld'),
        Segment('DIVE TOGETHER', [
            Shot('THE PORTAL', "THE WEEK'S NET, DIVED TOGETHER", [(0, 'dive-portal', 6, 1), (1.5, 'dive-portal', 262, 1)]),
            Shot('LIVE DIVERS', 'EVERY DIVER ON THE SEED, SEEN LIVE', [(0, 'dive-pack', 62, 1)]),
            Shot('CREWS', 'TEAM UP: A CREW OF UP TO', [(0, 'dive-crew-b', 90, 1), (2, 'dive-crew-b', 244, 1)], count='4'),
            Shot('JOIN IN', "A CREWMATE'S BATTLE CALLS YOU IN", [(0, 'dive-joinin-a', 2, 1), (1.5, 'dive-joinin-a', 78, 1)]),
            Shot('ONE FIELD', "THE GUARDIAN'S DOOR: A PARTY, ONE FIELD", [(0, 'dive-door', 316, 1), (1.5, 'dive-party', 484, 1)]),
        ], icon='icon-web'),
    ]


FIXES = house.changelog(VERSION)[1]
T = house.Trailer(
    version=VERSION, name=NAME, music=music, accent=GREEN, field=field, cut='sweep',
    # (Beast Out before BN5's viruses: the dark, the flare, the white, the beast; then three MegaMen on one field
    # against BlastMan, a bomb going off)
    cold=[(0, 'cold-a', 8, 1), (4, 'dive-party', 184, 1)],
    hello=HELLO, thanks=THANKS, segments=segments(),
    recap=[Tile('walkway', 0, ('THE NET BUILDS', '40% FASTER')),
           Tile('options-3d', 15, ('THE MUSIC PLAYS ON', 'UNDER THE MENUS')),
           Tile('options-update', 0, ('A NEW VERSION?', 'THE GAME TELLS YOU')),
           Tile('title-depth', 20, ('THE TITLE KEEPS', 'YOUR BEST DEPTH'))],
    recap_caption=('AND FIXES ACROSS THE WHOLE RUN:', str(FIXES)))

PLAY_LABEL = T.play_label
POSTER = T.poster

# YouTube (docs/project/youtube.md): the title's hook, and the thumbnails
# for Studio's Test & compare (tools/trailer_thumbnail.py), a region of a
# whole frame each with two or three words
YOUTUBE_HOOK = 'Dive Together, Cloud Saves and the PSP'
YOUTUBE_NOTE = ("And the week's net is dived together: every diver on its seed seen live, crews, and a party of MegaMen "
                'against a guardian on one field.')
THUMBNAILS = [
    # (three MegaMen on one field, a bomb going off before BlastMan; the beast's fight through its fire rings;
    # Green Area's stair and terrace)
    {'hero': ('dive-party', 204), 'region': (0, 60, 160), 'words': ('DIVE', 'TOGETHER!'), 'lit': 1, 'side': 'right'},
    {'hero': ('cold-b', 10), 'region': (0, 25, 240), 'words': ('CLOUD', 'SAVES!'), 'lit': 1, 'side': 'right'},
    {'hero': ('climb-a', 28), 'region': (0, 12, 240), 'words': ('THE NET', 'CLIMBS!'), 'lit': 1},
]


SERVED = tuple(getattr(runs, 'NEEDS_SERVER', ())) + ('options-update',)


def scenes():
    """the runs by themselves; those that need a server here (a Net
    Square, the update's manifest) are footage()'s"""
    return T.scenes({n: sc for n, sc in getattr(runs, 'SCENES', {}).items() if n not in SERVED})


def footage(keep):
    """the dive's takes, several games on a server of their own
    (v0_22_0_footage.py's online_footage); then the saves' cloud and code
    scenes against a Net Square server, and Options' update against a
    manifest of the release's notes served over HTTP, both on this
    machine, started here and stopped after (that file's docstring)"""
    import subprocess
    import time
    import trailer as kit
    runs.online_footage(kit.WORK, keep)
    need = [n for n in SERVED if n in runs.SCENES
            and not (keep and os.path.isdir(os.path.join(kit.WORK, n)) and os.listdir(os.path.join(kit.WORK, n)))]
    if not need:
        return
    state = os.path.join(kit.WORK, 'server')
    served = os.path.join(kit.WORK, 'update')
    os.makedirs(state, exist_ok=True)
    os.makedirs(served, exist_ok=True)
    ini = os.path.join(state, 'server.ini')
    with open(ini, 'w', encoding='utf-8') as f:
        f.write(runs.SERVER_INI.format(port=runs.SERVER_PORT, state=state))
    with open(os.path.join(served, 'update.json'), 'w', encoding='utf-8') as f:
        subprocess.check_call([sys.executable, os.path.join(kit.ROOT, 'tools', 'update_manifest.py'), 'v' + VERSION, served], stdout=f)
    quiet = dict(stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    servers = [subprocess.Popen([sys.executable, os.path.join(kit.ROOT, 'server', 'main.py'), '--config', ini], **quiet),
               subprocess.Popen([sys.executable, '-m', 'http.server', str(runs.UPDATE_PORT), '--bind', '0.0.0.0'], cwd=served, **quiet)]
    try:
        time.sleep(2)
        for n in need:
            kit.capture(n, *runs.SCENES[n])
            print('captured', n)
    finally:
        for server in servers:
            server.terminate()
            server.wait()


def compose(only=None):
    T.compose(only)
