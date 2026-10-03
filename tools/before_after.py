#!/usr/bin/env python3
"""before_after.py OLD [--new LABEL] [NAMES]: the net as release OLD drew it and
as this build does, side by side, for a release's notes (docs/releases/).

OLD's host binary is built once in a worktree of its tag (.build/before/OLD,
build.py host there); each scene below runs headless with it and with this
build's (build/host/cyberworld, build it first), on BN6's ROM alone, and
docs/screenshots/compare-NAME.png gets both frames at 2x, OLD's on the left,
each under its version. A layer seed makes the same layout in both where the
generator kept its layouts, and the autopilot walks it frame for frame alike,
so a pair shows one spot drawn twice.

  python3 tools/before_after.py v0.5.3 --new 0.6.0
  python3 tools/before_after.py v0.5.3 --new 0.6.0 sky nest

OLD may be any commit, and NEW too (--new-ref): a pair then isolates one
change on a layer both draw alike (a change of LAYER_MAKE between them
moves the layouts). --pick SHA applies a commit to each older tree before
it is built, as 4882961's dev steps in --input that a scene's "0:place"
needs.

  python3 tools/before_after.py 1a0f974 --old-label before --new 0.8.0 --pick 4882961 way
"""
import argparse
import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)
import build  # noqa: E402

WEAK = {'CYBERWORLD_AUTOPILOT': 'weak'}
WALK = ['--scene', 'emu', '--run-depth', '1', '--dev', 'quiet']
# name, game options, environment, frame
SCENES = [
    # (BlastMan's arena in the Robot Control Comp, as the autopilot walks in)
    ('robot-arena', build.RUN_7, WEAK, 1300),
    # (Mr. Weather Comp's arena ahead of a walkway, the same guardian)
    ('weather-arena', ['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '14', '--guardian', '12'], WEAK, 900),
    # (JudgeMan's arena on an Undernet layer)
    ('undernet-arena', ['--scene', 'emu', '--run-depth', '15', '--dev', 'quiet', '--net-biome', '5', '--seed', '5'], WEAK, 580),
    # (a catwalk and a field in Sky Area)
    ('sky', WALK + ['--net-biome', '2', '--seed', '2'], WEAK, 400),
    # (an Undernet layer's joints and planks, MegaMan at a ramp)
    ('undernet-joints', ['--scene', 'emu', '--net-biome', '5', '--run-depth', '2', '--seed', '4', '--dev', 'quiet'], WEAK, 1000),
    # (Sky Area's catwalks below a stair)
    ('sky-catwalks', ['--scene', 'emu', '--net-biome', '2', '--run-depth', '4', '--seed', '4', '--dev', 'quiet'], WEAK, 1000),
    # (the Undernet's bridges in a maze of them)
    ('undernet-bridges', ['--scene', 'emu', '--net-biome', '5', '--run-depth', '2', '--seed', '4', '--dev', 'quiet'], WEAK, 750),
    # (the Graveyard's slabs and their edges)
    ('graveyard', WALK + ['--net-biome', '4', '--seed', '3'], WEAK, 460),
    # (Seaside Area's boardwalks meeting a platform)
    ('seaside', ['--scene', 'emu', '--net-biome', '1', '--run-depth', '3', '--seed', '11', '--dev', 'quiet'], {}, 300),
    # (the Cybeast Nest and the void beside it)
    ('nest', ['--scene', 'emu', '--run-depth', '19', '--dev', 'quiet', '--net-biome', '7', '--seed', '3'], WEAK, 620),
    # (the Net Dealer's word on TenguMan before his arena, its third page)
    ('dealer', ['--scene', 'emu', '--run-depth', '9', '--seed', '1', '--guardian', '8', '--talk', 'shop:430', '--dev', 'quiet',
                '--input', '500:,4:A,100:,4:A,100:,4:A,160:'], {}, 860),
]
FONT = '/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf'


def old_binary(tag, picks=()):
    """OLD's host binary, built in a worktree of its tag the first time (the
    commits `picks` applied to it first, where it lacks them)."""
    tree = os.path.join(ROOT, '.build', 'before', tag)
    binary = os.path.join(tree, 'build', 'host', 'cyberworld')
    if not os.path.exists(binary):
        if not os.path.exists(tree):
            subprocess.run(['git', '-C', ROOT, 'worktree', 'add', '--detach', tree, tag], check=True)
        for sha in picks:
            if subprocess.run(['git', '-C', tree, 'merge-base', '--is-ancestor', sha, 'HEAD']).returncode:
                subprocess.run(['git', '-C', tree, 'cherry-pick', '--no-commit', sha], check=True)
        subprocess.run([sys.executable, os.path.join(tree, 'build.py'), 'host'], check=True)
    return os.path.relpath(binary, ROOT)


def frame(binary, args, env, at, out):
    """Frame `at` of a headless run of `binary` (relative to ROOT), the game's 240 x 160."""
    from PIL import Image
    tmp = os.path.join(ROOT, '.build', 'before', 'shot')
    shutil.rmtree(tmp, ignore_errors=True)
    os.makedirs(os.path.join(tmp, 'data'))
    saved = {k: os.environ.get(k) for k in env}
    os.environ.update(env)
    try:
        code = build.docker(binary, '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/before/shot/data', *args,
                            '--frames', str(at + 1), '--shot', f'{at}:/src/.build/before/shot/f.bmp', mounts=build.docs_rom_mounts())
    finally:
        for k, v in saved.items():
            if v is None:
                os.environ.pop(k, None)
            else:
                os.environ[k] = v
    if code:
        sys.exit(f'{out}: the run failed')
    im = Image.open(os.path.join(tmp, 'f.bmp')).convert('RGB')
    w, h = im.size
    return im.crop(((w - 240) // 2, (h - 160) // 2, (w + 240) // 2, (h + 160) // 2))


def pair(old, new, old_label, new_label):
    """Both frames at 2x with whole pixels, each under its version."""
    from PIL import Image, ImageDraw, ImageFont
    gap, bar = 8, 30
    im = Image.new('RGB', (2 * 480 + gap, bar + 320), (16, 24, 40))
    d = ImageDraw.Draw(im)
    font = ImageFont.truetype(FONT, 18) if os.path.exists(FONT) else ImageFont.load_default()
    for i, (frame_im, label, colour) in enumerate(((old, old_label, (170, 180, 200)), (new, new_label, (120, 248, 255)))):
        x = i * (480 + gap)
        im.paste(frame_im.resize((480, 320), Image.NEAREST), (x, bar))
        d.text((x + 240, bar // 2), label, fill=colour, font=font, anchor='mm')
    return im


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('old', help='the release to compare with, its tag (v0.5.3), or a commit')
    ap.add_argument('--new', default='now', help="this build's label (0.6.0)")
    ap.add_argument('--old-label', help="OLD's label (its tag without the v)")
    ap.add_argument('--new-ref', help='a commit to build as NEW, in place of this build')
    ap.add_argument('--pick', action='append', default=[], help='a commit applied to an older tree before it is built')
    ap.add_argument('names', nargs='*', help='scenes (all)')
    a = ap.parse_args()
    old = old_binary(a.old, a.pick)
    new = old_binary(a.new_ref, a.pick) if a.new_ref else os.path.join('build', 'host', 'cyberworld')
    if not os.path.exists(os.path.join(ROOT, new)):
        sys.exit('build this build first: python3 build.py host')
    out = os.path.join(ROOT, 'docs', 'screenshots')
    for name, args, env, at in SCENES:
        if a.names and name not in a.names:
            continue
        path = os.path.join(out, f'compare-{name}.png')
        pair(frame(old, args, env, at, name), frame(new, args, env, at, name), a.old_label or a.old.lstrip('v'), a.new).save(path, optimize=True)
        print('compared', name)


if __name__ == '__main__':
    main()
