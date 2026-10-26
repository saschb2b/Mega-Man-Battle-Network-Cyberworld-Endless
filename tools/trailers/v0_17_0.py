"""0.17.0's trailer, in the making: its brief and its look, drawn as stills
for the owner before the footage and the piece (the music's three sketches
are sketch_bust_a, _b and _c; the owner picks one by ear first). Not yet
in VERSIONS: no scenes(), no compose().

The brief. 0.17.0 sharpens the run rather than growing it (the course
check of 26 October 2026, docs/game/core-loop.md): a battle on every
layer without a guardian (and the guardian's on the rest), BN5's End
Area one wide, the main path's words cut; a won Server pays a pick of
one chip from three; MegaMan reads the layer behind each way at a fork;
BugFrags buy programs at the NaviCust vendor; finds keep the chip they
rolled and lean only its code to the folder; three more starting folders
(Z-Canon, Busting, Lean); the requests on BN6's own Request BBS;
guardians remember how the last fight ended; ProtoMan's netbattle stops
at 1 HP instead of ending the dive. What a player tells a friend: every
layer is a fight now, and every pick between them counts.

The idea: the trailer is one virus battle. The music grows from BN5's
"Virus Busting" (song 0x15), the song every layer now plays; the look is
a battle's own screens in the project's PET style: the Custom screen to
choose, the field to act, the results to end. Never a real-world
metaphor (the owner's gate 1); none of the last trailers' devices (0.16.0's
PET list and its cursor down rows, the band as headline, the page
arrows; 0.15.0's map and pans; 0.14.0's seam; 0.13.0's windows and links;
0.12.0's panels).

- The frame: the battle's net, a navy field under a cyan grid drifting
  as a battle's background does; the HP box top left holds the version;
  the Custom gauge across the top holds the section's label (small
  capitals, one place) and fills through the segment, full on its last
  beat.
- A segment: the Custom screen slides in on its section card (its first
  bar): the segment's feature as a battle chip, its art (the site's own
  icons, tools/site_art.py), its name and code; the cursor's orange
  brackets on it; OK pressed on the bar's last beat. Its shots are the
  CHIP SELECT row's chips, each sent into the column on its beat as the
  game's window takes its picture (the cut: the chip opening into the
  shot). The caption sits on the plate under both, where BN draws a used
  chip's name, and a count where there is one sits where its damage
  would.
- The title card: the logo and THE RUN, SHARPENED; 0.17.0 · OUT NOW on
  the PET's green button.
- The recap: the column's chips laid out as a folder, a tile each.
- The stinger: the segments' chips lined up in code order and merged in
  one flash into the logo, a Program Advance (their codes S, H, A, R, P).
- The end card: BN's results, in the PET's look: DELETE TIME (the cut's
  own length), BUSTING LEVEL S, GET DATA: Cyberworld Endless 0.17.0; the
  systems, the site and the ROM line; held still.

The arc in bars (150 beats a minute, a bar 48 frames; about 42 seconds):
cold open 2 (a battle's opening blows, no words: the hook's pickup and
the stab's bar), title 2 (the hook arrives), four segments of 1 + 3
(the battle floor; the picks: a Server's three, the fork read, the
BugFrag shop; folders and finds; the rivals remember: a guardian's
memory, ProtoMan's 1 HP), recap 2, stinger 1, end card 3.

    python3 tools/trailers/v0_17_0.py OUT_DIR   # the look's stills
"""
import math
import os
import sys

from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

VERSION = '0.17.0'
FPS, W, H = 30, 1280, 720
BPM = 150
BAR = round(FPS * 60 * 4 / BPM)           # (48 frames)
BEAT = BAR // 4

FONTS = os.path.join(ROOT, 'web', 'assets', 'fonts')
CAPS = os.path.join(FONTS, 'silkscreen-700.woff2')
CAPS_THIN = os.path.join(FONTS, 'silkscreen-400.woff2')
PIXEL = os.path.join(FONTS, 'pixelify-sans.woff2')
UI = os.path.join(ROOT, 'web', 'assets', 'ui')

# tools/site_art.py's colours, as 0.16.0's cut took them
FRAME_HI, FRAME, FRAME_LO = (107, 222, 255), (66, 198, 231), (16, 140, 239)
FIELD = (16, 82, 107)
WHITE = (247, 255, 247)
ORANGE, ORANGE_DARK, ORANGE_LIT = (255, 173, 16), (214, 107, 0), (255, 206, 24)
BUTTON, BUTTON_EDGE, BUTTON_SHADE = (16, 231, 41), (206, 222, 239), (41, 132, 148)
CYAN_TEXT = (107, 222, 255)
GOLD = (255, 214, 16)
# the battle's net under it, and BN's plates (the HP box, a chip's name)
NET, NET_LINE, NET_DOT = (8, 30, 66), (20, 66, 118), (40, 104, 160)
PLATE, PLATE_EDGE = (16, 33, 82), (156, 206, 239)
GAUGE_IN = (8, 22, 52)

SAFE_X, SAFE_Y = 64, 36
S = 3                                      # the look drawn at 427 x 240, then 3x
LW, LH = 427, 240
HP_BOX = (7, 9, 62, 24)
GAUGE = (70, 12, 419, 22)
CUSTOM = (7, 29, 154, 198)
ART = (15, 47, 115, 107)
NAME = (15, 110, 115, 122)
COLUMN = (125, 47, 141, 141)               # five slots
ROW = (15, 141)                            # CHIP SELECT: five slots of 16, 19 apart
PITCH = 19
OK = (113, 162, 147, 190)
GAME = (161, 29, 419, 198)                 # its picture at (170, 34)
PLATE_BOX = (7, 203, 419, 236)


def F(bar, beat=0.0):
    return round(bar * BAR + beat * BEAT)


_fonts = {}


def font(size, bold=False):
    k = (size, bold)
    if k not in _fonts:
        _fonts[k] = ImageFont.truetype(CAPS if bold else CAPS_THIN, size)
    return _fonts[k]


def text(d, xy, words, size, fill, bold=False, anchor='la', shade=None, width=None):
    while width and size > 12 and font(size, bold).getlength(words) > width:
        size -= 1
    if shade:
        d.text((xy[0] + 3, xy[1] + 3), words, font=font(size, bold), fill=shade, anchor=anchor)
    d.text(xy, words, font=font(size, bold), fill=fill, anchor=anchor)


def address(d, cx, cy, words, fill):
    """the site's address in Pixelify Sans, its B and C Silkscreen's"""
    p = ImageFont.truetype(PIXEL, 26)
    k = ImageFont.truetype(CAPS_THIN, 27)
    pb, kb = p.getbbox('B'), k.getbbox('B')
    widths = [k.getlength(c) if c in 'BC' else p.getlength(c) for c in words]
    x = cx - sum(widths) / 2
    y = cy - (pb[1] + pb[3]) / 2
    for c, w in zip(words, widths):
        if c in 'BC':
            d.text((x, y + pb[3] - kb[3]), c, font=k, fill=fill)
        else:
            d.text((x, y), c, font=p, fill=fill, features=['-liga'])
        x += w


# the codes and the rank in letters of our own: Silkscreen's S reads as
# its 5 (and Z as 2) at these sizes, and BN's busting levels run 1 to 10
# and S, so a code or the rank standing alone is drawn from these
GLYPHS = {
    'S': ('.###.', '#...#', '#....', '.###.', '....#', '#...#', '.###.'),
    'H': ('#...#', '#...#', '#...#', '#####', '#...#', '#...#', '#...#'),
    'A': ('.###.', '#...#', '#...#', '#####', '#...#', '#...#', '#...#'),
    'R': ('####.', '#...#', '#...#', '####.', '#.#..', '#..#.', '#...#'),
    'P': ('####.', '#...#', '#...#', '####.', '#....', '#....', '#....'),
}


def glyph(d, cx, cy, ch, px, fill, shade=None):
    """one of GLYPHS centred on (cx, cy), each of its pixels px square"""
    rows = GLYPHS[ch]
    x0, y0 = round(cx - 2.5 * px), round(cy - 3.5 * px)
    for dx, dy, c in ((max(1, px // 2), max(1, px // 2), shade), (0, 0, fill)):
        if c is None:
            continue
        for j, row in enumerate(rows):
            for i, on in enumerate(row):
                if on == '#':
                    d.rectangle([x0 + i * px + dx, y0 + j * px + dy, x0 + (i + 1) * px - 1 + dx,
                                 y0 + (j + 1) * px - 1 + dy], fill=c)


def lerp(a, b, t):
    return tuple(round(x + (y - x) * t) for x, y in zip(a, b))


def icon(name, scale=1):
    im = Image.open(os.path.join(UI, name + '.png')).convert('RGBA')
    return im.resize((im.width * scale, im.height * scale), Image.NEAREST) if scale > 1 else im


# ---- the battle's parts, at 427 x 240 ----

def net(fr):
    """the battle's net: a navy field, a cyan grid drifting down and right
    a pixel every other frame, a dot where its lines cross"""
    im = Image.new('RGB', (LW, LH), NET)
    d = ImageDraw.Draw(im)
    o = (fr // 2) % 16
    for x in range(-16 + o, LW, 16):
        d.line([(x, 0), (x, LH)], fill=NET_LINE)
    for y in range(-16 + o, LH, 16):
        d.line([(0, y), (LW, y)], fill=NET_LINE)
    for x in range(-16 + o, LW, 16):
        for y in range(-16 + o, LH, 16):
            d.point((x, y), fill=NET_DOT)
    return im


def window(d, box):
    """the PET's cyan window round a navy field"""
    x0, y0, x1, y1 = box
    d.rectangle([x0, y0, x1, y1], fill=FRAME_LO)
    d.rectangle([x0, y0, x1 - 1, y1 - 1], fill=FRAME_HI)
    d.rectangle([x0 + 1, y0 + 1, x1 - 1, y1 - 1], fill=FRAME)
    d.rectangle([x0 + 4, y0 + 4, x1 - 4, y1 - 4], fill=FRAME_LO)
    d.rectangle([x0 + 5, y0 + 5, x1 - 5, y1 - 5], fill=FIELD)


def plate(d, box):
    """BN's plate (the HP box, a used chip's name): navy in a light edge"""
    x0, y0, x1, y1 = box
    d.rectangle([x0, y0, x1, y1], fill=PLATE_EDGE)
    d.rectangle([x0 + 1, y0 + 1, x1 - 1, y1 - 1], fill=PLATE)
    d.line([(x0 + 2, y1 - 2), (x1 - 2, y1 - 2)], fill=GAUGE_IN)


def gauge(d, share, fr):
    """the Custom gauge: its fill cyan, gold and flashing when full"""
    x0, y0, x1, y1 = GAUGE
    d.rectangle([x0, y0, x1, y1], fill=PLATE_EDGE)
    d.rectangle([x0 + 1, y0 + 1, x1 - 1, y1 - 1], fill=GAUGE_IN)
    if share > 0:
        full = share >= 1
        c = (GOLD if (fr // 4) % 2 else ORANGE) if full else FRAME
        xe = x0 + 2 + round((x1 - x0 - 4) * min(1.0, share))
        d.rectangle([x0 + 2, y0 + 2, xe, y1 - 2], fill=c)
        d.line([(x0 + 2, y0 + 2), (xe, y0 + 2)], fill=WHITE if full else FRAME_HI)


def brackets(d, box, fr):
    """the cursor: four orange corners round a chip, breathing a pixel"""
    x0, y0, x1, y1 = box
    g = 1 + (fr // 6) % 2
    x0, y0, x1, y1 = x0 - g, y0 - g, x1 + g, y1 + g
    for cx, cy, sx, sy in ((x0, y0, 1, 1), (x1, y0, -1, 1), (x0, y1, 1, -1), (x1, y1, -1, -1)):
        d.line([(cx, cy), (cx + 4 * sx, cy)], fill=ORANGE, width=2)
        d.line([(cx, cy), (cx, cy + 4 * sy)], fill=ORANGE, width=2)
        d.point((cx + sx, cy + sy), fill=ORANGE_LIT)


def slot(d, x, y, n=16):
    d.rectangle([x, y, x + n - 1, y + n - 1], fill=GAUGE_IN, outline=FRAME_LO)


def chip_icon(im, x, y, name, scale=1):
    ic = icon(name, scale)
    im.paste(ic, (x, y), ic)


def three_cards(im, x, y, scale=1):
    """a Server's three: the site's card, the middle one raised"""
    c = icon('card', scale)
    for i, dy in enumerate((2, 0, 2)):
        im.paste(c, (x + i * 2 * scale - 2 * scale, y + dy * scale), c)


# ---- the screens, at video scale ----

CHIPS = [('icon-chip', 'BATTLE', 'S'), ('cards', 'PICK1OF3', 'H'), ('icon-code', 'FOLDERS', 'A'),
         ('icon-star', 'GRUDGE', 'R'), ('icon-bug', 'BUGSHOP', 'P')]


def put_chip(im, x, y, art, scale=1):
    if art == 'cards':
        three_cards(im, x + 2 * scale, y, scale)
    else:
        chip_icon(im, x, y, art, scale)


def battle_screen(fr, picture, label, share, chip, cursor, sent, caption, count=''):
    """a segment's shot: the Custom screen beside the field, the gauge and
    its label over them, the caption on the plate under them"""
    small = net(fr)
    d = ImageDraw.Draw(small)
    plate(d, HP_BOX)
    gauge(d, share, fr)
    window(d, CUSTOM)
    window(d, GAME)
    small.paste(picture.convert('RGB'), (GAME[0] + 9, GAME[1] + 5))
    # the chip's window: its art large, its name, its code
    d.rectangle(ART, fill=lerp(FIELD, FRAME_LO, 0.35), outline=FRAME_HI)
    art, name, code = CHIPS[chip]
    put_chip(small, (ART[0] + ART[2]) // 2 - 24, ART[1] + 6, art, 4)
    plate(d, NAME)
    # the column: the chips sent
    for i in range(5):
        x, y = COLUMN[0], COLUMN[1] + 19 * i
        slot(d, x, y)
        if i < sent:
            put_chip(small, x + 2, y + 2, CHIPS[i][0])
    # CHIP SELECT: the segment's chips, the cursor's brackets on one
    for i in range(5):
        x, y = ROW[0] + PITCH * i, ROW[1]
        slot(d, x, y)
        put_chip(small, x + 2, y + 2, CHIPS[i][0])
    brackets(d, (ROW[0] + PITCH * cursor, ROW[1], ROW[0] + PITCH * cursor + 15, ROW[1] + 15), fr)
    # OK: the PET's green button
    x0, y0, x1, y1 = OK
    d.rectangle([x0 + 2, y0 + 2, x1 + 2, y1 + 2], fill=BUTTON_SHADE)
    d.rectangle([x0, y0, x1, y1], fill=BUTTON_EDGE)
    d.rectangle([x0 + 2, y0 + 2, x1 - 2, y1 - 2], fill=BUTTON)
    plate(d, PLATE_BOX)
    big = small.resize((LW * S, LH * S), Image.NEAREST).crop((0, 0, W, H))
    d = ImageDraw.Draw(big)
    text(d, ((HP_BOX[0] + HP_BOX[2]) * S / 2, (HP_BOX[1] + HP_BOX[3]) * S / 2), VERSION, 30, WHITE, True, 'mm')
    text(d, (GAUGE[0] * S + 14, (GAUGE[1] + GAUGE[3]) * S / 2 + 1), label, 22, WHITE, True, 'lm', shade=GAUGE_IN)
    text(d, (CUSTOM[0] * S + 22, (CUSTOM[1] + 10) * S), 'CUSTOM', 18, CYAN_TEXT, True, 'lm')
    text(d, ((NAME[0] + NAME[2]) * S / 2, (NAME[1] + NAME[3]) * S / 2), name, 24, WHITE, True, 'mm',
         width=(NAME[2] - NAME[0]) * S - 16)
    glyph(d, NAME[0] * S + 20, (NAME[3] + 9) * S, code, 4, GOLD, GAUGE_IN)
    for i in range(5):
        glyph(d, (ROW[0] + PITCH * i + 8) * S, (ROW[1] + 24) * S, CHIPS[i][2], 3, GOLD, GAUGE_IN)
    text(d, ((OK[0] + OK[2]) * S / 2, (OK[1] + OK[3]) * S / 2), 'OK', 40, WHITE, True, 'mm', shade=(8, 140, 24))
    cy = (PLATE_BOX[1] + PLATE_BOX[3]) * S / 2
    text(d, (PLATE_BOX[0] * S + 30, cy), caption, 40, WHITE, True, 'lm', width=980)
    if count:
        text(d, (PLATE_BOX[2] * S - 30, cy), count, 48, GOLD, True, 'rm')
    return big


def results(fr, length):
    """the end card: BN's results in the PET's look, held still"""
    small = net(0)
    d = ImageDraw.Draw(small)
    window(d, (40, 8, 386, 232))
    big = small.resize((LW * S, LH * S), Image.NEAREST).crop((0, 0, W, H))
    d = ImageDraw.Draw(big)
    text(d, (40 * S + 30, 8 * S + 34), 'RESULTS', 22, CYAN_TEXT, True, 'lm')
    lg = Image.open(os.path.join(ROOT, 'linux', 'steam', 'logo.png')).convert('RGBA')
    lg = lg.resize((500, round(lg.height * 500 / lg.width)), Image.LANCZOS)
    y = 74
    big.paste(lg, (W // 2 - 250, y), lg)
    y += lg.height + 44
    rows = (('DELETE TIME', f'{int(length // 60)}:{length % 60:05.2f}', 30), ('BUSTING LEVEL', 'S', 44),
            ('GET DATA', VERSION + ' · OUT NOW', 30))
    for i, (k, v, size) in enumerate(rows):
        yy = y + 56 * i
        text(d, (310, yy), k, 30, CYAN_TEXT, True, 'lm')
        if v in GLYPHS:
            glyph(d, 950, yy - 2, v, 6, GOLD, GAUGE_IN)
        else:
            text(d, (970, yy), v, size, GOLD if i < 2 else WHITE, True, 'rm', shade=GAUGE_IN)
        d.line([(310, yy + 26), (970, yy + 26)], fill=FRAME_LO, width=3)
    y += 56 * 3 + 22
    systems = ('PortMaster handhelds · Linux · Windows · macOS',
               'Android · iPhone and iPad · New 3DS · the browser')
    for i, line in enumerate(systems):
        text(d, (W // 2, y + 34 * i), line, 22, WHITE, False, 'mm')
    address(d, W // 2, y + 90, 'saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless', CYAN_TEXT)
    foot = ("Needs your own Mega Man Battle Network 6: Cybeast Gregar (USA) ROM.",
            "BN5's areas need Mega Man Battle Network 5: Team Colonel (USA) too.",
            'An unofficial fan project, not affiliated with Capcom.')
    for i, line in enumerate(foot):
        text(d, (W // 2, y + 136 + 26 * i), line, 16, (198, 222, 239), False, 'mm')
    return big


def stills(out):
    os.makedirs(out, exist_ok=True)
    shot = Image.open(os.path.join(ROOT, 'docs', 'screenshots', 'battle.png'))
    battle_screen(10, shot, '01 · THE BATTLE FLOOR', 0.62, 0, 0, 1, 'A BATTLE ON EVERY LAYER').save(
        os.path.join(out, 'still_segment.png'))
    shot = Image.open(os.path.join(ROOT, 'docs', 'screenshots', 'codes.png'))
    battle_screen(10, shot, '03 · FOLDERS AND FINDS', 0.4, 2, 2, 3, 'NEW STARTING FOLDERS', '3').save(
        os.path.join(out, 'still_segment_count.png'))
    results(0, 42.24).save(os.path.join(out, 'still_end.png'))


if __name__ == '__main__':
    stills(sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, '.build', 'trailer-0.17.0'))
