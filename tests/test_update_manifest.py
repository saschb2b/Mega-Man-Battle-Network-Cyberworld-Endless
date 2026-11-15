#!/usr/bin/env python3
"""tools/update_manifest.py's changes: a release's notes as the game's
update box shows them (docs/engine/updates.md), from made-up players'
notes (the tagline, the summary, sections, headings, points; pictures,
rules and tables left off) and from a made-up CHANGELOG where a release
has no notes (headlines marked, sub-points kept, markdown and other
letters left off), a long one cut at a heading with the rest named."""
import os
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


if __name__ == '__main__':
    unittest.main()
