#!/usr/bin/env python3
"""The itch.io page's files (docs/ITCH.md): a release's own files from GitHub,
one butler channel each, and the browser player made into the zip itch
plays in its page.

    python3 tools/itch.py prepare [--tag vX.Y.Z]   # .build/itch/channels/, nothing sent
    python3 tools/itch.py push [--tag vX.Y.Z] [--hidden] [--dry-run]

`prepare` downloads the release's files (gh) and lays out one folder a
channel. `push` sends each with butler under the release's version; butler
must be logged in as the page's owner first (`butler login`), whose key
stays in butler's own file. A new channel's first push makes its upload on
the page; --hidden keeps it off the page until it is checked there.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
import zipfile

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import build  # noqa: E402  (the site's address and the release's file names)

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORK = os.path.join(ROOT, '.build', 'itch')
PAGE = 'saschb2b/cyberworld-endless'

# The uploads, in the page's order: channel, the release's file, and how it
# goes up. 'unzip': the archive's contents, a folder the itch app can start;
# 'file': the file as the release has it (an archive kept whole, as its
# system installs it). Channels naming windows, mac, linux or android are
# tagged with that platform by butler; the web one is marked played in the
# browser on the edit page once.
CHANNELS = [
    ('web', build.WEB_ZIP, 'web'),
    ('windows', 'cyberworld-endless-windows-x64.zip', 'unzip'),
    ('mac', 'cyberworld-endless-macos.dmg', 'file'),
    ('linux', 'cyberworld-endless-x86_64.AppImage', 'file'),
    ('linux-flatpak', 'cyberworld-endless.flatpak', 'file'),
    ('android', 'cyberworld-endless.apk', 'file'),
    ('ios', 'cyberworld-endless.ipa', 'file'),
    ('portmaster', build.PORT_ZIP, 'file'),
    ('3ds-cia', 'cyberworld-endless.cia', 'file'),
    ('3ds-3dsx', 'cyberworld-endless.3dsx', 'file'),
]


def latest_tag():
    out = subprocess.run(['gh', 'release', 'view', '--json', 'tagName', '-q', '.tagName'],
                         cwd=ROOT, capture_output=True, text=True, check=True)
    return out.stdout.strip()


def download(tag):
    """The release's files into .build/itch/TAG/, each fetched once."""
    where = os.path.join(WORK, tag)
    os.makedirs(where, exist_ok=True)
    wanted = [name for _, name, _ in CHANNELS if not os.path.isfile(os.path.join(where, name))]
    if wanted:
        cmd = ['gh', 'release', 'download', tag, '--dir', where]
        for name in wanted:
            cmd += ['--pattern', name]
        subprocess.run(cmd, cwd=ROOT, check=True)
    return where


def link_out(m):
    """A link of the player page's out of it: to the site's other pages at
    the site's own address, and every link in a new tab (the page runs in
    itch's frame, and GitHub and the rest refuse to be framed)."""
    tag, url = m.group(1), m.group(2)
    if url.startswith('../'):
        url = build.SITE_URL + url[3:]
    return f'{tag}href="{url}" target="_blank" rel="noopener"'


def web(site_zip, out):
    """The browser player as itch plays it: play/ at the zip's root with
    the site's assets it uses, the site's bar hidden (itch's page is the
    site there), every link to the site out of the frame."""
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    with zipfile.ZipFile(site_zip) as z:
        for info in z.infolist():
            name = info.filename
            if info.is_dir():
                continue
            if name.startswith('play/') and not name.endswith('manifest.webmanifest'):
                dest = name[len('play/'):]
            elif name.startswith(('assets/', 'licenses/')) or name == 'LICENSE.txt':
                dest = name
            else:
                continue
            path = os.path.join(out, dest)
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with z.open(info) as src, open(path, 'wb') as dst:
                shutil.copyfileobj(src, dst)
    page = os.path.join(out, 'index.html')
    with open(page, encoding='utf-8') as f:
        text = f.read()
    text = re.sub(r'\n<link rel="manifest"[^>]*>', '', text)
    for local in ('assets/', 'licenses/', 'LICENSE.txt'):
        text = text.replace(f'"../{local}', f'"{local}')
    text = re.sub(r'(<a\b[^>]*?)href="((?!\./"|#)[^"]*)"', link_out, text)
    text = text.replace('</head>', '<style>body.play > .pet-bar { display: none; }</style>\n</head>', 1)
    with open(page, 'w', encoding='utf-8') as f:
        f.write(text)
    # (the player's pictures named from play/, now at the root beside them:
    # on itch's server ../ leaves the game's folder)
    style = os.path.join(out, 'play.css')
    with open(style, encoding='utf-8') as f:
        text = f.read()
    with open(style, 'w', encoding='utf-8') as f:
        f.write(text.replace('url(../assets/', 'url(assets/'))
    files = sum(len(fs) for _, _, fs in os.walk(out))
    size = sum(os.path.getsize(os.path.join(d, n)) for d, _, fs in os.walk(out) for n in fs)
    # (itch's limits for a played zip: 1000 files, 500 MB, 200 MB a file)
    assert files <= 1000 and size <= 500 << 20, (files, size)
    print(f'web: {files} files, {size >> 10} KB')


def prepare(tag):
    """One folder a channel under .build/itch/channels/, from the release's files."""
    where = download(tag)
    channels = os.path.join(WORK, 'channels')
    shutil.rmtree(channels, ignore_errors=True)
    for channel, name, how in CHANNELS:
        out = os.path.join(channels, channel)
        src = os.path.join(where, name)
        if how == 'web':
            web(src, out)
        elif how == 'unzip':
            with zipfile.ZipFile(src) as z:
                z.extractall(out)
        else:
            os.makedirs(out)
            shutil.copy2(src, out)
        print(f'{channel}: {name}')
    return channels


def butler():
    found = shutil.which('butler') or os.path.join(ROOT, '.build', 'butler', 'butler')
    if not os.path.isfile(found):
        sys.exit('butler is not installed: https://itchio.itch.io/butler, then `butler login`')
    return found


def push(tag, hidden, dry_run):
    channels = prepare(tag)
    version = tag.lstrip('v')
    for channel, _, how in CHANNELS:
        cmd = [butler(), 'push', os.path.join(channels, channel), f'{PAGE}:{channel}', '--userversion', version]
        if how == 'file':
            cmd.append('--no-auto-unzip')
        if hidden:
            cmd.append('--hidden')
        if dry_run:
            cmd.append('--dry-run')
        print('$', ' '.join(cmd[1:]))
        subprocess.run(cmd, check=True)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('what', choices=('prepare', 'push'))
    ap.add_argument('--tag', help="the release (default: GitHub's latest)")
    ap.add_argument('--hidden', action='store_true', help="a new channel's upload hidden on the page")
    ap.add_argument('--dry-run', action='store_true', help="butler lists what it would send")
    a = ap.parse_args()
    tag = a.tag or latest_tag()
    if a.what == 'prepare':
        prepare(tag)
    else:
        push(tag, a.hidden, a.dry_run)


if __name__ == '__main__':
    main()
