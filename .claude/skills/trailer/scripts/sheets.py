#!/usr/bin/env python3
"""Contact sheets for a trailer, to pick its frames and to review it by eye.

    python3 .claude/skills/trailer/scripts/sheets.py scenes VERSION
        each scene's footage, about 40 pictures a sheet, labelled with the
        footage frame to give a shot (sheets/SCENE.png)
    python3 .claude/skills/trailer/scripts/sheets.py frames VERSION [STEP]
        the drawn trailer every STEP frames (12 by default), 45 a sheet
        (sheets/all0.png, all1.png, ...)
    python3 .claude/skills/trailer/scripts/sheets.py pick VERSION F,F,...
        those frames of the drawn trailer, larger (sheets/pick.png); draw
        them first with tools/trailer.py VERSION --keep --frames F,F,...

All in .build/trailer-VERSION/sheets/.
"""
import os
import sys

from PIL import Image, ImageDraw

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))


def grid(paths, labels, cols, size, out):
    rows = (len(paths) + cols - 1) // cols
    sheet = Image.new('RGB', (cols * size[0], rows * (size[1] + 12)), (0, 0, 0))
    d = ImageDraw.Draw(sheet)
    for i, (p, label) in enumerate(zip(paths, labels)):
        x, y = (i % cols) * size[0], (i // cols) * (size[1] + 12)
        sheet.paste(Image.open(p).convert('RGB').resize(size), (x, y))
        d.text((x + 2, y + size[1]), label, fill=(255, 255, 0))
    sheet.save(out)
    print('wrote', os.path.relpath(out, ROOT))


def main():
    if len(sys.argv) < 3 or sys.argv[1] not in ('scenes', 'frames', 'pick'):
        sys.exit(__doc__)
    what, version = sys.argv[1], sys.argv[2]
    work = os.path.join(ROOT, '.build', 'trailer-' + version)
    out = os.path.join(work, 'sheets')
    os.makedirs(out, exist_ok=True)
    if what == 'scenes':
        for name in sorted(os.listdir(work)):
            d = os.path.join(work, name)
            if name in ('sheets', 'frames', 'out') or name.startswith('data-') or not os.path.isdir(d):
                continue
            files = sorted(f for f in os.listdir(d) if f.endswith('.png') and f[:4].isdigit())
            pick = files[::max(1, len(files) // 40)][:40]
            grid([os.path.join(d, f) for f in pick], [f'{name} {f[:4]}' for f in pick], 8, (240, 160),
                 os.path.join(out, name + '.png'))
        return
    frames = os.path.join(work, 'frames')
    if what == 'frames':
        step = int(sys.argv[3]) if len(sys.argv) > 3 else 12
        have = sorted(int(f[:5]) for f in os.listdir(frames) if f.endswith('.png'))
        pick = [f for f in have if f % step == 0]
        for i in range(0, len(pick), 45):
            part = pick[i:i + 45]
            grid([os.path.join(frames, f'{f:05d}.png') for f in part], [str(f) for f in part], 9, (320, 180),
                 os.path.join(out, f'all{i // 45}.png'))
        return
    pick = [int(x) for x in sys.argv[3].split(',')]
    grid([os.path.join(frames, f'{f:05d}.png') for f in pick], [str(f) for f in pick], 3, (640, 360),
         os.path.join(out, 'pick.png'))


if __name__ == '__main__':
    main()
