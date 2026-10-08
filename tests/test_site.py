#!/usr/bin/env python3
"""The project's pages (web/) and their Japanese ones (web/ja/), as search
engines and visitors meet them: each page's language, its canonical address,
its hreflang links to both and the bar's link to the other; the two kept in
step (the same ids, which the shared scripts look up, the same pictures and
clips, the same scripts and styles, the FAQ's letters and the downloads'
systems in the same order); and every link between the pages, to their files
and to their anchors, there. Standard library only; build.py test runs it
after the C tests."""
import html.parser
import os
import sys
import urllib.parse

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WEB = os.path.join(ROOT, 'web')
SITE_URL = 'https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/'
PAGES = ('', 'download/', 'play/', 'faq/')
LANGS = {'en': '', 'ja': 'ja/'}
LOCALES = {'en': 'en_US', 'ja': 'ja_JP'}
# what build.py site puts beside the pages: the screenshots, the clips, the
# license, and the browser build's own files (not checked, made by its build)
BUILT = {'shots/': os.path.join(ROOT, 'docs', 'screenshots'), 'clips/': os.path.join(ROOT, 'docs', 'clips')}
MADE = ('licenses/', 'play/cyberworld.js', 'play/cyberworld.wasm')

failed = 0


def check(ok, what):
    global failed
    if not ok:
        failed += 1
        print('FAIL:', what)


class Page(html.parser.HTMLParser):
    """A page's facts: its language, links in the head, metas, ids, media,
    scripts and styles, every link's address, and the elements the scripts
    read their lists from."""
    def __init__(self, path):
        super().__init__()
        self.path = path            # under web/, e.g. 'ja/faq/index.html'
        self.lang = None
        self.links, self.metas = [], {}
        self.ids, self.media, self.code, self.urls = [], [], [], []
        self.mails, self.systems, self.assets, self.lang_links = [], [], [], []
        with open(os.path.join(WEB, path), encoding='utf-8') as f:
            self.feed(f.read())

    def handle_starttag(self, tag, attrs):
        a = dict(attrs)
        if tag == 'html':
            self.lang = a.get('lang')
        if 'id' in a:
            self.ids.append(a['id'])
        if tag == 'link':
            self.links.append(a)
            if a.get('rel') == 'stylesheet':
                self.code.append(self.site(a['href']))
        if tag == 'meta' and 'property' in a:
            self.metas.setdefault(a['property'], []).append(a.get('content'))
        if tag == 'script' and 'src' in a and not a['src'].startswith('http'):
            self.code.append(self.site(a['src']))
        if tag in ('img', 'source') and 'src' in a:
            self.media.append(self.site(a['src']))
        if tag == 'video' and 'poster' in a:
            self.media.append(self.site(a['poster']))
        if tag == 'article' and 'mail' in (a.get('class') or '').split():
            self.mails.append(a.get('id'))
        if tag == 'li' and 'data-os' in a:
            self.systems.append(a['data-os'])
        for k in ('data-asset', 'data-needs'):
            if k in a:
                self.assets.append(a[k])
        if tag == 'a' and 'lang' in (a.get('class') or '').split():
            self.lang_links.append(a)
        # (the head's canonical and hreflang addresses are the live site's,
        # which site() leaves out)
        self.urls += [a[k] for k in ('href', 'src', 'poster') if a.get(k)]

    def site(self, url):
        """`url` as a path under the site's root ('' the home page), or None
        for one elsewhere"""
        full = urllib.parse.urljoin('https://site/' + self.path, url)
        if not full.startswith('https://site/'):
            return None
        return urllib.parse.unquote(urllib.parse.urlsplit(full).path[len('/'):])

    def head(self, rel, **want):
        return [l for l in self.links if l.get('rel') == rel and all(l.get(k) == v for k, v in want.items())]


def target(path):
    """The file a site path is, or None for one the site's build makes"""
    if path.startswith(MADE):
        return None
    for prefix, folder in BUILT.items():
        if path.startswith(prefix):
            return os.path.join(folder, path[len(prefix):])
    if path == 'LICENSE.txt':
        return os.path.join(ROOT, 'LICENSE')
    return os.path.join(WEB, path + 'index.html' if path == '' or path.endswith('/') else path)


pages = {}
for lang, prefix in LANGS.items():
    for page in PAGES:
        path = prefix + page + 'index.html'
        check(os.path.isfile(os.path.join(WEB, path)), f'{path} is there')
        if os.path.isfile(os.path.join(WEB, path)):
            pages[lang, page] = Page(path)

ids_cache = {}


def ids_of(path):
    if path not in ids_cache:
        ids_cache[path] = set(Page(os.path.relpath(path, WEB)).ids) if path.startswith(WEB) else set()
    return ids_cache[path]


for (lang, page), p in pages.items():
    me = f'{LANGS[lang]}{page}index.html'
    url = SITE_URL + LANGS[lang] + page
    check(p.lang == lang, f'{me}: lang="{lang}", not {p.lang!r}')
    check([l.get('href') for l in p.head('canonical')] == [url], f'{me}: its canonical address is {url}')
    check(p.metas.get('og:url') == [url], f'{me}: og:url is its address')
    check(p.metas.get('og:locale') == [LOCALES[lang]], f'{me}: og:locale {LOCALES[lang]}')
    other = 'ja' if lang == 'en' else 'en'
    check(p.metas.get('og:locale:alternate') == [LOCALES[other]], f'{me}: og:locale:alternate {LOCALES[other]}')
    # (hreflang: both languages and the default, the English one, on both pages)
    for code, href in (('en', SITE_URL + page), ('ja', SITE_URL + 'ja/' + page), ('x-default', SITE_URL + page)):
        check([l.get('href') for l in p.head('alternate', hreflang=code)] == [href], f'{me}: hreflang {code} is {href}')
    # (the bar's link to the same page in the other language)
    langs = [a for a in p.lang_links if a.get('hreflang') == other]
    check(len(langs) == 1 and p.site(langs[0].get('href', '')) == LANGS[other] + page,
          f'{me}: the bar links to {LANGS[other]}{page} in {other}')
    # (every link to the site's own pages and files leads somewhere, and its anchor too)
    for u in p.urls:
        if u.startswith(('#', 'mailto:')) or urllib.parse.urlsplit(u).scheme not in ('', 'http', 'https'):
            continue
        path = p.site(u)
        if path is None:
            continue
        file = target(path)
        if file is None:
            continue
        check(os.path.isfile(file), f'{me}: {u} leads to a file ({os.path.relpath(file, ROOT)})')
        frag = urllib.parse.urlsplit(u).fragment
        if frag and file.endswith('.html') and os.path.isfile(file):
            check(frag in ids_of(file), f'{me}: {u} names an anchor its page has')

# the Japanese pages kept in step with the English ones
for page in PAGES:
    en, ja = pages.get(('en', page)), pages.get(('ja', page))
    if not en or not ja:
        continue
    me = f'ja/{page}index.html'
    missing, extra = sorted(set(en.ids) - set(ja.ids)), sorted(set(ja.ids) - set(en.ids))
    check(not missing and not extra, f'{me}: the same ids as {page}index.html (missing {missing}, extra {extra})')
    check(en.media == ja.media, f'{me}: the same pictures and clips, in order ({sorted(set(en.media) ^ set(ja.media))})')
    check(sorted(en.code) == sorted(ja.code), f'{me}: the same scripts and styles ({sorted(set(en.code) ^ set(ja.code))})')
    check(en.mails == ja.mails, f'{me}: the FAQ\'s letters in the same order')
    check(en.systems == ja.systems and en.assets == ja.assets, f'{me}: the same systems and files')

if failed:
    sys.exit(f'{failed} site checks failed')
print('all site checks passed')
