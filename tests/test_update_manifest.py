#!/usr/bin/env python3
"""tools/update_manifest.py's changes: a release's notes as the game's
update box shows them (docs/engine/updates.md), from made-up players'
notes (the tagline, the summary, sections, headings, points; pictures,
rules and tables left off) and from a made-up CHANGELOG where a release
has no notes (headlines marked, sub-points kept, markdown and other
letters left off), a long one cut at a heading with the rest named."""
import os
import struct
import sys
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import update_manifest  # noqa: E402

CHANGELOG = '''# Changelog

## Unreleased

- **Not this one.** Unreleased.

## 0.22.0 (2026-11-20)

- **The game tells you when a new version is out.** At each start it
  looks for one (`update.json`), and the title shows **New v0.22.0**.
  - On Windows it installs itself.
  - See [the guide](docs/GUIDE.md#updates) — it’s there.
- **Fixed: a | in a line.** Plain.

## 0.21.2 (2026-11-14)

- **Older.** Not here.
'''

NOTES = '''---
type: Release Notes
title: "Cyberworld Endless 0.22.0"
---
<p align="center"><a href="../clips/trailer.mp4"><img src="../clips/trailer-play.png" width="640" alt="x"></a></p>

<h2 align="center">The game updates itself.</h2>

<p align="center">At each start it looks, and **one press** updates it.</p>

---

## 🔄 Updates

### A box on the title

<p align="center"><img src="../screenshots/update-box.png" width="320" alt="The box"></p>

It shows what is new,
a page an entry.

- Left and Right turn the pages.

| A | table |
| --- | --- |

## Fixes

Nothing broke.
'''


class Changes(unittest.TestCase):
    def setUp(self):
        fd, self.path = tempfile.mkstemp(suffix='.md')
        with os.fdopen(fd, 'w', encoding='utf-8') as f:
            f.write(CHANGELOG)
        fd, self.notes = tempfile.mkstemp(suffix='.md')
        with os.fdopen(fd, 'w', encoding='utf-8') as f:
            f.write(NOTES)
        self.none = self.notes + '.none'   # (a release without notes)

    def tearDown(self):
        os.remove(self.path)
        os.remove(self.notes)

    def test_notes(self):
        self.assertEqual(update_manifest.changes('0.22.0', self.path, self.notes).split('\n'), [
            '= The game updates itself.',
            '> At each start it looks, and one press updates it.',
            '## Updates',
            '# A box on the title',
            'It shows what is new, a page an entry.',
            '- Left and Right turn the pages.',
            '## Fixes',
            'Nothing broke.',
        ])

    def test_section(self):
        self.assertEqual(update_manifest.changes('0.22.0', self.path, self.none).split('\n'), [
            '# The game tells you when a new version is out',
            'At each start it looks for one (update.json), and the title shows New v0.22.0.',
            '- On Windows it installs itself.',
            "- See the guide - it's there.",
            '# Fixed: a / in a line',
            'Plain.',
        ])

    def test_none(self):
        self.assertEqual(update_manifest.changes('9.9.9', self.path, self.none), '')

    def test_cut(self):
        """Past the limit the box gets the entries whole that fit, and a line naming the rest."""
        old = update_manifest.CHANGES_MAX
        update_manifest.CHANGES_MAX = 230
        try:
            lines = update_manifest.changes('0.22.0', self.path, self.none).split('\n')
        finally:
            update_manifest.CHANGES_MAX = old
        self.assertEqual(lines, ['# The game tells you when a new version is out',
                                 'At each start it looks for one (update.json), and the title shows New v0.22.0.',
                                 '- On Windows it installs itself.', "- See the guide - it's there.", update_manifest.MORE])


def unlz77(src):
    """The game's lz77_decompress (src/core/rom.c), line for line."""
    n = src[1] | src[2] << 8 | src[3] << 16
    out, p = bytearray(), 4
    while len(out) < n:
        flags = src[p]
        p += 1
        for _ in range(8):
            if len(out) >= n:
                break
            if flags & 0x80:
                v = src[p] << 8 | src[p + 1]
                p += 2
                for _ in range((v >> 12) + 3):
                    if len(out) < n:
                        out.append(out[-((v & 0xFFF) + 1)])
            else:
                out.append(src[p])
                p += 1
            flags = flags << 1 & 0xFF
    return bytes(out)


@unittest.skipUnless(update_manifest.HAS_PIL, 'Pillow is not installed')
class Pictures(unittest.TestCase):
    def picture(self, size, scale, colours):
        """A made-up capture: `colours` stripes at the game's `size`, `scale` times over."""
        from PIL import Image
        import pil_nearest  # noqa: F401,E402  (Pillow's nearest resize kept working: tools/pil_nearest.py)
        im = Image.new('RGB', size)
        im.putdata([((i * 37) % 256, (i * 91) % 256, i % 256) for i in range(colours)
                    for _ in range(size[0] * size[1] // colours)])
        im = im.resize((size[0] * scale, size[1] * scale), Image.NEAREST)
        fd, path = tempfile.mkstemp(suffix='.png')
        os.close(fd)
        im.save(path)
        self.addCleanup(os.remove, path)
        return path, im.resize(size, Image.NEAREST)

    def test_own_pixels(self):
        """A capture at 2x or 3x comes back to the game's own pixels, its colours exact."""
        for scale in (1, 2, 3):
            path, small = self.picture((240, 160), scale, 40)
            w, h, pal, idx = update_manifest.picture(path)
            self.assertEqual((w, h), (240, 160))
            colours = [tuple(pal[i * 3:i * 3 + 3]) for i in range(len(pal) // 3)]
            self.assertEqual([colours[i] for i in idx], list(small.getdata()))

    def test_wide_fits(self):
        """A wider strip is scaled into the game's frame, kept to 256 colours."""
        path, _ = self.picture((484, 175), 2, 300)
        w, h, pal, idx = update_manifest.picture(path)
        self.assertLessEqual(w, 240)
        self.assertLessEqual(h, 160)
        self.assertEqual(len(idx), w * h)
        self.assertLessEqual(len(pal), 256 * 3)

    def test_pack(self):
        """The file as the game reads it: its head, each picture's, LZ77 that the game's decoder unpacks."""
        pics = [(3, 2, bytes([1, 2, 3, 4, 5, 6]), bytes([0, 1, 0, 1, 1, 1])), (2, 2, bytes(3), bytes(4))]
        blob = update_manifest.pack(pics)
        self.assertEqual(blob[:4], b'CWPX')
        self.assertEqual(struct.unpack_from('<HH', blob, 4), (1, 2))
        at = 8
        for w, h, pal, idx in pics:
            pw, ph, colours, n = struct.unpack_from('<HHHI', blob, at)
            at += 10
            self.assertEqual((pw, ph, colours), (w, h, len(pal) // 3))
            self.assertEqual(blob[at:at + len(pal)], pal)
            at += len(pal)
            self.assertEqual(unlz77(blob[at:at + n]), idx)
            at += n
        self.assertEqual(at, len(blob))
        data = bytes(range(256)) * 20 + bytes(5000)
        self.assertEqual(unlz77(update_manifest.lz77(data)), data)


if __name__ == '__main__':
    unittest.main()
