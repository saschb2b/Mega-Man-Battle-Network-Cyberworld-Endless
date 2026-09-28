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
                                release files in build/release: a tar.gz,
                                an AppImage and a .deb
  python3 build.py windows      the Windows build (build/windows, MinGW-w64 in
                                `cyberworld-windows`) and its release files in
                                build/release: a zip and an installer (NSIS)
  python3 build.py macos        the macOS app in a .dmg (build/release), on a Mac:
                                SDL2 and mGBA built by macos/deps.sh
  python3 build.py flatpak      the Flatpak (linux/flatpak) built by flatpak-builder
                                from this checkout and bundled in build/release
                                (needs flatpak; see linux/flatpak/README.md)
  python3 build.py run ...      build the Linux desktop binary and play it here
                                in a window (game options may follow)
  python3 build.py web          the browser build, assembled as a site in
                                build/site (published on GitHub Pages)
  python3 build.py serve [PORT] build the site and serve it on localhost
  python3 build.py release      the release archives in build/release: the
                                PortMaster port, the Linux desktop build, the
                                Windows build and the browser site
  python3 build.py shot ...     run the host binary headlessly (options below)
  python3 build.py tour [BIOMES] the game itself warped through every room of
                                each area's layer, one sheet per area in
                                .build/tour (docs/DEVTOOLS.md)
  python3 build.py atlas [BIOMES] [SEEDS] [--baseline]
                                every area's layers drawn, one sheet per area
                                in .build/atlas, compared with (or written to,
                                --baseline) tests/atlas_baseline.txt
                                (docs/DEVTOOLS.md)
  python3 build.py tiles [SEEDS] [--baseline]
                                the tile test: the atlas of every area and
                                layout (SEEDS each, 2 by default), a table of
                                each area's tiles drawn with other floors
                                than they show and seams, close-ups of them
                                in .build/atlas/defects_bNN.png; fails where a
                                layer got worse than tests/atlas_baseline.txt
                                (docs/DEVTOOLS.md)
  python3 build.py pacing       every act's battles and guardians against
                                their bands, in .build/pacing.txt
                                (docs/DEVTOOLS.md)
  python3 build.py screenshots  the game captured headlessly for the site and
                                the README, into docs/screenshots
  python3 build.py clips [NAMES] short videos of the game for the site
                                (WebM, MP4 and a poster), into docs/clips
  python3 build.py town [SEEDS] the run's town drawn for a few seeds, and
                                the game shown around it, in .build/town
                                (docs/OVERWORLD.md)
  python3 build.py world        the real world's original maps drawn with
                                their walls and triggers, and the game
                                warped through them, in .build/world
  python3 build.py test         ROM-free unit tests
  python3 build.py package      assemble build/port/ for PortMaster
"""
import argparse
import os
import re
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))
IMAGE = 'cyberworld-build'
LINUX_IMAGE = 'cyberworld-linux'   # docker/Dockerfile.linux
WEB_IMAGE = 'cyberworld-web'       # docker/Dockerfile.web
WINDOWS_IMAGE = 'cyberworld-windows'   # docker/Dockerfile.windows
IMAGES = {IMAGE: 'Dockerfile', LINUX_IMAGE: 'Dockerfile.linux', WEB_IMAGE: 'Dockerfile.web',
          WINDOWS_IMAGE: 'Dockerfile.windows'}
CONTEXT = os.environ.get('DOCKER_CONTEXT_NAME', 'desktop-linux')
RELEASE = os.path.join(ROOT, 'build', 'release')
LINUX_NAME = 'cyberworld-endless-linux-x86_64'
APP_ID = 'io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless'   # src/core/platform.h, linux/
APPIMAGE_NAME = 'cyberworld-endless-x86_64.AppImage'
DEB_NAME = 'cyberworld-endless_amd64.deb'
REPO = 'saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless'
ICON_SIZES = (32, 64, 128, 256, 512)                # linux/icons, drawn by tools/app_icon.py


def default_rom_dir():
    return os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))


def docker(*cmd, mounts=(), image=IMAGE):
    args = ['docker']
    if CONTEXT:
        args += ['--context', CONTEXT]
    args += ['run', '--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src']
    for host, guest in mounts:
        args += ['-v', f'{host}:{guest}']
    for var in ('CYBERWORLD_AUDIO_DUMP', 'CYBERWORLD_SFX_LOG', 'CYBERWORLD_AUDIO_OFFLINE', 'CYBERWORLD_EMU_DEBUG', 'CYBERWORLD_TILE_AT', 'CYBERWORLD_AUTOPILOT', 'CYBERWORLD_TOWN_DEBUG', 'CYBERWORLD_TOWN_STYLE', 'CYBERWORLD_TOWN_VARIANT', 'CYBERWORLD_TOWN_TILE', 'CYBERWORLD_TOWN_START'):
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
    image = {'linux': LINUX_IMAGE, 'web': WEB_IMAGE, 'windows': WINDOWS_IMAGE}.get(target, IMAGE)
    ensure_image(image)
    werror = ['WERROR=1'] if os.environ.get('CI') else []
    extra = [f'FILE_VERSION={file_version()}'] if target == 'windows' else []
    if docker('make', f'TARGET={target}', f'VERSION={version()}', f'-j{os.cpu_count() or 4}', *werror, *extra, image=image) != 0:
        sys.exit(f'{target} build failed')
    if target == 'windows':
        # (SDL2 is linked in: its license travels with the .exe)
        if docker('sh', '-c', 'mkdir -p build/windows/licenses && cp /opt/sdl2/LICENSE.txt build/windows/licenses/SDL2.txt',
                  image=image) != 0:
            sys.exit('copying the SDL2 license failed')
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
    shutil.copytree(os.path.join(ROOT, 'linux', 'icons'), os.path.join(stage, 'icons'))
    copy_steam(os.path.join(stage, 'steam'))
    archive = os.path.join(RELEASE, LINUX_NAME + '.tar.gz')
    with tarfile.open(archive, 'w:gz') as tar:
        tar.add(stage, arcname=LINUX_NAME)
    shutil.rmtree(stage)
    print('released', archive)
    appimage()
    deb()


def file_version():
    """The version as Windows keeps it, four numbers: 0.1.0+12 is 0,1,0,12."""
    m = re.match(r'(\d+)\.(\d+)\.(\d+)(?:\+(\d+))?', version())
    return ','.join(m.groups('0')) if m else '0,0,0,0'


MACOS_DMG = 'cyberworld-endless-macos.dmg'


def macos_release():
    """build/release/cyberworld-endless-macos.dmg: the game as one app for Apple
    silicon and Intel Macs (macOS 11 on), signed ad hoc, not notarized. Runs on
    a Mac only, with Xcode's command line tools and CMake: the SDK is Apple's
    (CI's macOS job builds it)."""
    if sys.platform != 'darwin':
        sys.exit('the macOS build runs on a Mac (CI: the macos job)')
    deps = os.path.join(ROOT, '.build', 'macos-deps')
    if not os.path.exists(os.path.join(deps, 'lib', 'libmgba.a')):
        subprocess.check_call(['sh', os.path.join(ROOT, 'macos', 'deps.sh'), deps])
    werror = ['WERROR=1'] if os.environ.get('CI') else []
    if subprocess.call(['make', 'TARGET=macos', f'MACOS_DEPS={deps}', f'VERSION={version()}',
                        f'-j{os.cpu_count() or 4}', *werror], cwd=ROOT) != 0:
        sys.exit('macos build failed')
    app = os.path.join(ROOT, 'build', 'macos', 'Cyberworld Endless.app')
    shutil.rmtree(app, ignore_errors=True)
    contents = os.path.join(app, 'Contents')
    os.makedirs(os.path.join(contents, 'MacOS'))
    os.makedirs(os.path.join(contents, 'Resources', 'licenses'))
    shutil.copy2(os.path.join(ROOT, 'build', 'macos', 'cyberworld-endless'), os.path.join(contents, 'MacOS'))
    shutil.copy2(os.path.join(ROOT, 'macos', 'icon.icns'), os.path.join(contents, 'Resources'))
    short = '.'.join(file_version().split(',')[:3])
    with open(os.path.join(ROOT, 'macos', 'Info.plist')) as f:
        plist = f.read().replace('@SHORT_VERSION@', short).replace('@VERSION@', version())
    with open(os.path.join(contents, 'Info.plist'), 'w') as f:
        f.write(plist)
    shutil.copy2(os.path.join(ROOT, 'LICENSE'), os.path.join(contents, 'Resources', 'licenses', 'LICENSE.txt'))
    for name in ('mGBA.txt', 'SDL2.txt'):
        shutil.copy2(os.path.join(deps, 'share', 'licenses', name), os.path.join(contents, 'Resources', 'licenses'))
    # (an ad hoc signature: Apple silicon runs nothing unsigned)
    if subprocess.call(['codesign', '--force', '--deep', '--sign', '-', app]) != 0:
        sys.exit('codesign failed')
    stage = os.path.join(ROOT, 'build', 'macos-dmg')
    shutil.rmtree(stage, ignore_errors=True)
    os.makedirs(stage)
    subprocess.check_call(['cp', '-R', app, stage])
    os.symlink('/Applications', os.path.join(stage, 'Applications'))
    shutil.copy2(os.path.join(ROOT, 'macos', 'README.txt'), os.path.join(stage, 'Read me.txt'))
    os.makedirs(RELEASE, exist_ok=True)
    out = os.path.join(RELEASE, MACOS_DMG)
    if os.path.exists(out):
        os.remove(out)
    if subprocess.call(['hdiutil', 'create', '-volname', 'Cyberworld Endless', '-srcfolder', stage, '-ov',
                        '-format', 'UDZO', out]) != 0:
        sys.exit('hdiutil failed')
    print('released', out)


WINDOWS_ZIP = 'cyberworld-endless-windows-x64.zip'
WINDOWS_SETUP = 'cyberworld-endless-setup-x64.exe'


def windows_release():
    """build/release: cyberworld-endless-windows-x64.zip (the game in a folder,
    to unpack anywhere) and cyberworld-endless-setup-x64.exe (an installer
    for the user who runs it, no administrator needed; windows/installer.nsi)."""
    import zipfile
    build('windows')
    stage = os.path.join(ROOT, 'build', 'windows-stage')
    shutil.rmtree(stage, ignore_errors=True)
    os.makedirs(os.path.join(stage, 'licenses'))
    rel = os.path.relpath(stage, ROOT)
    if docker('x86_64-w64-mingw32-strip', '-o', f'{rel}/cyberworld-endless.exe', 'build/windows/cyberworld-endless.exe',
              image=WINDOWS_IMAGE) != 0:
        sys.exit('strip failed')
    shutil.copy2(os.path.join(ROOT, 'windows', 'README.txt'), stage)
    shutil.copy2(os.path.join(ROOT, 'LICENSE'), os.path.join(stage, 'LICENSE.txt'))
    for name in ('mGBA.txt', 'SDL2.txt'):
        shutil.copy2(os.path.join(ROOT, 'build', 'windows', 'licenses', name), os.path.join(stage, 'licenses'))
    # Windows' own line ends in the text files
    for path in (os.path.join(stage, 'README.txt'), os.path.join(stage, 'LICENSE.txt'),
                 *[os.path.join(stage, 'licenses', n) for n in os.listdir(os.path.join(stage, 'licenses'))]):
        with open(path, 'rb') as f:
            data = f.read().replace(b'\r\n', b'\n').replace(b'\n', b'\r\n')
        with open(path, 'wb') as f:
            f.write(data)
    os.makedirs(RELEASE, exist_ok=True)
    archive = os.path.join(RELEASE, WINDOWS_ZIP)
    with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED) as z:
        for d, _, files in os.walk(stage):
            for name in sorted(files):
                path = os.path.join(d, name)
                z.write(path, os.path.join('Cyberworld Endless', os.path.relpath(path, stage)))
    print('released', archive)
    if docker('makensis', '-V2', f'-DVERSION={version()}', f'-DFILEVERSION={file_version().replace(",", ".")}',
              f'-DSTAGE=/src/{rel}', f'-DOUT=/src/build/release/{WINDOWS_SETUP}', 'windows/installer.nsi',
              image=WINDOWS_IMAGE) != 0:
        sys.exit('makensis failed')
    print('released', os.path.join(RELEASE, WINDOWS_SETUP))


def version():
    """The release's version: the tag being released (TAG, as the release
    workflow sets it) or the latest v* tag with the commits since, else a
    development build's 0.0.1 with the commit count and the commit (the count
    orders them, as a package manager compares them; 0.0.1 comes after the
    0.0.0+git.HASH builds had, which did not)."""
    def git(*args):
        try:
            return subprocess.check_output(['git', *args], cwd=ROOT, stderr=subprocess.DEVNULL, text=True).strip()
        except (OSError, subprocess.CalledProcessError):
            return ''
    tag = os.environ.get('TAG', '')
    if not tag.startswith('v'):
        tag = git('describe', '--tags', '--match', 'v*')
    if tag.startswith('v'):
        m = re.match(r'(.+?)-(\d+)-(g[0-9a-f]+)$', tag[1:])   # v0.1.0-3-gabc123 -> 0.1.0+3.gabc123
        return f'{m[1]}+{m[2]}.{m[3]}' if m else tag[1:]
    return f'0.0.1+git{git("rev-list", "--count", "HEAD") or "0"}.{git("rev-parse", "--short", "HEAD") or "unknown"}'


STEAM_ART = ('capsule.png', 'wide.png', 'hero.png', 'logo.png')


def copy_steam(dst):
    """linux/steam: add-to-steam.py and the library artwork (tools/steam_art.py),
    with the 256 px icon as icon.png, into dst."""
    os.makedirs(dst, exist_ok=True)
    src = os.path.join(ROOT, 'linux', 'steam')
    shutil.copy2(os.path.join(src, 'add-to-steam.py'), dst)
    os.chmod(os.path.join(dst, 'add-to-steam.py'), 0o755)
    for name in STEAM_ART:
        shutil.copy2(os.path.join(src, name), dst)
    shutil.copy2(os.path.join(ROOT, 'linux', 'icons', '256.png'), os.path.join(dst, 'icon.png'))


def install_tree(root, doc_name):
    """The installed layout under root/usr: the program and SDL2 in
    lib/cyberworld-endless, the menu entry, icons, AppStream data and the
    licenses."""
    src = os.path.join(ROOT, 'build', 'linux')
    prog = os.path.join(root, 'usr', 'lib', 'cyberworld-endless')
    os.makedirs(prog)
    shutil.copy2(os.path.join(src, 'cyberworld'), os.path.join(prog, 'cyberworld-endless'))
    shutil.copytree(os.path.join(src, 'lib'), os.path.join(prog, 'lib'))
    rel = os.path.relpath(prog, ROOT)
    if docker('strip', '--strip-unneeded', f'{rel}/cyberworld-endless', f'{rel}/lib/libSDL2-2.0.so.0', image=LINUX_IMAGE) != 0:
        sys.exit('strip failed')
    share = os.path.join(root, 'usr', 'share')
    os.makedirs(os.path.join(share, 'applications'))
    shutil.copy2(os.path.join(ROOT, 'linux', APP_ID + '.desktop'), os.path.join(share, 'applications'))
    for size in ICON_SIZES:
        icons = os.path.join(share, 'icons', 'hicolor', f'{size}x{size}', 'apps')
        os.makedirs(icons)
        shutil.copy2(os.path.join(ROOT, 'linux', 'icons', f'{size}.png'), os.path.join(icons, APP_ID + '.png'))
    os.makedirs(os.path.join(share, 'metainfo'))
    date = subprocess.check_output(['git', 'log', '-1', '--format=%cs'], cwd=ROOT, text=True).strip()
    with open(os.path.join(ROOT, 'linux', APP_ID + '.metainfo.xml')) as f:
        meta = f.read().replace('<!-- release -->', f'<release version="{version()}" date="{date}"/>')
    with open(os.path.join(share, 'metainfo', APP_ID + '.metainfo.xml'), 'w') as f:
        f.write(meta)
    copy_steam(os.path.join(share, 'cyberworld-endless', 'steam'))
    doc = os.path.join(share, 'doc', doc_name)
    shutil.copytree(os.path.join(src, 'licenses'), doc)
    shutil.copy2(os.path.join(ROOT, 'LICENSE'), os.path.join(doc, 'LICENSE'))
    return prog


def appimage():
    """build/release/cyberworld-endless-x86_64.AppImage and its .zsync: one
    file that runs on any x86-64 distribution with glibc 2.34+, and offers to
    add itself to the application menu (src/core/desktop.c). The update
    information lets AppImageUpdate or Gear Lever fetch the next release."""
    appdir = os.path.join(ROOT, 'build', 'appdir')
    shutil.rmtree(appdir, ignore_errors=True)
    install_tree(appdir, 'cyberworld-endless')
    shutil.copy2(os.path.join(ROOT, 'linux', APP_ID + '.desktop'), appdir)
    shutil.copy2(os.path.join(ROOT, 'linux', 'icons', '256.png'), os.path.join(appdir, APP_ID + '.png'))
    os.symlink(APP_ID + '.png', os.path.join(appdir, '.DirIcon'))
    with open(os.path.join(appdir, 'AppRun'), 'w') as f:
        f.write('#!/bin/sh\n'
                'here=$(dirname "$(readlink -f "$0")")\n'
                'exec "$here/usr/lib/cyberworld-endless/cyberworld-endless" "$@"\n')
    os.chmod(os.path.join(appdir, 'AppRun'), 0o755)
    out = os.path.join(RELEASE, APPIMAGE_NAME)
    for old in (out, out + '.zsync'):
        if os.path.exists(old):
            os.remove(old)
    update = f'gh-releases-zsync|{REPO.replace("/", "|")}|latest|{APPIMAGE_NAME}.zsync'
    if docker('sh', '-c', 'cd build/release && HOME=/tmp ARCH=x86_64 /opt/appimage/appimagetool/AppRun '
              f'--runtime-file /opt/appimage/runtime-x86_64 -u "{update}" ../appdir {APPIMAGE_NAME}',
              image=LINUX_IMAGE) != 0:
        sys.exit('appimagetool failed')
    print('released', out)


def deb():
    """build/release/cyberworld-endless_amd64.deb: installs into /usr like
    any package (the program in /usr/lib/cyberworld-endless, the command in
    /usr/bin), for Debian, Ubuntu, Mint and Pop!_OS."""
    root = os.path.join(ROOT, 'build', 'deb')
    shutil.rmtree(root, ignore_errors=True)
    install_tree(root, 'cyberworld-endless')
    os.makedirs(os.path.join(root, 'usr', 'bin'))
    os.symlink('../lib/cyberworld-endless/cyberworld-endless', os.path.join(root, 'usr', 'bin', 'cyberworld-endless'))
    with open(os.path.join(root, 'usr', 'share', 'doc', 'cyberworld-endless', 'copyright'), 'w') as f:
        f.write('Format: https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/\n'
                'Upstream-Name: Cyberworld Endless\n'
                f'Source: https://github.com/{REPO}\n\n'
                'Files: *\nCopyright: Sascha Becker\nLicense: MIT\n See LICENSE.\n\n'
                'Files: usr/lib/cyberworld-endless/cyberworld-endless\nComment: embeds mGBA\nLicense: MPL-2.0\n See mGBA.txt.\n\n'
                'Files: usr/lib/cyberworld-endless/lib/*\nCopyright: Sam Lantinga and SDL contributors\nLicense: Zlib\n See SDL2.txt.\n')
    size = 0
    os.chmod(root, 0o755)
    for d, dirs, files in os.walk(root):
        for name in dirs:
            os.chmod(os.path.join(d, name), 0o755)
        for name in files:
            path = os.path.join(d, name)
            if not os.path.islink(path):
                os.chmod(path, 0o755 if name.startswith('cyberworld-endless') or name.endswith(('.so.0', '.py')) else 0o644)
                size += os.path.getsize(path)
    os.makedirs(os.path.join(root, 'DEBIAN'))
    with open(os.path.join(root, 'DEBIAN', 'control'), 'w') as f:
        f.write('Package: cyberworld-endless\n'
                f'Version: {version()}\n'
                'Architecture: amd64\n'
                'Maintainer: Sascha Becker <saschb2b@gmail.com>\n'
                f'Installed-Size: {(size + 1023) // 1024}\n'
                'Depends: libc6 (>= 2.34)\n'
                'Recommends: zenity | kdialog, xdg-utils\n'
                'Section: games\n'
                'Priority: optional\n'
                'Homepage: https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/\n'
                'Description: roguelike on Mega Man Battle Network 6\n'
                ' Every run builds a new net in the style of Mega Man Battle Network 6\n'
                ' and the game runs it: its battles, chips, Navis and music. It needs the\n'
                ' player\'s own Mega Man Battle Network 6: Cybeast Gregar (USA) ROM; no game\n'
                ' data is included. Unofficial, not affiliated with Capcom.\n')
    os.chmod(os.path.join(root, 'DEBIAN', 'control'), 0o644)
    out = os.path.join(RELEASE, DEB_NAME)
    if docker('dpkg-deb', '--root-owner-group', '-Zxz', '--build', 'build/deb', f'build/release/{DEB_NAME}',
              image=LINUX_IMAGE) != 0:
        sys.exit('dpkg-deb failed')
    print('released', out)


def site():
    """build/site: the project's pages (web/) with the browser build in play/, as GitHub Pages serves it."""
    out = os.path.join(ROOT, 'build', 'site')
    shutil.rmtree(out, ignore_errors=True)
    shutil.copytree(os.path.join(ROOT, 'web'), out)
    shutil.copytree(os.path.join(ROOT, 'docs', 'screenshots'), os.path.join(out, 'shots'))
    if os.path.isdir(os.path.join(ROOT, 'docs', 'clips')):
        shutil.copytree(os.path.join(ROOT, 'docs', 'clips'), os.path.join(out, 'clips'))
    for name in ('cyberworld.js', 'cyberworld.wasm'):
        shutil.copy2(os.path.join(ROOT, 'build', 'web', name), os.path.join(out, 'play'))
    shutil.copy2(os.path.join(ROOT, 'LICENSE'), os.path.join(out, 'LICENSE.txt'))
    os.makedirs(os.path.join(out, 'licenses'))
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
            with open(path[:-4] + '.box', 'w') as f:   # where the crop lies (the flagged tiles are listed uncropped)
                f.write(f'{box[0]} {box[1]}\n')
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
        name = os.path.basename(path)
        box = os.path.join(out, name[6:-4] + '.box') if name.startswith('seams_') else None
        if box and os.path.exists(box):   # (cut as the layer's picture is)
            x0, y0 = map(int, open(box).read().split())
            plain = Image.open(os.path.join(out, name[6:-4] + '.png'))
            im.crop((x0, y0, x0 + plain.width, y0 + plain.height)).save(path[:-4] + '.png')
        else:
            im.crop(im.getbbox()).save(path[:-4] + '.png')
        os.remove(path)
    if not os.path.exists(os.path.join(out, 'report.txt')):   # a group's maps alone (gNN)
        print('maps in .build/atlas')
        return 0
    report = open(os.path.join(out, 'report.txt')).read()
    print(report, end='')
    found = defect_sheets(out)
    if found:
        print('spots drawn with other floors than they show, per area (defects_bNN.png):',
              ', '.join(f'{b}: {n}' for b, n in sorted(found.items())))
        for b in sorted(WHY):
            print(f'  area {b:2d} tiles off by why:', ', '.join(f'{WHY_NAMES[k]} {v}' for k, v in enumerate(WHY[b]) if v))
    flagged = [l for l in report.splitlines() if 'NOT BUILT' in l or 'arena NO' in l or
               float(re.search(r'fallback ([\d.]+)%', l).group(1)) > 1.0]
    # a service or navi beside a panel-wide stretch of the floor as drawn: in the way on
    mouths = [l for l in report.splitlines() if re.search(r'at mouths [1-9]', l)]
    print(f'{len(layers)} sheets in .build/atlas; {len(flagged)} layers flagged, {len(mouths)} with a service at a walkway\'s mouth')
    for l in flagged + mouths:
        print('  !', l)
    return compare_baseline(report, write=baseline) or (1 if mouths else 0)


BASELINE = os.path.join(ROOT, 'tests', 'atlas_baseline.txt')
# how much worse a layer may get than the baseline before the atlas fails
TOLERANCE = {'near': 2.0, 'fallback': 0.15, 'seams': 1.10, 'inexact': 1.10, 'other': 1.10, 'off': 1.10}
AREAS = ['Central', 'Seaside', 'Sky', 'Green', 'Graveyard', 'Undernet', 'Secret', 'Nest', 'Comp', 'Homepage', 'Comp B',
         'Robot Comp', 'Aquarium Comp', 'Judge Comp', 'Weather Comp', 'CopyBot Comp', 'ACDC HP', 'Green HP', 'Sky HP']   # src/core/run.h


def tiles(seeds='2', baseline=False):
    """The tile test: every area's layers drawn (the atlas), and per area how many tiles were
    drawn with other floors than they show and how many seams show; nonzero where a layer got
    worse than the baseline."""
    import collections
    code = atlas('all', seeds, baseline=baseline)
    report = os.path.join(ROOT, '.build', 'atlas', 'report.txt')
    if not os.path.exists(report):
        return code or 1
    agg = collections.defaultdict(lambda: [0, 0, 0, 0])
    for l in open(report):
        m = re.match(r'biome +(\d+) .*?: (\d+) panels.*seams (\d+), off near (\d+)', l)
        if m:
            a = agg[int(m[1])]
            a[0] += 1; a[1] += int(m[2]); a[2] += int(m[3]); a[3] += int(m[4])
    print()
    print(f'{"area":16} {"layers":>6} {"panels":>7} {"off /100":>9} {"seams /100":>11}   why off')
    total = [0, 0, 0]
    for b, (n, panels, seams, off) in sorted(agg.items()):
        why = ', '.join(f'{WHY_NAMES[k]} {v}' for k, v in enumerate((WHY or {}).get(b, [])) if v) if WHY else ''
        print(f'{AREAS[b] if b < len(AREAS) else b:16} {n:6} {panels:7} {100 * off / panels:9.1f} {100 * seams / panels:11.1f}   {why}')
        total[0] += panels; total[1] += off; total[2] += seams
    print(f'{"all":16} {"":6} {total[0]:7} {100 * total[1] / total[0]:9.1f} {100 * total[2] / total[0]:11.1f}')
    print('off: tiles drawn with other floors than they show (framed magenta, close-ups in .build/atlas/defects_bNN.png);'
          ' seams: tiles meeting as no original map sets them (red)')
    return code


def bmp_sheet(paths, out, cols=3):
    """The game's frames (240 x 160 in the middle of each capture) in a sheet."""
    from PIL import Image
    if not paths:
        return
    rows = (len(paths) + cols - 1) // cols
    sheet = Image.new('RGB', (cols * 480, rows * 320))
    for i, path in enumerate(paths):
        im = Image.open(path).convert('RGB')
        w, h = im.size
        im = im.crop(((w - 240) // 2, (h - 160) // 2, (w + 240) // 2, (h + 160) // 2)).resize((480, 320), Image.NEAREST)
        sheet.paste(im, ((i % cols) * 480, (i // cols) * 320))
        os.remove(path)
    sheet.save(out)


def town(seeds='4'):
    """The towns runs of seeds 1..SEEDS start in, drawn; then the game around the first one."""
    import glob
    from PIL import Image
    out = os.path.join(ROOT, '.build', 'town')
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    rom_dir = default_rom_dir()
    code = docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/data',
                  '--atlas', f'/src/.build/town:town:{seeds}', mounts=[(rom_dir, '/rom:ro')])
    if code:
        return code
    for path in sorted(glob.glob(os.path.join(out, 'town_s*.bmp'))):
        im = Image.open(path).convert('RGB')
        im.crop(im.getbbox()).save(path[:-4] + '.png')
        os.remove(path)
    code = docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/data',
                  '--tour', '/src/.build/town:town', '--seed', '1', '--frames', '3000', mounts=[(rom_dir, '/rom:ro')])
    bmp_sheet(sorted(glob.glob(os.path.join(out, 'town_[0-9].bmp'))), os.path.join(out, 'tour.png'))
    print('the town in .build/town: town_sNN.png per seed (red: tiles without a match), tour.png in the game')
    return code


def world():
    """The real world's original maps, drawn and toured."""
    import collections
    import glob
    import re
    from PIL import Image
    out = os.path.join(ROOT, '.build', 'world')
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    rom_dir = default_rom_dir()
    code = docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/data',
                  '--atlas', '/src/.build/world:world', mounts=[(rom_dir, '/rom:ro')])
    if code:
        return code
    for path in glob.glob(os.path.join(out, 'world_*.bmp')):
        im = Image.open(path).convert('RGB')
        im.save(path[:-4] + '.png')
        os.remove(path)
    code = docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/data',
                  '--tour', '/src/.build/world:world', '--seed', '5', '--frames', '60000', mounts=[(rom_dir, '/rom:ro')])
    maps = collections.defaultdict(list)
    for path in sorted(glob.glob(os.path.join(out, 'world_*_*_*.bmp'))):
        m = re.match(r'world_(\w\w)_(\d+)_\d+\.bmp', os.path.basename(path))
        maps[(m[1], int(m[2]))].append(path)
    for (g, n), paths in maps.items():
        bmp_sheet(paths, os.path.join(out, f'tour_{g}_{n}.png'), cols=6)
    print('the real world in .build/world: world_GG_N.png drawn, _coords marked, tour_GG_N.png in the game')
    return code


def atlas_metrics(report):
    """Per layer (biome, layout, depth, seed): its near and fallback shares, seams, inexact panels and
    tiles in colours the area's own map never shows on its floors (0 where a report has none)."""
    import re
    out = {}
    for l in report.splitlines():
        m = re.match(r'biome +(\d+) layout (-?\d+) \(\w+\) depth (\d+) seed (\d+):', l)
        if not m or 'NOT BUILT' in l:
            continue
        def get(k, l=l):
            f = re.search(k + r' ([\d.]+)', l)
            return float(f.group(1)) if f else 0.0
        out[m.groups()] = {'near': get('near'), 'fallback': get('fallback'), 'seams': get('seams'),
                           'inexact': get('not exact'), 'other': get('other colours'), 'off': get('off near')}
    return out


def compare_baseline(report, write=False):
    """The atlas against tests/atlas_baseline.txt: nonzero if a layer drew worse."""
    now = atlas_metrics(report)
    if write:
        old = atlas_metrics(open(BASELINE).read()) if os.path.exists(BASELINE) else {}
        old.update(now)
        with open(BASELINE, 'w') as f:
            f.write('# build.py atlas --baseline: per layer (biome layout depth seed) its near and\n'
                    '# fallback shares (%), seams, panels not exact, tiles in colours the area\'s own map never\n'
                    '# shows on its floors and tiles drawn with other floors than they show; the atlas fails\n'
                    '# when one gets worse\n')
            for k in sorted(old, key=lambda k: tuple(int(v) for v in k)):
                v = old[k]
                f.write(f'biome {k[0]} layout {k[1]} (x) depth {k[2]} seed {k[3]}: near {v["near"]}, '
                        f'fallback {v["fallback"]}, seams {v["seams"]:.0f}, not exact {v["inexact"]:.0f}, '
                        f'other colours {v["other"]:.0f}, off near {v.get("off", 0):.0f}\n')
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


# Screenshots of the running game for the site and the README
# (docs/screenshots): name, game options, (frame, shot name) pairs, env.
# Screenshots are fine to publish; files extracted from the ROM are not.
SCREENSHOTS = [
    ('title', ['--scene', 'title'], [(80, 'title')], {}),
    ('run', ['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '11', '--guardian', '12'],
     [(240, 'net'), (1420, 'custom'), (1660, 'battle'), (1970, 'result'), (2220, 'guardian'),
      (2380, 'guardian-talk'), (2610, 'boss-custom'), (3670, 'reward'), (3720, 'restored'), (3880, 'area-clear')],
     {'CYBERWORLD_AUTOPILOT': 'weak'}),
    ('act', ['--scene', 'emu', '--seed', '11', '--net-biome', '8', '--guardian', '12', '--dev', 'quiet'], [(120, 'act-card')], {}),
    ('town-central', ['--scene', 'town', '--seed', '3'], [(280, 'town-central')], {}),
    ('town-acdc', ['--scene', 'town', '--seed', '5'], [(280, 'town-acdc')], {}),
    ('central', ['--scene', 'emu', '--net-biome', '0', '--run-depth', '2', '--seed', '3', '--dev', 'quiet'], [(420, 'central')], {}),
    ('seaside', ['--scene', 'emu', '--net-biome', '1', '--run-depth', '2', '--seed', '3', '--dev', 'quiet'], [(420, 'seaside')], {}),
    ('green', ['--scene', 'emu', '--net-biome', '3', '--run-depth', '2', '--seed', '3', '--dev', 'quiet'], [(420, 'green')], {}),
    ('undernet', ['--scene', 'emu', '--net-biome', '5', '--run-depth', '14', '--seed', '3', '--dev', 'quiet'], [(130, 'undernet')], {}),
    ('graveyard', ['--scene', 'emu', '--net-biome', '4', '--run-depth', '17', '--seed', '3', '--dev', 'quiet'], [(130, 'graveyard')], {}),
    ('nest', ['--scene', 'emu', '--net-biome', '7', '--run-depth', '19', '--seed', '3', '--dev', 'quiet'], [(420, 'nest')], {}),
]


def screenshots(only=None):
    """docs/screenshots/NAME.png: the 240x160 picture of chosen frames."""
    from PIL import Image
    out = os.path.join(ROOT, 'docs', 'screenshots')
    tmp = os.path.join(ROOT, '.build', 'screenshots')
    os.makedirs(out, exist_ok=True)
    for name, args, frames, env in SCREENSHOTS:
        if only and name not in only:
            continue
        shutil.rmtree(tmp, ignore_errors=True)
        os.makedirs(os.path.join(tmp, 'data'))
        shots = ','.join(f'{f}:/src/.build/screenshots/{n}.bmp' for f, n in frames)
        saved = {k: os.environ.get(k) for k in env}
        os.environ.update(env)
        code = docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/screenshots/data',
                      *args, '--frames', str(max(f for f, _ in frames) + 1), '--shot', shots,
                      mounts=[(default_rom_dir(), '/rom:ro')])
        for k, v in saved.items():
            if v is None:
                os.environ.pop(k, None)
            else:
                os.environ[k] = v
        if code:
            return code
        for _, n in frames:
            im = Image.open(os.path.join(tmp, f'{n}.bmp')).convert('RGB')
            w, h = im.size   # the canvas: the game's 240 x 160 in the middle
            im.crop(((w - 240) // 2, (h - 160) // 2, (w + 240) // 2, (h + 160) // 2)).save(
                os.path.join(out, f'{n}.png'), optimize=True)
            print('screenshot', n)
    return 0


# Short videos of the game for the site (docs/clips): name, game options,
# env, scripted input, first and last frame. Every second frame, at 30 fps.
FFMPEG_IMAGE = 'linuxserver/ffmpeg:9.0-cli-ls82'
# (seed 7's layer 3 in the Robot Control Comp against BlastMan, as the
# captures were timed: the areas' and guardians' draw has changed since)
RUN_7 = ['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '11', '--guardian', '12']
CLIPS = [
    ('title', ['--scene', 'title'], {}, None, 300, 780),
    ('net', RUN_7, {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 150, 600),
    ('battle', RUN_7, {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 1400, 1860),
    ('guardian', RUN_7, {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 2110, 2380),
    ('undernet', ['--scene', 'emu', '--net-biome', '5', '--run-depth', '14', '--seed', '3', '--dev', 'quiet'], {},
     '150:,70:RIGHT,50:UP,70:LEFT,50:DOWN,100:', 150, 480),
    ('jackin', ['--scene', 'title'], {}, '120:,4:START,60:,4:A,200:', 110, 222),
]


# the clips the README shows as GIFs too (GitHub plays no video from the repository)
README_GIFS = ('guardian',)


def clips(only=None):
    """docs/clips/NAME.webm, .mp4 and .png: frames of scripted runs, 4x, 30 fps (and
    NAME.gif, 2x, 15 fps, for those in README_GIFS)."""
    from PIL import Image
    out = os.path.join(ROOT, 'docs', 'clips')
    tmp = os.path.join(ROOT, '.build', 'clips')
    os.makedirs(out, exist_ok=True)
    for name, args, env, script, first, last in CLIPS:
        if only and name not in only:
            continue
        shutil.rmtree(tmp, ignore_errors=True)
        os.makedirs(os.path.join(tmp, 'data'))
        os.makedirs(os.path.join(tmp, 'png'))
        saved = {k: os.environ.get(k) for k in env}
        os.environ.update(env)
        extra = ['--input', script] if script else []
        code = docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/clips/data',
                      *args, *extra, '--frames', str(last + 1), '--shot-range', f'{first}:{last}:/src/.build/clips/f',
                      mounts=[(default_rom_dir(), '/rom:ro')])
        for k, v in saved.items():
            if v is None:
                os.environ.pop(k, None)
            else:
                os.environ[k] = v
        if code:
            return code
        n = 0
        for f in range(first, last + 1, 2):
            im = Image.open(os.path.join(tmp, f'f{f:05d}.bmp')).convert('RGB')
            w, h = im.size   # the canvas: the game's 240 x 160 in the middle
            im = im.crop(((w - 240) // 2, (h - 160) // 2, (w + 240) // 2, (h + 160) // 2))
            if n == 0:
                im.save(os.path.join(out, f'{name}.png'), optimize=True)
            im.save(os.path.join(tmp, 'png', f'{n:05d}.png'))
            n += 1
        # 4x with whole pixels (and the colour planes' 2x2 blocks inside them)
        common = ['-y', '-loglevel', 'error', '-framerate', '30', '-i', '/work/png/%05d.png',
                  '-vf', 'scale=960:640:flags=neighbor', '-an']
        encodes = [common + ['-c:v', 'libvpx-vp9', '-b:v', '0', '-crf', '36', '-row-mt', '1', '-pix_fmt', 'yuv420p', f'/out/{name}.webm'],
                   common + ['-c:v', 'libx264', '-crf', '24', '-preset', 'slow', '-pix_fmt', 'yuv420p', '-movflags', '+faststart',
                             f'/out/{name}.mp4']]
        # (the GIF from the same frames: 2x with whole pixels, its palette the clip's own colours)
        if name in README_GIFS:
            encodes.append(['-y', '-loglevel', 'error', '-framerate', '30', '-i', '/work/png/%05d.png', '-vf',
                            'fps=15,scale=480:320:flags=neighbor,split[a][b];[a]palettegen=max_colors=128:stats_mode=full[p];'
                            '[b][p]paletteuse=dither=none', f'/out/{name}.gif'])
        for enc in encodes:
            cmd = ['docker'] + (['--context', CONTEXT] if CONTEXT else []) + [
                'run', '--rm', '-u', f'{os.getuid()}:{os.getgid()}', '--entrypoint', 'ffmpeg', '-v', f'{tmp}:/work', '-v', f'{out}:/out',
                FFMPEG_IMAGE, *enc]
            if subprocess.call(cmd):
                return 1
        exts = ('webm', 'mp4', 'gif') if name in README_GIFS else ('webm', 'mp4')
        sizes = ', '.join(f'{ext} {os.path.getsize(os.path.join(out, name + "." + ext)) // 1024} KB' for ext in exts)
        print(f'clip {name}: {n} frames, {sizes}')
    return 0


FLATPAK_ID = 'io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless'


def flatpak():
    """build/release/cyberworld-endless.flatpak: linux/flatpak's manifest built on this machine
    by flatpak-builder (Flathub's org.flatpak.Builder when it is not installed as a command),
    from this checkout, then bundled with Flathub named for its runtime."""
    manifest = os.path.join(ROOT, 'linux', 'flatpak', FLATPAK_ID + '.yml')
    work = os.path.join(ROOT, '.build', 'flatpak')
    if not shutil.which('flatpak'):
        print('flatpak is not installed')
        return 1
    builder = ['flatpak-builder'] if shutil.which('flatpak-builder') else ['flatpak', 'run', 'org.flatpak.Builder']
    code = subprocess.call(builder + ['--user', '--install-deps-from=flathub', '--force-clean', '--disable-rofiles-fuse',
                                      f'--state-dir={work}/state', f'--repo={work}/repo', f'{work}/build', manifest])
    if code:
        return code
    os.makedirs(RELEASE, exist_ok=True)
    out = os.path.join(RELEASE, 'cyberworld-endless.flatpak')
    # (the runtime's repository in the bundle: installing it fetches the runtime too)
    code = subprocess.call(['flatpak', 'build-bundle', '--runtime-repo=https://dl.flathub.org/repo/flathub.flatpakrepo',
                            f'{work}/repo', out, FLATPAK_ID])
    if not code:
        print(f'flatpak {out}: {os.path.getsize(out) // 1024} KB')
    return code


# why a tile was drawn with other floors than it shows (src/map/tiles.h, TILE_WHY_*)
WHY_NAMES = ['-', 'unseen', 'pixels', 'plain', 'ranked']
WHY = None


def defect_sheets(out, per_area=24):
    """Per area, a sheet of close-ups of the spots where tiles were drawn with other floors
    than they show (offs_*.txt, framed magenta in the seams images), each as drawn and as marked."""
    import collections
    import glob
    import re
    from PIL import Image, ImageDraw
    global WHY
    WHY = collections.defaultdict(lambda: [0] * len(WHY_NAMES))
    spots = collections.defaultdict(list)
    for path in sorted(glob.glob(os.path.join(out, 'seams_b*.png'))):
        name = os.path.basename(path)[6:-4]
        biome = int(re.match(r'b(\d+)_', name).group(1))
        offs, box = os.path.join(out, 'offs_' + name + '.txt'), os.path.join(out, name + '.box')
        if not os.path.exists(offs) or not os.path.exists(box):
            continue
        bx, by = map(int, open(box).read().split())
        # the flagged tiles, as cells of the cut picture; then clusters of them two tiles apart
        cells = set()
        for line in open(offs):
            x, y, why = map(int, line.split())
            cells.add(((x - bx) // 8, (y - by) // 8))
            WHY[biome][why] += 1
        seen = set()
        for c in sorted(cells):
            if c in seen:
                continue
            group, todo = [], [c]
            seen.add(c)
            while todo:
                cx, cy = todo.pop()
                group.append((cx, cy))
                for dx in (-2, -1, 0, 1, 2):
                    for dy in (-2, -1, 0, 1, 2):
                        n = (cx + dx, cy + dy)
                        if n in cells and n not in seen:
                            seen.add(n)
                            todo.append(n)
            x0 = min(g[0] for g in group) * 8 - 24; x1 = max(g[0] for g in group) * 8 + 32
            y0 = min(g[1] for g in group) * 8 - 16; y1 = max(g[1] for g in group) * 8 + 24
            cx, cy = (x0 + x1) // 2, (y0 + y1) // 2
            w, h = max(x1 - x0, 96), max(y1 - y0, 64)
            box = (cx - w // 2, cy - h // 2, cx - w // 2 + w, cy - h // 2 + h)
            spots[biome].append((len(group), path, box))
    for biome, found in spots.items():
        found.sort(key=lambda f: -f[0])
        shown = found[:per_area]
        tiles = []
        for n, path, box in shown:
            plain = Image.open(path.replace('seams_', '')).convert('RGB')
            marked = Image.open(path).convert('RGB')
            a, b = plain.crop(box), marked.crop(box)
            scale = 3 if a.width <= 128 else 2
            pair = Image.new('RGB', (a.width * scale * 2 + 4, a.height * scale + 12), (20, 20, 24))
            pair.paste(a.resize((a.width * scale, a.height * scale), Image.NEAREST), (0, 12))
            pair.paste(b.resize((b.width * scale, b.height * scale), Image.NEAREST), (a.width * scale + 4, 12))
            ImageDraw.Draw(pair).text((2, 0), f'{os.path.basename(path)[6:-4]} @{box[0]},{box[1]}: {n} tiles', fill=(255, 255, 0))
            tiles.append(pair)
        cols = 2
        cw = max(t.width for t in tiles)
        rows_h = [max(t.height for t in tiles[r:r + cols]) for r in range(0, len(tiles), cols)]
        sheet = Image.new('RGB', (cols * (cw + 8), sum(rows_h) + 8 * len(rows_h)), (0, 0, 0))
        y = 0
        for r, rh in zip(range(0, len(tiles), cols), rows_h):
            for k, t in enumerate(tiles[r:r + cols]):
                sheet.paste(t, (k * (cw + 8), y))
            y += rh + 8
        sheet.save(os.path.join(out, f'defects_b{biome:02d}.png'))
    return {b: len(f) for b, f in spots.items()}


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
    ap.add_argument('action', nargs='?', default='all', choices=['all', 'host', 'device', 'linux', 'windows', 'macos', 'flatpak', 'run', 'web', 'serve', 'release', 'package', 'shot', 'asan', 'test', 'clean', 'atlas', 'tiles', 'tour', 'pacing', 'screenshots', 'clips', 'town', 'world'])
    ap.add_argument('rest', nargs=argparse.REMAINDER)
    a = ap.parse_args()
    if a.action == 'clean':
        shutil.rmtree(os.path.join(ROOT, 'build'), ignore_errors=True)
        return
    if a.action == 'test':
        ensure_image()
        code = docker('make', 'test', *(['WERROR=1'] if os.environ.get('CI') else []))
        # (linux/steam/add-to-steam.py, on the host's Python)
        if code == 0:
            code = subprocess.call([sys.executable, os.path.join(ROOT, 'tests', 'test_add_to_steam.py')])
        sys.exit(code)
    if a.action == 'tour':
        build('host')
        sys.exit(tour(*a.rest[:1]))
    if a.action == 'linux':
        build('linux')
        os.makedirs(RELEASE, exist_ok=True)
        linux_release()
        return
    if a.action == 'windows':
        windows_release()
        return
    if a.action == 'macos':
        macos_release()
        return
    if a.action == 'flatpak':
        sys.exit(flatpak())
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
        windows_release()
        web_release()
        return
    if a.action == 'clips':
        build('host')
        sys.exit(clips(a.rest or None))
    if a.action == 'screenshots':
        build('host')
        sys.exit(screenshots(a.rest or None))
    if a.action == 'pacing':
        build('host')
        rom_dir = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
        os.makedirs(os.path.join(ROOT, '.build', 'data'), exist_ok=True)
        sys.exit(docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/data',
                        '--pacing', '/src/.build/pacing.txt', mounts=[(rom_dir, '/rom:ro')]))
    if a.action == 'town':
        build('host')
        sys.exit(town(*a.rest[:1]))
    if a.action == 'world':
        build('host')
        sys.exit(world())
    if a.action == 'atlas':
        build('host')
        rest = [r for r in a.rest if r != '--baseline']
        sys.exit(atlas(*rest[:2], baseline='--baseline' in a.rest))
    if a.action == 'tiles':
        build('host')
        rest = [r for r in a.rest if r != '--baseline']
        sys.exit(tiles(*rest[:1], baseline='--baseline' in a.rest))
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
