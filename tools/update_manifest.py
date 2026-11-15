#!/usr/bin/env python3
"""A release's update.json: what the game reads at its start to know a
newer version is out, and where the copies that update themselves find
their file (src/update/, docs/engine/updates.md). The release workflow
attaches it to each release, so its address,

    https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases/latest/download/update.json

always serves the newest one. For each file the release has, under the
key the game asks for: its address at the release's tag, its size and its
SHA-256, which the game checks before it puts the file in place; and the
release's notes as plain text, which the update box shows a page an entry
(src/core/options_news.c): its players' notes (docs/releases/vX.Y.Z.md),
else its CHANGELOG section. A paragraph a line, marked by what it is:
"= " the tagline, "> " the summary under it, "## " a section, "# " an
entry's heading, "- " a point, nothing a paragraph.

    python3 tools/update_manifest.py vX.Y.Z build/release > update.json
"""
import hashlib
import json
import os
import re
import sys

REPO = 'saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless'
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CHANGES_MAX = 40000   # (the game's buffer is 48 KB: past this, the rest on the releases page)
MORE = 'More on the releases page'

# (the game's key for each file: src/update/update_install.c's, and the
# ones the systems that update themselves later will ask for)
FILES = {
    'appimage': 'cyberworld-endless-x86_64.AppImage',
    'windows-setup': 'cyberworld-endless-windows-x64-setup.exe',
    'windows-zip': 'cyberworld-endless-windows-x64.zip',
    'macos': 'cyberworld-endless-macos.dmg',
    'android': 'cyberworld-endless.apk',
    'portmaster': 'cyberworld-endless-portmaster.zip',
    '3ds-cia': 'cyberworld-endless.cia',
    '3ds-3dsx': 'cyberworld-endless.3dsx',
    'switch': 'cyberworld-endless.nro',
    'vita': 'cyberworld-endless.vpk',
    'ps4': 'cyberworld-endless.pkg',
    'ps5': 'cyberworld-endless-ps5.zip',
}


def digest(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for block in iter(lambda: f.read(1 << 20), b''):
            h.update(block)
    return h.hexdigest()


def plain(text):
    """Markdown to the PET font's plain ASCII: no bold, code, links or
    pictures, curly quotes and dashes straightened, '|' (the game's line
    break) and other letters it has not dropped."""
    text = re.sub(r'!\[[^\]]*\]\([^)]*\)', '', text)
    text = re.sub(r'\[([^\]]+)\]\([^)]*\)', r'\1', text)
    text = re.sub(r'<[^>]+>', '', text).replace('**', '').replace('`', '').replace('|', '/')
    text = text.translate(str.maketrans({'\u2019': "'", '\u2018': "'", '\u201c': '"', '\u201d': '"',
                                         '\u2013': '-', '\u2014': '-', '\u00d7': 'x'}))
    return re.sub(r'\s+', ' ', ''.join(c for c in text if 32 <= ord(c) < 127 or c == '\n')).strip()


def entries(version, changelog):
    """The section's entries: each its headline and its paragraphs (the text
    after the headline, then each sub-point)."""
    with open(changelog, encoding='utf-8') as f:
        text = f.read()
    section = re.search(r'^## ' + re.escape(version) + r' \(.*?$(.*?)(?=^## |\Z)', text, re.M | re.S)
    if not section:
        return []
    out = []
    for block in re.split(r'^- ', section.group(1), flags=re.M)[1:]:
        lead = re.match(r'\*\*(.+?)\*\*(.*)', block, re.S)
        head, rest = (lead.group(1), lead.group(2)) if lead else ('', block)
        parts = re.split(r'^\s+- ', rest, flags=re.M)
        paras = [plain(parts[0])] + ['- ' + plain(x) for x in parts[1:]]
        out.append((plain(head).rstrip('.'), [x for x in paras if x and x != '- ']))
    return out


def from_changelog(version, changelog):
    """The CHANGELOG section's entries as the box's lines: a heading each,
    then its paragraphs."""
    lines = []
    for head, paras in entries(version, changelog):
        lines += ([f'# {head}'] if head else []) + paras
    return lines


def from_notes(path):
    """The players' notes as the box's lines: the tagline and the summary
    under it, each section's name, each entry's heading and its
    paragraphs and points; pictures, rules and tables left to GitHub."""
    with open(path, encoding='utf-8') as f:
        text = re.sub(r'\A---\n.*?\n---\n', '', f.read(), flags=re.S)
    lines, para, summary = [], [], False

    def flush():
        if para:
            lines.append(plain(' '.join(para)))
            para.clear()
    for raw in text.split('\n'):
        line = raw.strip()
        tagline = re.match(r'<h2[^>]*>(.*?)</h2>', line)
        if tagline:
            flush()
            lines.append('= ' + plain(tagline.group(1)))
            summary = True
        elif summary and re.match(r'<p[^>]*>(?!\s*<)(.+?)</p>', line):
            lines.append('> ' + plain(re.match(r'<p[^>]*>(.+?)</p>', line).group(1)))
            summary = False
        elif line.startswith('## ') or line.startswith('### '):
            flush()
            mark = '# ' if line.startswith('### ') else '## '
            name = plain(line.lstrip('#'))
            if name:
                lines.append(mark + name)
        elif line.startswith('- ') or line.startswith('* '):
            flush()
            lines.append('- ' + plain(line[2:]))
        elif not line or line == '---' or line.startswith('|') or line.startswith('<') or line.startswith('!['):
            flush()
        else:
            para.append(line)
    flush()
    return [x for x in lines if x not in ('', '- ', '# ', '## ', '= ', '> ')]


def changes(version, changelog, notes=None):
    """The release's notes as the box shows them, a line a paragraph, past
    CHANGES_MAX cut at a heading and the rest named on the releases page."""
    notes = notes or os.path.join(ROOT, 'docs', 'releases', f'v{version}.md')
    lines = from_notes(notes) if os.path.isfile(notes) else from_changelog(version, changelog)
    out = []
    for line in lines:
        if len('\n'.join(out + [line])) > CHANGES_MAX - len(MORE) - 1:
            # (cut inside an entry: back to before its heading, so no entry shows in part)
            while not line.startswith('#') and out and not out[-1].startswith('#'):
                out.pop()
            if not line.startswith('#') and out:
                out.pop()
            out.append(MORE)
            break
        out.append(line)
    return '\n'.join(out)


def manifest(tag, folder, changelog=os.path.join(ROOT, 'CHANGELOG.md')):
    version = tag[1:] if tag.startswith('v') else tag
    files = {}
    for key, name in FILES.items():
        path = os.path.join(folder, name)
        if os.path.isfile(path):
            files[key] = {
                'url': f'https://github.com/{REPO}/releases/download/{tag}/{name}',
                'size': os.path.getsize(path),
                'sha256': digest(path),
            }
    return {
        'version': version,
        'notes': f'https://github.com/{REPO}/releases/tag/{tag}',
        'changes': changes(version, changelog),
        'files': files,
    }


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    tag, folder = sys.argv[1], sys.argv[2]
    if not tag.startswith('v') or tag[1:].count('.') != 2:
        sys.exit(f'{tag}: not a release tag (vX.Y.Z)')
    json.dump(manifest(tag, folder), sys.stdout, indent=1)
    sys.stdout.write('\n')


if __name__ == '__main__':
    main()
