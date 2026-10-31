#!/usr/bin/env python3
"""The itch.io page kept in step with the releases (docs/project/itch.md): a
release's own files from GitHub, one butler channel each, the browser
player made into the zip itch plays in its page, each release's devlog
post, and the record of what the page holds.

    python3 tools/itch.py prepare [--tag vX.Y.Z]   # .build/itch/channels/, nothing sent
    python3 tools/itch.py push [--tag vX.Y.Z] [--hidden] [--dry-run]
    python3 tools/itch.py devlog [--tag vX.Y.Z]    # .build/itch/devlog-vX.Y.Z.html, its title and type
    python3 tools/itch.py devlog --missing         # every release without a post: .build/itch/devlogs.json
    python3 tools/itch.py record --post vX.Y.Z URL # the post, once it is on itch
    python3 tools/itch.py description               # .build/itch/description.html for itch's editor
    python3 tools/itch.py record --page            # docs/itch/'s files, once applied on itch
    python3 tools/itch.py record --image NAME URL  # a description picture uploaded through itch's editor
    python3 tools/itch.py check                    # the page against the newest release (asks itch)
    python3 tools/itch.py lint                     # the same offline (build.py lint runs it)

`prepare` downloads the release's files (gh) and lays out one folder a
channel. `push` sends each with butler under the release's version; butler
must be logged in as the page's owner first (`butler login`), whose key
stays in butler's own file. A new channel's first push makes its upload on
the page; --hidden keeps it off the page until it is checked there. itch
has no API for posts or the page's text: an agent applies them in itch's
editor (docs/project/itch.md) and records them here, which lint holds it to.
"""
import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import zipfile

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import build  # noqa: E402  (the site's address and the release's file names)
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import itch_devlog  # noqa: E402
import itch_sync  # noqa: E402

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
    ('switch-nro', 'cyberworld-endless.nro', 'file'),
    ('vita-vpk', 'cyberworld-endless.vpk', 'file'),
    ('ps4-pkg', 'cyberworld-endless.pkg', 'file'),
    ('ps5-zip', 'cyberworld-endless-ps5.zip', 'file'),
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


def existing_channels():
    """The page's channels, as butler's status lists them."""
    out = subprocess.run([butler(), 'status', PAGE], capture_output=True, text=True, check=True).stdout
    return {m.group(1) for m in re.finditer(r'^\|\s*([a-z0-9-]+)\s*\|', out, re.M)} - {'channel'}


def push(tag, hidden, dry_run):
    channels = prepare(tag)
    version = tag.lstrip('v')
    # (itch takes --hidden only on a channel's first push: the new ones)
    known = existing_channels() if hidden else set()
    for channel, _, how in CHANNELS:
        cmd = [butler(), 'push', os.path.join(channels, channel), f'{PAGE}:{channel}', '--userversion', version]
        if how == 'file':
            cmd.append('--no-auto-unzip')
        if hidden and channel not in known:
            cmd.append('--hidden')
        if dry_run:
            cmd.append('--dry-run')
        print('$', ' '.join(cmd[1:]))
        subprocess.run(cmd, check=True)


def devlog(tag, missing=False):
    """A release's post written out for itch's editor, and what to set
    beside it; with missing, every release since the record's start that
    has none yet, oldest first, into .build/itch/devlogs.json."""
    os.makedirs(WORK, exist_ok=True)
    if missing:
        posts = [itch_devlog.post(itch_devlog.release(v)) for v in reversed(itch_sync.unposted())]
        with open(os.path.join(WORK, 'devlogs.json'), 'w', encoding='utf-8') as f:
            json.dump(posts, f, ensure_ascii=False, indent=1)
        return print(f"{len(posts)} posts to make, oldest first: {os.path.relpath(os.path.join(WORK, 'devlogs.json'), ROOT)}")
    p = itch_devlog.post(itch_devlog.release(tag))
    path = os.path.join(WORK, f"devlog-v{p['version']}.html")
    with open(path, 'w', encoding='utf-8') as f:
        f.write(p['html'] + '\n')
    subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'itch_art.py'), '--devlog', f"v{p['version']}"],
                   check=True, capture_output=True)
    print(f"title: {p['title']}\npost type: {p['type']}\noriginal publish date: {p['date']} (an older release's only)\n"
          f"content: {os.path.relpath(path, ROOT)} (its pictures through tools/itch_pictures.js)\n"
          f"cover: .build/itch/covers/v{p['version']}.png\n"
          f"attachments: the newest's Play in the browser build only; an older release's none")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('what', choices=('prepare', 'push', 'devlog', 'description', 'record', 'check', 'lint'))
    ap.add_argument('--tag', help="the release (default: GitHub's latest; devlog: the newest in CHANGELOG.md)")
    ap.add_argument('--hidden', action='store_true', help="a new channel's upload hidden on the page (a channel it has already, as it is)")
    ap.add_argument('--dry-run', action='store_true', help="butler lists what it would send")
    ap.add_argument('--post', nargs=2, metavar=('TAG', 'URL'), help='record: a release\'s devlog post on itch')
    ap.add_argument('--page', action='store_true', help="record: docs/itch/'s files as applied on itch")
    ap.add_argument('--posts', metavar='JSON', help='record: several posts at once, {"X.Y.Z": URL, ...}')
    ap.add_argument('--missing', action='store_true', help='devlog: every release without a post yet')
    ap.add_argument('--image', nargs=2, metavar=('NAME', 'URL'), help='record: a picture of the description at its itch address')
    a = ap.parse_args()
    if a.what == 'lint':
        sys.exit(0 if itch_sync.lint() else 1)
    if a.what == 'check':
        sys.exit(0 if itch_sync.check(butler(), PAGE, [c for c, _, _ in CHANNELS]) else 1)
    if a.what == 'devlog':
        return devlog(a.tag, a.missing)
    if a.what == 'description':
        html, missing = itch_sync.itch_html()
        if html is None:
            sys.exit('upload these through itch\'s editor first, then record --image: ' + ', '.join(missing))
        os.makedirs(WORK, exist_ok=True)
        with open(os.path.join(WORK, 'description.html'), 'w', encoding='utf-8') as f:
            f.write(html)
        return print(f'{os.path.relpath(os.path.join(WORK, "description.html"), ROOT)}: {len(html)} characters')
    if a.what == 'record':
        if not (a.post or a.page or a.image or a.posts):
            sys.exit('record: --post TAG URL, --posts JSON, --page or --image NAME URL')
        if a.image:
            itch_sync.record_image(*a.image)
        if a.page:
            itch_sync.record_page()
        if a.post:
            rel = itch_devlog.release(a.post[0])
            itch_sync.record_post(rel['version'], itch_devlog.post(rel)['title'], a.post[1])
        if a.posts:
            with open(a.posts, encoding='utf-8') as f:
                for version, url in json.load(f).items():
                    rel = itch_devlog.release(version)
                    itch_sync.record_post(rel['version'], itch_devlog.post(rel)['title'], url)
        return
    tag = a.tag or latest_tag()
    if a.what == 'prepare':
        prepare(tag)
    else:
        push(tag, a.hidden, a.dry_run)


if __name__ == '__main__':
    main()
