#!/usr/bin/env python3
"""tools/discord_release.py's post, without Discord: every release's fits
Discord's limits, a long one keeps its sections and counts what it leaves
out, the role is pinged only where it is given, and the links survive a
webhook that refuses buttons. No network, no cover (Pillow and the tags'
pictures are tools/itch_art.py's, checked there)."""
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import discord_release as dr  # noqa: E402
import site_versions  # noqa: E402

failed = 0


def expect(ok, what):
    global failed
    if not ok:
        failed += 1
        print('FAIL:', what)


rels = site_versions.releases()
for rel in rels:
    m = dr.post(rel, cover=False)
    e = m['embeds'][0]
    expect(not dr.check(m), f'{rel["tag"]}: {dr.check(m)}')
    expect(e['title'] and e['description'], f'{rel["tag"]}: a title and a description')
    expect('image' not in e and 'attachments' not in m, f'{rel["tag"]}: no picture asked for, none named')

# (a release's notes: their sections and entries' titles, the lead first)
rel = next(r for r in rels if r['tag'] == 'v0.23.0')
d = dr.description(rel)
expect(d.startswith('Your friends on Discord now see'), '0.23.0: the lead first')
expect('**🛠️ Fixed**' in d and '• SlipRunr into a guardian\'s arena' in d, '0.23.0: Fixed and its entries')

# (the longest notes held to the room, every section still named)
longest = max(rels, key=lambda r: len(dr.description(r, room=10 ** 6)))
short = dr.description(longest, room=1500)
expect(len(short) <= 1500, f'{longest["tag"]}: held to 1500, is {len(short)}')
expect(all(f'**{name}**' in short for name, _ in dr.sections(longest) if name) or short.endswith('…'),
       f'{longest["tag"]}: every section named, or the text cut')
expect('more in the notes' in short or short.endswith('…'), f'{longest["tag"]}: what is left out is counted')

# (the role: pinged where given, never @everyone)
m = dr.post(rel, role='42', cover=False)
expect(m['content'].startswith('<@&42> ') and m['allowed_mentions'] == {'parse': [], 'roles': ['42']}, 'the role pinged')
m = dr.post(rel, cover=False)
expect('<@&' not in m['content'] and m['allowed_mentions'] == {'parse': [], 'roles': []}, 'no role, no ping')

# (no buttons: the same links in the text)
plain = dr.without_buttons(m)
expect('components' not in plain, 'no components left')
expect(all(f'[{label}]({url})' in plain['embeds'][0]['description'] for label, url in dr.links(rel)), 'the links in the text')
expect('components' in m, 'the original keeps its buttons')

print(f'discord_release: {len(rels)} releases, {failed} failed')
sys.exit(1 if failed else 0)
