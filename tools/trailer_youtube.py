#!/usr/bin/env python3
"""The trailers for YouTube: each one's 4K master, its thumbnail and its
title, description and tags, into .build/youtube/ (docs/project/youtube.md).

    python3 tools/trailer_youtube.py [VERSION ...]

Every trailer in trailers.VERSIONS by default. A trailer's video is the
last docs/clips/trailer.mp4 committed while it was the current one (0.12.0's
came after its tag, 0.3.0's end card was corrected after its), or, where
tools/trailer.py rendered it here, .build/trailer-VERSION/youtube.mp4, the
master it writes from its lossless frames.

The master is the 1280x720 trailer scaled 3x, nearest neighbour, to
3840x2160, at a high quality: YouTube streams a 4K upload with VP9 or AV1
at every size and at a higher bitrate, where a 720p upload's H.264 blurs
the game's pixels.
"""
import html
import json
import os
import re
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import build  # noqa: E402  (the ffmpeg image)
import trailers  # noqa: E402

OUT = os.path.join(ROOT, '.build', 'youtube')
CLIP = 'docs/clips/trailer.mp4'
SITE = 'https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/'
REPO = 'https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless'
ITCH = 'https://saschb2b.itch.io/cyberworld-endless'
TAGS = ['Cyberworld Endless', 'Mega Man Battle Network', 'Mega Man Battle Network 6', 'MMBN6', 'Battle Network',
        'Rockman EXE', 'roguelike', 'fan game', 'GBA', 'trailer']

# YouTube's recommendations for an upload: H.264 High, two B-frames, a closed
# GOP of half the frame rate; the picture whole at three times its size, and
# capped at 20 Mbit/s, above what YouTube streams in 4K (a trailer's
# scanlines and flashes ran to 50 uncapped, kept from nobody)
VIDEO = ['-vf', 'scale=iw*3:ih*3:flags=neighbor,format=yuv420p', '-c:v', 'libx264', '-preset', 'medium', '-crf', '18',
         '-maxrate', '20M', '-bufsize', '40M',
         '-profile:v', 'high', '-bf', '2', '-g', '15', '-flags', '+cgop', '-r', '30', '-movflags', '+faststart']


def name(version):
    return f'cyberworld-endless-{version}-trailer'


def frames_job(work, dest):
    """ffmpeg's arguments for a master from a render's frames and music.wav
    (paths under /src, the tree's mount): tools/trailer.py's third job."""
    return ['-y', '-loglevel', 'error', '-framerate', '30', '-i', f'{work}/frames/%05d.png', '-i', f'{work}/music.wav',
            '-shortest', *VIDEO, '-c:a', 'aac', '-b:a', '384k', '-ar', '48000', dest]


def ffmpeg(args, tag):
    if subprocess.call(['docker', 'run', '--name', f'cw-youtube-{os.getpid()}-{tag}', '--rm', '-u', f'{os.getuid()}:{os.getgid()}',
                        '--entrypoint', 'ffmpeg', '-v', f'{ROOT}:/src', build.FFMPEG_IMAGE, *args]) != 0:
        sys.exit('ffmpeg failed')


def git(*args):
    return subprocess.run(['git', '-C', ROOT, *args], check=True, capture_output=True).stdout


def sources():
    """{version: commit}: the last commit of docs/clips/trailer.mp4 while
    each trailer was the current one (before tools/trailers/, 0.3.0's)."""
    found = {}
    for sha in git('log', '--format=%H', '--', CLIP).decode().split():
        try:
            init = git('show', f'{sha}:tools/trailers/__init__.py').decode()
            version = re.search(r"CURRENT = '([0-9.]+)'", init).group(1)
        except subprocess.CalledProcessError:
            version = '0.3.0'
        found.setdefault(version, sha)   # (the log runs newest first)
    return found


def notes(version):
    """The release notes' tagline and the paragraph under it, as text."""
    with open(os.path.join(ROOT, 'docs', 'releases', f'v{version}.md'), encoding='utf-8') as f:
        text = f.read()
    tagline = re.search(r'^description: "(.*)"$', text, re.M).group(1)
    para = re.search(r'<h2[^>]*>.*?</h2>\s*<p[^>]*>(.*?)</p>', text, re.S).group(1)
    para = re.sub(r'^\s*<b>[^<]*:</b>\s*', '', para)   # (no "the eighth beta": the news first)
    para = html.unescape(re.sub(r'<[^>]+>', '', para)).strip()
    return tagline, para[:1].upper() + para[1:]


def released(version):
    """The release's date, its tag's."""
    return git('log', '-1', '--format=%cs', f'v{version}').decode().strip()


def describe(version):
    tagline, para = notes(version)
    date = released(version)
    title = f'Cyberworld Endless {version} Trailer – {tagline}'
    if len(title) > 100:
        title = f'Cyberworld Endless {version} Trailer'
    text = '\n'.join([
        f'{tagline} {para}',
        '',
        f'Cyberworld Endless {version}, released {date}: a roguelike that runs Mega Man Battle Network 6 '
        'from your own ROM. Every shot is the game running; the music is original, written for this trailer.',
        '',
        f'Play in the browser: {SITE}play/',
        f'Download (PC, handhelds, phones, consoles): {SITE}download/',
        f'What {version} brought: {REPO}/releases/tag/v{version}',
        f'itch.io: {ITCH}',
        f'Source: {REPO}',
        '',
        'You need your own ROM: Mega Man Battle Network 6: Cybeast Gregar (USA). No download contains Capcom data.',
        'Mega Man Battle Network is © Capcom. This project is not affiliated with or endorsed by Capcom.',
        '',
        '#MegaManBattleNetwork #roguelike #indiegame',
    ])
    return {'version': version, 'title': title, 'description': text, 'tags': TAGS, 'recorded': date,
            'video': f'{name(version)}.mp4', 'thumbnail': f'{name(version)}.png'}


def export(version, sha):
    """The master, the thumbnail and the words of one trailer."""
    dest = os.path.join(OUT, name(version))
    rendered = os.path.join(ROOT, '.build', f'trailer-{version}', 'youtube.mp4')
    if os.path.exists(rendered):
        shutil.copyfile(rendered, dest + '.mp4')
    else:
        src = os.path.join(OUT, 'src', f'{version}.mp4')
        with open(src, 'wb') as f:
            f.write(git('show', f'{sha}:{CLIP}'))
        rel = lambda p: '/src/' + os.path.relpath(p, ROOT)  # noqa: E731
        ffmpeg(['-y', '-loglevel', 'error', '-i', rel(src), *VIDEO, '-c:a', 'copy', rel(dest + '.mp4')], version)
    with open(dest + '.png', 'wb') as f:
        f.write(git('show', f'{sha}:docs/clips/trailer.png'))
    return describe(version)


def main():
    versions = sys.argv[1:] or list(trailers.VERSIONS)
    found = sources()
    os.makedirs(os.path.join(OUT, 'src'), exist_ok=True)
    listed = os.path.join(OUT, 'videos.json')
    videos = {}
    if os.path.exists(listed):
        with open(listed, encoding='utf-8') as f:
            videos = {v['version']: v for v in json.load(f)}
    missing = [v for v in versions if v not in found]
    if missing:
        sys.exit(f'no committed trailer for {", ".join(missing)}')

    def one(version):
        videos[version] = export(version, found[version])
        print('wrote', os.path.relpath(os.path.join(OUT, name(version)), ROOT) + '.mp4, .png', 'from', found[version][:9], flush=True)
    with ThreadPoolExecutor(2) as pool:   # (two encodes at once, oldest first)
        list(pool.map(one, versions))
    order = sorted(videos.values(), key=lambda v: tuple(int(x) for x in v['version'].split('.')))
    with open(listed, 'w', encoding='utf-8') as f:
        json.dump(order, f, indent=1, ensure_ascii=False)
    print('wrote', os.path.relpath(listed, ROOT))


if __name__ == '__main__':
    main()
