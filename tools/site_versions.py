#!/usr/bin/env python3
"""The site's versions page (web/versions/, web/ja/versions/): every release,
the newest first, from the players' notes in docs/releases/vX.Y.Z.md, or the
CHANGELOG's section for a release without them, rendered at build time with
the standard library alone. build.py site fills the two pages' templates,
writes the feed beside them, and tests/test_site.py checks what it makes.

Each template holds its page's frame and its words; the build reads the words
the releases need (a script block of JSON, id "versions-words") and puts in:
<!-- versions:newest --> the newest version, <!-- versions:releases --> one
article a release and <!-- versions:list --> one row a release.

A release's notes name their pictures by paths relative to docs/releases/,
which tools/release_notes.py turns into the tag's raw files for GitHub. Here
the newest release's pictures are the site's own copies (shots/, clips/; a
GIF made for GitHub becomes its clip's video), and an older release's are
its tag's, as GitHub shows them. A trailer the notes open with plays in the
page: the current one from clips/, an older one from where its notes link.

  python3 tools/site_versions.py OUT   # the pages and the feed into OUT/
"""
import datetime
import html
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from release_notes import REPO, at_tag  # noqa: E402

SITE_URL = 'https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/'
GITHUB = f'https://github.com/{REPO}'
PAGES = {'en': 'versions/', 'ja': 'ja/versions/'}
# (docs/ folders the site carries, by their names there)
SITE_DIRS = {'screenshots': 'shots', 'clips': 'clips'}
FEED = 'versions/feed.xml'


# ---- the releases ----

def version_key(v):
    return tuple(int(p) for p in v.split('.'))


def releases():
    """Every release, the newest first: its version, tag, date (None for one
    the CHANGELOG has not dated yet), its markdown and where that came from"""
    with open(os.path.join(ROOT, 'CHANGELOG.md'), encoding='utf-8') as f:
        changelog = f.read()
    dated = dict(re.findall(r'^## (\d+\.\d+\.\d+) \((\d{4}-\d\d-\d\d)\)', changelog, re.M))
    notes = {}
    folder = os.path.join(ROOT, 'docs', 'releases')
    for name in os.listdir(folder):
        m = re.fullmatch(r'v(\d+\.\d+\.\d+)\.md', name)
        if m:
            with open(os.path.join(folder, name), encoding='utf-8') as f:
                notes[m.group(1)] = f.read().strip()
    out = []
    for v in sorted(set(notes) | set(dated), key=version_key, reverse=True):
        if v in notes:
            md, source = notes[v], 'notes'
        else:
            m = re.search(r'^## ' + re.escape(v) + r'(?![\w.-]).*?\n(.*?)(?=^## |\Z)', changelog, re.M | re.S)
            md, source = (m.group(1).strip() if m else ''), 'changelog'
        out.append({'version': v, 'tag': 'v' + v, 'date': dated.get(v), 'md': md, 'source': source})
    return out


# ---- markdown, as the notes write it ----

BLOCK_TAGS = {'p', 'div', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'table', 'details', 'summary', 'hr', 'br',
              'ul', 'ol', 'li', 'figure', 'section', 'blockquote', 'pre', 'video', 'center'}
TAG = re.compile(r'</?[A-Za-z][A-Za-z0-9-]*(?:\s[^<>]*)?/?>')


def esc(text):
    """Text as HTML: its entities kept"""
    return re.sub(r'&(?!#?\w+;)', '&amp;', text).replace('<', '&lt;').replace('>', '&gt;')


def inline(text):
    """A line's markdown: code, escapes, its own HTML, links, strong and emphasis"""
    keep = []

    def stash(s):
        keep.append(s)
        return f'\x00{len(keep) - 1}\x00'

    text = re.sub(r'(`+)(.+?)\1', lambda m: stash('<code>' + esc(m.group(2).strip()) + '</code>'), text)
    text = re.sub(r'<(https?://[^>\s]+)>', lambda m: stash(f'<a href="{html.escape(m.group(1))}">{esc(m.group(1))}</a>'), text)
    text = re.sub(r'\\([\\`*_{}\[\]()#+\-.!|<>~])', lambda m: stash(esc(m.group(1))), text)
    text = TAG.sub(lambda m: stash(m.group(0)), text)
    text = re.sub(r'!\[([^\]]*)\]\(([^)\s]+)\)',
                  lambda m: stash(f'<img src="{html.escape(m.group(2))}" alt="{html.escape(m.group(1))}">'), text)
    text = re.sub(r'\[([^\[\]]+)\]\(([^)\s]+)\)',
                  lambda m: stash(f'<a href="{html.escape(m.group(2))}">{emphasis(esc(m.group(1)))}</a>'), text)
    text = emphasis(esc(text))
    while '\x00' in text:
        text = re.sub(r'\x00(\d+)\x00', lambda m: keep[int(m.group(1))], text)
    return text


def emphasis(text):
    text = re.sub(r'\*\*(.+?)\*\*', r'<strong>\1</strong>', text)
    text = re.sub(r'(?<!\w)__(.+?)__(?!\w)', r'<strong>\1</strong>', text)
    text = re.sub(r'\*([^*\s](?:[^*]*[^*\s])?)\*', r'<em>\1</em>', text)
    return re.sub(r'(?<![\w\\])_([^_\s](?:[^_]*[^_\s])?)_(?!\w)', r'<em>\1</em>', text)


def html_block(line):
    """A line that opens a block of the notes' own HTML: a block tag, or a lone tag"""
    m = re.match(r'\s{0,3}<(/?)([A-Za-z][A-Za-z0-9-]*)', line)
    return bool(m) and (m.group(2).lower() in BLOCK_TAGS or bool(TAG.fullmatch(line.strip())))


def starts_block(line):
    return bool(re.match(r'\s{0,3}(#{1,6}\s|[-*+]\s|\d+[.)]\s|>|\||```|(-{3,}|\*{3,}|_{3,})\s*$)', line)) \
        or html_block(line)


def cells(row):
    row = row.strip()
    row = row[1:] if row.startswith('|') else row
    row = row[:-1] if row.endswith('|') and not row.endswith('\\|') else row
    return [c.strip() for c in re.split(r'(?<!\\)\|', row)]


def markdown(md, shift=1):
    """The notes' markdown as HTML, its headings `shift` levels down (the
    page's own headings above them)"""
    lines, out, i = md.split('\n'), [], 0
    while i < len(lines):
        line = lines[i]
        if not line.strip():
            i += 1
        elif re.match(r'\s{0,3}(-{3,}|\*{3,}|_{3,})\s*$', line):
            out.append('<hr>')
            i += 1
        elif re.match(r'\s{0,3}#{1,6}\s', line):
            m = re.match(r'\s{0,3}(#{1,6})\s+(.*?)(?:\s+#+)?\s*$', line)
            n = min(6, len(m.group(1)) + shift)
            out.append(f'<h{n}>{inline(m.group(2))}</h{n}>')
            i += 1
        elif line.startswith('```'):
            j = i + 1
            while j < len(lines) and not lines[j].startswith('```'):
                j += 1
            out.append('<pre><code>' + esc('\n'.join(lines[i + 1:j])) + '</code></pre>')
            i = j + 1
        elif html_block(line):
            j = i
            while j < len(lines) and lines[j].strip():
                j += 1
            out.append('\n'.join(lines[i:j]))
            i = j
        elif line.lstrip().startswith('|') and i + 1 < len(lines) and re.match(r'\s*\|?\s*:?-+', lines[i + 1]):
            head, aligns = cells(line), cells(lines[i + 1])
            align = ['right' if a.endswith(':') and not a.startswith(':') else 'center' if a.startswith(':') and a.endswith(':')
                     else None for a in aligns]
            j, rows = i + 2, []
            while j < len(lines) and lines[j].lstrip().startswith('|'):
                rows.append(cells(lines[j]))
                j += 1

            def row(cs, tag):
                styled = ['' if k >= len(align) or not align[k] else ' style="text-align:%s"' % align[k] for k in range(len(cs))]
                return '<tr>' + ''.join(f'<{tag}{styled[k]}>{inline(c)}</{tag}>' for k, c in enumerate(cs)) + '</tr>'
            out.append('<div class="table"><table><thead>' + row(head, 'th') + '</thead><tbody>'
                       + ''.join(row(r, 'td') for r in rows) + '</tbody></table></div>')
            i = j
        elif re.match(r'\s{0,3}([-*+]|\d+[.)])\s', line):
            ordered = bool(re.match(r'\s{0,3}\d', line))
            items, j = [], i
            while j < len(lines) and lines[j].strip():
                m = re.match(r'\s{0,3}(?:[-*+]|\d+[.)])\s+(.*)', lines[j])
                if m:
                    items.append(m.group(1))
                elif starts_block(lines[j]):
                    break
                else:
                    items[-1] += ' ' + lines[j].strip()
                j += 1
            tag = 'ol' if ordered else 'ul'
            out.append(f'<{tag}>' + ''.join(f'<li>{inline(t)}</li>' for t in items) + f'</{tag}>')
            i = j
        elif line.lstrip().startswith('>'):
            j = i
            while j < len(lines) and lines[j].lstrip().startswith('>'):
                j += 1
            out.append('<blockquote>' + markdown('\n'.join(re.sub(r'^\s*>\s?', '', l) for l in lines[i:j]), shift)
                       + '</blockquote>')
            i = j
        else:
            j = i + 1
            while j < len(lines) and lines[j].strip() and not starts_block(lines[j]):
                j += 1
            out.append('<p>' + inline(' '.join(l.strip() for l in lines[i:j])) + '</p>')
            i = j
    return '\n'.join(out)


# ---- where a release's files are ----

def image_size(path):
    """A PNG's or GIF's width and height, or None"""
    try:
        with open(path, 'rb') as f:
            head = f.read(26)
    except OSError:
        return None
    if head[:8] == b'\x89PNG\r\n\x1a\n':
        return int.from_bytes(head[16:20], 'big'), int.from_bytes(head[20:24], 'big')
    if head[:4] == b'GIF8':
        return int.from_bytes(head[6:8], 'little'), int.from_bytes(head[8:10], 'little')
    return None


def placed(md, rel, current):
    """The notes' relative paths resolved: the current release's pictures
    to the site's copies (marked \x01 for the page's way to the site's root),
    everything else at the release's tag as tools/release_notes.py does"""
    if current:
        def local(m):
            folder, rest = m.group(2), m.group(3)
            if os.path.isfile(os.path.join(ROOT, 'docs', folder, rest)):
                return f'{m.group(1)}\x01{SITE_DIRS[folder]}/{rest}'
            return m.group(0)
        md = re.sub(r'(\]\(|src="|href=")\.\./(screenshots|clips)/([^)"\s#?]+)', local, md)
    return at_tag(md, rel['tag'] if rel['date'] else 'main')


def site_file(url):
    """The local file of a site path ('shots/x.png'), or None"""
    for folder, there in SITE_DIRS.items():
        if url.startswith(there + '/'):
            return os.path.join(ROOT, 'docs', folder, url[len(there) + 1:])
    return None


def attrs(tag):
    return dict((k.lower(), html.unescape(v)) for k, v in re.findall(r'([\w-]+)="([^"]*)"', tag))


def pictures(text, up):
    """The rendered notes' pictures as the page shows them: lazy, sized where
    the site has the file, and a GIF the site has as a clip played as one"""
    def one(m):
        tag = m.group(0)
        a = attrs(tag)
        src = a.get('src', '')
        if src.startswith('\x01'):
            path = src[1:]
            stem, ext = os.path.splitext(path)
            size = image_size(site_file(path))
            width = int(a['width']) if a.get('width', '').isdigit() else (size[0] if size else None)
            height = round(width * size[1] / size[0]) if size and width else None
            dims = f' width="{width}"' + (f' height="{height}"' if height else '') if width else ''
            clip = site_file(stem)
            if ext == '.gif' and all(os.path.isfile(clip + e) for e in ('.webm', '.mp4', '.png')):
                return (f'<video class="clip" muted loop playsinline preload="none" poster="{up}{stem}.png"{dims}'
                        f' aria-label="{html.escape(a.get("alt", ""))}"><source src="{up}{stem}.webm" type="video/webm">'
                        f'<source src="{up}{stem}.mp4" type="video/mp4"></video>')
            return (f'<img src="{up}{path}"{dims} loading="lazy" decoding="async"'
                    f' alt="{html.escape(a.get("alt", ""))}">')
        if 'loading=' not in tag:
            tag = tag[:-1].rstrip('/').rstrip() + ' loading="lazy" decoding="async">'
        return tag
    text = re.sub(r'<img\s[^>]*>', one, text)
    return text.replace('\x01', up)


def trailer(intro, rel, up, words):
    """The trailer a release's notes open with (a picture linking its .mp4),
    as a video that plays in the page; None where they open with none"""
    m = re.search(r'<a href="([^"]+\.mp4)"[^>]*>\s*(<img\s[^>]*>)\s*</a>', intro)
    if not m:
        return None
    href, a = m.group(1), attrs(m.group(2))
    poster = a.get('src', '').replace('trailer-play.png', 'trailer.png')
    if href.startswith('\x01'):
        stem = href[1:-len('.mp4')]
        sources = [f'{up}{stem}.webm', f'{up}{stem}.mp4'] if os.path.isfile(site_file(stem) + '.webm') else [f'{up}{stem}.mp4']
        poster = poster.replace('\x01', up)
    else:
        sources = [href]
    # (the notes' own words for it on the English page, the page's on the other)
    label = a['alt'] if a.get('alt') and words.get('english') == '' else words['trailer'].format(version=rel['version'])
    return (f'<figure class="frame theater"><video class="trailer" controls playsinline preload="none" poster="{html.escape(poster)}"'
            f' width="1280" height="720" aria-label="{html.escape(label)}" data-version="{rel["version"]}">'
            + ''.join(f'<source src="{html.escape(s)}" type="video/{"webm" if s.endswith(".webm") else "mp4"}">' for s in sources)
            + '</video></figure>')


# ---- the page ----

MONTHS = ('January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October',
          'November', 'December')


def when(date, lang, short=False):
    """A date as the page's language writes it (whatever the builder's locale)"""
    if not date:
        return None
    d = datetime.date.fromisoformat(date)
    if lang == 'ja':
        return f'{d.year}年{d.month}月{d.day}日'
    month = MONTHS[d.month - 1]
    return f'{d.day} {month[:3] if short else month} {d.year}'


def parts(md):
    """The notes' opening (its picture or trailer, headline and lead, before
    the first rule) and the rest"""
    m = re.search(r'^---\s*$', md, re.M)
    if not m or not re.search(r'<h2 align="center">', md[:m.start()]):
        return '', md
    return md[:m.start()].strip(), md[m.end():].strip()


def headline(rel):
    """A release's one line: its notes' headline, or the CHANGELOG's first entry's title"""
    m = re.search(r'<h2 align="center">(.*?)</h2>', rel['md'], re.S)
    if m:
        return ' '.join(re.sub(r'<[^>]+>', '', m.group(1)).split())
    m = re.search(r'\*\*(.+?)\*\*', rel['md'], re.S) or re.search(r'^#{2,4}\s+(.+)$', rel['md'], re.M)
    if not m:
        return rel['version']
    title = re.sub(r'[*`\[\]#]', '', re.sub(r'\s*\([^)]*\)', '', m.group(1)))
    return ' '.join(title.split()).rstrip('.:;, ') + '.'


def changelog_url(rel):
    if not rel['date']:
        return f'{GITHUB}/blob/dev/CHANGELOG.md#unreleased'
    anchor = re.sub(r'[^\w\- ]', '', f'{rel["version"]} ({rel["date"]})'.lower()).replace(' ', '-')
    return f'{GITHUB}/blob/{rel["tag"]}/CHANGELOG.md#{anchor}'


def article(rel, current, newest, lang, up, words, newer=None, older=None):
    """A release on the page: `current` the first (its pictures the site's),
    `newest` the newest released (the one the download page offers)"""
    v, md = rel['version'], placed(rel['md'], rel, current)
    intro, body = parts(md)
    video = trailer(intro, rel, up, words)
    head = esc(headline(rel))
    lead, shown = '', ''
    if intro:
        blocks = re.findall(r'<p align="center">(.*?)</p>', intro, re.S)
        leads = [p for p in blocks if '<img' not in p]
        lead = ' '.join(leads[-1].split()) if leads else ''
        # (the opening's pictures; under a trailer, the first of the notes)
        shown = '\n'.join(f'<p align="center">{p.strip()}</p>' for p in blocks if '<img' in p and not re.search(r'\.mp4"', p))
        if video and shown:
            body, shown = shown + '\n\n' + body, ''
    date = when(rel['date'], lang)
    stamp = f'<time datetime="{rel["date"]}">{date}</time>' if date else words['coming']
    links = []
    if newest:
        links.append(f'<a class="exit" href="{up}download/" data-umami-event="go-download" data-umami-event-from="versions">'
                     f'{words["download"]}</a>')
    if rel['date']:
        links.append(f'<a class="plate" href="{GITHUB}/releases/tag/{rel["tag"]}" data-umami-event="outbound"'
                     f' data-umami-event-to="release" data-umami-event-version="{v}"><i class="icon i-get" aria-hidden="true"></i>'
                     f'{words["release" if newest else "files"]}</a>')
    links.append(f'<a class="pill" href="{changelog_url(rel)}" data-umami-event="outbound" data-umami-event-to="changelog"'
                 f' data-umami-event-version="{v}">{words["changes"]}</a>')
    en = ' lang="en"' if lang != 'en' else ''
    notes = pictures(markdown(body), up)
    a = anchor(v)
    mark = f' <b class="new">{words["newest"]}</b>' if newest else ''
    walk = ''.join(f'<a class="pill" href="#{anchor(o["version"])}" rel="{r}">{words[r].format(version=o["version"])}</a>'
                   for o, r in ((newer, 'next'), (older, 'prev')) if o)
    return (f'<article class="release{" current" if current else ""}" id="{a}" aria-labelledby="{a}-title">\n'
            f'<span class="who">{words["version"].format(version=v)} · {stamp}{mark}</span>\n'
            f'<div class="panel opening">\n'
            + (video or pictures(shown, up))
            + f'\n<h2 id="{a}-title"{en}>{head}</h2>\n'
            + (f'<p class="lead"{en}>{pictures(lead, up)}</p>\n' if lead else '')
            + f'<div class="actions">{"".join(links)}</div>\n</div>\n'
            + (f'<p class="in-english">{words["english"]}</p>\n' if lang != 'en' else '')
            + f'<div class="textbox notes"{en}>\n{notes}\n</div>\n'
            + (f'<nav class="walk" aria-label="{words["walk"]}">{walk}</nav>\n' if walk else '')
            + '</article>')


def anchor(version):
    """A release's id on the page: v0-13-0 (an address may name it v0.13.0 too, versions.js)"""
    return 'v' + version.replace('.', '-')


def row(rel, current, lang, words):
    v = rel['version']
    date = when(rel['date'], lang, short=True)
    stamp = f'<time datetime="{rel["date"]}">{date}</time>' if date else words['coming']
    kind = ' major' if v.endswith('.0') else ' patch'
    film = f' · <span class="film">{words["film"]}</span>' if re.search(r'<a href="[^"]+\.mp4"', rel['md']) else ''
    on = ' class="on"' if current else ''
    return (f'<li{on}><a class="row{kind}" href="#{anchor(v)}"><span class="ver">{v}</span>'
            f'<span class="what"><span class="head">{esc(headline(rel))}</span><span class="when">{stamp}{film}</span></span></a></li>')


def structured(rels, lang):
    """schema.org: the game at its newest version, these notes its release notes"""
    new = next((r for r in rels if r['date']), None)
    if not new:
        return ''
    page = SITE_URL + PAGES[lang]
    doc = {'@context': 'https://schema.org', '@type': 'VideoGame', 'name': 'Cyberworld Endless',
           'url': SITE_URL + ('ja/' if lang == 'ja' else ''), 'softwareVersion': new['version'],
           'datePublished': new['date'], 'releaseNotes': page + "#" + anchor(new["version"]),
           'downloadUrl': SITE_URL + ('ja/' if lang == 'ja' else '') + 'download/', 'inLanguage': lang}
    return '<script type="application/ld+json">\n' + json.dumps(doc, ensure_ascii=False) + '\n</script>\n'


def fill(template, lang, rels=None):
    """A versions page's template (its text) with the releases in it"""
    rels = rels if rels is not None else releases()
    m = re.search(r'<script type="application/json" id="versions-words">(.*?)</script>\n?', template, re.S)
    words = json.loads(m.group(1))
    text = template[:m.start()] + template[m.end():]
    up = '../' if lang == 'en' else '../../'
    newest = next((r for r in rels if r['date']), None)
    text = text.replace('<!-- versions:newest -->', newest['version'] if newest else '')
    text = text.replace('<!-- versions:releases -->', '\n'.join(
        article(r, k == 0, r is newest, lang, up, words, rels[k - 1] if k else None, rels[k + 1] if k + 1 < len(rels) else None)
        for k, r in enumerate(rels)))
    text = text.replace('<!-- versions:list -->', '\n'.join(row(r, k == 0, lang, words) for k, r in enumerate(rels)))
    return text.replace('</head>', structured(rels, lang) + '</head>', 1)


def feed(rels=None):
    """An Atom feed of the releases, each entry its headline and lead,
    linking its place on the versions page"""
    rels = [r for r in (rels if rels is not None else releases()) if r['date']]
    page = SITE_URL + PAGES['en']
    entries = []
    for r in rels:
        intro, _ = parts(r['md'])
        leads = [p for p in re.findall(r'<p align="center">(.*?)</p>', intro, re.S) if '<img' not in p]
        summary = ' '.join(re.sub(r'<[^>]+>', '', leads[-1]).split()) if leads else headline(r)
        entries.append(
            f'  <entry>\n    <title>Cyberworld Endless {r["version"]}: {html.escape(headline(r))}</title>\n'
            f'    <link rel="alternate" type="text/html" href="{page}#{anchor(r["version"])}"/>\n'
            f'    <link rel="related" href="{GITHUB}/releases/tag/{r["tag"]}"/>\n'
            f'    <id>{GITHUB}/releases/tag/{r["tag"]}</id>\n'
            f'    <updated>{r["date"]}T00:00:00Z</updated>\n'
            f'    <summary>{html.escape(html.unescape(summary))}</summary>\n  </entry>\n')
    updated = rels[0]['date'] if rels else '2026-01-01'
    return ('<?xml version="1.0" encoding="utf-8"?>\n<feed xmlns="http://www.w3.org/2005/Atom" xml:lang="en">\n'
            '  <title>Cyberworld Endless: versions</title>\n'
            '  <subtitle>Every release of Cyberworld Endless, the Mega Man Battle Network 6 roguelike</subtitle>\n'
            f'  <link rel="alternate" type="text/html" href="{page}"/>\n'
            f'  <link rel="self" type="application/atom+xml" href="{SITE_URL}{FEED}"/>\n'
            f'  <id>{page}</id>\n  <updated>{updated}T00:00:00Z</updated>\n'
            '  <author><name>Sascha Becker</name><uri>https://github.com/saschb2b</uri></author>\n'
            + ''.join(entries) + '</feed>\n')


def build(out):
    """The two pages in a built site (out/ as web/ copied) and the feed"""
    rels = releases()
    for lang, page in PAGES.items():
        path = os.path.join(out, page, 'index.html')
        with open(path, encoding='utf-8') as f:
            text = f.read()
        with open(path, 'w', encoding='utf-8') as f:
            f.write(fill(text, lang, rels))
    with open(os.path.join(out, FEED), 'w', encoding='utf-8') as f:
        f.write(feed(rels))


if __name__ == '__main__':
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    import shutil
    shutil.copytree(os.path.join(ROOT, 'web'), sys.argv[1], dirs_exist_ok=True)
    build(sys.argv[1])
