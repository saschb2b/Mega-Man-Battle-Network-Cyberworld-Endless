"""A release's devlog post on the itch.io page (docs/ITCH.md, Each release):
its title, post type and HTML from the same notes the site's versions page
renders (tools/site_versions.py: docs/releases/vX.Y.Z.md, else the
CHANGELOG's section), for itch's post editor.

The notes' pictures stay, at their raw GitHub address at the release's
tag; itch shows no picture from another site, so tools/itch_pictures.js,
run in itch's editor, uploads each one to itch and swaps its address
before the post is saved. A picture over itch's 3 MB is a link instead.
The post opens with the notes' own lead and ends with where the notes are
on the website and GitHub.
"""
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import site_versions  # noqa: E402

RAW = re.compile(r'<img\b[^>]*\bsrc="(https://raw\.githubusercontent\.com/[^/]+/[^/]+/([^/]+)/([^"]+))"[^>]*>')
ITCH_LIMIT = 3 * 1024 * 1024
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def too_big(m):
    """A picture over itch's limit, as a link to it."""
    url, ref, path = m.groups()
    size = subprocess.run(['git', 'cat-file', '-s', f'{ref}:{path}'], cwd=ROOT, capture_output=True, text=True).stdout
    if size.strip() and int(size) > ITCH_LIMIT:
        return f'<a href="{url}">{os.path.basename(path)}</a>'
    return m.group(0)


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
    """The notes as itch takes them: their own lead paragraph (the opening's
    picture and headline left out, the headline being the post's title),
    then the rest, links and pictures at the tag."""
    md = site_versions.at_tag(rel['md'], rel['tag'])
    if rel['source'] == 'notes':
        opening, md = site_versions.parts(md)
        lead = re.split(r'</h2>', opening, maxsplit=1)[-1] if '</h2>' in opening else ''
        md = (lead.strip() + '\n\n' + md) if lead.strip() else md
    else:
        md = entries(md)
    html = RAW.sub(too_big, site_versions.markdown(md, shift=0))
    # (the notes' centring, which itch's editor drops)
    return re.sub(r' align="center"', '', html)


def post(rel):
    """The post: its title, itch's post type (Major Update for a minor
    release, X.Y.0, General Update for a patch), its date and its HTML,
    which ends where the notes are with their pictures."""
    anchor = site_versions.anchor(rel['version'])
    end = (f"<p>These notes with their pictures are on the <a href=\"{site_versions.SITE_URL}versions/#{anchor}\">"
           f"project's website</a> and <a href=\"{site_versions.GITHUB}/releases/tag/{rel['tag']}\">GitHub</a>.</p>")
    return {'version': rel['version'], 'title': f"{rel['version']}: {site_versions.headline(rel).rstrip('.')}",
            'type': 'major_update' if rel['version'].endswith('.0') else 'general_update',
            'date': rel['date'], 'html': body(rel) + '\n' + end}
