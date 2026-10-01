#!/usr/bin/env python3
"""Build, test and package Cyberworld Endless.

Compilation runs inside Docker images: the host build, the tests and the
checks in `cyberworld-build` (Debian trixie), the PortMaster port's aarch64
binary in `cyberworld-portmaster` (bullseye, older than any firmware
PortMaster serves), the Linux desktop release in `cyberworld-linux`
(bookworm), whose older glibc runs on more distributions.

  python3 build.py              host and device binaries
  python3 build.py host         host binary only
  python3 build.py device       aarch64 binary only
  python3 build.py linux        Linux desktop binary (build/linux) and its
                                release files in build/release: a tar.gz,
                                an AppImage and a .deb
  python3 build.py windows      the Windows build (build/windows, MinGW-w64 in
                                `cyberworld-windows`) and its release files in
                                build/release: a zip and an installer (NSIS)
  python3 build.py android      the Android app (build/release/cyberworld-endless.apk,
                                the NDK and Gradle in `cyberworld-android`)
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
  python3 build.py lint [--update]
                                the code's checks (issue #19): GCC's analyzer,
                                the functions no build reaches, lizard's
                                complexity, the ROM offsets docs/ROM_DATA.md
                                names and the files git tracks, each against
                                its baseline in tests/lint (--update writes
                                them anew); a new finding fails
  python3 build.py package      build/port/cyberworld (for PortMaster-New) and the port's zip
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
ANDROID_IMAGE = 'cyberworld-android'   # docker/Dockerfile.android
N3DS_IMAGE = 'cyberworld-3ds'   # docker/Dockerfile.3ds
PORTMASTER_IMAGE = 'cyberworld-portmaster'   # docker/Dockerfile.portmaster
IMAGES = {IMAGE: 'Dockerfile', LINUX_IMAGE: 'Dockerfile.linux', WEB_IMAGE: 'Dockerfile.web',
          WINDOWS_IMAGE: 'Dockerfile.windows', ANDROID_IMAGE: 'Dockerfile.android', N3DS_IMAGE: 'Dockerfile.3ds',
          PORTMASTER_IMAGE: 'Dockerfile.portmaster'}
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


BN6_SHA1 = '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6'


def docs_rom_mounts(bn5=False):
    """The ROM folder as the docs' pictures see it: BN6's ROM alone, as most
    players have it (BN5's beside it dresses some areas as its own, seed by
    seed), or the whole folder for a picture of BN5's areas."""
    import glob
    import hashlib
    rom_dir = default_rom_dir()
    if not bn5:
        for path in sorted(glob.glob(os.path.join(rom_dir, '*.gba'))):
            with open(path, 'rb') as f:
                if hashlib.sha1(f.read()).hexdigest() == BN6_SHA1:
                    return [(path, '/rom/bn6.gba:ro')]
    return [(rom_dir, '/rom:ro')]


def docker(*cmd, mounts=(), image=IMAGE, env=()):
    args = ['docker']
    if CONTEXT:
        args += ['--context', CONTEXT]
    args += ['run', '--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src']
    for host, guest in mounts:
        args += ['-v', f'{host}:{guest}']
    for name, value in env:
        args += ['-e', f'{name}={value}']
    for var in ('CYBERWORLD_AUDIO_DUMP', 'CYBERWORLD_SFX_LOG', 'CYBERWORLD_AUDIO_OFFLINE', 'CYBERWORLD_EMU_DEBUG', 'CYBERWORLD_TILE_AT', 'CYBERWORLD_AUTOPILOT', 'CYBERWORLD_TOWN_DEBUG', 'CYBERWORLD_TOWN_STYLE', 'CYBERWORLD_TOWN_VARIANT', 'CYBERWORLD_TOWN_TILE', 'CYBERWORLD_TOWN_START', 'CYBERWORLD_EMU_THREAD'):
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
    image = {'linux': LINUX_IMAGE, 'web': WEB_IMAGE, 'windows': WINDOWS_IMAGE, '3ds': N3DS_IMAGE,
             'aarch64': PORTMASTER_IMAGE}.get(target, IMAGE)
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


# The release's files are named for the system each is for (players picked
# the PortMaster port's plain cyberworld.zip for Windows): all but the
# AppImage, whose name its update information points at, and the ones whose
# kind says it (.dmg, .apk, .deb, .flatpak, .cia, .3dsx).
PORT_ZIP = 'cyberworld-endless-rocknix-portmaster.zip'
WEB_ZIP = 'cyberworld-endless-website.zip'


def web_release():
    """build/release/cyberworld-endless-website.zip: the site and its player, to host elsewhere."""
    archive = shutil.make_archive(os.path.join(RELEASE, WEB_ZIP[:-4]), 'zip', site(analytics=False))
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


APK_NAME = 'cyberworld-endless.apk'


def android_release():
    """build/release/cyberworld-endless.apk: the Android app (android/), SDL's
    activity running the game, for phones, tablets and Android handhelds
    (arm64, 32-bit ARM, x86-64; Android 5 on). Gradle and its downloads live
    in .build/gradle-home, the debug key that signs a build without a release
    key in .build/android-home (android/README.md)."""
    ensure_image(ANDROID_IMAGE)
    os.makedirs(RELEASE, exist_ok=True)
    m = re.match(r'(\d+)\.(\d+)\.(\d+)(?:\+(\d+))?', version())
    # (a later version or build installs over an earlier one)
    code = sum(int(g or 0) * f for g, f in zip(m.groups(), (10 ** 7, 10 ** 5, 10 ** 3, 1))) if m else 1
    env = [('GRADLE_USER_HOME', '/src/.build/gradle-home'), ('ANDROID_USER_HOME', '/src/.build/android-home'), ('HOME', '/src/.build/android-home')]
    for name in ('CI', 'ANDROID_KEYSTORE', 'ANDROID_KEYSTORE_PASSWORD', 'ANDROID_KEY_ALIAS', 'ANDROID_KEY_PASSWORD'):
        if os.environ.get(name):
            env.append((name, os.environ[name]))
    if docker('gradle', '-p', 'android', '--no-daemon', '--console=plain', 'assembleRelease',
              f'-PcwVersion={version()}', f'-PcwVersionCode={code}', image=ANDROID_IMAGE, env=env) != 0:
        sys.exit('the Android build failed')
    shutil.copy2(os.path.join(ROOT, 'android', 'app', 'build', 'outputs', 'apk', 'release', 'app-release.apk'),
                 os.path.join(RELEASE, APK_NAME))
    print('released', os.path.join(RELEASE, APK_NAME))


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
WINDOWS_SETUP = 'cyberworld-endless-windows-x64-setup.exe'


def windows_release():
    """build/release: cyberworld-endless-windows-x64.zip (the game in a folder,
    to unpack anywhere) and cyberworld-endless-windows-x64-setup.exe (an installer
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


SITE_URL = 'https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/'
SITE_PAGES = ('', 'download/', 'play/', 'faq/')


def faq_structured(page):
    """The FAQ's letters as schema.org's FAQPage (JSON-LD, for search engines), from the page itself:
    each letter's heading the question, its paragraphs but the links under them the answer."""
    import html.parser
    import json

    class Letters(html.parser.HTMLParser):
        def __init__(self):
            super().__init__()
            self.items, self.depth, self.grab = [], 0, None
        def handle_starttag(self, tag, attrs):
            a = dict(attrs)
            if tag == 'article' and 'mail' in (a.get('class') or '').split():
                self.items.append({'q': '', 'a': []})
            elif self.items and tag == 'h2':
                self.grab = 'q'
            elif self.items and tag == 'p' and 'more' not in (a.get('class') or '').split():
                self.items[-1]['a'].append('')
                self.grab = 'a'
        def handle_endtag(self, tag):
            if tag in ('h2', 'p'):
                self.grab = None
        def handle_data(self, data):
            if self.grab == 'q':
                self.items[-1]['q'] += data
            elif self.grab == 'a':
                self.items[-1]['a'][-1] += data

    with open(page, encoding='utf-8') as f:
        text = f.read()
    letters = Letters()
    letters.feed(text)
    squash = lambda t: ' '.join(t.split())
    doc = {'@context': 'https://schema.org', '@type': 'FAQPage', 'mainEntity': [
        {'@type': 'Question', 'name': squash(i['q']),
         'acceptedAnswer': {'@type': 'Answer', 'text': ' '.join(squash(p) for p in i['a'] if p.strip())}}
        for i in letters.items if i['q'].strip()]}
    block = '<script type="application/ld+json">\n' + json.dumps(doc, ensure_ascii=False) + '\n</script>\n'
    with open(page, 'w', encoding='utf-8') as f:
        f.write(text.replace('</head>', block + '</head>', 1))


def fingerprint(out):
    """Each page's own scripts and styles named with their contents' hash (?v=...), so a visitor's
    browser never runs an older one against a newer page: GitHub Pages lets browsers keep them ten
    minutes."""
    import glob
    import hashlib
    import re
    for page in glob.glob(os.path.join(out, '**', '*.html'), recursive=True):
        with open(page, encoding='utf-8') as f:
            text = f.read()
        def stamp(m):
            path = os.path.normpath(os.path.join(os.path.dirname(page), m.group(2)))
            if not os.path.isfile(path):
                return m.group(0)
            with open(path, 'rb') as f:
                digest = hashlib.sha1(f.read()).hexdigest()[:10]
            return f'{m.group(1)}="{m.group(2)}?v={digest}"'
        text = re.sub(r'(src|href)="(?!https?:|//)([^"?#]+\.(?:js|css))"', stamp, text)
        with open(page, 'w', encoding='utf-8') as f:
            f.write(text)


def site(analytics=True):
    """build/site: the project's pages (web/) with the browser build in play/, as GitHub Pages serves it,
    with its sitemap for search engines; without the pages' visit counter (Umami, counting on the
    project's own domain only), the sitemap and the not-found page, whose links are the Pages site's,
    for a copy to host elsewhere."""
    out = os.path.join(ROOT, 'build', 'site')
    shutil.rmtree(out, ignore_errors=True)
    shutil.copytree(os.path.join(ROOT, 'web'), out)
    faq_structured(os.path.join(out, 'faq', 'index.html'))
    if analytics:
        with open(os.path.join(out, 'sitemap.xml'), 'w') as f:
            f.write('<?xml version="1.0" encoding="UTF-8"?>\n<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">\n')
            for page in SITE_PAGES:
                f.write(f'  <url><loc>{SITE_URL}{page}</loc></url>\n')
            f.write('</urlset>\n')
    if not analytics:
        import glob
        os.remove(os.path.join(out, '404.html'))
        for page in glob.glob(os.path.join(out, '**', '*.html'), recursive=True):
            with open(page) as f:
                lines = f.readlines()
            with open(page, 'w') as f:
                f.writelines(l for l in lines if 'umami.saschb2b.com' not in l)
    shutil.copytree(os.path.join(ROOT, 'docs', 'screenshots'), os.path.join(out, 'shots'))
    if os.path.isdir(os.path.join(ROOT, 'docs', 'clips')):
        shutil.copytree(os.path.join(ROOT, 'docs', 'clips'), os.path.join(out, 'clips'))
    for name in ('cyberworld.js', 'cyberworld.wasm'):
        shutil.copy2(os.path.join(ROOT, 'build', 'web', name), os.path.join(out, 'play'))
    shutil.copy2(os.path.join(ROOT, 'LICENSE'), os.path.join(out, 'LICENSE.txt'))
    os.makedirs(os.path.join(out, 'licenses'))
    shutil.copy2(os.path.join(ROOT, 'build', 'web', 'licenses', 'mGBA.txt'), os.path.join(out, 'licenses'))
    open(os.path.join(out, '.nojekyll'), 'w').close()
    # (the engine's two halves named by the wasm's hash, so a browser never
    # pairs a kept one with a new one; then every page's own files by theirs)
    import hashlib
    with open(os.path.join(out, 'play', 'cyberworld.wasm'), 'rb') as f:
        wasm = hashlib.sha1(f.read()).hexdigest()[:10]
    app = os.path.join(out, 'play', 'app.js')
    with open(app, encoding='utf-8') as f:
        text = f.read()
    with open(app, 'w', encoding='utf-8') as f:
        f.write(text.replace("const ENGINE_VERSION = '';", f"const ENGINE_VERSION = '{wasm}';", 1))
    fingerprint(out)
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
    """build/release/cyberworld-endless-rocknix-portmaster.zip: the port laid out as PortMaster's
    own zip of it (tools/build_release.py in PortMaster-New): the launcher, and cyberworld/
    with port.json, gameinfo.xml, the screenshot and the README as cyberworld.md beside the
    game (port.json keeps PortMaster's own name for it, cyberworld.zip)."""
    import zipfile
    port = package()
    os.makedirs(RELEASE, exist_ok=True)
    archive = os.path.join(RELEASE, PORT_ZIP)
    moved = {'README.md': 'cyberworld/cyberworld.md', 'port.json': 'cyberworld/port.json',
             'gameinfo.xml': 'cyberworld/gameinfo.xml', 'screenshot.png': 'cyberworld/screenshot.png'}
    with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for base, dirs, files in os.walk(port):
            dirs.sort()
            for name in sorted(files):
                path = os.path.join(base, name)
                rel = os.path.relpath(path, port).replace(os.sep, '/')
                info = zipfile.ZipInfo.from_file(path, moved.get(rel, rel))
                if rel.endswith('.sh') or rel.endswith('.aarch64'):
                    info.external_attr = 0o100755 << 16   # (unpacked by hand, both start as they are)
                with open(path, 'rb') as f:
                    z.writestr(info, f.read(), zipfile.ZIP_DEFLATED)
    print('released', archive)


def package():
    """build/port/cyberworld: the port as a folder of PortMaster-New's ports/, to copy there
    for a pull request: the launcher, port.json, README.md, gameinfo.xml and the screenshot
    beside cyberworld/, which holds the binary, one license per part and rom/."""
    out = os.path.join(ROOT, 'build', 'port')
    port = os.path.join(out, 'cyberworld')
    game = os.path.join(port, 'cyberworld')
    if os.path.isdir(out):
        shutil.rmtree(out)
    os.makedirs(os.path.join(game, 'rom'))
    os.makedirs(os.path.join(game, 'licenses'))
    # (copied without their modes: PortMaster keeps launchers at 644, the launcher sets the binary's)
    for name in ('Cyberworld Endless.sh', 'README.md', 'gameinfo.xml', 'port.json'):
        shutil.copyfile(os.path.join(ROOT, 'port', name), os.path.join(port, name))
    # (a 640x480 frame, as on the most common handheld screen: build.py screenshots portmaster)
    shutil.copyfile(os.path.join(ROOT, 'docs', 'screenshots', 'portmaster.png'), os.path.join(port, 'screenshot.png'))
    shutil.copy2(os.path.join(ROOT, 'build', 'aarch64', 'cyberworld.aarch64'), game)
    shutil.copyfile(os.path.join(ROOT, 'LICENSE'), os.path.join(game, 'licenses', 'LICENSE.cyberworld.txt'))
    shutil.copyfile(os.path.join(ROOT, 'build', 'aarch64', 'licenses', 'mGBA.txt'),
                    os.path.join(game, 'licenses', 'LICENSE.mgba.txt'))
    with open(os.path.join(game, 'rom', 'PUT_YOUR_ROM_HERE.txt'), 'w') as f:
        f.write('Copy your own Mega Man Battle Network 6: Cybeast Gregar (USA) .gba file into this folder.\n')
    print('packaged', port)
    return port


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
    # (the setup after NEW GAME at its Folder row, and the title with the
    # marks a profile can earn: a won short net, the endless Nest, the
    # Secret Area, the top threat rung)
    ('setup', ['--scene', 'setup', '--input', '60:,6:DOWN,20:,6:DOWN,40:'], [(150, 'setup')], {}),
    ('marks', ['--scene', 'title', '--marks', '18A'], [(80, 'marks')], {}),
    # (seed 7's layer 3 walks straight to BlastMan's arena, then layer 4's
    # first battle; retimed for 0.6.0's layers)
    ('run', ['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '11', '--guardian', '12'],
     [(240, 'net'), (1400, 'guardian'), (1540, 'guardian-talk'), (1800, 'boss-custom'), (2500, 'result'),
      (2900, 'reward'), (2950, 'restored'), (3500, 'area-clear'), (5150, 'custom'), (5540, 'battle')],
     {'CYBERWORLD_AUTOPILOT': 'weak'}),
    ('act', ['--scene', 'emu', '--seed', '11', '--net-biome', '8', '--guardian', '12', '--dev', 'quiet'], [(120, 'act-card')], {}),
    # (Chaud's first call on an act's duel layer, its second box)
    ('rival', ['--scene', 'emu', '--run-depth', '2', '--seed', '3', '--net-biome', '0', '--dev', 'quiet', '--input', '700:,6:A,60:'],
     [(840, 'rival')], {}),
    # (the seed picks the town: build.py town lists which each gets)
    ('town-central', ['--scene', 'town', '--seed', '2'], [(280, 'town-central')], {}),
    ('town-acdc', ['--scene', 'town', '--seed', '3'], [(280, 'town-acdc')], {}),
    ('town-seaside', ['--scene', 'town', '--seed', '5'], [(280, 'town-seaside')], {}),
    ('town-green', ['--scene', 'town', '--seed', '9'], [(280, 'town-green')], {}),
    # (the areas on their acts' third layers: ProtoMan's duel, and Chaud's
    # call with it, waits on the second)
    ('central', ['--scene', 'emu', '--net-biome', '0', '--run-depth', '3', '--seed', '3', '--dev', 'quiet'], [(420, 'central')], {}),
    ('seaside', ['--scene', 'emu', '--net-biome', '1', '--run-depth', '3', '--seed', '11', '--dev', 'quiet'], [(420, 'seaside')], {}),
    ('green', ['--scene', 'emu', '--net-biome', '3', '--run-depth', '3', '--seed', '3', '--dev', 'quiet'], [(420, 'green')], {}),
    ('undernet', ['--scene', 'emu', '--net-biome', '5', '--run-depth', '15', '--seed', '3', '--dev', 'quiet'], [(420, 'undernet')], {}),
    ('graveyard', ['--scene', 'emu', '--net-biome', '4', '--run-depth', '18', '--seed', '3', '--dev', 'quiet'], [(420, 'graveyard')], {}),
    ('nest', ['--scene', 'emu', '--net-biome', '7', '--run-depth', '19', '--seed', '3', '--dev', 'quiet'], [(250, 'nest')], {}),
    # (the PET's E-Mail on a profile that has met seven guardians: the list,
    # then Dad's Records mail on its second page)
    ('pet', ['--scene', 'emu', '--run-depth', '4', '--seed', '3', '--dev', 'quiet,veteran', '--input',
             '300:,6:A,6:,54:,6:A,6:,54:,6:A,6:,54:,6:A,6:,54:,120:,6:START,6:,50:,6:DOWN,6:,6:DOWN,6:,6:DOWN,6:,6:DOWN,6:,'
             '6:A,6:,80:,6:DOWN,6:,6:A,6:,150:,6:A,6:,150:'],
     [(885, 'pet-mail'), (1221, 'pet-records')], {}),
    # (a phone held sideways with the touch controls round the picture, a
    # thumb on the D-pad: the whole screen at a phone's pixels, halved)
    ('touch', ['--scene', 'emu', '--run-depth', '4', '--seed', '3', '--net-biome', '1', '--dev', 'quiet', '--touch', '--size', '2400x1080',
               '--dpi', '420', '--taps', '400:240,581>360,701'],
     [(418, 'touch')], {}, 2),
    # (the PortMaster port's screenshot: seed 7's BlastMan fight on the
    # most common handheld screen, 640x480, the picture filling its width)
    ('portmaster', ['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '11', '--guardian', '12',
                    '--size', '640x480'], [(2120, 'portmaster')], {'CYBERWORLD_AUTOPILOT': 'weak'}, 1),
    # (a New 3DS's two screens: seed 7's layer 3 on the top one, the
    # layer's map on the bottom one, a Net Dealer and a Recovery Mr. Prog
    # met on the way to BlastMan's arena)
    ('3ds', ['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '11', '--guardian', '12'],
     [(1000, '3ds')], {'CYBERWORLD_AUTOPILOT': 'weak'}, '3ds'),
]


def two_screens(top, bottom):
    """A New 3DS's two screens in one picture: the game's 240x160 at 1.5x on
    the top one (400x240), its pixels mixed at their edges as the GPU's filter
    mixes them, and the second screen (320x240) under it."""
    from PIL import Image
    im = Image.new('RGB', (416, 512), (28, 30, 38))
    im.paste(Image.new('RGB', (400, 240)), (8, 8))
    im.paste(top.resize((360, 240), Image.BILINEAR), (28, 8))
    im.paste(bottom, (48, 264))
    return im


def screenshots(only=None):
    """docs/screenshots/NAME.png: the 240x160 picture of chosen frames (or the
    whole screen, shrunk by the entry's last number; or '3ds', the picture
    and the second screen as a New 3DS shows them)."""
    from PIL import Image
    out = os.path.join(ROOT, 'docs', 'screenshots')
    tmp = os.path.join(ROOT, '.build', 'screenshots')
    os.makedirs(out, exist_ok=True)
    for name, args, frames, env, *whole in SCREENSHOTS:
        if only and name not in only:
            continue
        shutil.rmtree(tmp, ignore_errors=True)
        os.makedirs(os.path.join(tmp, 'data'))
        shots = ','.join(f'{f}:/src/.build/screenshots/{n}.bmp' for f, n in frames)
        dual = whole == ['3ds']
        # (the whole screen, the touch controls on it, where the canvas has the game alone)
        shot = '--screen-shot' if whole and not dual else '--shot'
        second = ['--second-shot', ','.join(f'{f}:/src/.build/screenshots/{n}-second.bmp' for f, n in frames)] if dual else []
        saved = {k: os.environ.get(k) for k in env}
        os.environ.update(env)
        code = docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/screenshots/data',
                      *args, '--frames', str(max(f for f, _ in frames) + 1), shot, shots, *second,
                      mounts=docs_rom_mounts())
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
            if not whole or dual:
                im = im.crop(((w - 240) // 2, (h - 160) // 2, (w + 240) // 2, (h + 160) // 2))
            elif whole[0] > 1:
                im = im.resize((w // whole[0], h // whole[0]), Image.LANCZOS)
            if dual:
                im = two_screens(im, Image.open(os.path.join(tmp, f'{n}-second.bmp')).convert('RGB'))
            im.save(os.path.join(out, f'{n}.png'), optimize=True)
            print('screenshot', n)
    return 0


# Short videos of the game for the site (docs/clips): name, game options,
# env, scripted input, first and last frame. Every second frame, at 30 fps.
FFMPEG_IMAGE = 'linuxserver/ffmpeg:9.0-cli-ls82'
# (seed 7's layer 3 in the Robot Control Comp against BlastMan, as the
# captures were timed: the areas' and guardians' draw has changed since)
RUN_7 = ['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '11', '--guardian', '12']
AREA_WALK = ['--scene', 'emu', '--run-depth', '1', '--dev', 'quiet']
CLIPS = [
    ('title', ['--scene', 'title'], {}, None, 300, 780),
    ('net', RUN_7, {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 150, 600),
    # (layer 4's first battle, from BATTLE START to the RESULT window; the
    # walk into BlastMan's arena and his card; the Undernet on its act's
    # third layer, where no call from Chaud comes)
    ('battle', RUN_7, {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 5400, 5830),
    ('guardian', RUN_7, {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 1200, 1470),
    ('undernet', ['--scene', 'emu', '--net-biome', '5', '--run-depth', '15', '--seed', '3', '--dev', 'quiet'], {},
     '150:,70:RIGHT,50:UP,70:LEFT,50:DOWN,100:', 150, 480),
    ('jackin', ['--scene', 'title'], {}, '120:,4:START,60:,4:A,200:', 110, 222),
    # (the areas walked, layer 1 by the weak autopilot past MegaMan's words;
    # on BN6's ROM alone, but acdc-bn5, with BN5's beside it: docs/MULTIROM.md)
    ('sky', AREA_WALK + ['--net-biome', '2', '--seed', '2'], {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 300, 620),
    ('weather', AREA_WALK + ['--net-biome', '14', '--seed', '3'], {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 260, 580),
    ('central', AREA_WALK + ['--net-biome', '0', '--seed', '11'], {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 260, 580),
    ('seaside', AREA_WALK + ['--net-biome', '1', '--seed', '11'], {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 260, 580),
    ('green', AREA_WALK + ['--net-biome', '3', '--seed', '3'], {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 260, 580),
    ('graveyard', AREA_WALK + ['--net-biome', '4', '--seed', '3'], {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 260, 580),
    ('acdc-bn5', AREA_WALK + ['--net-biome', '0', '--seed', '4'], {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 260, 580),
]


# the clips the README shows as GIFs too (GitHub plays no video from the repository):
# name, then its first seconds and frames a second (a walk's scrolling floor
# makes a GIF of all of it 2 MB)
README_GIFS = {'guardian': (None, 15), 'sky': (4, 12), 'weather': (4, 12), 'acdc-bn5': (4, 12)}


def clips(only=None):
    """docs/clips/NAME.webm, .mp4 and .png: frames of scripted runs, 4x, 30 fps (and
    NAME.gif, 2x, at the rate README_GIFS gives, for those in it)."""
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
                      mounts=docs_rom_mounts(bn5=name.endswith('-bn5')))
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
            secs, fps = README_GIFS[name]
            encodes.append(['-y', '-loglevel', 'error', '-framerate', '30', '-i', '/work/png/%05d.png', '-vf',
                            (f'trim=duration={secs},' if secs else '') + f'fps={fps},scale=480:320:flags=neighbor,split[a][b];'
                            '[a]palettegen=max_colors=128:stats_mode=full[p];[b][p]paletteuse=dither=none', f'/out/{name}.gif'])
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


# ---- the code's checks (issue #19) ----

LINT_DIR = os.path.join(ROOT, 'tests', 'lint')
LINT_CCN, LINT_NLOC = 25, 120        # a function past either is listed
LINT_BIG = 1 << 20                   # a tracked file this large belongs in docs/
LINT_BANNED = ('.gba', '.gb', '.gbc', '.sav', '.srm', '.state', '.ss0', '.ss1', '.ss2', '.ss3', '.ss4', '.ss5',
               '.ss6', '.ss7', '.ss8', '.ss9')


def lint_baseline(name):
    """A baseline's lines (a multiset), its # notes left out."""
    from collections import Counter
    path = os.path.join(LINT_DIR, name)
    if not os.path.exists(path):
        return Counter()
    with open(path) as f:
        return Counter(line.rstrip('\n') for line in f if line.strip() and not line.startswith('#'))


def lint_write(name, note, lines):
    os.makedirs(LINT_DIR, exist_ok=True)
    with open(os.path.join(LINT_DIR, name), 'w') as f:
        f.write(''.join(f'# {n}\n' for n in note.split('\n')))
        f.write(''.join(f'{line}\n' for line in sorted(lines)))


def lint_compare(title, name, note, found, update):
    """found (a Counter of lines) against the baseline: what is new fails."""
    base = lint_baseline(name)
    new, gone = found - base, base - found
    if update:
        lint_write(name, note, list(found.elements()))
        print(f'{title}: {sum(found.values())} listed in tests/lint/{name}')
        return True
    for line in sorted(new.elements()):
        print(f'  new: {line}')
    if gone:
        print(f'  ({sum(gone.values())} fixed since the baseline: build.py lint --update drops them)')
    print(f'{title}: {sum(found.values())}, {sum(new.values())} new')
    return not new


def lint_files():
    """No ROM, save or state in git, and no large file outside docs/ (AGENTS.md)."""
    names = subprocess.run(['git', 'ls-files', '-z'], cwd=ROOT, capture_output=True, check=True).stdout.decode().split('\0')
    bad = []
    for n in filter(None, names):
        path = os.path.join(ROOT, n)
        if n.lower().endswith(LINT_BANNED):
            bad.append(f'{n}: a ROM, save or state')
        elif not n.startswith('docs/') and os.path.isfile(path) and os.path.getsize(path) > LINT_BIG:
            bad.append(f'{n}: {os.path.getsize(path) >> 10} KB outside docs/')
    for b in bad:
        print(f'  refused: {b}')
    print(f'files: {len(bad)} refused')
    return not bad


def lint_rom_data(update):
    """Every address of src/emu/bn6.h and every RomLayout field named in docs/ROM_DATA.md."""
    from collections import Counter
    with open(os.path.join(ROOT, 'docs', 'ROM_DATA.md')) as f:
        doc = f.read().lower()
    with open(os.path.join(ROOT, 'src', 'emu', 'bn6.h')) as f:
        bn6 = f.read()
    with open(os.path.join(ROOT, 'src', 'core', 'rom.h')) as f:
        rom = f.read()
    missing = []
    for name, val in re.findall(r'#define\s+(BN6_\w+)\s+\(?(0x[0-9A-Fa-f]+)u?\)?', bn6):
        v = int(val, 16)
        forms = {name.lower(), val.lower(), f'0x{v:08x}', f'0x{v:x}', f'0x{v:06x}', f'0x{v:04x}'}
        if v >= 0x08000000:
            forms |= {f'0x{v - 0x08000000:06x}', f'0x{v - 0x08000000:x}'}
        if not any(form in doc for form in forms):
            missing.append(f'bn6.h {name}')
    m = re.search(r'typedef struct \{(.*?)\} RomLayout;', rom, re.S)
    for group in re.findall(r'uint32_t\s+([\w, ]+);', m.group(1) if m else ''):
        for field in (f.strip() for f in group.split(',')):
            if field.lower() not in doc:
                missing.append(f'RomLayout {field}')
    return lint_compare('ROM offsets without their note in docs/ROM_DATA.md', 'rom_data.txt',
                        'ROM offsets docs/ROM_DATA.md does not name (AGENTS.md: a new one needs its note);\n'
                        'this list only shrinks: build.py lint --update after naming one', Counter(missing), update)


def lint_complexity(update):
    """lizard over src/: a function past CCN LINT_CCN or LINT_NLOC lines of code is new,
    or a listed one grew."""
    import csv
    out = subprocess.run(['docker'] + (['--context', CONTEXT] if CONTEXT else []) +
                         ['run', '--rm', '-v', f'{ROOT}:/src', '-w', '/src', IMAGE, 'lizard', '--csv', 'src'],
                         capture_output=True, text=True)
    if out.returncode not in (0, 1) or not out.stdout:
        sys.exit('lizard failed in the build image: one made before it came in? docker rmi cyberworld-build, then again\n'
                 + out.stderr[-400:])
    now = {}
    for row in csv.reader(out.stdout.splitlines()):
        nloc, ccn, file, name = int(row[0]), int(row[1]), row[6], row[7]
        if ccn > LINT_CCN or nloc > LINT_NLOC:
            key = f'{file}:{name}'
            now[key] = max(now.get(key, (0, 0)), (ccn, nloc))
    note = (f'functions past CCN {LINT_CCN} or {LINT_NLOC} lines of code (lizard): "file:function CCN NLOC";\n'
            'none may join and none may grow: split one, then build.py lint --update')
    if update:
        lint_write('complexity.txt', note, [f'{k} {c} {n}' for k, (c, n) in now.items()])
        print(f'complexity: {len(now)} listed in tests/lint/complexity.txt')
        return True
    base = {}
    for line in lint_baseline('complexity.txt'):
        key, c, n = line.rsplit(' ', 2)
        base[key] = (int(c), int(n))
    bad = 0
    for key, (c, n) in sorted(now.items()):
        if key not in base:
            print(f'  new: {key} CCN {c}, {n} lines')
            bad += 1
        elif c > base[key][0] or n > base[key][1]:
            print(f'  grew: {key} CCN {base[key][0]} -> {c}, {base[key][1]} -> {n} lines')
            bad += 1
    print(f'complexity: {len(now)} functions past CCN {LINT_CCN} or {LINT_NLOC} lines, {bad} new or grown')
    return not bad


def lint_analyzer(update):
    """GCC's analyzer over the host build's sources, at -O2 as it builds."""
    from collections import Counter
    script = ('for f in src/*/*.c; do gcc -std=c11 -O2 -fanalyzer -fdiagnostics-plain-output -D_DEFAULT_SOURCE -DCW_DESKTOP '
              '$(for d in src/*/; do printf -- "-I%s " "$d"; done) -Ibuild/host/gen $(pkg-config --cflags sdl2 | sed "s/-I/-isystem /g") '
              '-isystem /opt/mgba/host/include -c -o /dev/null "$f"; done 2>&1')
    build('host')   # (its version.h)
    out = subprocess.run(['docker'] + (['--context', CONTEXT] if CONTEXT else []) +
                         ['run', '--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src', IMAGE,
                          'sh', '-c', script], capture_output=True, text=True).stdout
    found, fn = Counter(), '?'
    for line in out.splitlines():
        m = re.match(r"^(src/\S+): In function '([^']+)'", line)
        if m:
            fn = m.group(2)
            continue
        m = re.match(r'^(src/\S+?):\d+:\d+: warning: (.*?)(?: \[CWE-\d+\])? \[(-Wanalyzer-[a-z-]+)\]$', line)
        if m:
            found[f'{m.group(1)}: {fn}: {m.group(2)} [{m.group(3)}]'] += 1
    return lint_compare("GCC's analyzer", 'analyzer.txt',
                        "GCC's -fanalyzer (-O2, the host build's sources): \"file: function: finding\";\n"
                        'a new one fails; fix one, then build.py lint --update', found, update)


def lint_dead(update):
    """The game's functions no build reaches: the host and handheld builds and the
    tests, each linked with every function in its own section and the linker's
    list of those it drops. A function one of them reaches is kept."""
    from collections import Counter
    extra = ['EXTRA_CFLAGS=-ffunction-sections', 'EXTRA_LDFLAGS=-Wl,--gc-sections -Wl,--print-gc-sections']
    links = {}
    for target, image, binary, nm in (('host', IMAGE, 'cyberworld', 'nm'),
                                      ('aarch64', PORTMASTER_IMAGE, 'cyberworld.aarch64', 'aarch64-linux-gnu-nm')):
        ensure_image(image)
        out = f'build/lint/{target}'
        # (linked afresh each time: the linker's list is the report)
        run = subprocess.run(['docker'] + (['--context', CONTEXT] if CONTEXT else []) +
                             ['run', '--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src', image,
                              'sh', '-c', f'rm -f {out}/{binary}; make TARGET={target} OUT={out} VERSION=lint -j{os.cpu_count() or 4} '
                              f'"{extra[0]}" "{extra[1]}" {out}/{binary} 2>&1 >/dev/null; '
                              f'{nm} -A -g --defined-only {out}/obj/*/*.o'], capture_output=True, text=True)
        links[target] = run.stdout
    ensure_image()
    test = subprocess.run(['docker'] + (['--context', CONTEXT] if CONTEXT else []) +
                          ['run', '--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src', IMAGE,
                           'sh', '-c', f'rm -f build/lint/test_core; make -j{os.cpu_count() or 4} build/lint/test_core 2>&1 >/dev/null; '
                           'nm -A -g --defined-only build/lint/test/src/*/*.o'], capture_output=True, text=True)
    links['tests'] = test.stdout
    reached, present = set(), set()
    for target, text in links.items():
        here, dropped = set(), set()
        for line in text.splitlines():
            m = re.search(r"removing unused section '\.text\.([A-Za-z_]\w*)' in file '(?:build/lint/\w+/obj|build/lint/test/src)/(\w+/\w+)\.o'", line)
            if m:
                dropped.add((f'src/{m.group(2)}.c', m.group(1)))
                continue
            m = re.match(r'^(?:build/lint/\w+/obj|build/lint/test/src)/(\w+/\w+)\.o:[0-9a-f]* T (\w+)$', line)
            if m:
                here.add((f'src/{m.group(1)}.c', m.group(2)))
        if not here:
            sys.exit(f'the {target} link for build.py lint gave nothing to read:\n{text[-600:]}')
        present |= here
        reached |= here - dropped
    dead = Counter(f'{f}: {n}' for f, n in present - reached if n != 'main')
    return lint_compare('functions no build reaches', 'dead.txt',
                        'functions no build reaches (the host and handheld builds and the tests, build.py lint):\n'
                        'remove one, or make it static where its own file uses it; build.py lint --update', dead, update)


def lint(update=False):
    ok = lint_files()
    ok = lint_rom_data(update) and ok
    ok = lint_complexity(update) and ok
    ok = lint_analyzer(update) and ok
    ok = lint_dead(update) and ok
    return 0 if ok else 1


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('action', nargs='?', default='all', choices=['all', 'host', 'device', 'linux', 'windows', 'android', 'macos', 'flatpak', '3ds', 'run', 'web', 'serve', 'release', 'package', 'shot', 'asan', 'test', 'lint', 'clean', 'atlas', 'tiles', 'tour', 'pacing', 'screenshots', 'clips', 'town', 'world'])
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
    if a.action == 'lint':
        ensure_image()
        sys.exit(lint('--update' in a.rest))
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
    if a.action == 'android':
        android_release()
        return
    if a.action == '3ds':
        # the Nintendo 3DS (3ds/README.md): build/3ds/cyberworld-endless.3dsx
        build('3ds')
        print('built', os.path.join(ROOT, 'build', '3ds', 'cyberworld-endless.3dsx'), 'and', os.path.join(ROOT, 'build', '3ds', 'cyberworld-endless.cia'))
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
        port_release()
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
