#!/usr/bin/env python3
"""montage.py NAVI [OUT]: the last four sheets of guardian_watch.sh NAVI,
every frame small and numbered in one picture (96 frames, about 12 s)."""
import glob
import sys

from PIL import Image, ImageDraw

navi = sys.argv[1]
out_path = sys.argv[2] if len(sys.argv) > 2 else f'.build/play/w{navi}/montage.png'
frames = []
for f in sorted(glob.glob(f'.build/play/w{navi}/shots/*.png'))[-4:]:
    im = Image.open(f).convert('RGB')
    cw, ch = im.size[0] // 4, im.size[1] // 6
    for r in range(6):
        for c in range(4):
            cell = im.crop((c * cw, r * ch, (c + 1) * cw, r * ch + int(ch * 0.86)))
            if cell.getbbox():
                frames.append(cell.resize((200, 130)))
cols = 10
out = Image.new('RGB', (cols * 204, (len(frames) + cols - 1) // cols * 134))
d = ImageDraw.Draw(out)
for i, fr in enumerate(frames):
    x, y = i % cols * 204, i // cols * 134
    out.paste(fr, (x, y))
    d.text((x + 2, y + 2), str(i + 1), fill=(255, 255, 0))
out.save(out_path)
print(out_path, len(frames), 'frames')
