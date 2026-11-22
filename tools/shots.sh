#!/bin/bash
# Combine BMP captures into one PNG grid: shots.sh OUT.png COLS CROP(x,y,w,h|-) FILE...
out=$1; cols=$2; crop=$3; shift 3
python3 - "$out" "$cols" "$crop" "$@" <<'PY'
import sys
from PIL import Image
out, cols, crop, files = sys.argv[1], int(sys.argv[2]), sys.argv[3], sys.argv[4:]
ims = [Image.open(f).convert('RGB') for f in files]
if crop != '-':
    x, y, w, h = map(int, crop.split(','))
    ims = [im.crop((x, y, x + w, y + h)) for im in ims]
w, h = ims[0].size
rows = (len(ims) + cols - 1) // cols
sheet = Image.new('RGB', (cols * (w + 2), rows * (h + 2)), (255, 0, 0))
for i, im in enumerate(ims):
    sheet.paste(im, ((i % cols) * (w + 2), (i // cols) * (h + 2)))
sheet.save(out)
PY
