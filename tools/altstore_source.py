#!/usr/bin/env python3
"""The AltStore source of a release: the JSON that AltStore and SideStore
read to install the iPhone and iPad app and to offer its updates
(https://faq.altstore.io/developers/make-a-source). The release workflow
attaches it to each release beside the IPA, so the source's address,

    https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases/latest/download/altstore-source.json

always serves the newest one. AltStore checks the IPA it downloads against
it: the bundle identifier, the version (CFBundleShortVersionString) and the
build (CFBundleVersion), which build.py ios writes from the tag, and the
entitlements and privacy keys, of which the app has none.

    python3 tools/altstore_source.py vX.Y.Z build/release/cyberworld-endless.ipa > altstore-source.json
"""
import datetime
import json
import os
import sys

REPO = 'saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless'
SITE = 'https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/'
BUNDLE_ID = 'io.github.saschb2b.cyberworldendless'   # (ios/Info.plist's)
TINT = '4A7B42'                                        # (the site's theme colour)


def source(tag, size, date):
    version = tag[1:] if tag.startswith('v') else tag
    raw = f'https://raw.githubusercontent.com/{REPO}/{tag}'
    # (the game's own pictures; the 240 x 160 ones as they are, landscape)
    shots = [{'imageURL': f'{raw}/docs/screenshots/touch.png', 'width': 1200, 'height': 540}]
    shots += [{'imageURL': f'{raw}/docs/screenshots/{n}.png', 'width': 240, 'height': 160}
              for n in ('title', 'net', 'battle', 'guardian', 'custom')]
    about = ('A roguelike that runs Mega Man Battle Network 6: Cybeast Gregar on an embedded GBA core and turns it into '
             'endless, generated net layers: battles, Net Dealers, guardians, NaviCust drafts and runs that end. '
             'It needs your own copy of the game, an unzipped .gba file of Mega Man Battle Network 6: Cybeast Gregar (USA): '
             'choose it in Files when the app asks, or put it in Files, On My iPhone (or iPad), in the Cyberworld folder. '
             'Touch controls, or a controller. Free and open source; an unofficial fan project, not affiliated with Capcom.')
    app = {
        'name': 'Cyberworld Endless',
        'bundleIdentifier': BUNDLE_ID,
        'developerName': 'saschb2b',
        'subtitle': 'A roguelike on Mega Man Battle Network 6',
        'localizedDescription': about,
        'iconURL': f'{SITE}assets/icons/512.png',
        'tintColor': TINT,
        'category': 'games',
        'screenshots': shots,
        'versions': [{
            'version': version,
            'buildVersion': version,
            'date': date,
            'localizedDescription': f'What changed: https://github.com/{REPO}/releases/tag/{tag}',
            'downloadURL': f'https://github.com/{REPO}/releases/download/{tag}/cyberworld-endless.ipa',
            'size': size,
            'minOSVersion': '14.0',
        }],
        'appPermissions': {'entitlements': [], 'privacy': {}},
    }
    # (the keys of sources before AltStore 2, which older AltStore and
    # SideStore builds read instead of versions)
    v = app['versions'][0]
    app.update({'version': v['version'], 'versionDate': v['date'], 'versionDescription': v['localizedDescription'],
                'downloadURL': v['downloadURL'], 'size': v['size']})
    return {
        'name': 'Cyberworld Endless',
        'identifier': BUNDLE_ID + '.source',
        'subtitle': 'Mega Man Battle Network 6, endless',
        'description': f'The iPhone and iPad app of Cyberworld Endless, from its GitHub releases. {SITE}',
        'iconURL': f'{SITE}assets/icons/512.png',
        'headerURL': f'{SITE}clips/trailer.png',
        'website': SITE,
        'tintColor': TINT,
        'featuredApps': [BUNDLE_ID],
        'apps': [app],
        'news': [],
    }


def main():
    if len(sys.argv) != 3:
        sys.exit('usage: altstore_source.py vX.Y.Z path/to/cyberworld-endless.ipa')
    tag, ipa = sys.argv[1], sys.argv[2]
    date = datetime.datetime.now(datetime.timezone.utc).strftime('%Y-%m-%dT%H:%M:%SZ')
    json.dump(source(tag, os.path.getsize(ipa), date), sys.stdout, indent=2)
    sys.stdout.write('\n')


if __name__ == '__main__':
    main()
