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
  python3 build.py ios          the iPhone and iPad app (build/release/
                                cyberworld-endless.ipa, unsigned: AltStore or
                                SideStore signs it), on a Mac with Xcode: SDL2
                                and mGBA built by ios/deps.sh; --simulator for
                                the iOS Simulator's app (CI's smoke test)
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
  python3 build.py symbols [--bn6f DIR [--roms DIR]]
                                what the project has mapped of BN6 Gregar and
                                BN5 Team Colonel, in docs/symbols (a symbol
                                file for mGBA and no$gba, JSON and CSV per ROM;
                                docs/SYMBOLS.md), from the sources
                                (tools/symbols.py); with a bn6f checkout, the
                                full map with bn6f's functions located in the
                                ROMs too, in .build/symbols, never committed
                                (tools/bn6f_match.py)
  python3 build.py lint [--update]
                                the code's checks (issue #19): GCC's analyzer,
                                the functions no build reaches, lizard's
                                complexity, the ROM offsets docs/ROM_DATA.md
                                names, the files git tracks, each against its
                                baseline in tests/lint (--update writes them
                                anew), and docs/symbols as tools/symbols.py
                                writes it; a new finding fails
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


def engine_context():
    """The Docker engine builds run on: DOCKER_CONTEXT_NAME's where it is
    set (CI's "", the runner's own); else Linux's own engine where it
    answers, whose bind mounts are this machine's own files; else Docker
    Desktop's. Desktop runs containers in a VM and shares the repository
    into it, and that share hung under heavy writes (a Gradle build,
    thousands of pictures) with the VM frozen until Desktop was restarted,
    five times by 5 October 2026."""
    if 'DOCKER_CONTEXT_NAME' in os.environ:
        return os.environ['DOCKER_CONTEXT_NAME']
    if os.path.exists('/var/run/docker.sock'):
        try:
            probe = subprocess.run(['docker', '--context', 'default', 'info'], capture_output=True, timeout=15)
            if probe.returncode == 0:
                return 'default'
        except (OSError, subprocess.TimeoutExpired):
            pass
    return 'desktop-linux'


CONTEXT = engine_context()
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
BN5_SHA1 = '5f472f78d8de2df01d5039e045c043cb40969a39'   # Team Colonel (USA), optional beside it (docs/MULTIROM.md)


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
    for var in ('CYBERWORLD_AUDIO_DUMP', 'CYBERWORLD_SFX_LOG', 'CYBERWORLD_AUDIO_OFFLINE', 'CYBERWORLD_EMU_DEBUG', 'CYBERWORLD_WATCH', 'CYBERWORLD_TILE_AT', 'CYBERWORLD_AUTOPILOT', 'CYBERWORLD_TOWN_DEBUG', 'CYBERWORLD_TOWN_STYLE', 'CYBERWORLD_TOWN_VARIANT', 'CYBERWORLD_TOWN_TILE', 'CYBERWORLD_TOWN_START', 'CYBERWORLD_EMU_THREAD'):
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
PORT_ZIP = 'cyberworld-endless-portmaster.zip'
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


IOS_IPA = 'cyberworld-endless.ipa'
IOS_APP = 'CyberworldEndless.app'


def ios_release(simulator=False):
    """build/release/cyberworld-endless.ipa: the game for iPhone and iPad (iOS
    14 on, arm64), signed ad hoc and no more: AltStore or SideStore signs it
    again with the player's Apple ID as it installs it (ios/README.md). With
    simulator, build/ios-iphonesimulator/CyberworldEndless.app for the iOS
    Simulator, which CI starts. Runs on a Mac only, with Xcode and CMake: the
    SDK is Apple's (CI's ios job builds it)."""
    if sys.platform != 'darwin':
        sys.exit('the iOS build runs on a Mac with Xcode (CI: the ios job)')
    sdk = 'iphonesimulator' if simulator else 'iphoneos'
    deps = os.path.join(ROOT, '.build', 'ios-deps', sdk)
    if not os.path.exists(os.path.join(deps, 'lib', 'libmgba.a')):
        subprocess.check_call(['sh', os.path.join(ROOT, 'ios', 'deps.sh'), deps, sdk])
    werror = ['WERROR=1'] if os.environ.get('CI') else []
    if subprocess.call(['make', 'TARGET=ios', f'IOS_SDK={sdk}', f'IOS_DEPS={deps}', f'VERSION={version()}',
                        f'-j{os.cpu_count() or 4}', *werror], cwd=ROOT) != 0:
        sys.exit('ios build failed')
    out = os.path.join(ROOT, 'build', f'ios-{sdk}')
    payload = os.path.join(out, 'Payload')
    app = os.path.join(payload, IOS_APP)
    shutil.rmtree(payload, ignore_errors=True)
    os.makedirs(os.path.join(app, 'licenses'))
    shutil.copy2(os.path.join(out, 'cyberworld-endless'), app)
    short = '.'.join(file_version().split(',')[:3])
    with open(os.path.join(ROOT, 'ios', 'Info.plist')) as f:
        plist = f.read().replace('@SHORT_VERSION@', short).replace('@PLATFORM@', 'iPhoneSimulator' if simulator else 'iPhoneOS')
    with open(os.path.join(app, 'Info.plist'), 'w') as f:
        f.write(plist)
    # (the icons Info.plist names, at the bundle's top: no asset catalog)
    for name in os.listdir(os.path.join(ROOT, 'ios', 'icons')):
        if name.startswith('AppIcon') and '-1024' not in name:
            shutil.copy2(os.path.join(ROOT, 'ios', 'icons', name), app)
    shutil.copy2(os.path.join(ROOT, 'LICENSE'), os.path.join(app, 'licenses', 'LICENSE.txt'))
    for name in ('mGBA.txt', 'SDL2.txt'):
        shutil.copy2(os.path.join(deps, 'share', 'licenses', name), os.path.join(app, 'licenses'))
    # (ad hoc, no entitlements: Apple silicon's Simulator runs nothing
    # unsigned, and AltStore checks the IPA's entitlements against its
    # source's, none)
    if subprocess.call(['codesign', '--force', '--sign', '-', '--timestamp=none', app]) != 0:
        sys.exit('codesign failed')
    if simulator:
        print('built', app)
        return
    os.makedirs(RELEASE, exist_ok=True)
    ipa = os.path.join(RELEASE, IOS_IPA)
    if os.path.exists(ipa):
        os.remove(ipa)
    # (an IPA is a zip of Payload/; ditto keeps the bundle's attributes)
    if subprocess.call(['ditto', '-c', '-k', '--sequesterRsrc', '--keepParent', payload, ipa]) != 0:
        sys.exit('ditto failed')
    print('released', ipa)


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
    """The site on http://localhost:PORT; the developer's ROMs also there for tests in a local browser,
    told by their SHA-1: BN6's at /.dev/rom.gba, BN5's at /.dev/bn5.gba (the page itself takes only the
    files a player chooses)."""
    import glob
    import hashlib
    import http.server
    import functools
    out = site()
    roms = {}
    for path in sorted(glob.glob(os.path.join(default_rom_dir(), '*.gba'))):
        with open(path, 'rb') as f:
            digest = hashlib.sha1(f.read()).hexdigest()
        for url, sha1 in (('/.dev/rom.gba', BN6_SHA1), ('/.dev/bn5.gba', BN5_SHA1)):
            if digest == sha1:
                roms.setdefault(url, path)

    class Handler(http.server.SimpleHTTPRequestHandler):
        def do_GET(self):
            rom = roms.get(self.path)
            if rom:
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
    """build/release/cyberworld-endless-portmaster.zip: the port laid out as PortMaster's
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
         'Robot Comp', 'Aquarium Comp', 'Judge Comp', 'Weather Comp', 'CopyBot Comp', 'ACDC HP', 'Green HP', 'Sky HP',   # src/core/run.h
         # (then another game's areas, where its ROM is beside BN6's: src/core/rom.c's bn5_areas, docs/MULTIROM.md)
         'BN5 ACDC', 'BN5 SciLab', 'BN5 End', 'BN5 Nebula', 'BN5 Oran', 'BN5 Undernet']


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


# 0.9.0's program pick (program-fit): the run's gift Custom1 taken and put on
# the NaviCust where it comes first (PET, MegaMan, NaviCust, A, A, RUN, out),
# the dev menu's Next guardian to BlastMan's arena on layer 3, its arrival
# words paged, then his Guardian Data's words (--talk reward, past layer 1's
# frames: on a layer without a guardian it opens another chat)
FIT_GIFT = ('60:,' + '4:A,6:,' * 20 + '40:,10:,6:A,64:,6:A,94:,6:A,94:,6:A,84:,6:DOWN,14:,6:DOWN,14:,6:A,174:,6:A,134:,'
            '6:A,134:,')
FIT_INSTALL = ('6:START,54:,6:DOWN,14:,6:DOWN,14:,6:DOWN,24:,6:A,60:,6:A,60:,6:A,40:,6:A,40:,6:DOWN,30:,6:A,500:,'
               '6:A,100:,6:A,100:,6:B,100:,6:A,100:,6:B,100:,6:B,100:,')
FIT_NEXT = '10:SELECT,6:SELECT+R,4:SELECT,20:,' + '6:DOWN,6:,' * 8 + '10:,6:A,6:,'
FIT_PAGES = '300:,' + '6:A,134:,' * 4 + '2230:,' + '6:A,174:,' * 12

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
    # (seed 7's layer 3 walks straight to BlastMan's arena, then the first
    # random battle, on layer 5 since 0.9.0's shorter words)
    ('run', ['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '11', '--guardian', '12'],
     [(240, 'net'), (1400, 'guardian'), (1584, 'guardian-talk'), (1800, 'boss-custom'), (2500, 'result'),
      (3100, 'reward'), (2900, 'restored'), (3500, 'area-clear'), (8190, 'custom'), (8450, 'battle')],
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
             '300:,' + '6:A,6:,54:,' * 10 + '60:,6:START,6:,50:,6:DOWN,6:,6:DOWN,6:,6:DOWN,6:,6:DOWN,6:,'
             '6:A,6:,80:,6:DOWN,6:,6:A,6:,150:,6:A,6:,150:'],
     [(1221, 'pet-mail'), (1557, 'pet-records')], {}),
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
     [(1040, '3ds')], {'CYBERWORLD_AUTOPILOT': 'weak'}, '3ds'),
    # 0.8.0's notes (docs/releases/v0.8.0.md), each by its feature: MegaMan
    # placed at it ("0:place X Y FACE"), flags set ("0:flags FROM TO 1"),
    # the chats paged with A. (the setup's NEW marks: a veteran profile's
    # guardians fell, and no summary said so)
    ('setup-new', ['--scene', 'setup', '--dev', 'veteran', '--input', '60:,6:DOWN,20:,6:RIGHT,40:'], [(150, 'setup-new')], {}),
    # (a RegUp in a layer's blue data, MegaMan's word on it, the folder's REG tag)
    ('regup', ['--scene', 'emu', '--run-depth', '1', '--seed', '4', '--dev', 'quiet', '--input',
               '200:,0:place 460 -140 3,200:,6:A,74:,6:A,154:,6:A,74:,6:A,134:,6:A,94:,6:A,124:,6:A,124:,6:A,124:,6:A,124:,'
               '6:A,124:,6:A,124:,6:A,124:,6:A,84:,6:START,54:,6:A,54:,6:A,44:,6:A,64:,6:SELECT,44:,6:A,114:,6:A,54:,6:A,144:,6:A,64:'],
     [(940, 'regup'), (1080, 'regup-words'), (2640, 'regup-folder')], {}),
    # (Chaud's clearance brings the TagChip system; EDIT's SELECT; a tagged pair)
    ('tagchip', ['--scene', 'emu', '--run-depth', '1', '--seed', '4', '--dev', 'quiet,veteran', '--input',
                 '230:,6:A,134:,6:A,134:,6:A,134:,6:A,134:,6:A,134:,6:START,54:,6:A,54:,6:A,44:,6:A,64:,6:SELECT,66:,'
                 '6:DOWN,16:,6:A,106:,6:A,66:,6:A,106:,6:A,46:,6:DOWN,26:,6:A,106:,6:A,66:'],
     [(225, 'tagchip'), (1260, 'tagchip-menu'), (1820, 'tagchip-pair')], {}),
    # (three programs' compression flags, as the NaviCust sets them on a code
    # entered: the codebook keeps them, and Dad's Compression mail lists them)
    ('codes', ['--scene', 'emu', '--run-depth', '1', '--seed', '4', '--dev', 'quiet', '--input',
               '400:,6:A,74:,6:A,154:,6:A,74:,0:flags 0x2664 0x266B 1,0:flags 0x2694 0x2697 1,110:,6:A,144:,6:A,114:,6:A,74:,'
               '6:START,54:,6:DOWN,6:,6:DOWN,6:,6:DOWN,6:,6:DOWN,6:,12:,6:A,74:,6:DOWN,6:,6:DOWN,16:,6:A,134:,6:A,114:,6:A,114:,6:A,154:'],
     [(1950, 'codes')], {}),
    # (the Guardian Data's reward chat on layer 3, then the split's question:
    # a veteran profile names both guardians)
    ('split', ['--scene', 'emu', '--run-depth', '3', '--seed', '3', '--dev', 'quiet,veteran', '--talk', 'reward:100',
               '--input', '301:,' + '6:A,134:,' * 26 + '6:A,106:'],
     [(3790, 'split-question'), (4040, 'split')], {}),
    ('rumor', ['--scene', 'emu', '--run-depth', '1', '--seed', '4', '--net-biome', '3', '--dev', 'quiet', '--talk', 'rumor:100',
               '--input', '230:,6:A,64:,6:A,134:'],
     [(228, 'rumor'), (298, 'rumor-2'), (438, 'rumor-3')], {}),
    # (Central Town's seed 6: Lan beside the Giga-chip kid before Dad's words)
    ('schoolyard', ['--scene', 'town', '--seed', '6', '--input', '40:,0:place 178 86 3,10:,6:A,134:,6:A,94:'],
     [(185, 'schoolyard'), (290, 'schoolyard-2')], {}),
    ('bbs', ['--scene', 'emu', '--run-depth', '1', '--seed', '4', '--dev', 'quiet', '--input',
             '400:,6:A,74:,6:A,154:,6:A,94:,6:START,54:,6:DOWN,6:,6:DOWN,6:,6:DOWN,6:,6:DOWN,6:,12:,6:A,74:,6:DOWN,6:,6:DOWN,16:,'
             '6:A,134:,6:A,114:,6:A,114:,6:A,114:,6:A,114:,6:A,114:'],
     [(1340, 'bbs'), (1700, 'bbs-2')], {}),
    # (the layer's first data taken, MegaMan by a green one: the map's
    # counters, then L's in the corner)
    ('counters', ['--scene', 'emu', '--run-depth', '1', '--seed', '2', '--dev', 'quiet', '--input',
                  '400:,6:A,74:,6:A,154:,6:A,94:,0:flags 0x1400 0x1400 1,0:place -76 76 3,40:,60:SELECT,40:,6:L,154:,6:A,154:'],
     [(810, 'counters-map'), (1000, 'counters-l')], {}),
    ('hidden-hint', ['--scene', 'emu', '--run-depth', '4', '--net-biome', '1', '--seed', '23', '--dev', 'quiet,pieces=64', '--input',
                     '60:,' + '4:A,6:,' * 20 + '40:,0:place 76 164 1,30:,6:A,6:,160:,6:A,6:,160:'], [(640, 'hidden-hint')], {}),
    # (the rush clip's run: two Rushes across the gap, then the island's data)
    ('rush', ['--scene', 'emu', '--run-depth', '4', '--net-biome', '0', '--seed', '4', '--dev', 'quiet,pieces=2', '--talk', 'rushfood:2',
              '--input', '60:,' + '4:A,6:,' * 20 + '40:,0:place 404 -20 7,40:,6:A,6:,138:,6:B,34:,6:A,6:,243:,66:UP+LEFT+B,20:'],
     [(666, 'rush-bridge'), (864, 'rush-island')], {}),
    # (a security cube in Central, the navi who tells its P-Code, the code entered)
    ('cube', ['--scene', 'emu', '--run-depth', '3', '--net-biome', '0', '--seed', '4', '--dev', 'quiet,pieces=16', '--input',
              '300:,0:place 116 76 1,30:,6:A,150:,6:A,150:,6:A,40:,0:place 244 -276 5,30:,6:A,120:,6:A,150:,6:A,40:,0:place 116 76 1,30:,6:A,150:'],
     [(442, 'cube'), (962, 'cube-teller'), (1158, 'cube-code')], {}),
    # (the Undernet's doors: a skull door, then the WWW-ID shown; a number
    # door's question, two of the layer's braziers in view, the data beside
    # it taken first)
    ('doors-skull', ['--scene', 'emu', '--run-depth', '3', '--net-biome', '5', '--seed', '2', '--dev', 'quiet,pieces=16', '--talk', 'wwwid:700',
                     '--input', '300:,0:place 268 -268 7,30:,6:A,150:,6:A,150:,6:A,80:,6:A,150:,6:A,150:'],
     [(466, 'doors-skull'), (852, 'doors-skull-open')], {}),
    ('doors-number', ['--scene', 'emu', '--run-depth', '3', '--net-biome', '5', '--seed', '3', '--dev', 'quiet,pieces=16', '--input',
                      '300:,0:place 332 -140 3,30:,6:A,150:,6:A,150:,6:A,80:,0:place 332 -140 3,20:,6:A,150:,6:A,150:'],
     [(1017, 'doors-number')], {}),
    # (a purple data, locked; the Unlocker asked for; its chip)
    ('purple', ['--scene', 'emu', '--run-depth', '3', '--net-biome', '3', '--seed', '3', '--dev', 'quiet,pieces=1', '--talk', 'keys:100', '--input',
                '300:,0:place 52 -212 5,30:,6:A,150:,6:A,150:,6:A,150:'],
     [(450, 'purple'), (600, 'purple-unlock'), (740, 'purple-chip')], {}),
    # (L names a set piece once it has explained it: teleport pads explained
    # on layer 3, the dev menu's Next layer, then L on layer 4)
    ('sense', ['--scene', 'emu', '--run-depth', '3', '--net-biome', '3', '--seed', '10', '--dev', 'quiet,pieces=4', '--input',
               '1:,119:,6:L,6:,88:,' + '6:A,6:,88:,' * 13 + '10:SELECT,6:SELECT+R,4:SELECT,20:,' + '6:DOWN,6:,' * 7 +
               '10:,6:A,6:,388:,' + '6:A,6:,88:,' * 4 + '20:,6:L,6:,88:,6:A,6:,200:'],
     [(2700, 'sense')], {}),
    # 0.9.0's notes: Battle Network 5 beside BN6 (a name with "bn5" in it
    # gets the whole ROM folder, docs_rom_mounts). (BN5's areas on their
    # acts' third layers, MegaMan's first words of the older net paged
    # with A; Nebula Area's dark hole by MegaMan, placed beside it)
    ('bn5-oran', ['--scene', 'emu', '--net-biome', 'x4', '--run-depth', '3', '--seed', '3', '--dev', 'quiet', '--input', '100:,' + '4:A,6:,' * 30 + '300:'],
     [(620, 'bn5-oran')], {}),
    ('bn5-scilab', ['--scene', 'emu', '--net-biome', 'x1', '--run-depth', '3', '--seed', '4', '--dev', 'quiet', '--input', '100:,' + '4:A,6:,' * 30 + '300:'],
     [(620, 'bn5-scilab')], {}),
    ('bn5-undernet', ['--scene', 'emu', '--net-biome', 'x5', '--run-depth', '3', '--seed', '3', '--dev', 'quiet', '--input', '100:,' + '4:A,6:,' * 30 + '300:'],
     [(620, 'bn5-undernet')], {}),
    ('bn5-nebula', ['--scene', 'emu', '--net-biome', 'x3', '--run-depth', '3', '--seed', '5', '--dev', 'quiet', '--input',
                    '100:,' + '4:A,6:,' * 30 + '20:,0:place 20 172 3,200:'], [(560, 'bn5-nebula')], {}),
    # (KnightMan guarding ACDC Area's first act: the autopilot walks to
    # his arena, his card, his BN5 sprite, his own face in the chat, his
    # wrecking ball in BN5's battle; not the weak one, which holds him at 1 HP)
    ('bn5-guardian', ['--scene', 'emu', '--net-biome', 'x0', '--run-depth', '3', '--seed', '3', '--guardian', '28', '--dev', 'quiet'],
     [(1470, 'bn5-guardian-card'), (1545, 'bn5-guardian'), (1620, 'bn5-guardian-talk'), (2400, 'bn5-guardian-battle')],
     {'CYBERWORLD_AUTOPILOT': '1'}),
    # (NumberMan's Guardian Data giving his Soul, End Area; then a run
    # holding his Soul: BusterUp picked on BN5's Custom screen, UNITE, the
    # unison as the turn begins)
    ('bn5-soul', ['--scene', 'emu', '--net-biome', 'x2', '--run-depth', '3', '--seed', '3', '--guardian', '26', '--dev', 'quiet', '--talk', 'reward:420',
                  '--input', '100:,' + '4:A,6:,' * 30 + '100:,' + '6:A,94:,' * 12], [(700, 'bn5-soul'), (1080, 'bn5-soul-words')], {}),
    ('bn5-unite', ['--scene', 'emu', '--net-biome', 'x0', '--seed', '4', '--dev', 'quiet,souls=4', '--input',
                   '100:,' + '4:A,6:,' * 30 + '0:battle,160:,6:RIGHT,10:,6:RIGHT,10:,6:A,40:,6:RIGHT,8:,6:RIGHT,8:,6:RIGHT,10:,6:DOWN,40:,6:A,60:,6:UP,14:,6:A,500:'],
     [(710, 'bn5-unite'), (1040, 'bn5-unison')], {}),
    # (the older net's wait: BN5's first boot slowed, a battle at once)
    ('bn5-wait', ['--scene', 'emu', '--net-biome', 'x0', '--seed', '4', '--dev', 'quiet,slowboot=4', '--input', '300:battle'],
     [(480, 'bn5-wait'), (1050, 'bn5-wait-2'), (1650, 'bn5-wait-3')], {'CYBERWORLD_AUTOPILOT': '1'}),
    # (BN6's own DarkChips, on BN6's ROM alone: a flame on Central's middle
    # layer, Chaud's call paged, MegaMan placed by it and speaking to it;
    # DrkSword drawn in a battle's first hand, a BugFrag burnt for its 400,
    # then the 20 max HP it took)
    ('dark-flame', ['--scene', 'emu', '--net-biome', '0', '--run-depth', '2', '--seed', '3', '--dev', 'quiet', '--input',
                    '100:,' + '4:A,6:,' * 30 + '20:,0:place 20 -20 1,40:,6:A,140:,6:A,94:'], [(440, 'dark-flame'), (700, 'dark-flame-words')], {}),
    ('dark-chip', ['--scene', 'emu', '--net-biome', '0', '--run-depth', '1', '--seed', '4', '--dev', 'quiet,darkchips=0x2,folder=286/3', '--talk', 'bugfrags:420',
                   '--input', '100:,' + '4:A,6:,' * 30 + '40:,0:battle,220:,6:A,20:,6:START,20:,6:A,120:,6:RIGHT,20:,6:A,200:,6:A,60:,6:A,60:,6:A,60:,6:A,150:,'
                   + '6:A,100:,' * 10], [(650, 'dark-chip-custom'), (882, 'dark-chip'), (2160, 'dark-chip-price')], {}),
    # 0.9.0's notes, each by its feature. (MegaMan's word on GitHub, typed out)
    ('notice', ['--scene', 'intro'], [(230, 'notice')], {}),
    # (SELECT on the title: the controls screen, an Xbox pad attached)
    ('controls', ['--scene', 'title', '--pad', 'xbox', '--input', '60:,4:SELECT,40:'], [(100, 'controls')], {}),
    # (a Recovery Mr. Prog on an act's middle layer, MegaMan held at 40 HP:
    # half his max back, his one patch; asked again, spent)
    ('heal-once', ['--scene', 'emu', '--run-depth', '2', '--seed', '5', '--dev', 'quiet,hp=100/40', '--talk', 'heal:620,heal:1150',
                   '--input', '60:,' + '4:A,6:,' * 40 + '40:,290:,6:A,134:,6:A,134:,6:A,214:,6:A,94:'],
     [(900, 'heal-once'), (1060, 'heal-once-2'), (1270, 'heal-spent'), (1380, 'heal-spent-2')], {}),
    # (the setup's Help row on All *, turned on)
    ('all-star', ['--scene', 'setup', '--input', '60:,6:UP,20:,6:RIGHT,10:,6:RIGHT,10:,6:RIGHT,30:,6:A,60:'], [(200, 'all-star')], {}),
    # (a Seaside layer with a security cube and a Link Navi obstacle: SELECT's
    # map whole, both violet and the data's crystals; and as the layer
    # begins, the cube a ring until seen)
    ('map-marks', ['--scene', 'emu', '--run-depth', '3', '--net-biome', '1', '--seed', '11', '--dev', 'quiet,pieces=24,mapall',
                   '--input', '300:,60:SELECT,20:'], [(340, 'map-marks')], {}),
    ('map-marks-start', ['--scene', 'emu', '--run-depth', '3', '--net-biome', '1', '--seed', '11', '--dev', 'quiet,pieces=24',
                         '--input', '300:,60:SELECT,20:'], [(340, 'map-marks-start')], {}),
    # (seed 7's walk into BlastMan's arena: "Run saved" at its door, then
    # BlastMan in Gregar's own sprite; TenguMan in his battle sprite, and
    # his words with a face made from it)
    ('arena-save', ['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '11', '--guardian', '12'],
     [(1306, 'arena-save'), (1348, 'guardian-blastman')], {'CYBERWORLD_AUTOPILOT': 'weak'}),
    # (0.9.0's dialogue in BN6's voice: Lan answering MegaMan as they
    # arrive on the run's first layer)
    ('voice-banter', ['--scene', 'emu', '--run-depth', '1', '--seed', '4', '--dev', 'quiet', '--input', '700:,6:A,70:,6:A,70:,6:A,70:'],
     [(900, 'voice-banter')], {}),
    ('falzar-face', ['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '11', '--guardian', '8'],
     [(1348, 'guardian-tenguman'), (1600, 'falzar-face')], {'CYBERWORLD_AUTOPILOT': 'weak'}),
    # (the second screen following the game, issue #72: seed 7's Green Area
    # layer 2, its arrival's chats paged; the Custom screen's chip as a
    # card, the folder editor's whole folder, the NaviCustomizer's program)
    ('second-battle', ['--scene', 'emu', '--run-depth', '2', '--seed', '7', '--net-biome', '3', '--dev', 'quiet',
                       '--input', '600:,' + '4:A,6:,' * 110 + '60:,0:battle,400:'], [(2230, 'second-battle')], {}, '3ds'),
    ('second-folder', ['--scene', 'emu', '--run-depth', '2', '--seed', '7', '--net-biome', '3', '--dev', 'quiet',
                       '--input', '600:,' + '4:A,6:,' * 110 + '60:,6:START,40:,6:A,60:,6:A,60:,6:A,60:,6:DOWN,30:'], [(2080, 'second-folder')], {}, '3ds'),
    ('second-navicust', ['--scene', 'emu', '--run-depth', '2', '--seed', '7', '--net-biome', '3', '--dev', 'quiet,programs=8',
                         '--input', '600:,' + '4:A,6:,' * 110 + '60:,6:START,40:' + ',6:DOWN,8:' * 3 + ',6:A,60:,6:A,90:,6:DOWN,30:'],
     [(2070, 'second-navicust')], {}, '3ds'),
    # (the layer's map on a second screen, the guardian's mark on it)
    ('second-screen', ['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '11', '--guardian', '12'],
     [(1250, 'second-screen')], {'CYBERWORLD_AUTOPILOT': 'weak'}, '3ds'),
    # (a phone held upright, a thumb on the D-pad: the whole screen at a third)
    ('phone-upright', ['--scene', 'emu', '--run-depth', '4', '--seed', '3', '--net-biome', '1', '--dev', 'quiet', '--touch',
                       '--size', '1080x2400', '--dpi', '420', '--input', '60:,' + '4:A,6:,' * 20 + '40:',
                       '--taps', '330:315,1881>435,2001'], [(348, 'phone-upright')], {}, 3),
    # (the program pick after BlastMan: each program says whether it fits;
    # SuperArmor fits now)
    ('program-fit', ['--scene', 'emu', '--run-depth', '1', '--seed', '4', '--dev', 'quiet', '--talk', 'reward:2950',
                     '--input', (FIT_GIFT + FIT_INSTALL + FIT_NEXT + FIT_PAGES).rstrip(',')],
     [(7375, 'program-fit-now')], {}),
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
                      mounts=docs_rom_mounts(bn5='bn5' in name))
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
# (an act's first layer: the act's card and MegaMan's words paged with A)
PAGED = '60:,' + '4:A,6:,' * 20 + '40:,'
CLIPS = [
    ('title', ['--scene', 'title'], {}, None, 300, 780),
    ('net', RUN_7, {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 150, 600),
    # (the run's first random battle, on layer 6, from BATTLE START to the
    # RESULT window and its chip; the
    # walk into BlastMan's arena and his card; the Undernet on its act's
    # third layer, where no call from Chaud comes)
    ('battle', RUN_7, {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 7800, 8270),
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
    # (0.8.0's set pieces, each forced on an act's first layer, its arrival
    # paged with A, MegaMan placed at it: Rush bridging a gap and the walk
    # over him, a teleport pair, an arrow lane's ride, an invisible path)
    ('rush', ['--scene', 'emu', '--run-depth', '4', '--net-biome', '0', '--seed', '4', '--dev', 'quiet,pieces=2', '--talk', 'rushfood:2'], {},
     PAGED + '0:place 404 -20 7,40:,6:A,6:,138:,6:B,34:,6:A,6:,243:,66:UP+LEFT+B,20:', 446, 864),
    ('teleport', ['--scene', 'emu', '--run-depth', '4', '--net-biome', '3', '--seed', '2', '--dev', 'quiet,pieces=4'], {},
     PAGED + '0:place 276 -20 1,40:,70:UP+RIGHT,150:,40:DOWN,6:,6:A,6:,60:,6:A,6:,50:,6:A,6:,90:', 372, 800),
    ('arrows', ['--scene', 'emu', '--run-depth', '4', '--net-biome', '1', '--seed', '19', '--dev', 'quiet,pieces=32'], {},
     PAGED + '0:place 76 260 3,20:,130:DOWN+RIGHT,80:DOWN+LEFT,60:', 376, 565),
    ('hidden', ['--scene', 'emu', '--run-depth', '4', '--net-biome', '1', '--seed', '23', '--dev', 'quiet,pieces=64'], {},
     PAGED + '0:place -20 260 3,20:,170:DOWN+RIGHT,4:,6:A,12:,6:B,48:,6:A,6:,150:', 306, 650),
    # (HeatCross brought, a cybertree in Green: "Leave it to me!")
    ('obstacle', ['--scene', 'emu', '--run-depth', '3', '--net-biome', '3', '--seed', '3', '--dev', 'quiet,pieces=8', '--setup', 'endless,0,0,0,1'], {},
     '300:,0:place 20 108 5,20:,6:A,89:,6:A,134:,6:A,39:,6:A,60:,6:A,10:,70:DOWN+LEFT', 310, 729),
    # (0.9.0's: a random battle in BN5's engine on ACDC Area, from the
    # switch out of BN6's map through its first turn, the autopilot
    # fighting; UNITE with NumberMan's Soul on BN5's Custom screen to the
    # unison; DrkSword from a BN6 battle's first hand, its dark slash)
    ('bn5-battle', ['--scene', 'emu', '--net-biome', 'x0', '--seed', '4', '--dev', 'quiet'], {'CYBERWORLD_AUTOPILOT': '1'}, '460:battle', 436, 916),
    ('bn5-unite', ['--scene', 'emu', '--net-biome', 'x0', '--seed', '4', '--dev', 'quiet,souls=4'], {},
     '100:,' + '4:A,6:,' * 30 + '0:battle,160:,6:RIGHT,10:,6:RIGHT,10:,6:A,40:,6:RIGHT,8:,6:RIGHT,8:,6:RIGHT,10:,6:DOWN,40:,6:A,60:,6:UP,14:,6:A,500:', 550, 1030),
    ('dark-chip', ['--scene', 'emu', '--net-biome', '0', '--run-depth', '1', '--seed', '4', '--dev', 'quiet,darkchips=0x2,folder=286/3', '--talk', 'bugfrags:420'], {},
     '100:,' + '4:A,6:,' * 30 + '40:,0:battle,220:,6:A,20:,6:START,20:,6:A,120:,6:RIGHT,20:,6:A,200:', 606, 966),
    # (the older net's wait, BN5's first boot slowed to 12 of its frames a
    # frame: the switch, the wait filling, the battle opening behind it)
    ('bn5-wait', ['--scene', 'emu', '--net-biome', 'x0', '--seed', '4', '--dev', 'quiet,slowboot=12'], {'CYBERWORLD_AUTOPILOT': '1'}, '300:battle', 340, 820),
    # (0.9.0's start: the developer's boot screen, MegaMan's word on GitHub, the title)
    ('intro', ['--scene', 'intro'], {}, None, 1, 380),
]


# the clips the README or a release's notes show as GIFs too (GitHub plays no
# video from the repository): name, then its first seconds and frames a second
# (a walk's scrolling floor makes a GIF of all of it 2 MB)
README_GIFS = {'guardian': (None, 15), 'sky': (4, 12), 'weather': (4, 12), 'acdc-bn5': (4, 12),
               'rush': (None, 15), 'teleport': (None, 15), 'arrows': (None, 15), 'hidden': (None, 15), 'obstacle': (None, 15),
               'bn5-battle': (None, 15), 'bn5-unite': (None, 15), 'dark-chip': (None, 15), 'bn5-wait': (None, 15),
               'intro': (None, 15)}


def clips(only=None):
    """docs/clips/NAME.webm, .mp4 and .png: frames of scripted runs, 4x, 30 fps (and
    NAME.gif, 2x, at the rate README_GIFS gives, for those in it); a clip
    marked '3ds' as a New 3DS shows it, the second screen under the
    picture (two_screens), 2x and its GIF 1x."""
    from PIL import Image
    out = os.path.join(ROOT, 'docs', 'clips')
    tmp = os.path.join(ROOT, '.build', 'clips')
    os.makedirs(out, exist_ok=True)
    for name, args, env, script, first, last, *mode in CLIPS:
        if only and name not in only:
            continue
        dual = mode == ['3ds']
        shutil.rmtree(tmp, ignore_errors=True)
        os.makedirs(os.path.join(tmp, 'data'))
        os.makedirs(os.path.join(tmp, 'png'))
        saved = {k: os.environ.get(k) for k in env}
        os.environ.update(env)
        extra = ['--input', script] if script else []
        if dual:
            extra += ['--second-shot-range', f'{first}:{last}:/src/.build/clips/s']
        code = docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/clips/data',
                      *args, *extra, '--frames', str(last + 1), '--shot-range', f'{first}:{last}:/src/.build/clips/f',
                      mounts=docs_rom_mounts(bn5='bn5' in name))
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
            if dual:
                im = two_screens(im, Image.open(os.path.join(tmp, f's{f:05d}.bmp')).convert('RGB'))
            if n == 0:
                im.save(os.path.join(out, f'{name}.png'), optimize=True)
            im.save(os.path.join(tmp, 'png', f'{n:05d}.png'))
            n += 1
        # 4x with whole pixels (and the colour planes' 2x2 blocks inside them; two screens 2x)
        w, h = im.size
        big = f'{2 * w}:{2 * h}' if dual else f'{4 * w}:{4 * h}'
        common = ['-y', '-loglevel', 'error', '-framerate', '30', '-i', '/work/png/%05d.png',
                  '-vf', f'scale={big}:flags=neighbor', '-an']
        encodes = [common + ['-c:v', 'libvpx-vp9', '-b:v', '0', '-crf', '36', '-row-mt', '1', '-pix_fmt', 'yuv420p', f'/out/{name}.webm'],
                   common + ['-c:v', 'libx264', '-crf', '24', '-preset', 'slow', '-pix_fmt', 'yuv420p', '-movflags', '+faststart',
                             f'/out/{name}.mp4']]
        # (the GIF from the same frames: 2x with whole pixels, its palette the clip's own colours)
        if name in README_GIFS:
            secs, fps = README_GIFS[name]
            encodes.append(['-y', '-loglevel', 'error', '-framerate', '30', '-i', '/work/png/%05d.png', '-vf',
                            (f'trim=duration={secs},' if secs else '') + f'fps={fps},scale={w if dual else 2 * w}:{h if dual else 2 * h}:flags=neighbor,split[a][b];'
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


def lizard(*args):
    """lizard over src/ in the build image (its output)."""
    out = subprocess.run(['docker'] + (['--context', CONTEXT] if CONTEXT else []) +
                         ['run', '--rm', '-v', f'{ROOT}:/src', '-w', '/src', IMAGE, 'lizard', *args, 'src'],
                         capture_output=True, text=True)
    if out.returncode not in (0, 1) or not out.stdout:
        sys.exit('lizard failed in the build image: one made before it came in? docker rmi cyberworld-build, then again\n'
                 + out.stderr[-400:])
    return out.stdout


_lizard_rows = []


def lizard_rows():
    """Each function's row of lizard's CSV (nloc, ccn, tokens, params, length, location, file, name, long name,
    start, end), read once."""
    import csv
    if not _lizard_rows:
        _lizard_rows.extend(csv.reader(lizard('--csv').splitlines()))
    return _lizard_rows


def lint_complexity(update):
    """lizard over src/: a function past CCN LINT_CCN or LINT_NLOC lines of code is new,
    or a listed one grew."""
    now = {}
    for row in lizard_rows():
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
                           'sh', '-c', f'rm -f build/lint/test_core build/lint/test_emu; '
                           f'make -j{os.cpu_count() or 4} build/lint/test_core build/lint/test_emu 2>&1 >/dev/null; '
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


def lint_offsets(update):
    """No field of the game's structures as a bare offset outside bn6.h (issue #34): BN6_PLAYER + 0x1C is
    BN6_PLAYER_X there. Event flags (BN6_FLAG_ + n) and indexes (+ n * size) are numbers, not fields."""
    import glob
    from collections import Counter
    found = []
    for path in sorted(glob.glob(os.path.join(ROOT, 'src', '*', '*.[ch]'))):
        if os.path.basename(path) == 'bn6.h':
            continue
        with open(path) as f:
            for n, line in enumerate(f, 1):
                for m in re.finditer(r'\b(BN6_\w+)\s*\+\s*(0x[0-9A-Fa-f]+|\d+)\b(?!\s*\*)', line):
                    if not m.group(1).startswith('BN6_FLAG_'):
                        found.append(f'{os.path.relpath(path, ROOT)}: {m.group(0)}')
    return lint_compare('fields of the game\'s structures as bare offsets, outside src/emu/bn6.h', 'offsets.txt',
                        'bare offsets into bn6.h\'s structures (BN6_X + 0x..): name the field in bn6.h instead;\n'
                        'this list only shrinks: build.py lint --update after naming one', Counter(found), update)


def lint_symbols():
    """docs/symbols and SYMBOLS.md's numbers as tools/symbols.py writes them from
    the headers, rom.c and ROM_DATA.md, and every name in bn6.h and bn5.h
    described (docs/SYMBOLS.md); needs no bn6f."""
    sys.stdout.flush()
    return subprocess.call([sys.executable, os.path.join(ROOT, 'tools', 'symbols.py'), '--check']) == 0


# ---- the code's smells (AGENTS.md, Code structure) ----

SMELL_LINES = {'.c': 1000, '.m': 1000, '.h': 400}   # a file past this many lines holds more than one thing
SMELL_INCLUDES = 20     # a .c file including more of src/'s headers reaches into too many others
SMELL_PARAMS = 7        # a function taking more parameters wants a struct
SMELL_NESTING = 5       # a function's body indented deeper than this many levels wants a function of its own
SMELL_DUPLICATE = 10    # lines of a block repeated elsewhere (lizard -Eduplicate) want one function
# what characters say lives in its own files, apart from what decides when (docs/VOICE.md)
WORDS_FILES = ('_words.c', '_lines.c', '_text.c')
SMELLS = {
    'size': f'a file past {SMELL_LINES[".c"]} lines (.c) or {SMELL_LINES[".h"]} (.h): split it by what it does',
    'words': 'lines of speech outside a words file (*_words.c, *_lines.c, *_text.c): move them to the feature\'s words file',
    'includes': f'a .c file including more than {SMELL_INCLUDES} of src/\'s headers: it does too much; split it',
    'params': f'a function taking more than {SMELL_PARAMS} parameters: pass a struct',
    'nesting': f'a function indented past {SMELL_NESTING} levels: give the inner part a function of its own',
    'duplicate': f'blocks of {SMELL_DUPLICATE} lines or more repeated between two files (or in one): one function for both',
}
SPEECH_LITERAL = re.compile(r'"((?:[^"\\]|\\.)*)"')
SPEECH_CALL = re.compile(r'\bta_(?:say|talk|page|pages|say_flag)\s*\(')


def generated(path):
    """A file a tool writes (its first lines name it), which no smell counts."""
    with open(path, errors='replace') as f:
        head = ''.join(f.readline() for _ in range(3))
    return re.search(r'(written|drawn|made) by tools/', head) is not None


def speech_lines(path):
    """The lines of `path` holding a line a character says: a literal with a
    speaker's mark (@M ), chat boxes split by |, BN6's comma (\"Lan,look!\"),
    or words given to a chat (ta_say and the like)."""
    n = 0
    with open(path, errors='replace') as f:
        for line in f:
            s = line.strip()
            if s.startswith(('/*', '*', '//')):
                continue
            for m in SPEECH_LITERAL.finditer(line):
                lit = m.group(1)
                if '*.' in lit:   # (a file dialog's pattern)
                    continue
                if (re.search(r'@[A-Z] ', lit) or re.search(r'[A-Za-z!?.]\|[A-Za-z@*.]', lit) or
                        (re.search(r'[a-z!.?],[A-Z][a-z]', lit) and ' ' in lit) or
                        (SPEECH_CALL.search(line) and re.search(r'[A-Za-z]{2,} [A-Za-z]', lit))):
                    n += 1
                    break
    return n


def nesting(path, start, end):
    """How deep the body of the function on lines start-end is indented (its
    statements are level 1), its comments and preprocessor lines aside."""
    with open(os.path.join(ROOT, path), errors='replace') as f:
        lines = f.read().split('\n')[start - 1:end]
    base, deep = len(lines[0]) - len(lines[0].lstrip('\t')), 0
    for line in lines[1:]:
        s = line.lstrip('\t')
        if s.strip() and not s.lstrip().startswith(('/*', '*', '#', '//')):
            deep = max(deep, len(line) - len(s) - base)
    return deep


def smells_found():
    """Each smell found, as "kind where" -> its measure."""
    import glob
    found = {}
    files = sorted(glob.glob(os.path.join(ROOT, 'src', '*', '*.[chm]')))
    made = {os.path.relpath(p, ROOT) for p in files if generated(p)}
    for path in files:
        rel = os.path.relpath(path, ROOT)
        if rel in made:
            continue
        with open(path, errors='replace') as f:
            text = f.read()
        lines = text.count('\n')
        if lines > SMELL_LINES[os.path.splitext(rel)[1]]:
            found[f'size {rel}'] = lines
        if rel.endswith('.c'):
            n = len(re.findall(r'^#include "', text, re.M))
            if n > SMELL_INCLUDES:
                found[f'includes {rel}'] = n
            if not rel.endswith(WORDS_FILES):
                n = speech_lines(path)
                if n:
                    found[f'words {rel}'] = n
    for row in lizard_rows():
        file, name, params, start, end = row[6], row[7], int(row[3]), int(row[9]), int(row[10])
        if file in made:
            continue
        if params > SMELL_PARAMS:
            found[f'params {file}:{name}'] = max(found.get(f'params {file}:{name}', 0), params)
        deep = nesting(file, start, end)
        if deep > SMELL_NESTING:
            found[f'nesting {file}:{name}'] = max(found.get(f'nesting {file}:{name}', 0), deep)
    for block in lizard('-Eduplicate').split('Duplicate block:')[1:]:
        # (code only: in headers lizard takes one long enum for another)
        spans = [(f, int(a), int(b)) for f, a, b in re.findall(r'(src/\S+):(\d+) ~ (\d+)', block) if f not in made and f.endswith('.c')]
        if len(spans) < 2:
            continue
        length = max(b - a + 1 for _, a, b in spans)
        if length < SMELL_DUPLICATE:
            continue
        pair = ' '.join(sorted({f for f, _, _ in spans})) if len({f for f, _, _ in spans}) > 1 else spans[0][0]
        found[f'duplicate {pair}'] = found.get(f'duplicate {pair}', 0) + length
    return found


def lint_smells(update):
    """The code's smells against tests/lint/smells.txt: none may join, and
    none listed may grow (a file longer, a function deeper, more lines of
    speech where they do not belong)."""
    now = smells_found()
    note = ('the code\'s smells (build.py lint, AGENTS.md): "kind where measure"; none may join and none may grow:\n' +
            '\n'.join(f'  {k}: {v}' for k, v in SMELLS.items()) +
            '\nfix one, then build.py lint --update')
    if update:
        lint_write('smells.txt', note, [f'{k} {v}' for k, v in now.items()])
        print(f'smells: {len(now)} listed in tests/lint/smells.txt')
        return True
    base = {}
    for line in lint_baseline('smells.txt'):
        key, v = line.rsplit(' ', 1)
        base[key] = int(v)
    bad = 0
    for key, v in sorted(now.items()):
        if key not in base:
            print(f'  new: {key} ({v}): {SMELLS[key.split()[0]]}')
            bad += 1
        elif v > base[key]:
            print(f'  grew: {key} {base[key]} -> {v}')
            bad += 1
    fixed = [k for k in base if k not in now or now[k] < base[k]]
    if fixed:
        print(f'  ({len(fixed)} smaller or gone since the baseline: build.py lint --update takes them down)')
    print(f'smells: {len(now)}, {bad} new or grown')
    return not bad


def lint_headers():
    """Every header guarded and whole on its own (it compiles alone, as the
    host build compiles), and every .c file's own header the first it
    includes, so that it is checked so too."""
    import glob
    bad = []
    for h in sorted(glob.glob(os.path.join(ROOT, 'src', '*', '*.h'))):
        with open(h, errors='replace') as f:
            text = f.read()
        if not re.search(r'^#ifndef (\w+)\n#define \1$', text, re.M) and '#pragma once' not in text:
            bad.append(f'{os.path.relpath(h, ROOT)}: no include guard')
    for c in sorted(glob.glob(os.path.join(ROOT, 'src', '*', '*.c'))):
        own = os.path.basename(c)[:-2] + '.h'
        if not os.path.exists(os.path.join(os.path.dirname(c), own)):
            continue
        with open(c, errors='replace') as f:
            first = next((line.strip() for line in f if line.startswith('#include')), '')
        if first != f'#include "{own}"':
            bad.append(f'{os.path.relpath(c, ROOT)}: includes {first or "nothing"} before its own {own}')
    build('host')   # (its version.h)
    script = ('for h in src/*/*.h; do gcc -std=c11 -fsyntax-only -D_DEFAULT_SOURCE -DCW_DESKTOP '
              '$(for d in src/*/; do printf -- "-I%s " "$d"; done) -Ibuild/host/gen $(pkg-config --cflags sdl2 | sed "s/-I/-isystem /g") '
              '-isystem /opt/mgba/host/include -Wno-pragma-once-outside-header -x c "$h" 2>&1 | grep -q "error" && echo "$h"; done; true')
    out = subprocess.run(['docker'] + (['--context', CONTEXT] if CONTEXT else []) +
                         ['run', '--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src', IMAGE,
                          'sh', '-c', script], capture_output=True, text=True).stdout
    bad += [f'{h}: does not compile on its own' for h in out.split()]
    for b in bad:
        print(f'  refused: {b}')
    print(f'headers: {len(bad)} refused')
    return not bad


def symbols(rest=()):
    """docs/symbols from the sources (tools/symbols.py). With --bn6f DIR, a
    checkout of the bn6f disassembly, the full map too: tools/bn6f_match.py
    locates its functions in the ROMs (--roms DIR, else the usual ROM
    folder; a few minutes, on the host), then tools/symbols.py --full writes
    .build/symbols and SYMBOLS.md's counts. bn6f's names stay out of git
    (docs/SYMBOLS.md)."""
    ap = argparse.ArgumentParser(prog='build.py symbols')
    ap.add_argument('--bn6f', metavar='DIR', help='a checkout of github.com/dism-exe/bn6f: the full map in .build/symbols too')
    ap.add_argument('--roms', metavar='DIR', default=default_rom_dir(), help='the ROMs, told by their SHA-1')
    a = ap.parse_args(rest)
    tools = os.path.join(ROOT, 'tools')
    if not a.bn6f:
        return subprocess.call([sys.executable, os.path.join(tools, 'symbols.py')])
    code = subprocess.call([sys.executable, os.path.join(tools, 'bn6f_match.py'), '--bn6f', a.bn6f, '--roms', a.roms])
    return code or subprocess.call([sys.executable, os.path.join(tools, 'symbols.py'), '--full'])


def lint(update=False):
    ok = lint_files()
    ok = lint_rom_data(update) and ok
    ok = lint_offsets(update) and ok
    ok = lint_symbols() and ok
    ok = lint_complexity(update) and ok
    ok = lint_smells(update) and ok
    ok = lint_headers() and ok
    ok = lint_analyzer(update) and ok
    ok = lint_dead(update) and ok
    return 0 if ok else 1


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('action', nargs='?', default='all', choices=['all', 'host', 'device', 'linux', 'windows', 'android', 'macos', 'ios', 'flatpak', '3ds', 'run', 'web', 'serve', 'release', 'package', 'shot', 'asan', 'test', 'lint', 'clean', 'atlas', 'tiles', 'tour', 'pacing', 'screenshots', 'clips', 'town', 'world', 'symbols'])
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
    if a.action == 'symbols':
        sys.exit(symbols(a.rest))
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
    if a.action == 'ios':
        ios_release('--simulator' in a.rest)
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
