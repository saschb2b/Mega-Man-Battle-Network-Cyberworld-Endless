#!/usr/bin/env python3
"""The trailers. The current one is docs/clips/trailer.mp4, .webm and .png
and trailer-play.png, which the project page, the README, every page's
preview picture and the AltStore source show. A release that earns a new
one (AGENTS.md, Trailers) replaces them, and the one before stays at its
release's tag.

Each trailer is a cut in tools/trailers/ (trailers.VERSIONS): its footage,
scripted headless runs of the game from the developer's ROMs; its look,
drawn there over whole frames of the game at 4x; and its music, an
original piece played through tools/trailer_music.py's model of the Game
Boy Advance's sound. None of Capcom's sound is in any. This file is what
they share: the footage's capture, a scene's frames and the encoding.

    python3 build.py host && python3 tools/trailer.py [VERSION] [--keep] [--footage] [--frames A,B,...]

VERSION is the current trailer by default; only the current one writes
docs/clips, an older one .build/trailer-VERSION/out. --keep reuses the
footage of an earlier run, --footage only captures it, and --frames draws
only those frames (into .build/trailer-VERSION/frames), to look at.
"""
import os
import shutil
import subprocess
import sys
import threading

from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import build  # noqa: E402  (its docker helper, ROM folder, media entries and ffmpeg image)
import trailer_music  # noqa: E402
import trailers  # noqa: E402

OUT = os.path.join(ROOT, 'docs', 'clips')
FPS, W, H = 30, 1280, 720
HEAVY = '/usr/share/fonts/truetype/lato/Lato-Black.ttf'
NARROW = '/usr/share/fonts/opentype/urw-base35/NimbusSansNarrow-BoldOblique.otf'
WORK = None                  # .build/trailer-VERSION: the footage, the music and the frames


# ---- footage: scripted headless runs, every second frame (30 fps) ----

def capture(name, args, env, script, first, last, mounts=None):
    """A scene's footage: the game run headless with `args`, its input
    `script` and `env`, every second frame from `first` to `last` kept as
    NNNN.png in WORK/name (the ROM folder mounted whole unless `mounts`
    says otherwise; safe to run several at once)."""
    out = os.path.join(WORK, name)
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    data = os.path.join(WORK, 'data-' + name)
    shutil.rmtree(data, ignore_errors=True)
    os.makedirs(data)
    extra = ['--input', script] if script else []
    rel = os.path.relpath(WORK, ROOT)
    code = build.docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir',
                        f'/src/{rel}/data-{name}', *args, *extra, '--frames', str(last + 1),
                        '--shot-range', f'{first}:{last}:/src/{rel}/{name}/f',
                        mounts=mounts or [(build.default_rom_dir(), '/rom:ro')], env=sorted(env.items()))
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


def capture_all(scenes, keep, bn6_only=()):
    """Every scene's footage, eight runs at once; those in `bn6_only` with
    BN6's ROM alone, as most players have it."""
    todo = [n for n in scenes if not (keep and os.path.isdir(os.path.join(WORK, n))
                                      and os.listdir(os.path.join(WORK, n)))]
    gate = threading.Semaphore(8)
    bn6 = build.docs_rom_mounts()

    def run(name):
        with gate:
            capture(name, *scenes[name], mounts=bn6 if name in bn6_only else None)
            print('captured', name)

    threads = [threading.Thread(target=run, args=(n,)) for n in todo]
    for t in threads:
        t.start()
    for t in threads:
        t.join()


def from_shots(name, first, last):
    """A scene from build.py's SCREENSHOTS entry `name`: its run, its input
    and its environment, game frames first to last"""
    e = next(x for x in build.SCREENSHOTS if x[0] == name)
    argv = list(e[1])
    script = None
    if '--input' in argv:
        i = argv.index('--input')
        script = argv[i + 1]
        del argv[i:i + 2]
    return (argv, dict(e[3]), script, first, last)


def from_clips(name):
    """A scene from build.py's CLIPS entry `name`, as build.py clips runs it"""
    e = next(x for x in build.CLIPS if x[0] == name)
    return (e[1], dict(e[2]), e[3], e[4], e[5])


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


# ---- the video ----

def encode(out, poster, label):
    """The frames and music.wav in WORK into `out`/trailer.mp4 and .webm;
    frame `poster` as trailer.png, and with a play button labelled `label`
    as trailer-play.png."""
    os.makedirs(out, exist_ok=True)
    rel, dest = os.path.relpath(WORK, ROOT), os.path.relpath(out, ROOT)
    common = ['-y', '-loglevel', 'error', '-framerate', str(FPS), '-i', f'/src/{rel}/frames/%05d.png',
              '-i', f'/src/{rel}/music.wav', '-shortest']
    jobs = [common + ['-c:v', 'libx264', '-crf', '27', '-preset', 'slow', '-maxrate', '3M', '-bufsize', '6M', '-pix_fmt', 'yuv420p', '-c:a', 'aac', '-b:a', '160k',
                      '-movflags', '+faststart', f'/src/{dest}/trailer.mp4'],
            common + ['-c:v', 'libvpx-vp9', '-b:v', '2M', '-crf', '38', '-row-mt', '1', '-pix_fmt', 'yuv420p', '-c:a', 'libopus',
                      '-b:a', '128k', f'/src/{dest}/trailer.webm']]
    for i, args in enumerate(jobs):
        if subprocess.call(['docker', 'run', '--rm', '--name', f'cw-trailer-{os.getpid()}-{i}', '-u', f'{os.getuid()}:{os.getgid()}',
                            '-v', f'{ROOT}:/src', build.FFMPEG_IMAGE, *args]) != 0:
            sys.exit('ffmpeg failed')
    Image.open(os.path.join(WORK, 'frames', f'{poster:05d}.png')).save(os.path.join(out, 'trailer.png'), optimize=True)
    play_poster(out, label)


def play_poster(out, label):
    """trailer-play.png: the poster with a play button, for the README and
    the releases' notes, which link the MP4 (GitHub plays no video from a
    repository)."""
    im = Image.open(os.path.join(out, 'trailer.png')).convert('RGBA')
    over = Image.new('RGBA', im.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(over)
    font = ImageFont.truetype(NARROW, 44)
    w, h = int(d.textlength(label, font=font)) + 150, 92
    x0, y0 = (W - w) // 2, 572
    d.rounded_rectangle([x0, y0, x0 + w, y0 + h], radius=h // 2, fill=(7, 24, 58, 235), outline=(66, 198, 231, 255), width=5)
    cx, cy = x0 + 58, y0 + h // 2
    d.polygon([(cx - 14, cy - 20), (cx - 14, cy + 20), (cx + 22, cy)], fill=(255, 214, 16, 255))
    d.text((x0 + 100, cy), label, font=font, fill=(255, 255, 255, 255), anchor='lm')
    im.alpha_composite(over)
    im.convert('RGB').save(os.path.join(out, 'trailer-play.png'), optimize=True)


def main():
    global WORK
    args = sys.argv[1:]
    only = None
    if '--frames' in args:
        i = args.index('--frames')
        only = {int(x) for x in args[i + 1].split(',')}
        del args[i:i + 2]
    version = next((a for a in args if not a.startswith('--')), None)
    cut = trailers.module(version)
    WORK = os.path.join(ROOT, '.build', 'trailer-' + cut.VERSION)
    os.makedirs(WORK, exist_ok=True)
    capture_all(cut.scenes(), '--keep' in args, cut.BN6_ONLY)
    if hasattr(cut, 'footage'):
        cut.footage('--keep' in args)   # (a cut's own runs: 0.13.0's games in a Square, tools/trailer_online.py)
    if '--footage' in args:
        return
    trailer_music.render(os.path.join(WORK, 'music.wav'), trailers.music(cut.VERSION))
    cut.compose(only)
    if only is not None:
        return
    out = OUT if cut.VERSION == trailers.CURRENT else os.path.join(WORK, 'out')
    encode(out, cut.POSTER, cut.PLAY_LABEL)
    print(f'wrote {os.path.relpath(out, ROOT)}/trailer.mp4, .webm, .png and -play.png')


if __name__ == '__main__':
    import trailer   # (the cuts import this file by name: one module, one WORK)
    trailer.main()
