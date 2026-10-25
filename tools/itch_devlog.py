"""A release's devlog post on the itch.io page (docs/ITCH.md, Each release):
its title, post type and HTML from the same notes the site's versions page
renders (tools/site_versions.py: docs/releases/vX.Y.Z.md, else the
CHANGELOG's section), for itch's post editor.

itch's pages refuse pictures from other sites, so the notes' pictures are
left out; the post opens with where the release is and where its full
notes, pictures and all, are on the website.
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import site_versions  # noqa: E402

PICTURE = re.compile(r'<img\b[^>]*>|!\[[^\]]*\]\([^)]*\)')


def release(version=None):
    """The release of that version, or the newest dated one."""
    rels = [r for r in site_versions.releases() if r['date']]
    if version is None:
        return rels[0]
    for r in rels:
        if r['version'] == version.lstrip('v'):
            return r
    sys.exit(f'no dated release {version} in CHANGELOG.md')


def entries(md):
    """The CHANGELOG's top-level entries, "- **Title.** text" with their
    sub-lists, as markdown sections: a heading each, its text under it."""
    out = []
    for item in re.split(r'(?m)^- ', md):
        item = item.strip()
        if not item:
            continue
        m = re.match(r'\*\*(.+?)\*\*\s*(.*)', item, re.S)
        title, body = (m.group(1), m.group(2)) if m else ('', item)
        # (its sub-list's items, indented under it, as a list of their own)
        body = re.sub(r'(?m)^ {2,4}([-*+] )', r'\n\1', body)
        title = title.strip().rstrip('.:,')
        out.append((f'## {title}\n\n' if title else '') + body.strip())
    return '\n\n'.join(out)


def body(rel):
    """The notes as itch takes them: no pictures, no opening (the trailer
    and headline the site's article shows), links at the tag."""
    md = site_versions.at_tag(rel['md'], rel['tag'])
    if rel['source'] == 'notes':
        _, md = site_versions.parts(md)
    else:
        md = entries(md)
    md = PICTURE.sub('', md)
    html = site_versions.markdown(md, shift=0)
    # (blocks left empty by their pictures, and the notes' centring)
    html = re.sub(r'<p[^>]*>\s*(<br>\s*)*</p>\s*', '', html)
    return re.sub(r' align="center"', '', html)


def when(date):
    """2026-09-27 as 27 September 2026."""
    y, m, d = (int(x) for x in date.split('-'))
    months = ('January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September',
              'October', 'November', 'December')
    return f'{d} {months[m - 1]} {y}'


def post(rel, newest=None):
    """The post: its title, itch's post type (Major Update for a minor
    release, X.Y.0, General Update for a patch), its date and its HTML. The
    newest release's post says it is on the page; an older one's, back-dated
    to its release, says when it came out."""
    newest = newest or release()['version']
    anchor = site_versions.anchor(rel['version'])
    notes = (f"<a href=\"{site_versions.SITE_URL}versions/#{anchor}\">project's website</a> and "
             f"<a href=\"{site_versions.GITHUB}/releases/tag/{rel['tag']}\">GitHub</a>")
    if rel['version'] == newest:
        lead = (f"<p><strong>{rel['version']}</strong> is on this page: play it in the browser, or download it for "
                f"your system. Its full notes, with pictures, are on the {notes}.</p>")
    else:
        lead = (f"<p><strong>{rel['version']}</strong> came out on {when(rel['date'])}. Its full notes, with "
                f"pictures, are on the {notes}; the newest version is on this page.</p>")
    return {'version': rel['version'], 'title': f"{rel['version']}: {site_versions.headline(rel).rstrip('.')}",
            'type': 'major_update' if rel['version'].endswith('.0') else 'general_update',
            'date': rel['date'], 'html': lead + '\n' + body(rel)}
