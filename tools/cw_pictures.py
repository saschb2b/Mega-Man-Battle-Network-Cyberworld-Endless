"""The pictures tools/cw.py returns: the game's 240x160 cut from the
headless canvas, scaled by whole numbers, several frames on one labelled
sheet, two builds' frames side by side with what differs between them.

Every picture an agent reads costs tokens by its pixels (about one token
for each 750), so the default is the game's picture alone at 2x for one
frame and 1x for a sheet, never the 3x and 2x play.py writes for people.
"""
import io

try:
    from PIL import Image, ImageDraw
except ImportError:   # (the tools' other parts and their tests run without it: CI's)
    Image = ImageDraw = None

GAME_W, GAME_H = 240, 160
CANVAS = (256, 192)   # the headless canvas (--size 256x192): the game's picture in its middle
LONG_EDGE = 1568      # past this the model's reader scales a picture down
SHEET_COLS = 6


def load(path):
    with Image.open(path) as im:
        return im.convert('RGB')


def game(im, full=False):
    """The game's 240x160 where the canvas is the headless one's, else the
    canvas whole (full: always whole)."""
    if full or im.size != CANVAS:
        return im
    x, y = (im.width - GAME_W) // 2, (im.height - GAME_H) // 2
    return im.crop((x, y, x + GAME_W, y + GAME_H))


def scaled(im, scale):
    scale = max(1, min(4, int(scale)))
    return im if scale == 1 else im.resize((im.width * scale, im.height * scale), Image.NEAREST)


def sheet(frames, labels, scale=1, cols=SHEET_COLS):
    """Frames on one picture, in rows of `cols`, each labelled (its frame)."""
    tiles = [scaled(f, scale) for f in frames]
    w, h = tiles[0].size
    cols = max(1, min(cols, len(tiles), LONG_EDGE // w or 1))
    rows = (len(tiles) + cols - 1) // cols
    out = Image.new('RGB', (cols * w + (cols - 1) * 2, rows * h + (rows - 1) * 2), (255, 0, 255))
    draw = ImageDraw.Draw(out)
    for i, (t, label) in enumerate(zip(tiles, labels)):
        x, y = (i % cols) * (w + 2), (i // cols) * (h + 2)
        out.paste(t, (x, y))
        tag(draw, x + 2, y + h - 11, str(label))
    return out


def tag(draw, x, y, text):
    """A label readable on any picture: yellow on a black box."""
    draw.rectangle((x, y, x + 6 * len(text) + 2, y + 10), fill=(0, 0, 0))
    draw.text((x + 2, y), text, fill=(255, 255, 0))


def difference(a, b):
    """How two frames differ: (pixels that do, their bounding box or None)."""
    if a.size != b.size:
        return a.width * a.height, (0, 0, a.width, a.height)
    pa, pb = a.tobytes(), b.tobytes()
    n, x0, y0, x1, y1 = 0, a.width, a.height, -1, -1
    for i in range(0, len(pa), 3):
        if pa[i:i + 3] != pb[i:i + 3]:
            n += 1
            x, y = (i // 3) % a.width, (i // 3) // a.width
            x0, y0, x1, y1 = min(x0, x), min(y0, y), max(x1, x), max(y1, y)
    return n, ((x0, y0, x1 + 1, y1 + 1) if n else None)


def side_by_side(pairs, labels, scale=1):
    """Each pair (old, new) on a row, a gap between, a red frame round what
    differs on the new side."""
    rows = []
    for (a, b), label in zip(pairs, labels):
        a, b = scaled(a, scale), scaled(b, scale)
        row = Image.new('RGB', (a.width + 4 + b.width, max(a.height, b.height)), (255, 0, 255))
        row.paste(a, (0, 0))
        row.paste(b, (a.width + 4, 0))
        draw = ImageDraw.Draw(row)
        _, box = difference(a, b)
        if box:
            x0, y0, x1, y1 = box
            draw.rectangle((a.width + 4 + x0, y0, a.width + 4 + x1 - 1, y1 - 1), outline=(255, 0, 0))
        tag(draw, 2, row.height - 11, f'{label} old')
        tag(draw, a.width + 6, row.height - 11, f'{label} new')
        rows.append(row)
    out = Image.new('RGB', (max(r.width for r in rows), sum(r.height for r in rows) + 2 * (len(rows) - 1)), (255, 0, 255))
    y = 0
    for r in rows:
        out.paste(r, (0, y))
        y += r.height + 2
    return out


def png(im):
    buf = io.BytesIO()
    im.save(buf, 'PNG', optimize=True)
    return buf.getvalue()


def tokens(size):
    """What a picture of this size costs the model to read, about: its
    pixels over 750, after the reader scales it to fit 1568 on its long
    edge and about 1.15 megapixels."""
    w, h = size
    k = min(1.0, LONG_EDGE / max(w, h), (1_150_000 / (w * h)) ** 0.5)
    return int(w * k * h * k / 750)
