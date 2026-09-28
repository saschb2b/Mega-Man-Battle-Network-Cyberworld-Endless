#!/usr/bin/env python3
"""docs/clips/trailer.mp4, .webm and .png: a 20-second trailer in the manner of
a 2003 game advert. Chrome words slam in on the beat, white flashes cut
between shots, the picture punches in, scanlines over it all, and a battle
theme under it, 150 beats a minute, in the manner of the game's own.

The shots are whole frames of the game running from the developer's ROM,
at 4x (as build.py clips makes them); everything else is drawn here: the
backdrop, the words, the logo (tools/steam_art.py's) and the music
(tools/trailer_music.py), an original piece played through a model of the
Game Boy Advance's sound. None of Capcom's sound is in it.

    python3 build.py host && python3 tools/trailer.py [--keep]

--keep reuses the footage of an earlier run (.build/trailer).
"""
import math
import os
import random
import shutil
import subprocess
import sys

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)
import build  # noqa: E402  (its docker helper, ROM folder and ffmpeg image)
import trailer_music  # noqa: E402

WORK = os.path.join(ROOT, '.build', 'trailer')
OUT = os.path.join(ROOT, 'docs', 'clips')
FPS, W, H = 30, 1280, 720
BPM = 150
BEAT = 12                      # frames a beat at 30 fps
BAR = 4 * BEAT
BARS = 13
FRAMES = BARS * BAR + 18       # (and a breath of black at the end)
HEAVY = '/usr/share/fonts/truetype/lato/Lato-Black.ttf'
NARROW = '/usr/share/fonts/opentype/urw-base35/NimbusSansNarrow-BoldOblique.otf'

# ---- footage: scripted headless runs, every second frame (30 fps) ----

SCENES = {
    'jackin': (['--scene', 'title'], {}, '120:,4:START,60:,4:A,200:', 60, 240),
    'run': (build.RUN_7, {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 100, 2700),
    'fights': (['--scene', 'emu', '--run-depth', '5', '--seed', '41', '--net-biome', '1'],
               {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 100, 3700),
    'undernet': (['--scene', 'emu', '--net-biome', '5', '--run-depth', '14', '--seed', '3', '--dev', 'quiet'], {},
                 '150:,70:RIGHT,50:UP,70:LEFT,50:DOWN,100:', 150, 480),
}

# A HeatCross and a BeastOut, played once through tools/play.py and kept as
# its commands: a walk into a Seaside battle with every Cross and BeastOut
# open (--dev powers), CROSSSELECT's HEAT with two swords, OK; two turns
# later the BeastOut star, OK. Its footage starts at the first OK (frame
# 2083): the Cross's fire, flash and HeatCross from picture 61, the BeastOut's aura,
# flash and beast from 712 (the flash on the bar's third beat).
POWERS_ARGS = ['--dev', 'powers', '--scene', 'emu', '--run-depth', '3', '--net-biome', '1', '--seed', '41']
POWERS_PLAY = [
    'wait 1', 'wait 300; hold RIGHT 120; hold DOWN 120; hold LEFT 120; hold UP 120',
    'hold RIGHT 90; hold DOWN 90; hold LEFT 90; hold UP 90', 'hold RIGHT 90; hold DOWN 90; hold LEFT 90; hold UP 90',
    'hold RIGHT 90; hold DOWN 90; hold LEFT 90; hold UP 90', 'wait 60', 'wait 2; wait 2', 'press A; wait 30',
    'press LEFT; press LEFT; press LEFT; press LEFT; press A; press RIGHT; press A; wait 10; press START; wait 10',
    'press A; wait 36', 'wait 240', 'wait 400; press R; wait 40', 'wait 60; press R; wait 40',
    'press RIGHT; press RIGHT; press RIGHT; press RIGHT; press RIGHT; press DOWN; wait 10', 'press A; wait 20',
    'wait 336', 'press UP; wait 8', 'press A; wait 36', 'wait 300',
]
POWERS_FIRST, CROSS_SHOT, BEAST_SHOT = 2083, 61, 712


def powers_scene():
    sys.path.insert(0, os.path.join(ROOT, 'tools'))
    import play
    steps = [s for s in play.steps('; '.join(POWERS_PLAY)) if s != 'shot']
    script = ','.join(f'{n}:{b}' for n, b in steps)
    total = sum(n for n, _ in steps)
    return (POWERS_ARGS, {}, script, POWERS_FIRST, total - 2)


SCENES['powers'] = None   # (filled in main: it needs tools/play.py's parser)


def capture(name, args, env, script, first, last):
    out = os.path.join(WORK, name)
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    data = os.path.join(WORK, 'data-' + name)
    shutil.rmtree(data, ignore_errors=True)
    os.makedirs(data)
    saved = {k: os.environ.get(k) for k in env}
    os.environ.update(env)
    extra = ['--input', script] if script else []
    code = build.docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir',
                        f'/src/.build/trailer/data-{name}', *args, *extra, '--frames', str(last + 1),
                        '--shot-range', f'{first}:{last}:/src/.build/trailer/{name}/f',
                        mounts=[(build.default_rom_dir(), '/rom:ro')])
    for k, v in saved.items():
        if v is None:
            os.environ.pop(k, None)
        else:
            os.environ[k] = v
    if code:
        sys.exit(f'capturing {name} failed')
    n = 0
    for f in range(first, last + 1):
        p = os.path.join(out, f'f{f:05d}.bmp')
        if (f - first) % 2 == 0:
            im = Image.open(p).convert('RGB')
            w, h = im.size   # the canvas: the game's 240 x 160 in the middle
            im.crop(((w - 240) // 2, (h - 160) // 2, (w + 240) // 2, (h + 160) // 2)).save(os.path.join(out, f'{n:04d}.png'))
            n += 1
        os.remove(p)


_frames = {}


def shot(scene, i):
    """Frame i of a scene's footage (30 fps), or a screenshot by name."""
    key = (scene, i)
    if key not in _frames:
        if scene == 'still':
            path = os.path.join(ROOT, 'docs', 'screenshots', i + '.png')
        else:
            # (past the end of a scene's footage, its last frame)
            last = len([f for f in os.listdir(os.path.join(WORK, scene)) if f.endswith('.png')]) - 1
            path = os.path.join(WORK, scene, f'{min(i, last):04d}.png')
        _frames[key] = Image.open(path).convert('RGB')
    return _frames[key]


# ---- the look ----

def lerp(a, b, t):
    return tuple(round(x + (y - x) * t) for x, y in zip(a, b))


def chrome(text, size, slant=0.22, font=HEAVY, top=(255, 255, 255), mid=(150, 215, 255),
           low=(10, 40, 130), shine=(90, 200, 255), glow=(30, 130, 255)):
    """Words as the adverts set them: a chrome fill with a dark horizon, a
    black edge, a blue glow, leaning forward."""
    f = ImageFont.truetype(font, size)
    l, t, r, b = f.getbbox(text)
    w, h = r - l, b - t
    stroke = max(4, size // 14)
    pad = stroke * 4 + 24
    cw, ch = w + pad * 2 + int(h * slant) + 8, h + pad * 2
    mask = Image.new('L', (cw, ch), 0)
    ImageDraw.Draw(mask).text((pad - l, pad - t), text, font=f, fill=255)
    edge = Image.new('L', (cw, ch), 0)
    ImageDraw.Draw(edge).text((pad - l, pad - t), text, font=f, fill=255, stroke_width=stroke, stroke_fill=255)
    halo = edge.filter(ImageFilter.MaxFilter(9)).filter(ImageFilter.GaussianBlur(10))
    # the chrome: sky above the horizon, dark metal below, a shine at the foot
    ramp = Image.new('RGB', (1, ch))
    for y in range(ch):
        v = (y - pad) / max(h, 1)
        if v < 0.48:
            c = lerp(top, mid, max(v, 0) / 0.48)
        elif v < 0.56:
            c = lerp((40, 70, 150), low, (v - 0.48) / 0.08)
        else:
            c = lerp(low, shine, min((v - 0.56) / 0.44, 1))
        ramp.putpixel((0, y), c)
    fill = ramp.resize((cw, ch))
    out = Image.new('RGBA', (cw, ch), (0, 0, 0, 0))
    out.paste(glow + (255,), (0, 0), halo.point(lambda a: a * 3 // 4))
    out.paste((5, 10, 25, 255), (0, 0), edge)
    out.paste(fill, (0, 0), mask)
    # a thin light line along the letters' tops
    top_line = ImageChops.subtract(mask, mask.transform(mask.size, Image.AFFINE, (1, 0, 0, 0, 1, -3)))
    out.paste((255, 255, 255, 255), (0, 0), top_line.point(lambda a: a // 2))
    return out.transform((cw, ch), Image.AFFINE, (1, slant, -slant * ch * 0.6, 0, 1, 0), Image.BICUBIC)


def small(text, size=34, color=(210, 235, 255)):
    f = ImageFont.truetype(NARROW, size)
    l, t, r, b = f.getbbox(text)
    im = Image.new('RGBA', (r - l + 16, b - t + 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.text((8 - l, 8 - t), text, font=f, fill=color, stroke_width=3, stroke_fill=(5, 10, 30))
    return im


def backdrop():
    """The net behind the picture: a dark blue with its grid's diagonals, to
    be scrolled, and a lighter middle."""
    tile = 64
    im = Image.new('RGB', (W + tile, H + tile))
    px = im.load()
    for y in range(H + tile):
        for x in range(W + tile):
            d = math.hypot((x - W / 2) / W, (y - H / 2) / H)
            base = lerp((14, 46, 96), (2, 8, 24), min(d * 1.6, 1))
            if (x + y) % tile < 2 or (x - y) % tile < 2:
                base = lerp(base, (40, 130, 200), 0.45)
            px[x, y] = base
    return im


def scanlines():
    im = Image.new('RGBA', (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for y in range(0, H, 3):
        d.line([(0, y), (W, y)], fill=(0, 0, 0, 46))
    # and the corners darker
    v = Image.new('L', (W // 8, H // 8))
    vp = v.load()
    for y in range(H // 8):
        for x in range(W // 8):
            d2 = math.hypot((x - W / 16) / (W / 16), (y - H / 16) / (H / 16))
            vp[x, y] = int(max(0, min(170, (d2 - 0.75) * 260)))
    im.alpha_composite(Image.merge('RGBA', (Image.new('L', (W, H)),) * 3 + (v.resize((W, H), Image.BICUBIC),)))
    return im


def panel(frame, zoom=1.0):
    """The game's picture at 4x, framed as a screen in the adverts."""
    s = 4 * zoom
    w, h = round(240 * s), round(160 * s)
    pic = frame.resize((w, h), Image.NEAREST)
    frame_im = Image.new('RGBA', (w + 20, h + 20), (0, 0, 0, 0))
    d = ImageDraw.Draw(frame_im)
    d.rectangle([0, 0, w + 19, h + 19], fill=(8, 20, 50, 255))
    d.rectangle([3, 3, w + 16, h + 16], fill=(120, 210, 255, 255))
    d.rectangle([6, 6, w + 13, h + 13], fill=(10, 40, 90, 255))
    frame_im.paste(pic, (10, 10))
    return frame_im


def logo():
    lg = Image.open(os.path.join(ROOT, 'linux', 'steam', 'logo.png')).convert('RGBA')
    return lg


def speed_lines(im, t, seed, strength):
    """Streaks from the middle, as an impact's."""
    rnd = random.Random(seed)
    layer = Image.new('RGBA', (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    cx, cy = W / 2, H / 2
    for _ in range(46):
        a = rnd.uniform(0, 2 * math.pi)
        r0 = rnd.uniform(260, 420) + t * 40
        r1 = r0 + rnd.uniform(160, 420)
        wdt = rnd.uniform(0.006, 0.02)
        pts = [(cx + math.cos(a) * r0, cy + math.sin(a) * r0),
               (cx + math.cos(a + wdt) * r1, cy + math.sin(a + wdt) * r1),
               (cx + math.cos(a - wdt) * r1, cy + math.sin(a - wdt) * r1)]
        d.polygon(pts, fill=(255, 255, 255, int(150 * strength)))
    im.alpha_composite(layer)


# ---- the timeline ----
# Each shot: (bar, beat, beats long, scene, first frame, frames advance a
# frame, zoom punch on its start). Words: (bar, beat, beats, text, where).

def B(bar, beat=0):
    return bar * BAR + beat * BEAT


SHOTS = [
    (1, 0, 4, 'jackin', 30, 1, True),
    (2, 0, 2, 'fights', 100, 1, True), (2, 2, 1, 'run', 150, 1, False), (2, 3, 1, 'undernet', 40, 1, False),
    (3, 0, 1, 'fights', 262, 1, True), (3, 1, 1, 'fights', 300, 1, False), (3, 2, 2, 'fights', 372, 1, False),
    (4, 0, 1, 'fights', 1528, 1, True), (4, 1, 1, 'fights', 532, 1, False),
    (4, 2, 2, 'powers', CROSS_SHOT, 1, True),
    (5, 0, 4, 'powers', BEAST_SHOT, 1, True),
    (6, 0, 4, 'run', 596, 1, True),
    (7, 0, 2, 'run', 655, 1, True), (7, 2, 1, 'run', 1040, 1, False), (7, 3, 1, 'run', 1078, 1, False),
    (8, 0, 4, 'undernet', 60, 1, True),
    (9, 0, 4, 'still', 'setup', 0, True),
    (10, 0, 4, 'still', 'marks', 0, True),
]

WORDS = [
    (0, 1, 1, 'THE NET', 'mid'), (0, 2, 1, 'IS', 'mid'), (0, 3, 1, 'ENDLESS', 'mid'),
    (1, 0, 2, 'JACK IN!', 'low'),
    (2, 0, 2, 'A NEW NET', 'low'), (2, 2, 2, 'EVERY RUN', 'low'),
    (3, 0, 4, 'BATTLE!', 'low'),
    (4, 0, 2, 'CHIPS.', 'low'), (4, 2, 2, 'CROSSES.', 'low'),
    (5, 0, 4, 'BEASTOUT!', 'low'),
    (6, 0, 4, 'GUARDIANS', 'low'),
    (7, 0, 4, 'WHO REMEMBER YOU', 'top'),
    (8, 0, 4, 'GO DEEPER', 'low'),
    (9, 0, 2, 'EVERY RUN', 'top'), (9, 2, 2, 'LEAVES SOMETHING', 'top'),
    (10, 0, 4, 'EARN THE MARKS', 'low'),
]

CAPTIONS = [
    (2, 0, 4, 'NEW LAYERS EVERY DIVE  ·  IN BN6\'S OWN MAPS'),
    (8, 0, 4, 'THE UNDERNET  ·  THE SECRET AREA  ·  THE CYBEAST NEST'),
    (9, 0, 4, 'NEW FOLDERS  ·  THREAT RUNGS  ·  HELPERS'),
    (10, 0, 4, 'BN6\'S OWN TITLE MARKS, FOR WHAT YOUR RUNS DID'),
]


def compose():
    frames_dir = os.path.join(WORK, 'frames')
    shutil.rmtree(frames_dir, ignore_errors=True)
    os.makedirs(frames_dir)
    bg = backdrop()
    lines = scanlines()
    word_im = {text: chrome(text, 150 if len(text) < 9 else 118 if len(text) < 14 else 96) for _, _, _, text, _ in WORDS}
    cap_im = {c[3]: small(c[3]) for c in CAPTIONS}
    lg = logo()
    hero = Image.open(os.path.join(ROOT, 'linux', 'steam', 'hero.png')).convert('RGB')
    hero = hero.resize((round(hero.width * H / hero.height), H), Image.NEAREST)
    subtitle = small('A ROGUELIKE FOR MEGA MAN BATTLE NETWORK 6', 40, (255, 240, 160))
    plats = small('WINDOWS  ·  MAC  ·  LINUX  ·  STEAM DECK  ·  ROCKNIX  ·  BROWSER', 34)
    free = chrome('FREE  ·  BRING YOUR OWN ROM', 60, slant=0.18)
    url = small('saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless', 30, (170, 220, 255))
    notice = small('Unofficial fan project, not affiliated with Capcom. Runs from your own Mega Man Battle Network 6: '
                   'Cybeast Gregar (USA) ROM.', 20, (150, 170, 200))
    cuts = {B(s[0], s[1]) for s in SHOTS} | {B(0, b) for b in (1, 2, 3)} | {B(11)} | {B(12, b) for b in (0, 1, 2)}
    rnd = random.Random(7)
    for fr in range(FRAMES):
        bar, beat, sub = fr // BAR, (fr % BAR) // BEAT, fr % BEAT
        im = bg.crop((fr * 2 % 64, fr % 64, fr * 2 % 64 + W, fr % 64 + H)).convert('RGBA')
        impact = min((fr - c for c in cuts if c <= fr), default=99)
        # the shot
        if 1 <= bar <= 10:
            s = [x for x in SHOTS if B(x[0], x[1]) <= fr < B(x[0], x[1]) + x[2] * BEAT][-1]
            start = B(s[0], s[1])
            if s[3] == 'still':
                pic = shot('still', s[4])
            else:
                pic = shot(s[3], s[4] + (fr - start) * s[5])
            zoom = 1.0 + (0.07 * max(0, 6 - (fr - start)) / 6 if s[6] else 0) + 0.012 * (fr - start) / (s[2] * BEAT)
            p = panel(pic, zoom)
            im.alpha_composite(p, ((W - p.width) // 2, (H - p.height) // 2))
        elif bar == 0:
            # the intro: static and glitch over black
            im = Image.new('RGBA', (W, H), (0, 0, 0, 255))
            if rnd.random() < 0.35:
                noise = Image.effect_noise((W // 4, H // 4), 64).resize((W, H), Image.NEAREST)
                im.alpha_composite(Image.merge('RGBA', (noise, noise, noise, noise.point(lambda a: 40))))
            if sub < 3 and beat >= 1:
                lg2 = lg.resize((lg.width // 2, lg.height // 2), Image.NEAREST)
                off = rnd.randint(-30, 30)
                im.alpha_composite(lg2, ((W - lg2.width) // 2 + off, 90))
        elif bar == 11:
            # the logo over the net
            k = fr - B(11)
            x = -int((hero.width - W) * min(k / (2 * BAR), 1) * 0.6)
            im.paste(hero, (x, 0))
            im.alpha_composite(Image.new('RGBA', (W, H), (0, 0, 20, 90)))
            sc = 1.0 + 0.9 * max(0, 5 - k) / 5
            lg2 = lg.resize((round(lg.width * 0.95 * sc), round(lg.height * 0.95 * sc)), Image.NEAREST)
            im.alpha_composite(lg2, ((W - lg2.width) // 2, 180 - (lg2.height - int(lg.height * 0.95)) // 2))
            if k >= 12:
                im.alpha_composite(subtitle, ((W - subtitle.width) // 2, 470))
            # a flare across the mark
            fx = int(200 + k * 22)
            flare = Image.new('RGBA', (W, H), (0, 0, 0, 0))
            fd = ImageDraw.Draw(flare)
            for r, a in ((70, 30), (40, 60), (16, 140)):
                fd.ellipse([fx - r, 250 - r, fx + r, 250 + r], fill=(255, 250, 220, a))
            fd.rectangle([fx - 420, 247, fx + 420, 253], fill=(255, 255, 255, 90))
            im.alpha_composite(flare.filter(ImageFilter.GaussianBlur(6)))
        else:
            # the end card
            k = fr - B(12)
            im.alpha_composite(Image.new('RGBA', (W, H), (0, 0, 30, 150)))
            lg2 = lg.resize((lg.width * 3 // 5, lg.height * 3 // 5), Image.NEAREST)
            im.alpha_composite(lg2, ((W - lg2.width) // 2, 70))
            if k >= 3:
                im.alpha_composite(free, ((W - free.width) // 2, 270))
            if k >= BEAT:
                im.alpha_composite(plats, ((W - plats.width) // 2, 420))
            if k >= 2 * BEAT:
                im.alpha_composite(url, ((W - url.width) // 2, 500))
            im.alpha_composite(notice, ((W - notice.width) // 2, 650))
        # the words: in with a slam (big, trailing), out with a stretch
        for (wb, wbeat, wlen, text, where) in WORDS:
            start = B(wb, wbeat)
            k = fr - start
            n = wlen * BEAT
            if not 0 <= k < n:
                continue
            w = word_im[text]
            if k < 4:
                sc = 1.0 + (4 - k) * 0.35
                alpha = 0.4 + 0.15 * k
            elif k >= n - 3:
                sc = 1.0 + (k - (n - 3)) * 0.12
                alpha = max(0.0, 1 - (k - (n - 3)) * 0.33)
            else:
                sc, alpha = 1.0 + 0.004 * (k - 4), 1.0
            ww = w.resize((max(1, round(w.width * sc)), max(1, round(w.height * sc))), Image.BILINEAR)
            if alpha < 1:
                ww.putalpha(ww.getchannel('A').point(lambda a, al=alpha: int(a * al)))
            # (low: over the picture's floor; top: clear of a chat box or a screen's lines)
            y = {'mid': (H - ww.height) // 2, 'top': 170 - ww.height // 2}.get(where, H - 170 - ww.height // 2)
            im.alpha_composite(ww, ((W - ww.width) // 2, y))
            if k < 4:
                speed_lines(im, k, start, 1 - k / 4)
        for (cb, cbeat, clen, text) in CAPTIONS:
            k = fr - B(cb, cbeat)
            if 6 <= k < clen * BEAT - 2:
                c = cap_im[text]
                im.alpha_composite(c, ((W - c.width) // 2, 34))
        rgb = im.convert('RGB')
        # the cut's flash, the shake and the colours split on impact
        if impact < 3:
            rgb = Image.blend(rgb, Image.new('RGB', (W, H), (255, 255, 255)), (0.85, 0.45, 0.15)[impact])
        if impact < 5:
            dx, dy = rnd.randint(-8, 8) * (5 - impact) // 5, rnd.randint(-6, 6) * (5 - impact) // 5
            rgb = ImageChops.offset(rgb, dx, dy)
            r, g, b = rgb.split()
            sh = 5 - impact
            rgb = Image.merge('RGB', (ImageChops.offset(r, -sh, 0), g, ImageChops.offset(b, sh, 0)))
        out = rgb.convert('RGBA')
        out.alpha_composite(lines)
        # the last beat fades to black
        end = B(BARS) - 6
        if fr > end:
            out = Image.blend(out, Image.new('RGBA', (W, H), (0, 0, 0, 255)), min(1, (fr - end) / 18))
        out.convert('RGB').save(os.path.join(frames_dir, f'{fr:05d}.png'), compress_level=1)
        if fr % 60 == 0:
            print('frame', fr, '/', FRAMES)


def encode():
    rel = os.path.relpath(WORK, ROOT)
    common = ['-y', '-loglevel', 'error', '-framerate', str(FPS), '-i', f'/src/{rel}/frames/%05d.png',
              '-i', f'/src/{rel}/music.wav', '-shortest']
    jobs = [common + ['-c:v', 'libx264', '-crf', '27', '-preset', 'slow', '-maxrate', '3M', '-bufsize', '6M', '-pix_fmt', 'yuv420p', '-c:a', 'aac', '-b:a', '160k',
                      '-movflags', '+faststart', '/src/docs/clips/trailer.mp4'],
            common + ['-c:v', 'libvpx-vp9', '-b:v', '2M', '-crf', '38', '-row-mt', '1', '-pix_fmt', 'yuv420p', '-c:a', 'libopus',
                      '-b:a', '128k', '/src/docs/clips/trailer.webm']]
    for args in jobs:
        if subprocess.call(['docker', 'run', '--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src',
                            build.FFMPEG_IMAGE, *args]) != 0:
            sys.exit('ffmpeg failed')
    poster = Image.open(os.path.join(WORK, 'frames', f'{B(11) + 20:05d}.png'))
    poster.save(os.path.join(OUT, 'trailer.png'), optimize=True)
    play_poster()


def play_poster():
    """docs/clips/trailer-play.png: the poster with a play button, for the
    README, which links the MP4 (GitHub plays no video from a repository)."""
    im = Image.open(os.path.join(OUT, 'trailer.png')).convert('RGBA')
    over = Image.new('RGBA', im.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(over)
    label = 'WATCH THE TRAILER'
    font = ImageFont.truetype(NARROW, 44)
    w, h = int(d.textlength(label, font=font)) + 150, 92
    x0, y0 = (W - w) // 2, 572
    d.rounded_rectangle([x0, y0, x0 + w, y0 + h], radius=h // 2, fill=(7, 24, 58, 235), outline=(66, 198, 231, 255), width=5)
    cx, cy = x0 + 58, y0 + h // 2
    d.polygon([(cx - 14, cy - 20), (cx - 14, cy + 20), (cx + 22, cy)], fill=(255, 214, 16, 255))
    d.text((x0 + 100, cy), label, font=font, fill=(255, 255, 255, 255), anchor='lm')
    im.alpha_composite(over)
    im.convert('RGB').save(os.path.join(OUT, 'trailer-play.png'), optimize=True)


def main():
    keep = '--keep' in sys.argv
    os.makedirs(WORK, exist_ok=True)
    SCENES['powers'] = powers_scene()
    for name, spec in SCENES.items():
        if not (keep and os.path.isdir(os.path.join(WORK, name)) and os.listdir(os.path.join(WORK, name))):
            capture(name, *spec)
    trailer_music.render(os.path.join(WORK, 'music.wav'))
    compose()
    encode()
    print('wrote docs/clips/trailer.mp4, .webm and .png')


if __name__ == '__main__':
    main()
