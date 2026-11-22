#!/usr/bin/env python3
"""discord_release.py TAG: a release's post in the Discord server's
#announcements (docs/project/discord.md, Releases there), sent through the
channel's webhook after the release is published (.github/workflows/
discord-release.yml, which release.yml calls).

The post is made from the players' notes (docs/releases/TAG.md, else the
CHANGELOG's section), as the GitHub release, the site's versions page and
the itch.io devlog are: a ping for the Release pings role, then an embed
with the notes' headline, their lead paragraph, the notes' sections with
their entries' titles, the release's 16:9 cover (the itch devlog's, from
tools/itch_art.py) and the systems it runs on, and link buttons to the
download page, the browser build and the full notes.

  DISCORD_RELEASES_WEBHOOK  the channel webhook's address (a secret: never
                            in the repository); none, and nothing is sent
  DISCORD_RELEASE_ROLE      the Release pings role's id (a repository
                            variable); none, and the post pings nobody

  python3 tools/discord_release.py v0.23.0 --dry-run   # the post as JSON, its cover into .build/discord/
  python3 tools/discord_release.py v0.23.0             # sent, where the webhook is set
"""
import argparse
import html
import io
import json
import os
import re
import sys
import urllib.error
import urllib.request
import uuid

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import site_versions  # noqa: E402
from release_notes import REPO  # noqa: E402

SITE = site_versions.SITE_URL
NAME = 'Cyberworld Endless'
COLOUR = 0x08BD73   # the site's green (web/assets/site.css, --green)
SYSTEMS = 'Windows · macOS · Linux · Android · iOS · PortMaster · 3DS · Switch · Vita · PSP · PS4 · PS5 · browser'
OUT = os.path.join(ROOT, '.build', 'discord')
# (Discord's limits on a message's embed: docs.discord.com, Embed limits)
LIMITS = {'title': 256, 'description': 4096, 'footer': 2048, 'author': 256, 'total': 6000, 'content': 2000}
DESCRIPTION_ROOM = 3000   # the description's share, the rest left to the title and footer
PER_SECTION = 8           # entries a section lists before it counts the rest (a post, not the notes)


def release(tag):
    for rel in site_versions.releases():
        if rel['tag'] == tag:
            return rel
    sys.exit(f'{tag}: no release in docs/releases/ or the CHANGELOG')


def plain(text):
    """HTML and markdown's emphasis out of a line, its entities read"""
    text = re.sub(r'<[^>]+>', '', text)
    text = re.sub(r'\*\*|__|`', '', text)
    return ' '.join(html.unescape(text).split())


def lead(rel):
    """The notes' own paragraph under their headline, or the CHANGELOG's first entry's text"""
    opening, _ = site_versions.parts(rel['md'])
    if '</h2>' in opening:
        return plain(opening.split('</h2>', 1)[1])
    return ''


def sections(rel):
    """The notes' sections and their entries' titles: [(section, [title, ...])].
    Notes: their "## " sections and "### " entries. The CHANGELOG: its entries' bold titles."""
    if rel['source'] == 'changelog':
        titles = [plain(m.group(1)).rstrip('.:,') for m in re.finditer(r'(?m)^- \*\*(.+?)\*\*', rel['md'])]
        return [('', titles)] if titles else []
    _, rest = site_versions.parts(rel['md'])
    out = []
    for line in rest.splitlines():
        if line.startswith('## '):
            out.append((plain(line[3:]), []))
        elif line.startswith('### '):
            if not out:
                out.append(('', []))
            out[-1][1].append(plain(line[4:]))
    return [(name, items) for name, items in out if items or name]


def description(rel, room=DESCRIPTION_ROOM):
    """The lead, then each section with its entries' titles, PER_SECTION at
    most; where that is more than the room, fewer. The rest are counted."""
    secs = sections(rel)
    most = min(PER_SECTION, max((len(items) for _, items in secs), default=0))
    for keep in range(most, -1, -1):
        parts = [lead(rel)]
        for name, items in secs:
            block = [f'**{name}**'] if name else []
            block += [f'• {t}' for t in items[:keep]]
            if len(items) > keep:
                block.append(f'…and {len(items) - keep} more in the notes')
            parts.append('\n'.join(block))
        text = '\n\n'.join(p for p in parts if p)
        if len(text) <= room:
            return text
    return text[:room - 1] + '…'


def trailer_video(version):
    """The release's trailer on YouTube, where docs/project/youtube.md lists it (uploaded after the tag, so mostly not yet)"""
    try:
        with open(os.path.join(ROOT, 'docs', 'project', 'youtube.md'), encoding='utf-8') as f:
            m = re.search(r'^\| ' + re.escape(version) + r' \| <(https://youtu\.be/[^>]+)>', f.read(), re.M)
        return m.group(1) if m else None
    except OSError:
        return None


def links(rel):
    return [('Download', SITE + 'download/'), ('Play in the browser', SITE + 'play/'),
            ('Full notes', f'https://github.com/{REPO}/releases/tag/{rel["tag"]}')]


def post(rel, role=None, cover=True):
    """The webhook's message: its JSON, with the cover as its one attachment where there is one"""
    title = site_versions.headline(rel).rstrip('.')
    words = f'**{NAME} {rel["version"]}** is out'
    video = trailer_video(rel['version'])
    content = (f'<@&{role}> ' if role else '') + words + (f'\n{video}' if video else '')
    # (the GitHub release's label, as release.yml gives it: 0.1 to 0.4 alphas, 0.5 on betas)
    beta = ' (alpha)' if re.match(r'0\.[0-4]\.', rel['version']) else ' (beta)' if rel['version'].startswith('0.') else ''
    embed = {
        'author': {'name': f'{NAME} · {rel["tag"]}{beta}', 'url': SITE},
        'title': title[:LIMITS['title']],
        'url': links(rel)[2][1],
        'description': description(rel),
        'color': COLOUR,
        'footer': {'text': SYSTEMS},
    }
    if rel['date']:
        embed['timestamp'] = rel['date'] + 'T12:00:00Z'
    if cover:
        embed['image'] = {'url': 'attachment://cover.png'}
    message = {
        'username': NAME,
        'avatar_url': f'https://raw.githubusercontent.com/{REPO}/{rel["tag"]}/linux/icons/256.png',
        'content': content,
        'embeds': [embed],
        'allowed_mentions': {'parse': [], 'roles': [role] if role else []},
        'components': [{'type': 1, 'components': [{'type': 2, 'style': 5, 'label': label, 'url': url}
                                                  for label, url in links(rel)]}],
    }
    if cover:
        message['attachments'] = [{'id': 0, 'filename': 'cover.png'}]
    return message


def check(message):
    """Discord's limits, as problems in words (none: it fits)"""
    e = message['embeds'][0]
    sizes = {'title': len(e['title']), 'description': len(e['description']), 'footer': len(e['footer']['text']),
             'author': len(e['author']['name']), 'content': len(message['content'])}
    sizes['total'] = sum(v for k, v in sizes.items() if k != 'content')
    return [f'{k} {v} over {LIMITS[k]}' for k, v in sizes.items() if v > LIMITS[k]]


def cover_png(tag):
    import itch_art
    for _, cover in itch_art.covers([tag]):
        buf = io.BytesIO()
        cover.convert('RGB').save(buf, 'PNG', optimize=True)
        return buf.getvalue()
    return None


def without_buttons(message):
    """The same message where the webhook takes no components: the links in the description instead"""
    m = json.loads(json.dumps(message))
    m.pop('components', None)
    e = m['embeds'][0]
    e['description'] += '\n\n' + ' · '.join(f'[{label}]({url})' for label, url in links_of(message))
    return m


def links_of(message):
    return [(b['label'], b['url']) for row in message.get('components', []) for b in row['components']]


def send(webhook, message, png):
    """POST to the webhook (wait=true: Discord answers with the message); the cover as files[0]"""
    sep = '&' if '?' in webhook else '?'
    url = webhook + sep + 'wait=true' + ('&with_components=true' if 'components' in message else '')
    boundary = uuid.uuid4().hex
    body = io.BytesIO()
    body.write(f'--{boundary}\r\nContent-Disposition: form-data; name="payload_json"\r\n'
               f'Content-Type: application/json\r\n\r\n'.encode())
    body.write(json.dumps(message).encode() + b'\r\n')
    if png:
        body.write(f'--{boundary}\r\nContent-Disposition: form-data; name="files[0]"; filename="cover.png"\r\n'
                   f'Content-Type: image/png\r\n\r\n'.encode())
        body.write(png + b'\r\n')
    body.write(f'--{boundary}--\r\n'.encode())
    req = urllib.request.Request(url, data=body.getvalue(), method='POST', headers={
        'Content-Type': f'multipart/form-data; boundary={boundary}', 'User-Agent': 'cyberworld-release-post'})
    with urllib.request.urlopen(req, timeout=60) as r:
        return json.loads(r.read() or b'{}')


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('tag')
    ap.add_argument('--dry-run', action='store_true', help='print the message and write its cover; send nothing')
    ap.add_argument('--no-cover', action='store_true', help='no picture (no Pillow, or no tags to draw it from)')
    a = ap.parse_args()
    tag = a.tag if a.tag.startswith('v') else 'v' + a.tag
    rel = release(tag)
    role = os.environ.get('DISCORD_RELEASE_ROLE', '').strip() or None
    png = None if a.no_cover else cover_png(tag)
    message = post(rel, role, cover=png is not None)
    problems = check(message)
    if problems:
        sys.exit(f'{tag}: the post does not fit Discord: ' + ', '.join(problems))
    if a.dry_run:
        os.makedirs(OUT, exist_ok=True)
        if png:
            with open(os.path.join(OUT, tag + '.png'), 'wb') as f:
                f.write(png)
        print(json.dumps(message, indent=2, ensure_ascii=False))
        return
    webhook = os.environ.get('DISCORD_RELEASES_WEBHOOK', '').strip()
    if not webhook:
        print(f'{tag}: DISCORD_RELEASES_WEBHOOK is not set, nothing sent')
        return
    try:
        answer = send(webhook, message, png)
    except urllib.error.HTTPError as e:
        if e.code != 400 or 'components' not in message:
            sys.exit(f'{tag}: Discord answered {e.code}: {e.read().decode(errors="replace")[:300]}')
        # (a webhook no application owns may refuse buttons: the links go in the text)
        print(f'{tag}: buttons refused ({e.read().decode(errors="replace")[:200]}), sent with links in the text')
        answer = send(webhook, without_buttons(message), png)
    print(f'{tag}: posted, message {answer.get("id")} in channel {answer.get("channel_id")}')


if __name__ == '__main__':
    main()
