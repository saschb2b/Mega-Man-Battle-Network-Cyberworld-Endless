"""What the itch.io page holds, kept in step with the repository
(docs/ITCH.md, Keeping it in step): docs/itch/state.json records the page's
files as last applied on itch (each one's SHA-1), itch's address for each
picture the description shows (uploaded through itch's editor, which takes
no picture from another site), and the devlog post of every release since
the page began.

lint() is offline, for build.py lint: the description names no version,
every file of docs/itch/ is the one applied, and every release but the
newest has its post (the newest is posted after its tag). check() asks
itch too: every butler channel at the newest release, and its post there.
"""
import hashlib
import json
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import site_versions  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PAGE = os.path.join(ROOT, 'docs', 'itch')
STATE = os.path.join(PAGE, 'state.json')
# (the page's files an agent applies on itch by hand: the description in
# its editor, the pictures in the editor and the theme)
APPLIED = ['description.html', 'cover.png', 'banner.png', 'embed.png', 'background.png']
VERSION = re.compile(r'(?<![\w.])\d+\.\d+\.\d+(?![\w.])|\bnew in\b', re.I)


def load():
    with open(STATE, encoding='utf-8') as f:
        return json.load(f)


def save(state):
    with open(STATE, 'w', encoding='utf-8') as f:
        json.dump(state, f, indent=1, ensure_ascii=False)
        f.write('\n')


def sha1(name):
    with open(os.path.join(PAGE, name), 'rb') as f:
        return hashlib.sha1(f.read()).hexdigest()


def released():
    """Every dated release's version, the newest first."""
    return [r['version'] for r in site_versions.releases() if r['date']]


def itch_html():
    """The description as itch's editor takes it: the comments out, each
    picture at its itch address. None and the pictures to upload first
    when one has none yet."""
    state = load()
    with open(os.path.join(PAGE, 'description.html'), encoding='utf-8') as f:
        text = re.sub(r'<!--.*?-->\n?', '', f.read(), flags=re.S)
    missing = []

    def one(m):
        url = state['images'].get(m.group(1))
        if not url:
            missing.append(m.group(1))
        return f'src="{url}"'
    text = re.sub(r'src="\.\./clips/([^"]+)"', one, text)
    return (None, missing) if missing else (re.sub(r'\n{2,}', '\n', text).strip() + '\n', [])


def record_image(name, url):
    """A picture of the description, uploaded through itch's editor."""
    state = load()
    state['images'][name] = url
    save(state)


def record_page():
    """The page's files as they are now, applied on itch."""
    state = load()
    state['page'] = {name: sha1(name) for name in APPLIED}
    save(state)


def record_post(version, title, url):
    """A release's devlog post, posted on itch."""
    state = load()
    posts = [p for p in state['devlog']['posts'] if p['version'] != version]
    posts.append({'version': version, 'title': title, 'url': url})
    state['devlog']['posts'] = sorted(posts, key=lambda p: site_versions.version_key(p['version']), reverse=True)
    save(state)


def unposted():
    """Every release since the record's start without a post, the newest first."""
    state = load()
    posted = {p['version'] for p in state['devlog']['posts']}
    since = site_versions.version_key(state['devlog']['since'])
    return [v for v in released() if site_versions.version_key(v) >= since and v not in posted]


def problems(newest_too=False):
    """What is out of step, offline: each a line saying what to do."""
    state, out = load(), []
    with open(os.path.join(PAGE, 'description.html'), encoding='utf-8') as f:
        text = re.sub(r'<!--.*?-->', '', f.read(), flags=re.S)
    for m in VERSION.finditer(text):
        out.append(f'docs/itch/description.html names a version ("{m.group(0)}"): the page stays version-free, '
                   'a release\'s news goes in its devlog post')
    if itch_html()[0] is None:
        out.append(f'docs/itch/description.html shows {", ".join(itch_html()[1])} with no itch address: upload '
                   "it in itch's editor, then `python3 tools/itch.py record --image NAME URL`")
    for name in APPLIED:
        if state['page'].get(name) != sha1(name):
            out.append(f'docs/itch/{name} is not what the itch page holds: apply it there (docs/ITCH.md), '
                       'then `python3 tools/itch.py record --page`')
    posted = {p['version'] for p in state['devlog']['posts']}
    since = site_versions.version_key(state['devlog']['since'])
    owed = [v for v in released() if site_versions.version_key(v) >= since]
    for v in owed if newest_too else owed[1:]:
        if v not in posted:
            out.append(f'{v} has no devlog post on itch: `python3 tools/itch.py devlog --tag v{v}`, post it, then '
                       f'`python3 tools/itch.py record --post v{v} URL`')
    return out


def lint():
    """build.py lint's check: True when the page is in step."""
    found = problems()
    for p in found:
        print(f'  itch: {p}')
    print(f'itch: {len(found)} out of step')
    return not found


def channels(butler, page):
    """Each butler channel's live version, from `butler status`."""
    out = subprocess.run([butler, 'status', page], capture_output=True, text=True, check=True).stdout
    return dict(re.findall(r'^\|\s*(\S+)\s*\|[^|]*\|[^|]*\|\s*(\S+)\s*\|\s*$', out, re.M))


def check(butler, page, names):
    """The page against the newest release, itch asked too: True when in step."""
    newest = released()[0]
    found = problems(newest_too=True)
    live = channels(butler, page)
    for name in names:
        if live.get(name) != newest:
            found.append(f'channel {name} serves {live.get(name, "nothing")}, not {newest}: `python3 tools/itch.py push`')
    for p in found:
        print(f'  {p}')
    print(f'itch.io against {newest}: ' + ('in step' if not found else f'{len(found)} out of step'))
    return not found
