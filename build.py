#!/usr/bin/env python3
"""Build, test and package Cyberworld Endless.

Compilation runs inside the `cyberworld-build` Docker image (Debian trixie),
which matches the glibc and SDL2 that current ROCKNIX ships; the Linux
desktop release builds in `cyberworld-linux` (Debian bookworm), whose older
glibc runs on more distributions.

  python3 build.py              host and device binaries
  python3 build.py host         host binary only
  python3 build.py device       aarch64 binary only
  python3 build.py linux        Linux desktop binary (build/linux) and its
                                release archive in build/release
  python3 build.py run ...      build the Linux desktop binary and play it here
                                in a window (game options may follow)
  python3 build.py web          the browser build, assembled as a site in
                                build/site (published on GitHub Pages)
  python3 build.py serve [PORT] build the site and serve it on localhost
  python3 build.py release      the release archives in build/release: the
                                PortMaster port, the Linux desktop build and
                                the browser site
  python3 build.py shot ...     run the host binary headlessly (options below)
  python3 build.py tour [BIOMES] the game itself warped through every room of
                                each area's layer, one sheet per area in
                                .build/tour (docs/DEVTOOLS.md)
  python3 build.py atlas [BIOMES] [SEEDS] [--baseline]
                                every area's layers drawn, one sheet per area
                                in .build/atlas, compared with (or written to,
                                --baseline) tests/atlas_baseline.txt
                                (docs/DEVTOOLS.md)
  python3 build.py pacing       every act's battles and guardians against
                                their bands, in .build/pacing.txt
                                (docs/DEVTOOLS.md)
  python3 build.py test         ROM-free unit tests
  python3 build.py package      assemble build/port/ for PortMaster
"""
import argparse
import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))
IMAGE = 'cyberworld-build'
LINUX_IMAGE = 'cyberworld-linux'   # docker/Dockerfile.linux
WEB_IMAGE = 'cyberworld-web'       # docker/Dockerfile.web
IMAGES = {IMAGE: 'Dockerfile', LINUX_IMAGE: 'Dockerfile.linux', WEB_IMAGE: 'Dockerfile.web'}
CONTEXT = os.environ.get('DOCKER_CONTEXT_NAME', 'desktop-linux')
RELEASE = os.path.join(ROOT, 'build', 'release')
LINUX_NAME = 'cyberworld-endless-linux-x86_64'


def default_rom_dir():
    return os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))


def docker(*cmd, mounts=(), image=IMAGE):
    args = ['docker']
    if CONTEXT:
        args += ['--context', CONTEXT]
    args += ['run', '--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src']
    for host, guest in mounts:
        args += ['-v', f'{host}:{guest}']
    for var in ('CYBERWORLD_AUDIO_DUMP', 'CYBERWORLD_SFX_LOG', 'CYBERWORLD_AUDIO_OFFLINE', 'CYBERWORLD_EMU_DEBUG', 'CYBERWORLD_AUTOPILOT'):
        if os.environ.get(var):
            args += ['-e', f'{var}={os.environ[var]}']
    args += [image, *cmd]
    return subprocess.call(args)


def ensure_image(image=IMAGE):
    dockerfile = IMAGES[image]
    probe = ['docker'] + (['--context', CONTEXT] if CONTEXT else []) + ['image', 'inspect', image]
    if subprocess.call(probe, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL) != 0:
        build = ['docker'] + (['--context', CONTEXT] if CONTEXT else []) + ['build', '-t', image, '-f',
                 os.path.join(ROOT, 'docker', dockerfile), os.path.join(ROOT, 'docker')]
        if subprocess.call(build) != 0:
            sys.exit('docker build failed')


def build(target):
    image = {'linux': LINUX_IMAGE, 'web': WEB_IMAGE}.get(target, IMAGE)
    ensure_image(image)
    werror = ['WERROR=1'] if os.environ.get('CI') else []
    if docker('make', f'TARGET={target}', f'-j{os.cpu_count() or 4}', *werror, image=image) != 0:
        sys.exit(f'{target} build failed')
    if target == 'linux':
        # SDL2 travels with the binary (its rpath is $ORIGIN/lib), with its license
        if docker('sh', '-c', 'mkdir -p build/linux/lib build/linux/licenses && '
                  'cp -L /opt/sdl2/lib/libSDL2-2.0.so.0 build/linux/lib/ && '
                  'cp /opt/sdl2/LICENSE.txt build/linux/licenses/SDL2.txt', image=image) != 0:
            sys.exit('copying SDL2 failed')


def web_release():
    """build/release/cyberworld-endless-web.zip: the site, to serve anywhere."""
    archive = shutil.make_archive(os.path.join(RELEASE, 'cyberworld-endless-web'), 'zip', site())
    print('released', archive)


def linux_release():
    """build/release/cyberworld-endless-linux-x86_64.tar.gz: the desktop build, SDL2 and the notes."""
    import tarfile
    src = os.path.join(ROOT, 'build', 'linux')
    stage = os.path.join(RELEASE, LINUX_NAME)
    shutil.rmtree(stage, ignore_errors=True)
    os.makedirs(os.path.join(stage, 'rom'))
    shutil.copy2(os.path.join(src, 'cyberworld'), os.path.join(stage, 'cyberworld-endless'))
    shutil.copytree(os.path.join(src, 'lib'), os.path.join(stage, 'lib'))
    shutil.copytree(os.path.join(src, 'licenses'), os.path.join(stage, 'licenses'))
    shutil.copy2(os.path.join(ROOT, 'LICENSE'), stage)
    shutil.copy2(os.path.join(ROOT, 'linux', 'README.md'), stage)
    shutil.copy2(os.path.join(ROOT, 'linux', 'install.sh'), stage)
    with open(os.path.join(stage, 'rom', 'PUT_YOUR_ROM_HERE.txt'), 'w') as f:
        f.write('Copy your own Mega Man Battle Network 6: Cybeast Gregar (USA) .gba file into this folder,\n'
                'or into ~/.local/share/cyberworld-endless/rom/.\n')
    archive = os.path.join(RELEASE, LINUX_NAME + '.tar.gz')
    with tarfile.open(archive, 'w:gz') as tar:
        tar.add(stage, arcname=LINUX_NAME)
    shutil.rmtree(stage)
    print('released', archive)


def site():
    """build/site: the browser build and its page, as GitHub Pages serves it."""
    out = os.path.join(ROOT, 'build', 'site')
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(os.path.join(out, 'licenses'))
    for name in ('index.html', 'app.js', 'style.css'):
        shutil.copy2(os.path.join(ROOT, 'web', name), out)
    for name in ('cyberworld.js', 'cyberworld.wasm'):
        shutil.copy2(os.path.join(ROOT, 'build', 'web', name), out)
    shutil.copy2(os.path.join(ROOT, 'LICENSE'), os.path.join(out, 'LICENSE.txt'))
    shutil.copy2(os.path.join(ROOT, 'build', 'web', 'licenses', 'mGBA.txt'), os.path.join(out, 'licenses'))
    open(os.path.join(out, '.nojekyll'), 'w').close()
    print('site in', out)
    return out


def serve(port=8080):
    """The site on http://localhost:PORT; the developer's ROM also at /.dev/rom.gba for tests."""
    import http.server
    import functools
    out = site()
    rom = next((os.path.join(default_rom_dir(), n) for n in sorted(os.listdir(default_rom_dir()))
                if n.lower().endswith('.gba')), None) if os.path.isdir(default_rom_dir()) else None

    class Handler(http.server.SimpleHTTPRequestHandler):
        def do_GET(self):
            if self.path == '/.dev/rom.gba' and rom:
                with open(rom, 'rb') as f:
                    data = f.read()
                self.send_response(200)
                self.send_header('Content-Type', 'application/octet-stream')
                self.send_header('Content-Length', str(len(data)))
                self.end_headers()
                self.wfile.write(data)
                return
            super().do_GET()

    server = http.server.ThreadingHTTPServer(('127.0.0.1', int(port)), functools.partial(Handler, directory=out))
    print(f'serving {out} on http://localhost:{port}/')
    server.serve_forever()


def port_release():
    """build/release/cyberworld.zip: the PortMaster port, as port.json names it."""
    package()
    archive = shutil.make_archive(os.path.join(RELEASE, 'cyberworld'), 'zip', os.path.join(ROOT, 'build', 'port'))
    print('released', archive)


def package():
    out = os.path.join(ROOT, 'build', 'port')
    game = os.path.join(out, 'cyberworld')
    if os.path.isdir(out):
        shutil.rmtree(out)
    os.makedirs(os.path.join(game, 'rom'))
    shutil.copy2(os.path.join(ROOT, 'build', 'aarch64', 'cyberworld.aarch64'), game)
    shutil.copy2(os.path.join(ROOT, 'port', 'README.md'), game)
    shutil.copy2(os.path.join(ROOT, 'port', 'gameinfo.xml'), game)
    shutil.copy2(os.path.join(ROOT, 'port', 'port.json'), game)
    shutil.copy2(os.path.join(ROOT, 'LICENSE'), game)
    shutil.copytree(os.path.join(ROOT, 'build', 'aarch64', 'licenses'), os.path.join(game, 'licenses'))
    with open(os.path.join(game, 'rom', 'PUT_YOUR_ROM_HERE.txt'), 'w') as f:
        f.write('Copy your own Mega Man Battle Network 6: Cybeast Gregar (USA) .gba file into this folder.\n')
    shutil.copy2(os.path.join(ROOT, 'port', 'Cyberworld Endless.sh'), out)
    print('packaged', out)


def atlas(biomes='all', seeds='1', baseline=False):
    """Every area's generated layers, drawn headless, one PNG sheet per area."""
    out = os.path.join(ROOT, '.build', 'atlas')
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    rom_dir = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
    code = docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/data',
                  '--atlas', f'/src/.build/atlas:{biomes}:{seeds}', mounts=[(rom_dir, '/rom:ro')])
    if code:
        return code
    from PIL import Image
    import collections
    import glob
    import re
    layers = collections.defaultdict(list)
    for path in sorted(glob.glob(os.path.join(out, 'b*.bmp'))):
        layers[int(re.match(r'b(\d+)_', os.path.basename(path)).group(1))].append(path)
    for biome, paths in layers.items():
        whole, crops = [], []
        for path in paths:
            im = Image.open(path).convert('RGB')
            box = im.getbbox() or (0, 0, im.width, im.height)
            im = im.crop(box)
            im.save(path[:-4] + '.png')   # kept whole, for zooming in
            thumb = im.copy()
            thumb.thumbnail((400, 400))
            whole.append(thumb)
            crops.append(densest(im, 200, 130).resize((400, 260), Image.NEAREST))
        sheet = Image.new('RGB', (400 * len(paths), 400 + 260), (20, 20, 24))
        for i, (t, c) in enumerate(zip(whole, crops)):
            sheet.paste(t, (i * 400, 0))
            sheet.paste(c, (i * 400, 400))
        sheet.save(os.path.join(out, f'sheet_b{biome:02d}.png'))
        for path in paths:
            os.remove(path)
    for path in glob.glob(os.path.join(out, 'seams_*.bmp')) + glob.glob(os.path.join(out, 'src_*.bmp')):   # seams marked; the originals
        im = Image.open(path).convert('RGB')
        im.crop(im.getbbox()).save(path[:-4] + '.png')
        os.remove(path)
    report = open(os.path.join(out, 'report.txt')).read()
    print(report, end='')
    flagged = [l for l in report.splitlines() if 'NOT BUILT' in l or 'arena NO' in l or
               float(re.search(r'fallback ([\d.]+)%', l).group(1)) > 1.0]
    print(f'{len(layers)} sheets in .build/atlas; {len(flagged)} layers flagged')
    for l in flagged:
        print('  !', l)
    return compare_baseline(report, write=baseline)


BASELINE = os.path.join(ROOT, 'tests', 'atlas_baseline.txt')
# how much worse a layer may get than the baseline before the atlas fails
TOLERANCE = {'near': 2.0, 'fallback': 0.15, 'seams': 1.10, 'inexact': 1.10}


def atlas_metrics(report):
    """Per layer (biome, layout, depth, seed): its near and fallback shares, seams and inexact panels."""
    import re
    out = {}
    for l in report.splitlines():
        m = re.match(r'biome +(\d+) layout (-?\d+) \(\w+\) depth (\d+) seed (\d+):', l)
        if not m or 'NOT BUILT' in l:
            continue
        get = lambda k: float(re.search(k + r' ([\d.]+)', l).group(1))
        out[m.groups()] = {'near': get('near'), 'fallback': get('fallback'), 'seams': get('seams'),
                           'inexact': get('not exact')}
    return out


def compare_baseline(report, write=False):
    """The atlas against tests/atlas_baseline.txt: nonzero if a layer drew worse."""
    now = atlas_metrics(report)
    if write:
        old = atlas_metrics(open(BASELINE).read()) if os.path.exists(BASELINE) else {}
        old.update(now)
        with open(BASELINE, 'w') as f:
            f.write('# build.py atlas --baseline: per layer (biome layout depth seed) its near and\n'
                    '# fallback shares (%), seams and panels not exact; the atlas fails when one gets worse\n')
            for k in sorted(old, key=lambda k: tuple(int(v) for v in k)):
                v = old[k]
                f.write(f'biome {k[0]} layout {k[1]} (x) depth {k[2]} seed {k[3]}: near {v["near"]}, '
                        f'fallback {v["fallback"]}, seams {v["seams"]:.0f}, not exact {v["inexact"]:.0f}\n')
        print(f'baseline: {len(now)} layers written to tests/atlas_baseline.txt')
        return 0
    if not os.path.exists(BASELINE):
        return 0
    base = atlas_metrics(open(BASELINE).read())
    worse = []
    for k, v in now.items():
        b = base.get(k)
        if not b:
            continue
        for name, tol in TOLERANCE.items():
            limit = b[name] + tol if name in ('near', 'fallback') else max(b[name] * tol, b[name] + 3)
            if v[name] > limit:
                worse.append(f'biome {k[0]} layout {k[1]} depth {k[2]} seed {k[3]}: {name} {b[name]:g} -> {v[name]:g}')
    compared = sum(1 for k in now if k in base)
    print(f'baseline: {compared} layers compared, {len(worse)} worse')
    for w in worse:
        print('  worse:', w)
    return 1 if worse else 0


def tour(biomes='all'):
    """The game warped through every room of each area's layer, one sheet per area."""
    out = os.path.join(ROOT, '.build', 'tour')
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    rom_dir = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
    code = docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/data',
                  '--tour', f'/src/.build/tour:{biomes}', '--seed', '5', '--frames', '200000', mounts=[(rom_dir, '/rom:ro')])
    if code:
        return code
    from PIL import Image
    import collections
    import glob
    import re
    rooms = collections.defaultdict(list)
    for path in sorted(glob.glob(os.path.join(out, 'tour_b*.bmp'))):
        rooms[int(re.match(r'tour_b(\d+)_', os.path.basename(path)).group(1))].append(path)
    for biome, paths in rooms.items():
        cols = 4
        sheet = Image.new('RGB', (cols * 480, ((len(paths) + cols - 1) // cols) * 320))
        for i, path in enumerate(paths):
            im = Image.open(path).convert('RGB')
            w, h = im.size   # the canvas: the game's 240 x 160 in the middle
            im = im.crop(((w - 240) // 2, (h - 160) // 2, (w + 240) // 2, (h + 160) // 2)).resize((480, 320), Image.NEAREST)
            sheet.paste(im, ((i % cols) * 480, (i // cols) * 320))
            os.remove(path)
        sheet.save(os.path.join(out, f'tour_b{biome:02d}.png'))
    print(f'{len(rooms)} sheets in .build/tour')
    return 0


def densest(im, w, h):
    """The w x h window with the most floor in it."""
    px = im.load()
    step = 10
    best = (0, 0, -1)
    for y0 in range(0, max(1, im.height - h), step):
        for x0 in range(0, max(1, im.width - w), step):
            n = sum(px[x, y] != (40, 40, 48) for y in range(y0, min(im.height, y0 + h), step) for x in range(x0, min(im.width, x0 + w), step))
            if n > best[2]:
                best = (x0, y0, n)
    return im.crop((best[0], best[1], best[0] + w, best[1] + h))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('action', nargs='?', default='all', choices=['all', 'host', 'device', 'linux', 'run', 'web', 'serve', 'release', 'package', 'shot', 'asan', 'test', 'clean', 'atlas', 'tour', 'pacing'])
    ap.add_argument('rest', nargs=argparse.REMAINDER)
    a = ap.parse_args()
    if a.action == 'clean':
        shutil.rmtree(os.path.join(ROOT, 'build'), ignore_errors=True)
        return
    if a.action == 'test':
        ensure_image()
        sys.exit(docker('make', 'test', *(['WERROR=1'] if os.environ.get('CI') else [])))
    if a.action == 'tour':
        build('host')
        sys.exit(tour(*a.rest[:1]))
    if a.action == 'linux':
        build('linux')
        os.makedirs(RELEASE, exist_ok=True)
        linux_release()
        return
    if a.action == 'run':
        # the desktop build on this machine, its saves apart from a player's
        build('linux')
        data = os.path.join(ROOT, '.build', 'desktop')
        os.makedirs(data, exist_ok=True)
        extra = [] if '--data-dir' in a.rest else ['--data-dir', data]
        if '--rom-dir' not in a.rest and os.path.isdir(default_rom_dir()):
            extra += ['--rom-dir', default_rom_dir()]
        sys.exit(subprocess.call([os.path.join(ROOT, 'build', 'linux', 'cyberworld'), *extra, *a.rest]))
    if a.action == 'web':
        build('web')
        site()
        return
    if a.action == 'serve':
        build('web')
        serve(*a.rest[:1])
        return
    if a.action == 'release':
        build('aarch64')
        build('linux')
        build('web')
        os.makedirs(RELEASE, exist_ok=True)
        port_release()
        linux_release()
        web_release()
        return
    if a.action == 'pacing':
        build('host')
        rom_dir = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
        os.makedirs(os.path.join(ROOT, '.build', 'data'), exist_ok=True)
        sys.exit(docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/data',
                        '--pacing', '/src/.build/pacing.txt', mounts=[(rom_dir, '/rom:ro')]))
    if a.action == 'atlas':
        build('host')
        rest = [r for r in a.rest if r != '--baseline']
        sys.exit(atlas(*rest[:2], baseline='--baseline' in a.rest))
    if a.action in ('all', 'host', 'shot'):
        build('host')
    if a.action == 'asan':
        build('asan')
    if a.action in ('all', 'device', 'package'):
        build('aarch64')
    if a.action == 'package':
        package()
    if a.action in ('shot', 'asan'):
        # Headless run inside the build image; the ROM directory is mounted read-only.
        rom_dir = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
        extra = [] if '--data-dir' in a.rest else ['--data-dir', '/src/.build/data']
        os.makedirs(os.path.join(ROOT, '.build', 'data'), exist_ok=True)
        binary = 'build/asan/cyberworld' if a.action == 'asan' else 'build/host/cyberworld'
        code = docker(binary, '--headless', '--rom-dir', '/rom', *extra, *a.rest,
                      mounts=[(rom_dir, '/rom:ro')])
        sys.exit(code)


if __name__ == '__main__':
    main()
