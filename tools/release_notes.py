#!/usr/bin/env python3
"""release_notes.py TAG: a GitHub release's notes. The players' own page for
the tag, docs/releases/TAG.md, where there is one; else the CHANGELOG's
section for its version (its "## VERSION" heading to the next "## "). Exits
1 when there is neither, so a tag is never released without notes.

  python3 tools/release_notes.py v0.1.0 > NOTES.md
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    tag = sys.argv[1] if sys.argv[1].startswith('v') else 'v' + sys.argv[1]
    version = tag[1:]
    page = os.path.join(ROOT, 'docs', 'releases', tag + '.md')
    if os.path.exists(page):
        with open(page, encoding='utf-8') as f:
            print(f.read().strip())
        return
    with open(os.path.join(ROOT, 'CHANGELOG.md'), encoding='utf-8') as f:
        text = f.read()
    m = re.search(r'^## ' + re.escape(version) + r'(?![\w.-]).*?\n(.*?)(?=^## |\Z)', text, re.M | re.S)
    if not m or not m.group(1).strip():
        sys.exit(f'CHANGELOG.md has no section for {version}')
    print(m.group(1).strip())


if __name__ == '__main__':
    main()
