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
                                .build/tour (docs/devtools/)
  python3 build.py atlas [BIOMES] [SEEDS] [--baseline]
                                every area's layers drawn, one sheet per area
                                in .build/atlas, compared with (or written to,
                                --baseline) tests/atlas_baseline.txt
                                (docs/devtools/); BIOMES intoN (xN another
                                game's area): a layer of every other area
                                as a Glitch layer's flip into N draws it,
                                fromN: one of N's in every other area's look
                                (docs/game/glitch-layer.md)
  python3 build.py tiles [SEEDS] [--baseline]
                                the tile test: the atlas of every area and
                                layout (SEEDS each, 2 by default), a table of
                                each area's tiles drawn with other floors
                                than they show and seams, close-ups of them
                                in .build/atlas/defects_bNN.png; fails where a
                                layer got worse than tests/atlas_baseline.txt
                                (docs/devtools/)
  python3 build.py sizes [SEEDS]
                                the layers' sizes: every area's layers over
                                runs of seeds 1 to SEEDS (300), every depth
                                and side layer's kind, the largest tile map
                                and coordinate data against their regions;
                                fails where one is past it
                                (docs/engine/emulation/free-space.md)
  python3 build.py pacing      every act's battles and guardians against
                                their bands, and the battles per layer and
                                act played, in .build/pacing.txt
  python3 build.py economy [RUNS]  what a run hands out by act (chips, good
                                chips, codes, zenny, Mystery Data) against
                                their bands, in .build/economy.txt
  python3 build.py npc-anims    every NPC in another game's sprite (BN5's
                                and BN4's guardians, Darkloids, teammates,
                                bystanders, map objects): each animation
                                our code tells it to play there, and drawn
                                where it stands in it, in .build/npc-anims.txt;
                                local only, the ROMs (docs/devtools/)
  python3 build.py xdiff [SPEC ...] [--mutations]
                                the differential (issue #82): a family moved
                                into BN6's engine against its own game's
                                battle, frame by frame (tools/xdiff.py, the
                                specs and masks in tests/xdiff); local only,
                                both ROMs
                                (docs/devtools/)
  python3 build.py screenshots  the game captured headlessly for the site and
                                the README, into docs/screenshots
  python3 build.py clips [NAMES] short videos of the game for the site
                                (WebM, MP4 and a poster), into docs/clips
  python3 build.py town [SEEDS] the run's town drawn for a few seeds, and
                                the game shown around it, in .build/town
                                (docs/world/overworld.md)
  python3 build.py world        the real world's original maps drawn with
                                their walls and triggers, and the game
                                warped through them, in .build/world
  python3 build.py test         ROM-free unit tests
  python3 build.py server       build the ROM-free server zipapp
  python3 build.py server-run   run the built server (arguments follow)
  python3 build.py server-test  server/protocol tests, without game dependencies
  python3 build.py online-pair --rom PATH --state PATH
                                BN6's Practice NetBattle on one linked pair of
                                cores, a snapshot of the pair restored and
                                replayed midway (docs/online/link.md; the ROM
                                and a boot-4.state, both read-only)
  python3 build.py symbols [--bn6f DIR [--roms DIR]]
                                what the project has mapped of BN6 Gregar and
                                BN5 Team Colonel, in docs/symbols (a symbol
                                file for mGBA and no$gba, JSON and CSV per ROM;
                                docs/rom/symbols.md), from the sources
                                (tools/symbols.py); with a bn6f checkout, the
                                full map with bn6f's functions located in the
                                ROMs too, in .build/symbols, never committed
                                (tools/bn6f_match.py)
  python3 build.py lint [--update]
                                the code's checks (issue #19): GCC's analyzer,
                                the functions no build reaches, lizard's
                                complexity, the ROM offsets docs/rom/
                                names, the files git tracks, the layers'
                                includes (tools/layers.py), each against its
                                baseline in tests/lint (--update writes them
                                anew), and the generated files' sources and
                                that a branch leaves those files to dev
                                (tools/generated.py); a new finding fails
  python3 build.py package      build/port/cyberworld (for PortMaster-New) and the port's zip
"""
import argparse
import os
import re
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))


def keep_pil_nearest():
    """Pillow's nearest resize kept working where a Pillow build breaks it
    (tools/pil_nearest.py; Ubuntu's python3-pil 12.1.1-2ubuntu1.5)."""
    sys.path.insert(0, os.path.join(ROOT, 'tools'))
    try:
        import pil_nearest  # noqa: F401
    finally:
        sys.path.pop(0)
IMAGE = 'cyberworld-build'
LINUX_IMAGE = 'cyberworld-linux'   # docker/Dockerfile.linux
WEB_IMAGE = 'cyberworld-web'       # docker/Dockerfile.web
WINDOWS_IMAGE = 'cyberworld-windows'   # docker/Dockerfile.windows
ANDROID_IMAGE = 'cyberworld-android'   # docker/Dockerfile.android
N3DS_IMAGE = 'cyberworld-3ds'   # docker/Dockerfile.3ds
SWITCH_IMAGE = 'cyberworld-switch'   # docker/Dockerfile.switch
VITA_IMAGE = 'cyberworld-vita'   # docker/Dockerfile.vita
PSP_IMAGE = 'cyberworld-psp'   # docker/Dockerfile.psp
PS4_IMAGE = 'cyberworld-ps4'   # docker/Dockerfile.ps4
PS5_IMAGE = 'cyberworld-ps5'   # docker/Dockerfile.ps5
PORTMASTER_IMAGE = 'cyberworld-portmaster'   # docker/Dockerfile.portmaster
IMAGES = {IMAGE: 'Dockerfile', LINUX_IMAGE: 'Dockerfile.linux', WEB_IMAGE: 'Dockerfile.web',
          WINDOWS_IMAGE: 'Dockerfile.windows', ANDROID_IMAGE: 'Dockerfile.android', N3DS_IMAGE: 'Dockerfile.3ds', SWITCH_IMAGE: 'Dockerfile.switch', VITA_IMAGE: 'Dockerfile.vita', PSP_IMAGE: 'Dockerfile.psp', PS4_IMAGE: 'Dockerfile.ps4', PS5_IMAGE: 'Dockerfile.ps5',
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
BN5_SHA1 = '5f472f78d8de2df01d5039e045c043cb40969a39'   # Team Colonel (USA), optional beside it (docs/multirom/)
BN4_SHA1 = 'a97e96a7da03abd70f7953328e511b9fa29179f1'   # Red Sun (USA), read for research until it is offered (docs/multirom/bn4.md)


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


def rom_mounts_no_bn4():
    """BN6's and BN5's ROMs alone, for a picture of the NetPass with BN4's
    slot open (the ROM folder's files by their hashes)."""
    import glob
    import hashlib
    out = []
    for path in sorted(glob.glob(os.path.join(default_rom_dir(), '*.gba'))):
        with open(path, 'rb') as f:
            sha1 = hashlib.sha1(f.read()).hexdigest()
        if sha1 in (BN6_SHA1, BN5_SHA1):
            out.append((path, '/rom/bn6.gba:ro' if sha1 == BN6_SHA1 else '/rom/bn5.gba:ro'))
    return out


def whole_rom_folder(name):
    """A picture or clip that sees the whole ROM folder (docs_rom_mounts):
    one named for BN5 or BN4, whose areas, Navis or bracket it shows, and
    the Glitch layer's, whose flips were timed with every ROM beside BN6's."""
    return 'bn5' in name or 'bn4' in name or name.startswith('glitch') or name in WHOLE_ROMS_A


DOCKER_RUNS = [0]


def docker_run():
    """A container's start, `docker run`; with CYBERWORLD_DOCKER_NAME=NAME each
    one named NAME-PID-N, so work side by side tells its own containers from
    another's."""
    args = ['docker'] + (['--context', CONTEXT] if CONTEXT else []) + ['run']
    if os.environ.get('CYBERWORLD_DOCKER_NAME'):
        DOCKER_RUNS[0] += 1
        args += ['--name', f"{os.environ['CYBERWORLD_DOCKER_NAME']}-{os.getpid()}-{DOCKER_RUNS[0]}"]
    return args


def docker(*cmd, mounts=(), image=IMAGE, env=()):
    args = docker_run() + ['--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src']
    for host, guest in mounts:
        args += ['-v', f'{host}:{guest}']
    for name, value in env:
        args += ['-e', f'{name}={value}']
    for var in ('CYBERWORLD_AUDIO_DUMP', 'CYBERWORLD_SOUND_MUTE', 'CYBERWORLD_SOUND_PHASE', 'CYBERWORLD_SFX_LOG', 'CYBERWORLD_AUDIO_OFFLINE', 'CYBERWORLD_EMU_DEBUG', 'CYBERWORLD_FREE_SPACE', 'CYBERWORLD_WATCH', 'CYBERWORLD_BTRACE', 'CYBERWORLD_AUTOPILOT_BACK', 'CYBERWORLD_AUTOPILOT_ROUTE', 'CYBERWORLD_TILE_AT', 'CYBERWORLD_VOIDS', 'CYBERWORLD_AUTOPILOT', 'CYBERWORLD_TOWN_DEBUG', 'CYBERWORLD_TOWN_STYLE', 'CYBERWORLD_TOWN_VARIANT', 'CYBERWORLD_TOWN_TILE', 'CYBERWORLD_TOWN_START', 'CYBERWORLD_EMU_THREAD', 'CYBERWORLD_XDRAWS', 'CYBERWORLD_XBRACKET_MUTATE', 'CYBERWORLD_XBRACKET_FACES', 'CYBERWORLD_BATTLE_PROBE', 'CYBERWORLD_ARENA_LOOKS_PROBE'):
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
    image = {'linux': LINUX_IMAGE, 'web': WEB_IMAGE, 'windows': WINDOWS_IMAGE, '3ds': N3DS_IMAGE, 'switch': SWITCH_IMAGE, 'vita': VITA_IMAGE, 'psp': PSP_IMAGE, 'ps4': PS4_IMAGE, 'ps5': PS5_IMAGE,
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
# kind says it (.dmg, .apk, .deb, .flatpak, .cia, .3dsx, .nro, .vpk, .pkg).
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


PSP_ZIP = 'cyberworld-endless-psp.zip'


def psp_release():
    """build/psp/cyberworld-endless-psp.zip: the memory stick's PSP/GAME/cyberworld-endless/
    with the EBOOT, an empty rom/ for BN6, and the licenses; gpSP, which the PSP's
    build links, is GPL-2.0, so the source's place goes with it (psp/README.md)."""
    import zipfile
    out = os.path.join(ROOT, 'build', 'psp')
    with open(os.path.join(ROOT, 'docker', 'gpsp.sh'), encoding='utf-8') as f:
        commit = re.search(r'^COMMIT=([0-9a-f]+)', f.read(), re.M)[1]
    tag = os.environ.get('TAG', '')
    ref = tag if tag.startswith('v') else 'dev'
    source = (f'Cyberworld Endless {version()} for the PSP\n\n'
              'This build links libretro gpSP (GPL-2.0, licenses/gpSP.txt) and parts of\n'
              'mGBA (MPL-2.0, licenses/mGBA.txt); the game is MIT (LICENSE.txt). It is\n'
              'distributed under the GPL\'s terms. Its source:\n\n'
              f'  the game:  https://github.com/{REPO}/tree/{ref}\n'
              f'  gpSP:      https://github.com/libretro/gpsp/tree/{commit}\n'
              '             with docker/gpsp-cyberworld.patch from the game\'s source over it\n'
              '             (docker/gpsp.sh builds it; psp/README.md builds the rest)\n')
    base = 'PSP/GAME/cyberworld-endless/'
    archive = os.path.join(out, PSP_ZIP)
    with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED) as z:
        z.write(os.path.join(out, 'EBOOT.PBP'), base + 'EBOOT.PBP')
        z.writestr(base + 'rom/', '')
        z.write(os.path.join(ROOT, 'LICENSE'), base + 'LICENSE.txt')
        for name in ('gpSP.txt', 'mGBA.txt'):
            z.write(os.path.join(out, 'licenses', name), base + 'licenses/' + name)
        z.writestr(base + 'SOURCE.txt', source)
    print('released', archive)


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
SITE_PAGES = ('', 'download/', 'play/', 'faq/', 'versions/')
# (each page's Japanese one: web/ja/, the same pages; tests/test_site.py keeps them in step)
SITE_JA = 'ja/'


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
    for faq in ('faq', SITE_JA + 'faq'):
        faq_structured(os.path.join(out, faq, 'index.html'))
    # (every release's notes on the versions page, and its feed)
    sys.path.insert(0, os.path.join(ROOT, 'tools'))
    import site_versions
    site_versions.build(out)
    if analytics:
        # (each page and its Japanese one, each naming both as their hreflang
        # links do, the English one the default)
        with open(os.path.join(out, 'sitemap.xml'), 'w') as f:
            f.write('<?xml version="1.0" encoding="UTF-8"?>\n<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9"'
                    ' xmlns:xhtml="http://www.w3.org/1999/xhtml">\n')
            for page in SITE_PAGES:
                alternates = ''.join(f'    <xhtml:link rel="alternate" hreflang="{code}" href="{SITE_URL}{where}{page}"/>\n'
                                     for code, where in (('en', ''), ('ja', SITE_JA), ('x-default', '')))
                for where in ('', SITE_JA):
                    f.write(f'  <url>\n    <loc>{SITE_URL}{where}{page}</loc>\n{alternates}  </url>\n')
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
    told by their SHA-1: BN6's at /.dev/rom.gba, BN5's at /.dev/bn5.gba, BN4 Red Sun's at /.dev/bn4.gba (the
    page itself takes only the files a player chooses; BN4's only with ?dev=showbn4 until it is offered)."""
    import glob
    import hashlib
    import http.server
    import functools
    out = site()
    roms = {}
    for path in sorted(glob.glob(os.path.join(default_rom_dir(), '*.gba'))):
        with open(path, 'rb') as f:
            digest = hashlib.sha1(f.read()).hexdigest()
        for url, sha1 in (('/.dev/rom.gba', BN6_SHA1), ('/.dev/bn5.gba', BN5_SHA1), ('/.dev/bn4.gba', BN4_SHA1)):
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
    beside cyberworld/, which holds the binary, update.py (the game's own update), one license
    per part and rom/."""
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
    # (the game's own update, put in place by the launcher once the game ends: docs/engine/updates.md)
    shutil.copyfile(os.path.join(ROOT, 'port', 'update.py'), os.path.join(game, 'update.py'))
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
    keep_pil_nearest()
    import collections
    import glob
    import re
    if biomes.startswith(('into', 'from')):   # a Glitch layer's looks (docs/game/glitch-layer.md, Checking an area's look)
        for path in sorted(glob.glob(os.path.join(out, 'look_*.bmp'))):
            im = Image.open(path).convert('RGB')
            im.crop(im.getbbox() or (0, 0, im.width, im.height)).save(path[:-4] + '.png')
            os.remove(path)
        print(open(os.path.join(out, 'report.txt')).read(), end='')
        print('looks in .build/atlas: look_intoNN_fromNN_*.png, the layer as built on the left, the flip on the right')
        return 0
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
    # a backdrop the game would load over the layer's tiles, drawn and turned over its floor in the
    # game though no picture here shows it (docs/rom/bn4.md, Net maps in 256 colours)
    over = [l for l in report.splitlines() if 'BACKDROP OVER TILES' in l]
    flagged = [l for l in report.splitlines() if 'NOT BUILT' in l or 'arena NO' in l or
               ('BACKDROP' not in l and float(re.search(r'fallback ([\d.]+)%', l).group(1)) > 1.0)]
    # a service or navi beside a panel-wide stretch of the floor as drawn: in the way on
    mouths = [l for l in report.splitlines() if re.search(r'at mouths [1-9]', l)]
    print(f'{len(layers)} sheets in .build/atlas; {len(flagged)} layers flagged, {len(mouths)} with a service at a walkway\'s mouth, '
          f'{len(over)} with a backdrop over their tiles')
    for l in flagged + mouths + over:
        print('  !', l)
    return compare_baseline(report, write=baseline) or (1 if mouths or over else 0)


BASELINE = os.path.join(ROOT, 'tests', 'atlas_baseline.txt')
# how much worse a layer may get than the baseline before the atlas fails
TOLERANCE = {'near': 2.0, 'fallback': 0.15, 'seams': 1.10, 'inexact': 1.10, 'other': 1.10, 'off': 1.10, 'breaks': 1.10, 'nicks': 1.10}
AREAS = ['Central', 'Seaside', 'Sky', 'Green', 'Graveyard', 'Undernet', 'Secret', 'Nest', 'Comp', 'Homepage', 'Comp B',
         'Robot Comp', 'Aquarium Comp', 'Judge Comp', 'Weather Comp', 'CopyBot Comp', 'ACDC HP', 'Green HP', 'Sky HP',   # src/core/run.h
         # (then another game's areas, where its ROM is beside BN6's: src/core/rom.c's bn5_areas, its Liberation
         # Missions' maps and bn4_areas, held out of runs; docs/multirom/)
         'BN5 ACDC', 'BN5 SciLab', 'BN5 End', 'BN5 Nebula', 'BN5 Oran', 'BN5 Undernet'] + [f'BN5 dark {n}' for n in range(1, 10)] + [
         'BN4 ACDC', 'BN4 Town', 'BN4 Park', 'BN4 Countries', 'BN4 Undernet']


def tiles(seeds='2', baseline=False):
    """The tile test: every area's layers drawn (the atlas), and per area how many tiles were
    drawn with other floors than they show and how many seams show; nonzero where a layer got
    worse than the baseline."""
    import collections
    code = atlas('all', seeds, baseline=baseline)
    report = os.path.join(ROOT, '.build', 'atlas', 'report.txt')
    if not os.path.exists(report):
        return code or 1
    agg = collections.defaultdict(lambda: [0, 0, 0, 0, 0, 0, 0])
    for l in open(report):
        m = re.match(r'biome +(\d+) .*?: (\d+) panels.*seams (\d+), breaks (\d+), nicks (\d+), off near (\d+)', l)
        if m:
            a = agg[int(m[1])]
            a[0] += 1; a[1] += int(m[2]); a[2] += int(m[3]); a[3] += int(m[6]); a[4] += int(m[4])
            # (the holes in its floor's top the safety net found, and those it left: src/map/voids.h)
            v = re.search(r'voids (\d+), holes left (\d+)', l)
            if v:
                a[5] += int(v[1]); a[6] += int(v[2])
    print()
    print(f'{"area":16} {"layers":>6} {"panels":>7} {"breaks /100":>12} {"off /100":>9} {"seams /100":>11} {"holes":>6} {"left":>5}   why off')
    total = [0, 0, 0, 0, 0, 0]
    for b, (n, panels, seams, off, breaks, voids, left) in sorted(agg.items()):
        why = ', '.join(f'{WHY_NAMES[k]} {v}' for k, v in enumerate((WHY or {}).get(b, [])) if v) if WHY else ''
        print(f'{AREAS[b] if b < len(AREAS) else b:16} {n:6} {panels:7} {100 * breaks / panels:12.1f} {100 * off / panels:9.1f} '
              f'{100 * seams / panels:11.1f} {voids:6} {left:5}   {why}')
        total[0] += panels; total[1] += off; total[2] += seams; total[3] += breaks; total[4] += voids; total[5] += left
    print(f'{"all":16} {"":6} {total[0]:7} {100 * total[3] / total[0]:12.1f} {100 * total[1] / total[0]:9.1f} {100 * total[2] / total[0]:11.1f}'
          f' {total[4]:6} {total[5]:5}')
    print('breaks: tile edges across which the picture shows a strip of pixels no original map shows at a tile\'s edge, a panel'
          ' line cut off or a block of another shade (yellow in the seams pictures, listed in .build/atlas/breaks_*.txt);'
          ' off: tiles drawn with other floors than they show (framed magenta, close-ups in .build/atlas/defects_bNN.png);'
          ' seams: tiles meeting as no original map sets them (red); holes: tiles whose floor\'s top showed the void'
          ' (src/map/voids.h), framed white where the safety net mended them, blue where it left one (left)')
    return code


def layer_sizes(seeds='300'):
    """The layers' sizes against their regions (src/dev/layer_sizes.c): every area's layers over
    runs of seeds 1 to SEEDS, at every depth and side layer's kind, an area a container, side by
    side; the largest tile map and coordinate data, nonzero where either is past its region
    (docs/engine/emulation/free-space.md, Moving regions)."""
    import concurrent.futures
    import glob
    import re
    out = os.path.join(ROOT, '.build', 'sizes')
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    rom_dir = default_rom_dir()

    def one(area):
        with open(os.path.join(out, f'log{area}.txt'), 'w') as log:
            args = docker_run() + ['--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src',
                                   '-v', f'{rom_dir}:/rom:ro', IMAGE, 'build/host/cyberworld', '--headless', '--rom-dir', '/rom',
                                   '--data-dir', f'/src/.build/sizes/data{area}', '--atlas', f'/src/.build/sizes:sizes={area}:{seeds}']
            return subprocess.call(args, stdout=log, stderr=log)
    # (BN6's areas, then the other games' read beside it: 39 at most, those absent written as none)
    with concurrent.futures.ThreadPoolExecutor(os.cpu_count() or 4) as ex:
        codes = list(ex.map(one, range(39)))
    largest = [0, 0, '', '']
    for path in sorted(glob.glob(os.path.join(out, 'sizes=*.txt')), key=lambda p: int(re.search(r'=(\d+)', p)[1])):
        for line in open(path):
            m = re.match(r'area +(\d+): (\d+) layers built, (\d+) not; tile map (\d+) .*coordinates (\d+) with pads', line)
            if m and int(m[2]):
                print(line.rstrip())
                if int(m[4]) > largest[0]:
                    largest[0], largest[2] = int(m[4]), line.split(';')[1].split(',')[0].strip()
                if int(m[5]) > largest[1]:
                    largest[1], largest[3] = int(m[5]), line.split(',', 2)[-1].strip()
            m = re.match(r'largest: tile map \d+ of (\d+), coordinates \d+ of (\d+)', line)
            if m:
                rooms = int(m[1]), int(m[2])
    print(f'largest: tile map {largest[0]} of {rooms[0]}, coordinates {largest[1]} of {rooms[1]} '
          f'(a quarter more: {largest[1] * 5 // 4})')
    return 1 if any(codes) or largest[0] > rooms[0] or largest[1] > rooms[1] else 0


def bmp_sheet(paths, out, cols=3):
    """The game's frames (240 x 160 in the middle of each capture) in a sheet."""
    from PIL import Image
    keep_pil_nearest()
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
    keep_pil_nearest()
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
    keep_pil_nearest()
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
        # (the breaks and nicks in a layer's picture, where the line has them: a baseline written before they
        # were counted holds none, and none is compared)
        for k in ('breaks', 'nicks'):
            if re.search(k + r' \d', l):
                out[m.groups()][k] = get(k)
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
                    '# shows on its floors, tiles drawn with other floors than they show, and the breaks and nicks\n'
                    '# in its picture (tile edges across which it shows what no original map shows there, half an\n'
                    '# edge or more and less); the atlas fails when one gets worse\n')
            for k in sorted(old, key=lambda k: tuple(int(v) for v in k)):
                v = old[k]
                f.write(f'biome {k[0]} layout {k[1]} (x) depth {k[2]} seed {k[3]}: near {v["near"]}, '
                        f'fallback {v["fallback"]}, seams {v["seams"]:.0f}, not exact {v["inexact"]:.0f}, '
                        f'other colours {v["other"]:.0f}, off near {v.get("off", 0):.0f}, '
                        f'breaks {v.get("breaks", 0):.0f}, nicks {v.get("nicks", 0):.0f}\n')
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
            if name not in b or name not in v:
                continue
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
    keep_pil_nearest()
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

# 0.15.0's liberation (docs/game/liberation/), with BN5's ROM beside BN6's: an
# act 2 dark layer (seed 1's, --dev dark) played by the autopilot, its
# battles won in one hit (onehit, god); and seed 7's fork at home before act
# 2, its pink pad the dark way: the welcome's pages, MegaMan away from the
# pad (its swirl shows only then), then at it, A: "Part of it's gone dark."
LIB_BOARD = ['--scene', 'emu', '--run-depth', '5', '--seed', '1', '--dev', 'dark,god,onehit']
LIB_FORK = ['--scene', 'home', '--run-depth', '4', '--seed', '7', '--dev', 'quiet', '--input',
            '40:,' + '4:A,12:,' * 4 + '160:,' + '4:A,12:,' * 6 + '0:place 120 -15 1,180:,0:place 165 -15 1,30:,6:A,'
            + '90:,4:A,60:,4:A,60:,4:A,60:,4:A,60:,4:A,200:']
# 0.16.0's. (the Cross Navis on a dark board, docs/game/liberation/building/
# cross-navis-team.md: act 3's board on seed 1 with every Soul held and
# HeatCross brought, seven Navis for two places, the TEAM screen's cursor
# up from GO! to HeatMan; act 2's on seed 2 with ChargeCross brought,
# played by the autopilot: ChargeMan's words as he takes his turn, his
# BombLiberation in BN5's command window, his move onto the dark)
LIB_TEAM = ['--scene', 'emu', '--run-depth', '8', '--seed', '1', '--setup', 'endless,0,0,0,1', '--dev', 'dark,god,onehit,souls=0x3F']
LIB_CHARGE = ['--scene', 'emu', '--run-depth', '5', '--seed', '2', '--setup', 'endless,0,0,0,5', '--dev', 'dark,god,onehit']
# (BN6's Virus Battler, docs/world/level-design/virus-battler.md: Battler 1
# on seed 1's first layer, --dev battler=1, MegaMan placed beside him by
# frame 400 and A; on a fresh profile the BtlrCard given;
# with the card held, ErthDrgn on it, and 3000 zenny: his first pair, the
# stake's question, Yes, ErthDrgn on BN6's VIRUS LIST, picked with Mettaur,
# placed, the fight won, his next pair, TAKE THE POT, the pot paid)
VB_RUN = ['--scene', 'emu', '--seed', '1', '--dev', 'quiet,battler=1']
VB_HELD = VB_RUN[:-1] + ['quiet,battler=1,zenny=3000,viruses=0x10000']
VB_MET = '150:,' + '4:A,6:,' * 13 + '100:,0:place 44 72 1,20:,'
VB_SERIES = (VB_MET + '4:A,6:,110:,' * 3 + '4:A,6:,120:,4:LEFT,6:,10:,4:A,6:,110:,4:A,6:,420:,' + '3:DOWN,6:,8:,' * 13 + '20:,3:A,6:,30:,'
             + '3:UP,6:,8:,' * 13 + '3:A,6:,30:,3:A,6:,60:,3:A,6:,40:,3:UP,6:,20:,3:A,6:,40:,60:,3:A,6:,1300:,' + '4:A,6:,110:,' * 4)

# Screenshots of the running game for the site and the README
# (docs/screenshots): name, game options, (frame, shot name) pairs, env.
# Screenshots are fine to publish; files extracted from the ROM are not.
# (the second screen's, issue #72: seed 7's Green Area layer 2, its
# arrival's chats paged with A; and 0.10.0's notes' run there: the map, a
# battle's Custom screen, its cards and picks, the fight's hand as the
# first Cannon counters the Mettaur into Full Synchro)
SECOND_7 = ['--scene', 'emu', '--run-depth', '2', '--seed', '7', '--net-biome', '3', '--dev', 'quiet']
# (BN5's UNITE on BN6's Custom screen: ACDC Area's first battle, its first
# hand's BusterUp, four of them in the folder, picked in the fourth slot;
# on to OK, down to UNITE, A, back up to OK, A: NumberMan's Soul as the
# turn begins)
BN5_UNITE = ('100:,' + '4:A,6:,' * 17 + '0:battle,190:,6:RIGHT,10:,6:RIGHT,10:,6:RIGHT,10:,6:A,40:,6:RIGHT,8:,6:RIGHT,10:,6:DOWN,40:,'
             '6:A,60:,6:UP,14:,6:A,500:')
# 0.12.0's one net (issue #82): BN5's viruses, Navis, Souls and DarkChips in
# BN6's own engine. (ACDC Area's arrival words paged, then the foes step's
# battle on BN6's plain field, its Custom screen open by frame 500)
ONE_NET = ['--scene', 'emu', '--net-biome', 'x0', '--seed', '4']
ONE_PAGED = '100:,' + '4:A,6:,' * 17
# (ShadowSoul: a folder of Sword, its first fifteen Invisibl, Invisibl
# ShadowSoul's kind; Powie, Marina and Powie V3; a Sword picked, an
# Invisibl last, UNITE, OK; the Sword held charged as the turn begins: he
# steps behind Powie V3 and slashes)
ONE_SOUL = (ONE_PAGED + '0:chip 177 * 15,0:foes 20c@52/260@63/20e@41,230:,6:A,8:,6:RIGHT,6:,6:RIGHT,6:,6:RIGHT,6:,6:A,8:,'
            '6:RIGHT,6:,6:RIGHT,6:,6:DOWN,30:,6:A,60:,6:UP,14:,6:A,330:,70:A,120:,100:B,100:')
# (BN5's worried rule: MegaMan held at 200 of 999 HP, a DarkChip in the
# folder's first entry; the first screen's chip picked and used, the
# second screen by L with the DarkChip at the hand's end, under the cursor)
ONE_WORRIED = '230:,6:A,10:,6:START,10:,6:A,150:,6:A,520:,6:L,70:,'
ONE_DARKINVS = ONE_PAGED + '0:chip 289 * 1,0:foes 1D4@52,' + ONE_WORRIED + '6:A,20:,6:START,20:,6:A,150:,6:A,800:'
PAGED_7 = '600:,' + '4:A,6:,' * 110
SECOND_HERO = (PAGED_7 + '30:,40:UP,0:battle,215:,6:A,24:,6:RIGHT,24:,6:RIGHT,24:,6:A,24:,6:START,36:,6:A,150:,'
               '6:DOWN,12:,6:A,70:')
# (a run's opening words paged in Lan's room, Lan free by frame 530)
HOME_PAGED = '40:,' + '4:A,6:,' * 46 + '30:,'
# (... then down the stairs and out of the front door, by frame 834; into
# AsterLand, by frame 1066; up to its counter and into the Order Service,
# 550 frames more)
HOME_OUT = HOME_PAGED + '24:DOWN,90:,0:place -78 55 1,10:,30:DOWN,150:,'
HOME_ASTER = HOME_OUT + '0:place -156 -40 7,10:,62:UP+LEFT,160:,'
HOME_COUNTER = '30:UP+RIGHT,124:UP+LEFT,10:UP+LEFT,6:,6:A,56:,6:A,46:,6:A,46:,6:A,46:,6:A,156:,'
# (... and its list sorted, issue #124: a Library of every chip the Order
# Service holds, as a long-time player's, the clerk's list from frame
# 1600; START through BN6's seven keys, SELECT the other way at Attack and
# at ID, a sort each 70 frames)
HOME_SORT = ('0:flags 0x1E21 0x1F32 1,' + HOME_ASTER + HOME_COUNTER + '60:,' + '6:START,64:,' * 3 + '6:SELECT,64:,' +
             '6:START,64:,' * 4 + '6:SELECT,90:')
# (... or through the Cyber Academy's gate, the foyer's turnstile, the 1F
# hallway's stairs and the 2F hallway's door into class 6-1, by frame 1484)
HOME_ACADEMY = HOME_OUT + ''.join(f'0:place {p},10:,{n}:UP+LEFT,120:,' for p, n in
                                  (('92 -232 7', 40), ('24 16 7', 30), ('280 -40 7', 30), ('-208 -40 7', 30)))
# (... or out of the front door to the plaza's fountain, by frame 874)
TOWN_PLAZA = HOME_OUT + '0:place 160 -120 4,40:'
# (Lan's HP after act 2, seed 3: the arrival's words paged, MegaMan free
# from frame 1200)
HP_7 = ['--scene', 'home', '--run-depth', '7', '--seed', '3']
HP_PAGED = '600:,' + '6:A,94:,' * 6
# 0.13.0's notes. (the Net Square's gate the first time, issue #132:
# MegaMan stepped onto it, the keeper's words paged, Lan typed on his
# name screen, START, then his word on what the server sees and his
# question, the cursor on No)
GATE_NAME = ('4:RIGHT,6:,' * 11 + '4:A,10:,4:DOWN,6:,4:DOWN,6:,' + '4:LEFT,6:,' * 9 + '4:A,10:,4:DOWN,6:,4:RIGHT,6:,'
             '4:A,20:,6:START,60:,')
GATE_KEEPER = HP_PAGED + '0:place 16 80 3,20:,8:UP,40:,6:A,60:,6:A,60:,' + GATE_NAME + '6:A,74:,' * 5 + '120:'
# (Trade Central's spares list out of the Square, issue #145: the trade
# dev step in seed 3's run at depth 2, nine spares given by the chip step,
# BN5's MrkCan1 and MrkCan3 among them, both ROMs in (the full set's);
# the cursor down the list, each chip's card beside it, START through
# the sorts)
TRADE_SPARES = ('250:,' + ''.join(f'0:chip {c} 1,' for c in ('1 A', '20 F', '72 S', '89 G', '121 D', '154 L', '227 H', '411 M', '413 E')) +
                '0:trade list,50:,' + '4:DOWN,36:,' * 4 + '4:START,56:,' * 2 + '4:DOWN,46:,' + '4:DOWN,30:,' * 5 + '4:START,60:')
# (the Jukebox, docs/game/pet.md: Dad's call paged in Lan's room, the PET's
# KeyItem, A on the Jukebox; BN6's Central Town played, then R to BN5's
# The Net and its Undernet; the sound rendered frame by frame, so the
# lights and the time are the song's)
JUKEBOX = (HOME_PAGED + '6:START,40:,' + '6:DOWN,6:,' * 5 + '6:A,40:,6:DOWN,6:,6:RIGHT,20:,6:A,30:,6:A,330:,' + '6:R,6:,' * 6 +
           '6:DOWN,6:,6:DOWN,6:,6:A,330:')
JUKEBOX_SOUND = {'CYBERWORLD_AUDIO_DUMP': '/src/.build/screenshots/sound.raw', 'CYBERWORLD_AUDIO_OFFLINE': '1'}
# (issue #98's identity: a layer's arrival words paged, then the sig dev
# step puts MegaMan in the room the layer is remembered by)
IDENTITY = ['--scene', 'emu', '--dev', 'quiet']
SIG_PAGED = '150:,' + '4:A,6:,' * 30 + '0:sig,'
# 0.17.0's notes. (the visit's requests on BN6's Request BBS, seed 2 after
# act 2: in AsterLand, A at the board, its words and Lan's paged, the list
# up from frame 1440, the newest post read out from 1700, its second page
# by 2060; the courier in Lan's HP on the same seed, his four boxes from
# frame 1230: the BBS, then each offer and its pay)
HOME_BBS = HOME_ASTER + '0:place -76 -38 0,20:,6:A,60:,6:A,80:,6:A,60:,6:A,120:,62:,6:A,200:,6:A,150:,6:A,300:'
HP_2 = ['--scene', 'home', '--run-depth', '7', '--seed', '2']
COURIER_OFFERS = HP_PAGED + '0:place -84 60 7,10:,6:A,124:,' + '6:A,104:,' * 3 + '120:'
# (an endless run's first layer, seed 1: its three boxes paged, then its
# first battle, the Custom screen open by frame 920)
FIRST_HAND = ['--scene', 'emu', '--seed', '1', '--dev', 'quiet']
FIRST_HAND_IN = '330:,6:A,94:,6:A,94:,6:A,94:,6:A,94:,0:battle,250:,'
# (a screenshot's run that another runs before it in the same data folder:
# its options, environment and frames)
SCREENSHOT_PREP = {
    # (MegaMan loses to ElecMan, at 1 HP from the first hit: the record his
    # next meeting speaks from)
    'guardian-memory': (['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '11', '--guardian', '2', '--dev', 'fragile'],
                        {'CYBERWORLD_AUTOPILOT': '1'}, 4000),
}
# (0.17.0's: the Server's talk opened on seed 8's layer 4, paged to its
# question, Yes; the battle (OldHeatr, Shooter, Mettaur2, one hit each) won
# by the buster down each row; the result and the pick's words paged)
SERVER_8 = ['--scene', 'emu', '--run-depth', '4', '--seed', '8', '--dev', 'god,onehit,quiet', '--talk', 'challenge:200']
SERVER_SHOTS = '6:B,6:,12:,' * 6
SERVER_PICK = ('1:,300:,' + '6:A,6:,70:,' * 3 + '6:A,6:,90:,6:LEFT,6:,10:,6:A,6:,400:,6:START,6:,10:,6:A,6:,120:,' + SERVER_SHOTS +
               '8:UP,15:,' + SERVER_SHOTS + '8:DOWN,15:,8:DOWN,15:,' + SERVER_SHOTS + '200:,60:,' + '6:A,6:,60:,' * 2 +
               '6:A,6:,200:,' + '6:A,6:,90:,' * 2)
# (0.17.0's battle floor, PR #247: seed 2's layer 4 with its random battles
# on, its arrival paged; MegaMan put two panels short of the exit pad, no
# battle yet on the layer, walks onto it: the pad holds the layer's battle)
FLOOR_PAD = ['--scene', 'emu', '--run-depth', '4', '--seed', '2']
FLOOR_PAD_INPUT = '1:,150:,' + '6:A,6:,30:,' * 14 + '60:,0:place 316 84 1,10:,55:UP+RIGHT,300:'
# 0.19.0's a GBA's screen (issue #265): seed 1's first layer, its words
# paged, its first random battle, the hand sent, then MegaMan up and down a
# row firing his buster from frame 1200 to 1500 or so. At 720x480 the canvas
# is the game's 240x160 and the screen it 3x, lcd3x's stripes whole there.
GBA_SCREEN = ['--scene', 'emu', '--dev', 'quiet', '--size', '720x480', '--input',
              '150:,' + '4:A,6:,' * 60 + '20:,0:battle,220:,4:A,16:,4:A,16:,4:START,16:,4:A,110:,6:A,24:,'
              + '6:B,14:,6:UP,10:,6:B,14:,6:DOWN,10:,' * 6 + '200:']
GBA_SHOT = 1280
# (a Sky layer's map held over the game, the engine's drawing on BN6's picture)
MAP_SKY = ['--scene', 'emu', '--net-biome', '2', '--run-depth', '4', '--seed', '1', '--dev', 'quiet,mapall',
           '--input', '250:' + ',6:A,24:' * 14 + ',60:,40:SELECT']
# (Options' Display on the title, its scene staged: down its lines, the
# scene changing with the line; Colour correction from Off through GBA,
# VBA, DS and PSP and back, the whole screen recoloured each time; Frame
# blending's scene MegaMan hit and flickering, Simple, Smart and LCD)
OPTIONS_LIVE = ('60:,4:SELECT,150:,' + '4:DOWN,6:,40:,' + '4:DOWN,6:,100:,' + '4:DOWN,6:,60:,' + '4:RIGHT,6:,50:,' * 5 +
                '4:DOWN,6:,100:,' + '4:RIGHT,6:,70:,' * 3)
# 0.20.0's: the tournament forced on seed 6's act 3 (layer 9), the autopilot
# in god mode with one hit a deletion: the door's save at 1000, BN4's
# bracket with every entrant's face from 1050, round 1's card (GutsMan),
# his battle; round 1's chip and the bracket again, the beaten grey, at 2930
TOURNEY = ['--scene', 'emu', '--run-depth', '9', '--seed', '6', '--dev', 'tourney,god,onehit,quiet']
# (a battle against one of BN4's Navis, started on BN4's ACDC Area's first
# layer by the foes step, the autopilot fighting in god mode)
BN4_NAVI = ['--scene', 'emu', '--net-biome', 'x15', '--run-depth', '1', '--seed', '4', '--dev', 'quiet,god']
# (BN4's areas' first layers, the weak autopilot walking past the words)
BN4_AREA = ['--scene', 'emu', '--run-depth', '1', '--dev', 'quiet', '--seed', '4']
# 0.21.0's Navis and chips (B). (a chip's battle: seed 4's layer 4 in Central
# Area, three Mettaur3 set by the foes step, the autopilot playing a
# folder of thirty of the chip in god mode; a Program Advance's: the same
# battle by script, the arrival's words paged, the folder's three parts
# put in ten each, the hand's three picked in the recipe's order, OK, A)
B_AP = {'CYBERWORLD_AUTOPILOT': '1'}
B_CHIP = ['--scene', 'emu', '--net-biome', '0', '--run-depth', '4', '--seed', '4']
B_METT = '0:foes 03@52/03@41/03@63,'
B_PAGE = '300:,' + '4:A,26:,' * 20
B_R4, B_L4 = '6:RIGHT,10:,' * 4, '6:LEFT,10:,' * 4
B_OK = '30:,6:START,14:,6:A,'
B_HEATSPRD = (B_PAGE + '0:chip 590 D 30,0:chip 589 D 20,0:chip 588 D 10,' + B_METT + '270:,' + B_R4 + '6:A,14:,' + B_L4 +
              '6:A,14:,6:RIGHT,10:,6:A,' + B_OK + '330:,6:A,400:')
B_WILDBIRD = (B_PAGE + '0:chip 519 S 30,0:chip 435 S 20,' + B_METT + '270:,6:A,14:,6:RIGHT,10:,6:RIGHT,10:,6:A,14:,6:LEFT,10:,6:A,' +
              B_OK + '400:,6:A,400:')
B_CROSOVER = (B_PAGE + '0:chip 526 D 30,0:chip 525 D 20,0:chip 524 D 10,' + B_METT + '270:,' + B_R4 + '6:A,14:,' + B_L4 +
              '6:A,14:,6:RIGHT,10:,6:A,' + B_OK + '400:,6:A,400:')
# (a Navi of BN5's fought on BN5's ACDC Area's first layer by the foes
# step, as BN4_NAVI fights BN4's; the deep's two guarding an Undernet act 5)
B5_NAVI = ['--scene', 'emu', '--net-biome', 'x0', '--run-depth', '1', '--seed', '4', '--dev', 'quiet,god']
B_DEEP = ['--scene', 'emu', '--run-depth', '15', '--net-biome', '5', '--dev', 'god,onehit,hp=999']
B_DEEP_FOES = ['--scene', 'emu', '--run-depth', '15', '--net-biome', '5', '--dev', 'quiet,god,hp=999']
# (a Net Dealer on BN4's ACDC Area, seed 4's layer 2: Chaud's call and the
# dealer's words paged, the cursor down to Ligtnin1, R for its card)
B_DEALER = '100:,' + '6:A,54:,' * 22 + '80:,' + '6:A,54:,' * 15 + '150:,6:DOWN,20:,6:DOWN,40:,6:R,300:'
# (Colonel's Guardian Data by its talk: the Soul, the chip, then the draft,
# the cursor down to SoulT+1, A)
B_SOULT = '420:,' + '6:A,34:,' * 23 + '40:,6:DOWN,10:,6:DOWN,60:,6:A,60:,6:A,200:'
B_LANE = '60:,' + '4:A,6:,' * 60
# (the Glitch layer forced: the warp on seed 2's layer 4, MegaMan put two
# panels from it; a Glitch layer held at its last stage, MegaMan walked
# round its floor as other areas' looks flip in)
GLITCH_WARP = ['--scene', 'emu', '--run-depth', '4', '--seed', '2', '--dev', 'glitch,quiet',
               '--input', '150:,' + '4:A,6:,' * 30 + '40:,0:glitch,400:']
GLITCH_LAYER = ['--scene', 'emu', '--run-depth', '4', '--seed', '2', '--dev', 'glitch,side=4,glitchstage=5,quiet',
                '--input', '150:,' + '4:A,6:,' * 30 + '40:,100:RIGHT,100:DOWN,100:LEFT,100:UP,400:']
# 0.21.0 (A)
# (the Battle Lab, docs/game/battle-lab.md: Central Area's first layer, seed
# 15, its words paged, the builder opened with every family by the dev step
# `lab all`, open by frame 700; every ROM read, so BN5's and BN4's panels,
# backgrounds and songs are on its rows)
LAB = ['--scene', 'emu', '--net-biome', '0', '--seed', '15', '--dev', 'quiet']
LAB_OPEN = '300:,' + '4:A,6:,' * 30 + '40:,0:lab all,60:,'
# (FOES: Mettaur lifted and set down, B back to the list, down to Swordy,
# its version turned to V2, set a panel on, by frame 1036; FIELD: the
# cursor up from MegaMan's panel, A turning each panel through the types:
# BN5's lava, sea and magnet panel, BN4's Hole, ice and grass, by 2006;
# STAGE: the background turned back to BN4's ACDC Area, the song to BN4's
# Virus Busting, Folder to the Lab's, by 2346)
LAB_FOES = LAB_OPEN + '6:A,24:,6:A,24:,6:B,24:,' + '6:DOWN,14:,' * 3 + '6:RIGHT,14:,6:A,24:,6:RIGHT,14:,6:DOWN,14:,6:A,24:,6:B,60:,'
LAB_FIELD = ('6:R,54:,6:UP,14:,' + '4:A,6:,' * 12 + '6:RIGHT,14:,' + '4:A,6:,' * 13 + '6:DOWN,14:,6:DOWN,14:,' + '4:A,6:,' * 14 +
             '6:LEFT,14:,' + '4:A,6:,' * 15 + '6:RIGHT,14:,' * 3 + '4:A,6:,' * 6 + '6:UP,14:,6:UP,14:,' + '4:A,6:,' * 5 + '60:,')
LAB_STAGE = '6:R,54:,' + '6:LEFT,34:,' * 2 + '6:DOWN,34:,' + '6:LEFT,34:,' * 2 + '6:DOWN,34:,6:RIGHT,34:,'
LAB_BUILD = LAB + ['--input', LAB_FOES + LAB_FIELD + LAB_STAGE + '6:A,200:']
# (BATTLE!: START from the builder, the first hand's first chip sent)
LAB_GO = '6:START,90:,6:A,20:,6:START,20:,6:A,200:,6:A,60:,'
# (a test battle won as it is played: two Mettaurs set in the enemy's front
# column, BATTLE!, WideSwrd sent, a step forward and the slash through both;
# BN6's RESULT, A, NO DATA, A, the builder with the time; L to SETS)
LAB_METS = LAB_OPEN + '6:A,24:,6:LEFT,14:,6:UP,14:,6:A,24:,6:DOWN,14:,6:DOWN,14:,6:A,24:,6:B,40:,'
LAB_WON = LAB_METS + '6:START,90:,6:A,20:,6:START,20:,6:A,130:,6:RIGHT,10:,6:A,260:,6:A,120:,6:A,200:,6:L,60:,200:'
# (a new run in Lan's room, seed 2: Dad's call paged, the PET opened)
PET_ROOM = '40:,' + '4:A,6:,' * 46 + '30:,6:START,40:,'
LAYER_15 = '300:,' + '4:A,6:,' * 30 + '40:,'
# (every act has its virus, docs/game/progression/encounter-groups.md: seed
# 2's act 2 on BN6's ROM alone, Aquarium HP, its family Gunner: a layer's
# words paged, then its first battle, the act's beat, to the Custom screen
# and into its first turn)
GROUPS = ['--scene', 'emu', '--seed', '2', '--dev', 'quiet']
GROUPS_IN = '150:,' + '4:A,6:,' * 40 + '40:,0:battle,300:,6:A,20:,6:START,20:,6:A,300:'
# (the fixed menus on BN4's areas, every ROM read: seed 4's BN4 ACDC Area;
# its first layer's words paged, the PET's ChipFolder opened; its second
# layer's Net Dealer, Chaud's call and his eight boxes paged to his stock,
# down to BN4's Ligtnin1, R for its card; home before act 2 on seed 6, the
# act ahead one of BN4's areas)
BN4_MENU = ['--scene', 'emu', '--seed', '4', '--net-biome', 'x15']
BN4_FOLDER = BN4_MENU + ['--run-depth', '1', '--dev', 'quiet', '--input', '300:,' + '4:A,6:,' * 30 + '40:,6:START,40:,6:A,60:,6:A,100:']
BN4_SHOP = BN4_MENU + ['--run-depth', '2', '--dev', 'quiet,zenny=9000', '--talk', 'shop:430', '--input',
                       '500:' + ',4:A,6:,96:' * 15 + ',60:,6:DOWN,20:,6:DOWN,20:,60:,6:R,200:']
BN4_HOME = ['--scene', 'home', '--run-depth', '4', '--seed', '6', '--input', HP_PAGED + '200:']
# (reward picks on their screen, docs/game/reward-pick.md: the run's opening
# gift on seed 15, its HPMemory, down to the chip, down to the program; a
# guardian's Guardian Data on seed 1's layer 6, its draft of programs)
PICK_GIFT = ['--scene', 'emu', '--seed', '15', '--dev', 'quiet', '--input', '150:,' + '4:A,6:,' * 30 + '100:,6:DOWN,60:,6:DOWN,60:,6:UP,60:']
PICK_DRAFT = ['--scene', 'emu', '--run-depth', '6', '--seed', '1', '--dev', 'quiet', '--talk', 'reward:300', '--input', '400:,' + '6:A,81:,' * 4 + '400:']
# (the NetPass, docs/online/netpass.md: Central Area's first layer paged,
# then the greeter's word on the set grown and the pass as it then opens)
NETPASS_GREW = LAB + ['--input', LAYER_15 + '0:netpass grew,120:,6:B,100:,6:B,100:,6:B,100:,0:netpass grown,200:']
# (ProtoMan's netbattle, the rivalry's top rung (duels=2), on seed 7's
# layer 8: his terms opened, paged, Yes, the Custom screen, BATTLE START!)
SHOWDOWN = ['--scene', 'emu', '--run-depth', '8', '--seed', '7', '--net-biome', '11', '--dev', 'quiet,duels=2', '--talk', 'duel:900',
            '--input', '300:,' + '4:A,6:,' * 56 + '40:,' + '6:A,114:,' * 5 + '150:,6:LEFT,14:,6:A,400:,6:START,4:,6:A,600:']
# (the names of this release's pictures that see the whole ROM folder
# without BN5 or BN4 in their name: whole_rom_folder)
WHOLE_ROMS_A = ('lab-build', 'lab-won', 'library-pages', 'library')
# (and those that see BN6's and BN5's ROMs without BN4's: rom_mounts_no_bn4)
NO_BN4_A = ('netpass-slot',)
SCREENSHOTS = [
    ('title', ['--scene', 'title'], [(80, 'title')], {}),
    # (the setup after NEW GAME at its Folder row, and the title with the
    # marks a profile can earn: a won short net, the endless Nest, the
    # Secret Area, the top threat rung)
    ('setup', ['--scene', 'setup', '--input', '60:,6:DOWN,20:,6:DOWN,40:'], [(150, 'setup')], {}),
    # (a won short net's summary: the run, then the records and what it opened)
    ('summary', ['--scene', 'summary', '--setup', 'short', '--run-depth', '10', '--input', '100:,4:A,36:'],
     [(90, 'summary'), (139, 'summary-records')], {}),
    ('marks', ['--scene', 'title', '--marks', '18A'], [(80, 'marks')], {}),
    # (issue #104: the statistics' question at a first start, the data folder fresh)
    ('stats-ask', ['--dev', 'statistics=ask'], [(100, 'stats-ask')], {}),
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
    # (home, docs/world/home/: a run's start in Lan's room, Dad's call; his PC's
    # jack-in; down the stairs and out of the front door into Central Town;
    # the plaza's fountain as the run begins and at night before the Nest
    # (the short net's layer 10); Lan's HP after act 2 (issues #108, #110):
    # MegaMan up the corridor from the blue pad, the link up top lit, two
    # ways back's orange pads and the courier's "!!"; the courier's word on
    # a finished request; a way back checked, its question on No)
    ('home-room', ['--scene', 'town', '--seed', '2'], [(280, 'home-room')], {}),
    ('jukebox-bn5', ['--scene', 'town', '--seed', '2', '--input', JUKEBOX], [(1086, 'jukebox'), (1520, 'jukebox-bn5')], JUKEBOX_SOUND),
    ('home-pc', ['--scene', 'town', '--seed', '2', '--input', HOME_PAGED + '0:place -40 -8 7,10:,6:R,100:'], [(616, 'home-pc')], {}),
    ('town-central', ['--scene', 'town', '--seed', '2', '--input', HOME_PAGED + '24:DOWN,90:,0:place -78 55 1,10:,30:DOWN,150:'],
     [(830, 'town-central')], {}),
    ('town-morning', ['--scene', 'town', '--seed', '2', '--input', TOWN_PLAZA], [(870, 'town-morning')], {}),
    ('town-night', ['--scene', 'town', '--seed', '2', '--run-depth', '10', '--input', TOWN_PLAZA], [(870, 'town-night')], {}),
    ('home-hp', HP_7 + ['--input', HP_PAGED + '0:place 0 -20 1,30:'], [(1229, 'home-hp')], {}),
    ('home-courier', HP_7 + ['--dev', 'job=2,jobstate=2', '--input', HP_PAGED + '0:place -84 60 7,10:,6:A,124:'],
     [(1330, 'home-courier')], {}),
    ('home-back', HP_7 + ['--input', HP_PAGED + '0:place -28 -54 7,10:,' + '6:A,100:,' * 3 + '6:A,130:,6:A,100:'],
     [(1760, 'home-back')], {}),
    # (home after act 2, issue #92: the plaza's Mr.Prog calls the Net's
    # news at evening; in AsterLand the request board's offer, and the
    # clerk's Order Service with the second screen's card; Lan's HP with
    # the home panel, a request held)
    ('home-news', ['--scene', 'town', '--seed', '2', '--run-depth', '7', '--input', HOME_OUT + '0:place 60 -138 7,10:,4:UP+LEFT,10:,6:A,66:,6:A,66:'],
     [(1000, 'home-news')], {}),
    ('home-request', ['--scene', 'town', '--seed', '2', '--run-depth', '7', '--input', HOME_ASTER + '0:place -92 -14 7,10:,8:UP+LEFT,20:,' + '6:A,66:,' * 2],
     [(1262, 'home-request')], {}),
    ('home-order', ['--scene', 'town', '--seed', '2', '--run-depth', '7', '--dev', 'zenny=8000', '--input', HOME_ASTER + HOME_COUNTER + '6:DOWN,20:,6:DOWN,40:'],
     [(1680, 'home-order')], {}, '3ds'),
    # (issue #124: the clerk's list by Attack, its strongest first)
    ('home-sort', ['--scene', 'town', '--seed', '2', '--run-depth', '7', '--dev', 'zenny=8000', '--input', HOME_SORT],
     [(1876, 'home-sort')], {}),
    ('home-panel', ['--scene', 'home', '--run-depth', '4', '--seed', '3', '--setup', 'short,0,0,0', '--dev', 'job=2,jobstate=1', '--input', '600:,' + '6:A,94:,' * 8],
     [(1395, 'home-panel')], {}, '3ds'),
    # (issue #96: Lan's class 6-1, the NetBattle club's member and her challenge)
    ('home-academy', ['--scene', 'town', '--seed', '2', '--run-depth', '7', '--input', HOME_ACADEMY + '0:place -148 22 7,10:,6:A,130:'],
     [(1620, 'home-academy')], {}),
    # (the areas on their acts' third layers: ProtoMan's duel, and Chaud's
    # call with it, waits on the second)
    ('central', ['--scene', 'emu', '--net-biome', '0', '--run-depth', '3', '--seed', '3', '--dev', 'quiet'], [(420, 'central')], {}),
    ('seaside', ['--scene', 'emu', '--net-biome', '1', '--run-depth', '3', '--seed', '11', '--dev', 'quiet'], [(420, 'seaside')], {}),
    ('green', ['--scene', 'emu', '--net-biome', '3', '--run-depth', '3', '--seed', '3', '--dev', 'quiet'], [(420, 'green')], {}),
    ('undernet', ['--scene', 'emu', '--net-biome', '5', '--run-depth', '15', '--seed', '3', '--dev', 'quiet'], [(420, 'undernet')], {}),
    ('graveyard', ['--scene', 'emu', '--net-biome', '4', '--run-depth', '18', '--seed', '3', '--dev', 'quiet'], [(420, 'graveyard')], {}),
    ('nest', ['--scene', 'emu', '--net-biome', '7', '--run-depth', '19', '--seed', '3', '--dev', 'quiet'], [(250, 'nest')], {}),
    # (issue #98: each area's layer in its signature, docs/world/level-design/identity.md,
    # Identity: Central's crater framed by an avenue of cybertrees, Seaside's
    # great field round its pool, Sky's ring road round its pad, Green's
    # grove at its giant cybertree, the Graveyard's slab round its hole below
    # the monument, the Undernet's court with its statue between braziers;
    # tools/before_after.py's identity pairs are the same layers' maps)
    ('identity-central', IDENTITY + ['--net-biome', '0', '--run-depth', '2', '--seed', '1', '--input', SIG_PAGED + '70:'],
     [(520, 'identity-central')], {}),
    ('identity-seaside', IDENTITY + ['--net-biome', '1', '--run-depth', '4', '--seed', '2', '--input', SIG_PAGED + '70:'],
     [(520, 'identity-seaside')], {}),
    ('identity-sky', IDENTITY + ['--net-biome', '2', '--run-depth', '4', '--seed', '1', '--input', SIG_PAGED + '70:'], [(520, 'identity-sky')], {}),
    ('identity-green', IDENTITY + ['--net-biome', '3', '--run-depth', '4', '--seed', '3', '--input', SIG_PAGED + '24:UP,46:'],
     [(520, 'identity-green')], {}),
    ('identity-graveyard', IDENTITY + ['--net-biome', '4', '--run-depth', '4', '--seed', '5', '--input', SIG_PAGED + '70:'],
     [(520, 'identity-graveyard')], {}),
    ('identity-undernet', IDENTITY + ['--net-biome', '5', '--run-depth', '4', '--seed', '1', '--input', SIG_PAGED + '24:UP+LEFT,46:'],
     [(520, 'identity-undernet')], {}),
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
     [(1160, '3ds')], {'CYBERWORLD_AUTOPILOT': 'weak'}, '3ds'),
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
    # rocks in BN6's battle, his own moved in; not the weak one, which
    # holds him at 1 HP)
    ('bn5-guardian', ['--scene', 'emu', '--net-biome', 'x0', '--run-depth', '3', '--seed', '3', '--guardian', '28', '--dev', 'quiet'],
     [(1130, 'bn5-guardian-card'), (1213, 'bn5-guardian'), (1290, 'bn5-guardian-talk'), (2060, 'bn5-guardian-battle')],
     {'CYBERWORLD_AUTOPILOT': '1'}),
    # (NumberMan's Guardian Data giving his Soul, End Area; then a run
    # holding his Soul, four BusterUps in its folder: BusterUp picked on
    # BN6's Custom screen, UNITE in the emblem's place, the unison as the
    # turn begins)
    ('bn5-soul', ['--scene', 'emu', '--net-biome', 'x2', '--run-depth', '3', '--seed', '3', '--guardian', '26', '--dev', 'quiet', '--talk', 'reward:420',
                  '--input', '100:,' + '4:A,6:,' * 30 + '100:,' + '6:A,94:,' * 12], [(700, 'bn5-soul'), (1080, 'bn5-soul-words')], {}),
    ('bn5-unite', ['--scene', 'emu', '--net-biome', 'x0', '--seed', '4', '--dev', 'quiet,souls=4,folder=175/4', '--input', BN5_UNITE],
     [(600, 'bn5-unite'), (880, 'bn5-unison')], {}),
    # (BN6's own DarkChips, on BN6's ROM alone: a flame on Central's middle
    # layer, Chaud's call paged, MegaMan placed by it and speaking to it;
    # DrkSword drawn in a battle's first hand, a BugFrag burnt for its 400,
    # then the 20 max HP it took)
    ('dark-flame', ['--scene', 'emu', '--net-biome', '0', '--run-depth', '2', '--seed', '3', '--dev', 'quiet', '--input',
                    '100:,' + '4:A,6:,' * 30 + '20:,0:place 20 -20 1,40:,6:A,140:,6:A,94:'], [(440, 'dark-flame'), (700, 'dark-flame-words')], {}),
    ('dark-chip', ['--scene', 'emu', '--net-biome', '0', '--run-depth', '1', '--seed', '4', '--dev', 'quiet,darkchips=0x2,folder=286/3', '--talk', 'bugfrags:420',
                   '--input', '100:,' + '4:A,6:,' * 30 + '40:,0:battle,220:,6:A,20:,6:START,20:,6:A,120:,6:RIGHT,20:,6:A,200:,6:A,60:,6:A,60:,6:A,60:,6:A,150:,'
                   + '6:A,100:,' * 10], [(650, 'dark-chip-custom'), (882, 'dark-chip'), (2160, 'dark-chip-price')], {}),
    # 0.12.0's notes, one net (issue #82): BN5's TinHawk, Powie and Marina
    # with BN6's HUD; ToadMan guarding SciLab Area, his card and his sea; the
    # Dark Soul's buster; ShadowSoul's charged Sword behind Powie V3;
    # KnightSoul's Chaos Unison, a folder of DarkDril, its drill at Colonel
    ('bn5-viruses', ONE_NET + ['--dev', 'quiet', '--input', ONE_PAGED + '0:foes 28a@42/20c@53/260@63,260:,4:START,10:,4:A,600:'],
     [(820, 'bn5-viruses')], {}),
    ('bn5-toadman', ['--scene', 'emu', '--net-biome', 'x1', '--run-depth', '3', '--seed', '3', '--guardian', '29', '--dev', 'quiet'],
     [(1050, 'bn5-toadman-card'), (2500, 'bn5-toadman')], {'CYBERWORLD_AUTOPILOT': '1'}),
    ('bn5-darksoul', ONE_NET + ['--dev', 'quiet', '--input', ONE_PAGED + '0:foes 1DC@52,260:,4:START,10:,4:A,600:'],
     [(780, 'bn5-darksoul')], {}),
    ('bn5-one-net', ONE_NET + ['--dev', 'quiet,god,souls=2,folder=71', '--input', ONE_SOUL], [(1140, 'bn5-shadowsoul')], {}),
    ('bn5-chaos', ONE_NET + ['--dev', 'quiet,god,souls=16,folder=218', '--input', ONE_PAGED + '0:foes 1C4@52,230:,6:A,10:,'
                             + '6:RIGHT,8:,' * 5 + '6:DOWN,30:,6:A,60:,6:UP,14:,6:A,380:,45:B,300:'], [(1186, 'bn5-chaos')], {}),
    # (BN5's DarkChips: its purple flame on ACDC Area's middle layer, MegaMan
    # placed by it, its DrkSonic named and offered; DarkInvs at the hand's
    # end, the game fighting KnightMan for MegaMan, grey; DarkPlus, then
    # the Cannon it made 50 stronger)
    ('bn5-dark-flame', ['--scene', 'emu', '--net-biome', 'x0', '--run-depth', '2', '--seed', '3', '--dev', 'quiet', '--input',
                        '100:,' + '4:A,6:,' * 30 + '20:,0:place -116 76 7,40:,6:A,140:,' + '6:A,74:,' * 16],
     [(686, 'bn5-dark-flame-words'), (1484, 'bn5-dark-flame')], {}),
    ('bn5-darkinvs', ONE_NET + ['--dev', 'quiet,worried,hp=999/200', '--input', ONE_DARKINVS],
     [(1240, 'bn5-darkinvs-offer'), (1940, 'bn5-darkinvs')], {}),
    ('bn5-darkplus', ONE_NET + ['--dev', 'quiet,worried,hp=999/200,folder=1', '--input', ONE_PAGED + '0:chip 290 * 1,0:foes 260@52/20d@43,'
                                + ONE_WORRIED + '6:A,20:,6:LEFT,10:,6:A,20:,6:START,20:,6:A,150:,6:A,60:,6:A,200:'], [(1580, 'bn5-darkplus')], {}),
    # (BN5's lava and sea as BN6's panels: MetFires on BN5's field 0x65 as
    # BN6 has it, Marina and Powie on its sea and grass, 0x66; a BN6
    # Central Area battle of act 2 with BN5's Cactikil EX visiting, every
    # battle taking one)
    ('bn5-lava', ONE_NET + ['--dev', 'quiet', '--input', ONE_PAGED + '0:foes 21e@52/21f@43/field@65,260:,4:START,10:,4:A,600:'],
     [(820, 'bn5-lava')], {}),
    ('bn5-sea', ONE_NET + ['--dev', 'quiet', '--input', ONE_PAGED + '0:foes 260@52/20c@43/field@66,260:,4:START,10:,4:A,600:'],
     [(900, 'bn5-sea')], {}),
    ('bn5-visitor', ['--scene', 'emu', '--net-biome', '0', '--run-depth', '5', '--seed', '3', '--dev', 'quiet,visitors', '--input', '300:battle'],
     [(580, 'bn5-visitor')], {'CYBERWORLD_AUTOPILOT': '1'}),
    # 0.9.0's notes, each by its feature. (MegaMan's word on GitHub, typed out)
    ('notice', ['--scene', 'intro'], [(230, 'notice')], {}),
    # (the title's menu after START, OPTIONS under NEW GAME and CONTINUE; the
    # PET's menu in Lan's room, Options in Comm's row)
    ('title-menu', ['--scene', 'title', '--input', '60:,4:START,50:'], [(110, 'title-menu')], {}),
    ('pet-options', ['--scene', 'town', '--seed', '2', '--input', HOME_PAGED + '6:START,60:'], [(590, 'pet-options')], {}),
    # (SELECT on the title: Options on its Display page, its scene being
    # staged, then running; R to its Audio page, R to its Game page)
    ('options', ['--scene', 'title', '--input', '60:,4:SELECT,150:,4:R,40:,4:R,40:'],
     [(100, 'options'), (210, 'options-display'), (255, 'options-audio'), (300, 'options-game')], {}),
    # (SELECT on the title: Options, then its Controls, an Xbox pad attached)
    ('controls', ['--scene', 'title', '--pad', 'xbox', '--input', '60:,4:SELECT,30:,4:A,40:'], [(134, 'controls')], {}),
    # (0.23.0's: Options' Game page from the title, UP to its last line,
    # Discord, on; A turns it off)
    ('options-discord', ['--scene', 'title', '--dev', 'discord', '--input', '60:,4:SELECT,150:,4:R,40:,4:R,40:,4:UP,40:,4:A,40:'],
     [(345, 'options-discord'), (390, 'options-discord-off')], {}),
    # (a Recovery Mr. Prog on an act's middle layer, MegaMan held at 40 HP:
    # half his max back, his one patch; asked again, spent)
    ('heal-once', ['--scene', 'emu', '--run-depth', '2', '--seed', '5', '--dev', 'quiet,hp=100/40', '--talk', 'heal:620,heal:1150',
                   '--input', '60:,' + '4:A,6:,' * 40 + '40:,290:,6:A,134:,6:A,134:,6:A,214:,6:A,94:'],
     [(900, 'heal-once'), (1060, 'heal-once-2'), (1270, 'heal-spent'), (1380, 'heal-spent-2')], {}),
    # (the setup's helpers, its Help row's page, on All *, turned on)
    ('all-star', ['--scene', 'setup', '--input', '60:,6:UP,20:,6:A,20:,6:DOWN,10:,6:DOWN,10:,6:DOWN,20:,6:A,60:'], [(200, 'all-star')], {}),
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
    ('second-battle', SECOND_7 + ['--input', PAGED_7 + '60:,0:battle,400:'], [(2230, 'second-battle')], {}, '3ds'),
    ('second-folder', SECOND_7 + ['--input', PAGED_7 + '60:,6:START,40:,6:A,60:,6:A,60:,6:A,60:,6:DOWN,30:'], [(2080, 'second-folder')], {}, '3ds'),
    ('second-navicust', SECOND_7[:-1] + ['quiet,programs=8',
                         '--input', PAGED_7 + '60:,6:START,40:' + ',6:DOWN,8:' * 3 + ',6:A,60:,6:A,90:,6:DOWN,30:'],
     [(2070, 'second-navicust')], {}, '3ds'),
    # (0.10.0's notes: the fight's hand, a Counter's Full Synchro; the PET
    # at home in the town, MegaMan's status and the Library from the PET's
    # menu; a shop's card; the Custom screen's card on an AYN Thor's lower
    # display, 413 x 360, the second screen alone)
    ('second-fight', SECOND_7 + ['--input', SECOND_HERO], [(2380, 'second-fight')], {}, '3ds'),
    ('second-town', ['--scene', 'town', '--seed', '2'], [(280, 'second-town')], {}, '3ds'),
    ('second-status', SECOND_7 + ['--input', PAGED_7 + '60:,6:START,40:,6:DOWN,8:,6:DOWN,8:,6:DOWN,8:,6:A,60:'],
     [(1912, 'second-status')], {}, '3ds'),
    ('second-library', SECOND_7 + ['--input', PAGED_7 + '60:,6:START,40:,6:DOWN,8:,6:DOWN,8:,6:A,90:'],
     [(1925, 'second-library')], {}, '3ds'),
    # (issue #83: Dad's Dive report open in E-Mail, his face beside it; the
    # PET's menu over the home panel in Lan's HP, ACCESSING)
    ('second-mail', SECOND_7 + ['--input', PAGED_7 + '60:,6:START,40:,' + '6:DOWN,8:,' * 4 + '6:A,90:,6:A,90:'],
     [(2050, 'second-mail')], {}, '3ds'),
    ('second-accessing', ['--scene', 'home', '--run-depth', '4', '--seed', '3', '--setup', 'short,0,0,0', '--dev', 'job=2,jobstate=1',
                          '--input', '600:,' + '6:A,94:,' * 8 + '6:START,60:'], [(1460, 'second-accessing')], {}, '3ds'),
    ('second-shop', ['--scene', 'emu', '--run-depth', '4', '--seed', '3', '--dev', 'quiet', '--talk', 'shop:430',
                     '--input', '500:' + ',4:A,96:' * 8 + ',4:A,56:' * 6 + ',6:DOWN,24:' * 2 + ',60:'], [(1740, 'second-shop')], {}, '3ds'),
    ('second-thor', SECOND_7 + ['--second-size', '413x360', '--input', PAGED_7 + '60:,0:battle,400:'], [(2230, 'second-thor')], {}, 'second'),
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
    # 0.11.0's notes. (an ordinary guardian's meeting, issue #100: MegaMan
    # stepped up beside BlastMan, facing him, the chat box below them both)
    ('guardian-meet', ['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '11', '--guardian', '12'],
     [(1730, 'guardian-meet')], {'CYBERWORLD_AUTOPILOT': 'weak'}),
    # (Dad's call after the Cybeast's first fall at seed 7's endless Nest,
    # issue #109: its power is MegaMan's now; one-hit enemies end the
    # battle at once, the autopilot paging the words)
    ('cybeast-den', ['--scene', 'emu', '--run-depth', '19', '--seed', '7', '--dev', 'god,onehit,quiet,hp=800'],
     [(3464, 'cybeast-den')], {'CYBERWORLD_AUTOPILOT': '1'}),
    # (the setup on a profile whose endless Nest fell, issue #99: BeastOut
    # the fifth helper, NEW, turned on)
    ('setup-beast', ['--scene', 'setup', '--marks', '100', '--input', '60:,6:A,20:,6:A,60:'], [(150, 'setup-beast')], {}),
    # (the ROMs screen, issue #97, BN6 and BN5 both ready: a desktop's
    # 1280 x 720 window, halved, and a phone held upright, at a third;
    # screenshots() gives it both ROMs and a Linux player's saves folder)
    ('launcher-desktop', ['--scene', 'launcher', '--size', '1280x720'], [(90, 'launcher-desktop')], {}, 2),
    ('launcher-phone', ['--scene', 'launcher', '--size', '1080x2400', '--dpi', '420'], [(90, 'launcher-phone')], {}, 3),
    # 0.12.0's notes. (SAVES, issue #114, at the game's own 240 x 160: this
    # device's run on seed 7's layer 2 at home; IMPORT, its file another
    # device's, seed 3's layer 8 on a veteran profile, the two side by side;
    # USE THE FILE'S, the result offering CONTINUE and UNDO IMPORT;
    # screenshots() makes both devices' saves, saves_devices)
    ('saves', ['--scene', 'saves', '--size', '240x160', '--input', '60:,6:RIGHT,20:,6:A,80:,6:A,80:'],
     [(50, 'saves-home'), (150, 'saves-compare'), (270, 'saves-result')], {}),
    # (BN5's ACDC Area on a New 3DS's two screens, issue #9: seed 4's first
    # layer walked by the weak autopilot, the layer's map on the bottom one)
    ('3ds-bn5', ['--scene', 'emu', '--net-biome', 'x0', '--seed', '4', '--dev', 'quiet'],
     [(760, '3ds-bn5')], {'CYBERWORLD_AUTOPILOT': 'weak'}, '3ds'),
    # 0.13.0's notes. (the gate's keeper the first time: what the server
    # sees, and his question)
    ('gate-keeper', HP_7 + ['--input', GATE_KEEPER],
     [(1930, 'gate-keeper'), (2140, 'gate-consent')], {}),
    # 0.15.0's notes. (liberation: the fork's dark way and MegaMan's reading
    # of it; the board's Darkloid at the opening, PLAYER PHASE 01, a
    # liberation battle's TURN 1 START! and its 1 TURN LIBERATE!, the
    # Darkloid's line in his phase and his blow's 40 inside his reach lit
    # red, LIBERATE FAILED! as the final turn's gauge fills, the Darkloid's
    # ENEMY DELETED! and the signal's chip at par)
    ('bn5-lib-fork', LIB_FORK, [(420, 'liberation-fork'), (812, 'liberation-way')], {}),
    # (a guardian's draft beside HP+50 held, issue of #211: Attack+1, HP+50
    # Held 1, MegFldr1; the NaviCust with SuperArmor's known code, #212:
    # held over the board compressed, 4 blocks)
    ('navicust-held', ['--scene', 'emu', '--run-depth', '3', '--seed', '11', '--dev', 'program=41', '--talk', 'reward:300',
                       '--input', '400:,' + '4:A,40:,' * 6 + '200:'], [(700, 'navicust-held')], {}),
    ('navicust-compressed', SECOND_7[:-1] + ['quiet,programs=2,codes=2',
                             '--input', PAGED_7 + '60:,6:START,40:' + ',6:DOWN,8:' * 3 + ',6:A,60:,6:A,90:,6:A,40:'],
     [(2054, 'navicust-compressed')], {}),
    ('bn5-lib-board', LIB_BOARD,
     [(144, 'liberation-darkloid'), (360, 'liberation-phase'), (1680, 'liberation-turn'), (1830, 'liberation-win'),
      (13760, 'liberation-showtime'), (13850, 'liberation-reach'), (16450, 'liberation-failed'),
      (23600, 'liberation-deleted'), (23900, 'liberation-par'), (23972, 'liberation-chip')], {'CYBERWORLD_AUTOPILOT': '1'}),
    # 0.16.0's notes. (Options' Audio with a DualSense attached: the cursor
    # on Vibration, then on Trigger effects, each explained)
    ('options-feel', ['--scene', 'title', '--pad', 'ps5', '--input', '60:,4:SELECT,6:,40:,4:R,6:,60:,'
                      '4:DOWN,6:,20:,4:DOWN,6:,20:,4:DOWN,6:,30:,4:DOWN,6:,30:'], [(279, 'options-vibration'), (319, 'options-triggers')], {}),
    # (the Virus Battler: MegaMan by him, the card; then a series)
    ('vb-card', VB_RUN + ['--input', VB_MET + '4:A,6:,110:,' * 4], [(398, 'vb-battler'), (860, 'vb-card')], {}),
    ('vb-series', VB_HELD + ['--input', VB_SERIES],
     [(750, 'vb-pair'), (1701, 'vb-list'), (3780, 'vb-choice'), (3926, 'vb-cashout')], {}),
    # (the Cross Navis on a dark board: the TEAM screen, ChargeMan's turn)
    ('bn5-team-pick', LIB_TEAM + ['--input', '600:,4:UP,6:,30:'], [(639, 'liberation-team')], {}),
    ('bn5-lib-charge', LIB_CHARGE, [(3420, 'liberation-chargeman'), (3740, 'liberation-bomb')], {'CYBERWORLD_AUTOPILOT': '1'}),
    # 0.17.0's notes. (the requests on one board: BN6's Request BBS with
    # the visit's three posts, one read out; the courier naming an offer)
    ('home-bbs', ['--scene', 'town', '--seed', '2', '--run-depth', '7', '--input', HOME_BBS],
     [(1600, 'home-bbs'), (2090, 'home-bbs-post')], {}),
    ('home-offers', HP_2 + ['--input', COURIER_OFFERS], [(1530, 'home-courier-offer')], {}),
    # (ElecMan met again five runs after he won (SCREENSHOT_PREP's run;
    # --dev veteran makes the profile's runs six), the weak autopilot
    # walking into his arena: his second box names how long it has been)
    ('guardian-memory', ['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '11', '--guardian', '2', '--dev', 'veteran,quiet'],
     [(1680, 'guardian-memory')], {'CYBERWORLD_AUTOPILOT': 'weak'}),
    # (the three new starting folders on the setup's Folder row, each
    # explained; each one's first hand, Z-Canon's cursor on Cannon A)
    ('setup-folders', ['--scene', 'setup', '--dev', 'folders', '--input', '60:,6:DOWN,20:,6:DOWN,40:,' + '6:RIGHT,34:,' * 6],
     [(245, 'setup-zcanon'), (285, 'setup-busting'), (325, 'setup-lean')], {}),
    ('hand-zcanon', FIRST_HAND + ['--setup', 'endless,3,0,0', '--input', FIRST_HAND_IN + '6:RIGHT,100:'], [(1040, 'hand-zcanon')], {}),
    ('hand-busting', FIRST_HAND + ['--setup', 'endless,4,0,0', '--input', FIRST_HAND_IN + '100:'], [(1040, 'hand-busting')], {}),
    ('hand-lean', FIRST_HAND + ['--setup', 'endless,5,0,0', '--input', FIRST_HAND_IN + '100:'], [(1040, 'hand-lean')], {}),
    # 0.17.0's notes. (MegaMan reads the way ahead, docs/world/level-design/
    # fork-read.md: seed 30's layer 4, a dark warp beside its exit pad,
    # MegaMan put within three panels of the pad by the warp; Lan's HP
    # before act 3 on seed 7, A facing the pink pad, its third box the read)
    ('fork-pad', ['--scene', 'emu', '--run-depth', '4', '--seed', '30', '--dev', 'quiet', '--input', '300:,0:place 284 -300 1,110:'],
     [(405, 'fork-pad')], {}),
    ('fork-portal', ['--scene', 'home', '--run-depth', '7', '--seed', '7', '--input',
                     HP_PAGED + '0:place 160 37 0,20:,6:A,100:,6:A,90:,6:A,90:'], [(1516, 'fork-portal')], {}),
    # (a won Server pays a pick: seed 8's layer 4, its offer naming the
    # three, Yes, the battle won by the buster down each row, then the pick)
    ('server-pick', SERVER_8 + ['--input', SERVER_PICK], [(546, 'server-offer'), (2556, 'server-pick')], {}),
    # (BugFrags buy programs: seed 3's layer 2, an act's middle layer, 50
    # BugFrags given, Chaud's call paged, MegaMan beside the NaviCust
    # vendor; her greeting, then BN6's shop screen in BugFrags; with BN5's
    # ROM beside BN6's, which dresses the layer as the Nest's copy)
    ('bn5-vendor', ['--scene', 'emu', '--run-depth', '2', '--seed', '3', '--dev', 'quiet', '--talk', 'bugfrags:100', '--input',
                    '1:,300:,' + '6:A,6:,60:,' * 21 + '30:,0:place -12 -260 7,20:,6:A,6:,90:,' + '6:A,6:,80:,' * 2],
     [(1963, 'vendor-greeting'), (2147, 'vendor-shop')], {}),
    # (a battle on every layer: MegaMan steps onto the exit pad of a layer
    # still owed its battle, and the battle starts there)
    ('floor-pad', FLOOR_PAD + ['--input', FLOOR_PAD_INPUT], [(863, 'floor-pad'), (868, 'floor-pad-start')], {}),
    # 0.19.0's notes. (a GBA's screen: one battle frame with each Colour
    # correction, side by side in SCREENSHOT_SHEETS; LCD 3x, the whole
    # screen at 720x480, its stripes 1:1; the map held over a Sky layer,
    # Off and PSP: the engine's drawing recoloured with BN6's picture)
    ('gba-off', GBA_SCREEN, [(GBA_SHOT, 'gba-off')], {}),
    ('gba-gba', GBA_SCREEN + ['--colour-correction', 'gba'], [(GBA_SHOT, 'gba-gba')], {}),
    ('gba-vba', GBA_SCREEN + ['--colour-correction', 'vba'], [(GBA_SHOT, 'gba-vba')], {}),
    ('gba-nds', GBA_SCREEN + ['--colour-correction', 'nds'], [(GBA_SHOT, 'gba-nds')], {}),
    ('gba-psp', GBA_SCREEN + ['--colour-correction', 'psp'], [(GBA_SHOT, 'gba-psp')], {}),
    ('gba-lcd3x', GBA_SCREEN + ['--lcd3x', 'on'], [(GBA_SHOT, 'gba-lcd3x')], {}, 1),
    ('map-off', MAP_SKY, [(755, 'map-off')], {}),
    ('map-psp', MAP_SKY + ['--colour-correction', 'psp'], [(755, 'map-psp')], {}),
    # (the sound your way, #270: Options' Audio from the title, Music turned
    # down to 7; then down to Output, Headphones, explained)
    ('options-sound', ['--scene', 'title', '--input', '60:,4:SELECT,6:,40:,4:R,6:,60:,' + '4:LEFT,6:,10:,' * 3 + '40:,' +
                       '4:DOWN,6:,20:,' * 2 + '4:RIGHT,6:,50:'], [(275, 'options-levels'), (395, 'options-output')], {}),
    # 0.20.0's notes. (the setup's EXTRAS page, BN4's ROM beside BN6's so
    # Older nets is on it: Glitch and DarkChips switched off, the cursor on
    # Older nets)
    ('setup-extras-bn4', ['--scene', 'setup', '--input', '60:,6:R,30:,6:R,40:,' + '6:DOWN,10:,' * 3 + '6:A,10:,' + '6:DOWN,10:,' * 2 +
                      '6:A,10:,6:DOWN,40:'], [(420, 'setup-extras')], {}),
    # (BN4 Red Sun's tournament, its ROM beside BN6's: the bracket before
    # round 1 and after it, the round's chip between)
    ('bn4-bracket', TOURNEY, [(1060, 'bn4-bracket'), (2912, 'bn4-round-chip'), (2930, 'bn4-bracket-after')],
     {'CYBERWORLD_AUTOPILOT': '1'}),
    # (BN4's seven Navis, each in a BN6 battle on BN4's ACDC Area, at a
    # frame of his attack; Roll with the Mettaur her summon brought)
    ('bn4-gutsman', BN4_NAVI + ['--input', '460:,0:foes 384@52'], [(1275, 'bn4-gutsman')], {'CYBERWORLD_AUTOPILOT': '1'}),
    ('bn4-thunderman', BN4_NAVI + ['--input', '460:,0:foes 394@52'], [(1725, 'bn4-thunderman')], {'CYBERWORLD_AUTOPILOT': '1'}),
    ('bn4-windman', BN4_NAVI + ['--input', '460:,0:foes 388@52'], [(975, 'bn4-windman')], {'CYBERWORLD_AUTOPILOT': '1'}),
    ('bn4-fireman', BN4_NAVI + ['--input', '460:,0:foes 390@52'], [(2025, 'bn4-fireman')], {'CYBERWORLD_AUTOPILOT': '1'}),
    ('bn4-searchman', BN4_NAVI + ['--input', '460:,0:foes 38C@52'], [(1725, 'bn4-searchman')], {'CYBERWORLD_AUTOPILOT': '1'}),
    ('bn4-aquaman', BN4_NAVI + ['--input', '460:,0:foes 3A8@52'], [(1950, 'bn4-aquaman')], {'CYBERWORLD_AUTOPILOT': '1'}),
    ('bn4-roll', BN4_NAVI + ['--input', '460:,0:foes 380@52'], [(2550, 'bn4-roll')], {'CYBERWORLD_AUTOPILOT': '1'}),
    # (BN4's areas, each its first layer: ACDC, Town and the countries; its
    # Park Area (x17) and Undernet (x19) left out while some of their
    # walkways' edges and pads draw wrong tiles)
    ('bn4-acdc', BN4_AREA + ['--net-biome', 'x15'], [(700, 'bn4-acdc')], {'CYBERWORLD_AUTOPILOT': 'weak'}),
    ('bn4-town', BN4_AREA + ['--net-biome', 'x16'], [(400, 'bn4-town')], {'CYBERWORLD_AUTOPILOT': 'weak'}),
    ('bn4-countries', BN4_AREA + ['--net-biome', 'x18'], [(650, 'bn4-countries')], {'CYBERWORLD_AUTOPILOT': 'weak'}),
    # (BN4's Billy, Walla and CirKill in a BN6 battle of Central Area, named
    # on BN6's Custom screen, then Walla's ring and Billy's thunder)
    ('bn4-visitors', ['--scene', 'emu', '--net-biome', '0', '--run-depth', '4', '--seed', '4', '--dev', 'quiet,god',
                      '--input', '460:,0:foes 2C0@41/2D2@52/2DE@63'], [(700, 'bn4-visitors'), (1300, 'bn4-visitors-fight')],
     {'CYBERWORLD_AUTOPILOT': '1'}),
    # (the Glitch layer at its last stage, another area's look flipped in)
    ('glitch-layer', GLITCH_LAYER, [(950, 'glitch-flip')], {}),
    # 0.21.0 (B)
    # (BN4's chips in a BN6 battle: Meteors1's card on the Custom screen and
    # a meteor falling, MagBolt1's pull, RollAro1's arrow, GutPnch1's punch)
    ('bn4-chip-meteors', B_CHIP + ['--dev', 'quiet,god,folder=674', '--input', '460:,' + B_METT],
     [(700, 'bn4-chip-meteors-card'), (940, 'bn4-chip-meteors')], B_AP),
    ('bn4-chip-magbolt', B_CHIP + ['--dev', 'quiet,god,folder=642', '--input', '460:,' + B_METT], [(940, 'bn4-chip-magbolt')], B_AP),
    ('bn4-chip-rollaro', B_CHIP + ['--dev', 'quiet,god,folder=665', '--input', '460:,' + B_METT], [(1040, 'bn4-chip-rollaro')], B_AP),
    ('bn4-chip-gutpnch', B_CHIP + ['--dev', 'quiet,god,folder=668', '--input', '460:,' + B_METT], [(1020, 'bn4-chip-gutpnch')], B_AP),
    # (BN4's HeatSprd: HeatShot, Heat-V and HeatSide D named as it forms,
    # its name, its burst over the three Mettaur3)
    ('bn4-pa-heatsprd', B_CHIP + ['--dev', 'quiet,god', '--input', B_HEATSPRD],
     [(1485, 'bn4-pa-heatsprd-parts'), (1530, 'bn4-pa-heatsprd'), (1770, 'bn4-pa-heatsprd-burst')], {}),
    # (a Net Dealer's stock on BN4's ACDC Area: Ligtnin1 W and its card)
    ('bn4-dealer', ['--scene', 'emu', '--net-biome', 'x15', '--run-depth', '2', '--seed', '4', '--dev', 'quiet', '--talk', 'shop:1500',
                    '--input', B_DEALER], [(2700, 'bn4-dealer')], {}),
    # (GutsMan guarding BN4's ACDC Area: his card, his fight)
    ('bn4-guardian', ['--scene', 'emu', '--net-biome', 'x15', '--run-depth', '3', '--guardian', '35', '--dev', 'god'],
     [(1550, 'bn4-guardian-card'), (2900, 'bn4-guardian-fight')], B_AP),
    # (the tournament forced on act 3, the pool at its 48 (#473, which
    # changed every draw): seed 11's bracket and round 1's card, JunkMan;
    # seed 3's, TopMan)
    ('bn4-tourney-junkman', ['--scene', 'emu', '--run-depth', '9', '--seed', '11', '--dev', 'tourney,god,onehit,quiet'],
     [(1038, 'bn4-bracket-junkman'), (1200, 'bn4-round-junkman')], B_AP),
    ('bn4-tourney-topman', ['--scene', 'emu', '--run-depth', '9', '--seed', '3', '--dev', 'tourney,god,onehit,quiet'],
     [(1402, 'bn4-bracket-topman'), (1570, 'bn4-round-topman')], B_AP),
    # (seed 9's round 1, BurnerMan, and seed 22's, SparkMan, each fought in
    # its tournament at the round's HP: the bracket, the card, the fight;
    # then, the autopilot's folder spent, the onehit step ends it: BN6's
    # RESULT, the round's chip, the bracket again with his plate grey and
    # MegaMan's a step on)
    ('bn4-tourney-burnerman', ['--scene', 'emu', '--run-depth', '9', '--seed', '9', '--dev', 'tourney,god,hp=999,quiet',
                               '--input', '9000:,0:onehit'],
     [(1336, 'bn4-bracket-burnerman'), (1500, 'bn4-round-burnerman'), (2300, 'bn4-round-burnerman-fight'),
      (10190, 'bn4-round-burnerman-result'), (10294, 'bn4-round-burnerman-chip'), (10315, 'bn4-bracket-burnerman-after')], B_AP),
    ('bn4-tourney-sparkman', ['--scene', 'emu', '--run-depth', '9', '--seed', '22', '--dev', 'tourney,god,hp=999,quiet',
                              '--input', '5000:,0:onehit'],
     [(840, 'bn4-bracket-sparkman'), (1060, 'bn4-round-sparkman'), (2500, 'bn4-round-sparkman-fight'),
      (10160, 'bn4-round-sparkman-result'), (10258, 'bn4-round-sparkman-chip'), (10272, 'bn4-bracket-sparkman-after')], B_AP),
    # (the tournament's four new Navis, each in a BN6 battle on BN4's ACDC
    # Area by the foes step, at a frame of his attack)
    ('bn4-topman', BN4_NAVI + ['--input', '460:,0:foes 3B0@52'], [(1990, 'bn4-topman')], B_AP),
    ('bn4-sparkman', BN4_NAVI + ['--input', '460:,0:foes 3C0@52'], [(1150, 'bn4-sparkman')], B_AP),
    ('bn4-burnerman', BN4_NAVI + ['--input', '460:,0:foes 3B8@52'], [(1150, 'bn4-burnerman')], B_AP),
    ('bn4-junkman', BN4_NAVI + ['--input', '460:,0:foes 3A4@52'], [(1120, 'bn4-junkman')], B_AP),
    # (the dark boards' two: CloudMan on seed 9's act 4 board, CosmoMan on
    # its act 5 board; each in a battle by the foes step)
    ('bn5-cloudman-board', ['--scene', 'emu', '--run-depth', '11', '--seed', '9', '--dev', 'dark,god,onehit'],
     [(120, 'bn5-cloudman-board')], B_AP),
    ('bn5-cosmoman-board', ['--scene', 'emu', '--run-depth', '14', '--seed', '9', '--dev', 'dark,god,onehit'],
     [(200, 'bn5-cosmoman-board')], B_AP),
    ('bn5-cloudman', B5_NAVI + ['--input', '460:,0:foes 1E8@52'], [(2080, 'bn5-cloudman')], B_AP),
    ('bn5-cosmoman', B5_NAVI + ['--input', '460:,0:foes 1EC@52'], [(1270, 'bn5-cosmoman')], B_AP),
    # (the deep's two guarding an Undernet act 5: the card and his Guardian
    # Data's SP chip, his fight won in one hit; LarkMan V2 and GridMan V3 at
    # their HP in a battle on the Undernet by the foes step)
    ('bn5-larkman-guard', B_DEEP + ['--guardian', '65'], [(3750, 'bn5-larkman-card'), (4980, 'bn5-larkman-chip')], B_AP),
    ('bn5-gridman-guard', B_DEEP + ['--guardian', '66'], [(3750, 'bn5-gridman-card'), (5740, 'bn5-gridman-chip')], B_AP),
    ('bn5-larkman', B_DEEP_FOES + ['--input', '460:,0:foes 3ED@52'], [(1000, 'bn5-larkman')], B_AP),
    ('bn5-gridman', B_DEEP_FOES + ['--input', '460:,0:foes 3F2@52'], [(1110, 'bn5-gridman')], B_AP),
    # (BN5's Navi chips: ShadeMan's, CosmoMan's; WildBird formed of two
    # Katana1 and LarkMan SP, and fired; CrosOver of Django's three)
    ('bn5-chip-shademan', B_CHIP + ['--dev', 'quiet,god,folder=505', '--input', '460:,' + B_METT], [(1060, 'bn5-chip-shademan')], B_AP),
    ('bn5-chip-cosmoman', B_CHIP + ['--dev', 'quiet,god,folder=515', '--input', '460:,' + B_METT], [(1140, 'bn5-chip-cosmoman')], B_AP),
    ('bn5-pa-wildbird', B_CHIP + ['--dev', 'quiet,god', '--input', B_WILDBIRD],
     [(1395, 'bn5-pa-wildbird-parts'), (1440, 'bn5-pa-wildbird'), (1860, 'bn5-pa-wildbird-dive')], {}),
    ('bn5-pa-crosover', B_CHIP + ['--dev', 'quiet,god', '--input', B_CROSOVER],
     [(1485, 'bn5-pa-crosover-parts'), (1530, 'bn5-pa-crosover')], {}),
    # (SoulT+1 in Colonel's draft, and taken)
    ('bn5-soult', ['--scene', 'emu', '--run-depth', '3', '--net-biome', 'x0', '--guardian', '24', '--dev', 'quiet', '--talk', 'reward:400',
                   '--input', B_SOULT], [(1440, 'bn5-soult-draft'), (1520, 'bn5-soult-got')], {}),
    # (BN4's VolGear, and Kilby with MagTect, in a BN6 battle of Central
    # Area by the foes step: named on the Custom screen, then at work)
    ('bn4-volgear', B_CHIP + ['--dev', 'quiet,god', '--input', '460:,0:foes 2F0@52/2F0@63'],
     [(680, 'bn4-volgear-names'), (1240, 'bn4-volgear')], B_AP),
    ('bn4-kilby-magtect', B_CHIP + ['--dev', 'quiet,god', '--input', '460:,0:foes 2F6@52/2FC@63/2F6@41'],
     [(740, 'bn4-kilby-magtect-names'), (1140, 'bn4-kilby-magtect')], B_AP),
    # (BN5's arrow lanes, forced on seed 10's Oran Area and seed 2's
    # Undernet, MegaMan put at the lane's foot; a battle won on BN5's ACDC
    # Area, its results)
    ('bn5-lane-oran', ['--scene', 'emu', '--run-depth', '4', '--net-biome', 'x4', '--seed', '10', '--dev', 'quiet,pieces=32',
                       '--input', B_LANE + '0:place 164 -116 1,200:'], [(700, 'bn5-lane-oran')], {}),
    ('bn5-lane-undernet', ['--scene', 'emu', '--run-depth', '4', '--net-biome', 'x5', '--seed', '2', '--dev', 'quiet,pieces=32',
                           '--input', B_LANE + '0:place 68 -36 1,200:'], [(700, 'bn5-lane-undernet')], {}),
    ('bn5-win', ['--scene', 'emu', '--run-depth', '3', '--net-biome', 'x0', '--guardian', '24', '--dev', 'god,onehit,hp=999'],
     [(1670, 'bn5-win')], B_AP),
    # (Team ProtoMan's MagnetMan, a test fight by the foes step: no run
    # meets him yet)
    ('bn5-magnetman', B5_NAVI + ['--input', '460:,0:foes 3E4@52'], [(1180, 'bn5-magnetman')], B_AP),
    # 0.21.0 (A)
    ('lab-build', LAB_BUILD, [(1000, 'lab-foes'), (2000, 'lab-field'), (2140, 'lab-stage'), (2260, 'lab-stage-song'),
                              (2340, 'lab-stage-folder'), (2460, 'lab-folder')], {}),
    # (a test battle won: the slash through both Mettaurs, BN6's RESULT with
    # NO DATA, the builder back with the time, SETS with the set's best)
    ('lab-won', LAB + ['--input', LAB_WON], [(1250, 'lab-running'), (1600, 'lab-result'), (1800, 'lab-time'), (1880, 'lab-sets')], {}),
    # (the keeper at the lab's station in Lan's HP (--dev lab: the Lab sent),
    # MegaMan put before him, his three boxes)
    ('lab-keeper', HP_2 + ['--dev', 'lab', '--input', HP_PAGED + '0:place -76 -34 7,20:,6:A,100:,6:A,100:,6:A,100:,100:'],
     [(1218, 'lab-keeper'), (1320, 'lab-keeper-talk'), (1425, 'lab-keeper-data'), (1530, 'lab-keeper-offer')], {}),
    # (Dad's mail on a run's first layer: E-Mail, down to Battle Lab, read)
    ('lab-mail', LAB[:-1] + ['quiet,lab', '--input', LAYER_15 + '6:START,40:,' + '6:DOWN,10:,' * 4 + '6:A,60:,' + '6:DOWN,10:,' * 3 +
                             '6:A,100:,' + '6:A,100:,' * 5],
     [(965, 'lab-mail'), (1175, 'lab-mail-2'), (1340, 'lab-mail-3')], {}),
    # (a profile with one win over Bass and the Cybeast, `lab open` listing
    # what it has: Gregar set, alone; his battle; the blow that would delete
    # MegaMan ending it, the builder back)
    ('lab-navi', LAB[:-1] + ['quiet,bass=1,beast=1', '--input', LAYER_15 + '0:lab open,60:,6:DOWN,24:,6:A,24:,6:A,24:,6:B,100:,'
                             '6:START,90:,6:A,20:,6:START,20:,6:A,400:'],
     [(860, 'lab-navi'), (1210, 'lab-navi-battle'), (1460, 'lab-navi-deleted')], {}),
    # (the Library, a page per game: every third chip in the profile's
    # Library (--dev library=3), every ROM read; BN6's StdChip page, right
    # to its MegaChip and GigaChip, BN5's three, BN4's three)
    ('library-pages', ['--scene', 'town', '--seed', '2', '--dev', 'library=3', '--input', PET_ROOM + '6:DOWN,6:,6:DOWN,6:,6:A,60:,' +
                       '6:RIGHT,34:,' * 3 + '6:R,34:,' + '6:RIGHT,34:,' * 3 + '6:R,34:,6:R,34:,6:DOWN,14:,' + '6:RIGHT,34:,' * 2 + '40:'],
     [(640, 'library-bn6'), (680, 'library-bn6-mega'), (720, 'library-bn6-giga'), (800, 'library-bn5'), (840, 'library-bn5-mega'),
      (1042, 'library-bn4'), (1076, 'library-bn4-mega'), (1120, 'library-bn4-giga')], {}),
    # (the Jukebox with BN4's ROM read: R to its BN4 Towns section, a song played)
    ('jukebox-bn4', ['--scene', 'town', '--seed', '2', '--input', PET_ROOM + '6:DOWN,6:,' * 5 + '6:A,60:,6:DOWN,6:,6:RIGHT,14:,6:A,40:,' +
                     '6:R,34:,' * 11 + '6:DOWN,14:,6:DOWN,14:,6:A,200:'],
     [(1390, 'jukebox-bn4')], {}),
    # (the act's card naming its family; the three beats, each layer's first
    # battle on BN6's Custom screen: Gunner alone, beside Catack, in its
    # constellation with Armadill and Piranha; side by side in a sheet)
    ('groups-act', GROUPS + ['--run-depth', '4', '--input', GROUPS_IN], [(150, 'groups-act-card'), (880, 'groups-beat-1')], {}),
    ('groups-second', GROUPS + ['--run-depth', '5', '--input', GROUPS_IN], [(880, 'groups-beat-2')], {}),
    ('groups-third', GROUPS + ['--run-depth', '6', '--input', GROUPS_IN], [(880, 'groups-beat-3'), (1100, 'groups-beat-3-fight')], {}),
    # (BN4's Walla the signature of act 3 in BN6's Sky Area, every ROM read:
    # seed 6's layer 7, its card and its first battle)
    ('groups-bn4-walla', ['--scene', 'emu', '--seed', '6', '--run-depth', '7', '--net-biome', '2', '--dev', 'quiet', '--input', GROUPS_IN],
     [(150, 'groups-bn4-card'), (880, 'groups-bn4-battle'), (1100, 'groups-bn4-fight')], {}),
    # (the Chip Folder and a Net Dealer's stock on a layer of BN4's ACDC
    # Area, BN4's Ligtnin1 on his list and its card; Lan's HP before one of
    # BN4's areas)
    ('bn4-folder', BN4_FOLDER, [(760, 'bn4-folder')], {}),
    ('bn4-shop', BN4_SHOP, [(2200, 'bn4-shop'), (2360, 'bn4-shop-card')], {}),
    ('bn4-home', BN4_HOME, [(1300, 'bn4-home')], {}),
    # (the opening gift's pick: HPMemory, the chip, the program; a
    # guardian's draft)
    ('pick-gift', PICK_GIFT, [(480, 'pick-gift'), (600, 'pick-gift-chip'), (660, 'pick-gift-program')], {}),
    ('pick-draft', PICK_DRAFT, [(640, 'pick-draft')], {}),
    # (the NetPass: with BN6's and BN5's ROMs alone, the greeter's word on
    # the new slot and the pass with BN4's slot open; with BN4's too, full)
    ('netpass-slot', NETPASS_GREW, [(750, 'netpass-slot-greeter'), (930, 'netpass-slot-shut'), (1155, 'netpass-slot')], {}),
    ('netpass-bn4', LAB + ['--input', LAYER_15 + '0:netpass,200:'], [(760, 'netpass-bn4')], {}),
    # (Showdown and Sorrow, heard, not seen: ProtoMan's netbattle as it
    # starts, and a lost run's summary)
    ('showdown', SHOWDOWN, [(2060, 'showdown-custom'), (2180, 'showdown-netbattle')], {}),
    ('sorrow', ['--scene', 'summary', '--run-depth', '6', '--seed', '3'], [(300, 'sorrow-summary')], {}),
    # (NumbrOpn's ten chips beside five: numbropn-ten.png and numbropn-five.png
    # are the differential's own pictures of BN6's Custom screen, tests/xdiff's
    # ncp_numbropn and ncp_numbropn_control with "shots": [[6, 1, 30, PATH]]
    # added, by `build.py xdiff`; the sheet lays them side by side)
    # 0.21.0 (A) ends: stills
    # 0.21.1
    # (BN4's Park Area's first layer, seed 4, as the weak autopilot walks it:
    # the orange road on the way to the exit between two framed rooms)
    ('bn4-park-road', ['--scene', 'emu', '--run-depth', '1', '--dev', 'quiet', '--net-biome', 'x17', '--seed', '4'],
     [(320, 'bn4-park-road')], {'CYBERWORLD_AUTOPILOT': 'weak'}),
    # (Depth lighting off and on, the whole screen at 3x: Sky Area's first
    # layer, seed 2, and the run's first random battle on layer 6 of the
    # clips' RUN_7, written out: it is set further down)
    ('depth-off', ['--scene', 'emu', '--run-depth', '1', '--dev', 'quiet', '--net-biome', '2', '--seed', '2', '--size', '720x480',
                   '--depth-lighting', 'off'], [(600, 'depth-off')], {'CYBERWORLD_AUTOPILOT': 'weak'}, 1),
    ('depth-on', ['--scene', 'emu', '--run-depth', '1', '--dev', 'quiet', '--net-biome', '2', '--seed', '2', '--size', '720x480',
                  '--depth-lighting', 'on'], [(600, 'depth-on')], {'CYBERWORLD_AUTOPILOT': 'weak'}, 1),
    ('depth-battle', ['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '11', '--guardian', '12',
                      '--size', '720x480', '--depth-lighting', 'on'], [(7700, 'depth-battle')], {'CYBERWORLD_AUTOPILOT': 'weak'}, 1),
]

# 0.22.0's notes: saves, the dive, systems
# (the routes are the 0.22.0 trailer's, tools/trailers/v0_22_0_footage.py,
# written out: a run's arrival on seed 7's layer 5 paged so that its
# checkpoint is written, the soft reset to the title (A+B+SELECT+START) and
# L, SAVES over a title with a run saved, at the 240x160 it lays itself out
# for; its pages turned with R; CLOUD's Turn on, its question, Yes and the
# sync, and CODE's Send and its code, both on a Net Square server of this
# machine (SCREENSHOT_SERVERS); the title's menu with that run's CONTINUE)
N22_ARRIVE = '150:,' + '6:A,4:,' * 30 + '20:,'
N22_TITLE = '100:,' + '4:A,6:,' * 30 + '100:,6:A+B+SELECT+START,200:,'
N22_SAVES = N22_TITLE + '6:L,60:,'
N22_SAVED = ['--scene', 'emu', '--run-depth', '5', '--seed', '7', '--dev', 'quiet']
N22_SQUARE_PORT, N22_UPDATE_PORT = 18732, 18733
N22_ONLINE = ['--scene', 'emu', '--run-depth', '5', '--seed', '7', '--dev', f'quiet,online=ws://172.17.0.1:{N22_SQUARE_PORT}/presence',
              '--size', '240x160']
# (the update box on the title, a look for a made-up update.json of
# 0.22.0 from a build that calls itself 0.21.2: the box as START first
# shows it, a page on, Later; SELECT, Options' Game on its Update line)
N22_UPDATE = '240:,6:pad.start,180:,6:pad.dpright,80:,6:pad.b,60:,6:pad.back,160:'
# (Options from the PET on seed 4's second layer, its Audio page; the
# builder with two Mettaurs set, B at FOES; the Jukebox from Lan's room,
# L from BN6's Towns six sections back to BN5's Unused)
N22_OPTIONS = N22_ARRIVE + '6:START,40:,' + '6:DOWN,6:,' * 6 + '6:A,120:,6:R,60:'
N22_JUKEBOX = ('40:,' + '6:A,4:,' * 46 + '30:,6:START,6:,40:,' + '6:DOWN,6:,' * 5 + '6:A,6:,60:,6:DOWN,6:,6:RIGHT,6:,14:,6:A,6:,40:,'
               + '40:,6:L,60:,6:DOWN,40:,6:DOWN,60:,' + '6:L,10:,' * 6 + '60:')
SCREENSHOTS += [
    ('saves-pages', N22_SAVED + ['--size', '240x160', '--input', N22_SAVES + '70:,6:R,74:,6:DOWN,40:,6:R,120:'],
     [(800, 'saves-page-cloud'), (890, 'saves-page-code'), (1010, 'saves-page-file')], {}),
    ('saves-cloud', N22_ONLINE + ['--input', N22_SAVES + '60:,6:A,90:,6:LEFT,40:,6:A,900:'],
     [(960, 'saves-cloud-ask'), (1870, 'saves-cloud')], {}),
    ('saves-code', N22_ONLINE + ['--input', N22_SAVES + '60:,6:R,60:,6:A,900:'], [(1800, 'saves-code')], {}),
    ('update-box', ['--scene', 'title', '--pad', 'xbox', '--input', N22_UPDATE],
     [(306, 'update-box'), (500, 'update-box-page'), (560, 'update-hint'), (790, 'options-update')], {}),
    ('options-touch', ['--scene', 'title', '--touch', '--input', '60:,4:SELECT,100:,4:L,6:,60:,4:DOWN,6:,110:,4:RIGHT,6:,170:'],
     [(410, 'options-touch')], {}),
    ('options-depth', ['--scene', 'title', '--input', '60:,4:SELECT,100:,4:UP,6:,90:,4:RIGHT,6:,170:'], [(310, 'options-depth')], {}),
    ('options-music', ['--scene', 'emu', '--run-depth', '2', '--seed', '4', '--dev', 'quiet', '--input', N22_OPTIONS], [(760, 'options-music')], {}),
    # (Depth lighting on a layer with heights, the whole screen at 3x: Green
    # Area's layer 4 of seed 2, MegaMan up its yellow stair on the terrace)
    ('depth-climb', ['--scene', 'emu', '--run-depth', '4', '--seed', '2', '--net-biome', '3', '--dev', 'quiet', '--size', '720x480',
                     '--depth-lighting', 'on', '--input', N22_ARRIVE + '100:UP+RIGHT,60:'], [(600, 'depth-climb')], {}, 1),
    ('lab-leave', LAB + ['--input', LAB_METS + '6:B,60:'], [(936, 'lab-leave')], {}),
    ('title-best', N22_SAVED + ['--input', N22_TITLE + '60:,6:START,150:'], [(840, 'title-best')], {}),
    ('jukebox-bn5-unused', ['--scene', 'town', '--seed', '2', '--input', N22_JUKEBOX], [(1144, 'jukebox-bn5-unused')], {}),
]
WHOLE_ROMS_A += ('lab-leave',)
# (the pictures that talk to a server of this machine, started before the
# run and stopped after it: 'square' a Net Square server from source,
# 'update' a folder with a release's update.json over HTTP)
SCREENSHOT_SERVERS = {'saves-cloud': 'square', 'saves-code': 'square', 'update-box': 'update'}
N22_SQUARE_INI = ('[server]\nname = Cyberworld Endless\nmotd = Welcome\nhost = 0.0.0.0\nport = {port}\nseed = 120\ncapacity = 8\n'
                  'player_cap = 64\ntick_hz = 20\nstate_dir = {state}\nwhitelist =\nfree_text = false\ntrading = true\narena = true\norigins =\n')


def screenshot_server(kind, tmp):
    """A picture's server (SCREENSHOT_SERVERS), which a container reaches at
    Docker's bridge address: its process (None where the picture needs none)
    and what the run's environment gets for it."""
    import time
    if kind == 'square':
        state = os.path.join(tmp, 'square')
        os.makedirs(state)
        ini = os.path.join(state, 'server.ini')
        with open(ini, 'w', encoding='utf-8') as f:
            f.write(N22_SQUARE_INI.format(port=N22_SQUARE_PORT, state=state))
        server, env = subprocess.Popen([sys.executable, os.path.join(ROOT, 'server', 'main.py'), '--config', ini],
                                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL), []
    elif kind == 'update':
        folder = os.path.join(tmp, 'update')
        os.makedirs(folder)
        with open(os.path.join(folder, 'update.json'), 'w', encoding='utf-8') as f:
            if subprocess.call([sys.executable, os.path.join(ROOT, 'tools', 'update_manifest.py'), 'v0.22.0', folder], stdout=f):
                sys.exit('screenshot_server: no update.json')
        # (the notes' pictures from this server too: the release's own are not on GitHub before the tag)
        with open(os.path.join(folder, 'update.json'), encoding='utf-8') as f:
            manifest = f.read().replace('https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases/download/v0.22.0/'
                                        'update-pictures.bin', f'http://172.17.0.1:{N22_UPDATE_PORT}/update-pictures.bin')
        with open(os.path.join(folder, 'update.json'), 'w', encoding='utf-8') as f:
            f.write(manifest)
        server = subprocess.Popen([sys.executable, '-m', 'http.server', str(N22_UPDATE_PORT), '--bind', '0.0.0.0', '--directory', folder],
                                  stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        env = [('CYBERWORLD_UPDATE_URL', f'http://172.17.0.1:{N22_UPDATE_PORT}/update.json'), ('CYBERWORLD_UPDATE_FROM', '0.21.2')]
    else:
        return None, []
    time.sleep(2)
    if server.poll() is not None:
        sys.exit(f'screenshot_server: the {kind} server did not start')
    return server, env

# pictures of others side by side for a release's notes (made after the
# screenshots they hold): name, then its rows of (screenshot, label), each
# at 2x under its label as tools/before_after.py's pairs
SCREENSHOT_SHEETS = {
    'gba-colour': [[('gba-off', 'Off'), ('gba-gba', 'Colour correction: GBA')]],
    'gba-profiles': [[('gba-gba', 'GBA'), ('gba-vba', 'VBA')], [('gba-nds', 'DS'), ('gba-psp', 'PSP')]],
    'gba-map': [[('map-off', 'Off'), ('map-psp', 'Colour correction: PSP')]],
    'bn4-navis': [[('bn4-gutsman', 'GutsMan'), ('bn4-thunderman', 'ThunderMan'), ('bn4-windman', 'WindMan'), ('bn4-fireman', 'FireMan')],
                  [('bn4-searchman', 'SearchMan'), ('bn4-aquaman', 'AquaMan'), ('bn4-roll', 'Roll')]],
    'bn4-areas': [[('bn4-acdc', 'ACDC Area'), ('bn4-town', 'Town Area'), ('bn4-countries', 'The countries')]],
    # 0.21.0 (A)
    'numbropn-hand': [[('numbropn-five', 'Five chips'), ('numbropn-ten', 'NumbrOpn: ten chips')]],
    'groups-beats': [[('groups-beat-1', 'Layer 1: alone'), ('groups-beat-2', 'Layer 2: beside a partner'),
                      ('groups-beat-3', 'Layer 3: its constellation')]],
    # 0.22.0's notes: saves, the dive, systems
    'saves-pages': [[('saves-page-cloud', 'CLOUD'), ('saves-page-code', 'CODE'), ('saves-page-file', 'FILE')]],
}
SHEET_FONT = '/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf'


def sheet(rows):
    """Pictures side by side and rows under rows, each at 2x with whole
    pixels under its label: rows of (PIL image, label)."""
    from PIL import Image, ImageDraw, ImageFont
    gap, bar = 8, 30
    cells = [[(im.resize((2 * im.width, 2 * im.height), Image.NEAREST), label) for im, label in row] for row in rows]
    w, h = cells[0][0][0].size
    cols = max(len(row) for row in cells)
    out = Image.new('RGB', (cols * w + (cols - 1) * gap, len(cells) * (bar + h) + (len(cells) - 1) * gap), (16, 24, 40))
    d = ImageDraw.Draw(out)
    font = ImageFont.truetype(SHEET_FONT, 18) if os.path.exists(SHEET_FONT) else ImageFont.load_default()
    for r, row in enumerate(cells):
        y = r * (bar + h + gap)
        for c, (im, label) in enumerate(row):
            x = c * (w + gap)
            out.paste(im, (x, y + bar))
            d.text((x + w // 2, y + bar // 2), label, fill=(120, 248, 255), font=font, anchor='mm')
    return out


def screenshot_sheets(only=None):
    """docs/screenshots/NAME.png for each SCREENSHOT_SHEETS entry whose
    pictures were just made (all of them, with no names)."""
    from PIL import Image
    keep_pil_nearest()
    out = os.path.join(ROOT, 'docs', 'screenshots')
    for name, rows in SCREENSHOT_SHEETS.items():
        if only and name not in only:
            continue
        sheet([[(Image.open(os.path.join(out, f'{n}.png')).convert('RGB'), label) for n, label in row] for row in rows]).save(
            os.path.join(out, f'{name}.png'), optimize=True)
        print('screenshot', name)


SAVES_HOME = '/home/player'


def saves_devices(tmp):
    """SAVES' pictures (issue #114): two devices' saves, each with a run's
    checkpoint the game wrote. The other device, a veteran profile on seed
    3's layer 8, exports with EXPORT into its Downloads (no file dialog in
    the build image: the file goes there at once); this device, seed 7's
    layer 2, is tmp/data, with that file in its player's Downloads
    (tmp/home, which the run sees as SAVES_HOME: the screen shows the
    folder's whole path)."""
    work = os.path.join(ROOT, '.build', 'saves-devices')
    shutil.rmtree(work, ignore_errors=True)
    paged = '100:,' + '4:A,6:,' * 40 + '100:'
    devices = (('other', ['--scene', 'emu', '--run-depth', '8', '--seed', '3', '--dev', 'veteran,quiet']),
               ('here', ['--scene', 'emu', '--run-depth', '2', '--seed', '7', '--dev', 'quiet']),
               ('other', ['--scene', 'saves', '--size', '240x160', '--input', '60:,6:A,60:']))
    for dev, args in devices:
        os.makedirs(os.path.join(work, dev, 'home'), exist_ok=True)
        run = ['--input', paged, '--frames', '700'] if args[1] == 'emu' else ['--frames', '140']
        code = docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', f'/src/.build/saves-devices/{dev}/data',
                      *args, *run, mounts=docs_rom_mounts(), env=[('HOME', f'/src/.build/saves-devices/{dev}/home')])
        if code:
            return code
    exported = os.path.join(work, 'other', 'home', 'Downloads', 'cyberworld-endless.cwsave')
    if not os.path.exists(os.path.join(work, 'here', 'data', 'savedata', 'run.state')) or not os.path.exists(exported):
        print('saves_devices: a checkpoint or the export is missing')
        return 1
    shutil.rmtree(os.path.join(tmp, 'data'), ignore_errors=True)
    shutil.copytree(os.path.join(work, 'here', 'data'), os.path.join(tmp, 'data'))
    os.makedirs(os.path.join(tmp, 'home', 'Downloads'))
    shutil.copy(exported, os.path.join(tmp, 'home', 'Downloads'))
    return 0


def two_screens(top, bottom):
    """A New 3DS's two screens in one picture: the game's 240x160 at 1.5x on
    the top one (400x240), its pixels mixed at their edges as the GPU's filter
    mixes them, and the second screen (320x240) under it."""
    from PIL import Image
    keep_pil_nearest()
    im = Image.new('RGB', (416, 512), (28, 30, 38))
    im.paste(Image.new('RGB', (400, 240)), (8, 8))
    im.paste(top.resize((360, 240), Image.BILINEAR), (28, 8))
    im.paste(bottom, (48, 264))
    return im


def screenshot_prep(args, env, frames):
    """The run a screenshot's needs before it (SCREENSHOT_PREP), headless in
    the same data folder, on BN6's ROM alone: nonzero if it failed."""
    saved = {k: os.environ.get(k) for k in env}
    os.environ.update(env)
    try:
        return docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/screenshots/data',
                      *args, '--frames', str(frames), mounts=docs_rom_mounts())
    finally:
        for k, v in saved.items():
            if v is None:
                os.environ.pop(k, None)
            else:
                os.environ[k] = v


def screenshots(only=None):
    """docs/screenshots/NAME.png: the 240x160 picture of chosen frames (or the
    whole screen, shrunk by the entry's last number; or '3ds', the picture
    and the second screen as a New 3DS shows them; or 'second', the second
    screen alone)."""
    from PIL import Image
    keep_pil_nearest()
    out = os.path.join(ROOT, 'docs', 'screenshots')
    tmp = os.path.join(ROOT, '.build', 'screenshots')
    os.makedirs(out, exist_ok=True)
    # (a sheet named, or one of its pictures: all of its pictures, and it)
    only = set(only or ())
    for name, rows in SCREENSHOT_SHEETS.items():
        parts = {n for row in rows for n, _ in row}
        if name in only or parts & only:
            only |= parts | {name}
    for name, args, frames, env, *whole in SCREENSHOTS:
        if only and name not in only:
            continue
        shutil.rmtree(tmp, ignore_errors=True)
        os.makedirs(os.path.join(tmp, 'data'))
        shots = ','.join(f'{f}:/src/.build/screenshots/{n}.bmp' for f, n in frames)
        dual, alone = whole == ['3ds'], whole == ['second']
        # (the whole screen, the touch controls on it, where the canvas has the game alone)
        shot = '--screen-shot' if whole and not dual and not alone else '--shot'
        second = ['--second-shot', ','.join(f'{f}:/src/.build/screenshots/{n}-second.bmp' for f, n in frames)] if dual or alone else []
        if name in SCREENSHOT_PREP and screenshot_prep(*SCREENSHOT_PREP[name]):
            return 1
        saved = {k: os.environ.get(k) for k in env}
        os.environ.update(env)
        # (the ROMs screen: both ROMs, and the saves where a Linux player's
        # are, which it shows as ~/.local/share/cyberworld-endless/savedata)
        rom_screen = name.startswith('launcher')
        home = '/src/.build/screenshots/home'
        data = home + '/.local/share/cyberworld-endless' if rom_screen else '/src/.build/screenshots/data'
        # (SAVES: two devices' saves, this one's player's home a neutral one)
        saves = name == 'saves'
        mounts = rom_mounts_no_bn4() if name in NO_BN4_A else docs_rom_mounts(bn5=whole_rom_folder(name) or rom_screen)
        if saves:
            code = saves_devices(tmp)
            if code:
                return code
            home = SAVES_HOME
            mounts = mounts + [(os.path.join(tmp, 'home'), SAVES_HOME)]
        server, served = screenshot_server(SCREENSHOT_SERVERS.get(name), tmp)
        try:
            code = docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', data,
                          *args, '--frames', str(max(f for f, _ in frames) + 1), shot, shots, *second,
                          mounts=mounts, env=([('HOME', home)] if rom_screen or saves else []) + served)
        finally:
            if server:
                server.terminate()
                server.wait()
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
            if alone:
                im = Image.open(os.path.join(tmp, f'{n}-second.bmp')).convert('RGB')
                im.save(os.path.join(out, f'{n}.png'), optimize=True)
                print('screenshot', n)
                continue
            if not whole or dual:
                im = im.crop(((w - 240) // 2, (h - 160) // 2, (w + 240) // 2, (h + 160) // 2))
            elif whole[0] > 1:
                im = im.resize((w // whole[0], h // whole[0]), Image.LANCZOS)
            if dual:
                im = two_screens(im, Image.open(os.path.join(tmp, f'{n}-second.bmp')).convert('RGB'))
            im.save(os.path.join(out, f'{n}.png'), optimize=True)
            print('screenshot', n)
    screenshot_sheets(only)
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
    # (the areas walked, layer 1 by the weak autopilot past MegaMan's words;
    # on BN6's ROM alone, but acdc-bn5, with BN5's beside it: docs/multirom/)
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
    # (0.9.0's, made again with every battle in BN6's engine: a random
    # battle of BN5's ACDC Area from BN6's map through its first turn, the
    # autopilot fighting; UNITE with NumberMan's Soul on BN6's Custom
    # screen to the unison; DrkSword from a BN6 battle's first hand, its
    # dark slash)
    ('bn5-battle', ['--scene', 'emu', '--net-biome', 'x0', '--seed', '4', '--dev', 'quiet'], {'CYBERWORLD_AUTOPILOT': '1'}, '460:battle', 436, 916),
    ('bn5-unite', ['--scene', 'emu', '--net-biome', 'x0', '--seed', '4', '--dev', 'quiet,souls=4,folder=175/4'], {}, BN5_UNITE, 500, 1040),
    ('dark-chip', ['--scene', 'emu', '--net-biome', '0', '--run-depth', '1', '--seed', '4', '--dev', 'quiet,darkchips=0x2,folder=286/3', '--talk', 'bugfrags:420'], {},
     '100:,' + '4:A,6:,' * 30 + '40:,0:battle,220:,6:A,20:,6:START,20:,6:A,120:,6:RIGHT,20:,6:A,200:', 606, 966),
    # (0.9.0's start: the developer's boot screen, MegaMan's word on GitHub, the title)
    ('intro', ['--scene', 'intro'], {}, None, 1, 380),
    # (0.10.0's: the second screen following the game, as a New 3DS shows it)
    ('two-screens', SECOND_7, {}, SECOND_HERO, 1735, 2410, '3ds'),
    # (home after act 2, issue #92: Lan crosses Central Town at evening into
    # AsterLand, up to its counter and into the Order Service, its chips'
    # cards on the second screen)
    ('home-visit', ['--scene', 'town', '--seed', '2', '--run-depth', '7', '--dev', 'zenny=8000'], {},
     HOME_OUT + '0:place -156 30 7,20:,122:UP+LEFT,160:,' + HOME_COUNTER + '6:DOWN,30:,6:DOWN,30:,6:DOWN,60:', 856, 1822, '3ds'),
    # (issue #124: the clerk's list sorted, START through BN6's seven keys,
    # SELECT the other way)
    ('home-sort', ['--scene', 'town', '--seed', '2', '--run-depth', '7', '--dev', 'zenny=8000'], {}, HOME_SORT, 1610, 2330),
    # (issue #98: a walk round an Undernet layer's signature, the court of
    # identity-undernet, past its statue between braziers)
    ('identity', IDENTITY + ['--net-biome', '5', '--run-depth', '4', '--seed', '1'], {},
     SIG_PAGED + '20:,70:LEFT,80:DOWN,80:RIGHT,70:UP,100:', 460, 740),
    # (0.11.0's: the super bosses' entrances, issue #100, the autopilot in
    # god mode at 800 HP: Bass in the Secret Area with the title's S held,
    # from the bridge into his arena, his stone, the white and his card;
    # the Cybeast at seed 7's endless Nest, the shaking, the white, its card)
    ('bass-arrival', ['--scene', 'emu', '--run-depth', '8', '--seed', '7', '--marks', '08', '--dev', 'god,quiet,hp=800,side=2'],
     {'CYBERWORLD_AUTOPILOT': '1'}, None, 1090, 1630),
    ('cybeast-arrival', ['--scene', 'emu', '--run-depth', '19', '--seed', '7', '--dev', 'god,quiet,hp=800'],
     {'CYBERWORLD_AUTOPILOT': '1'}, None, 1150, 1690),
    # (0.11.2: BlastMan's last word, his log-out by BN6's beam, his Guardian
    # Data turning where he stood, seed 7's layer 3)
    ('guardian-logout', ['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '11', '--guardian', '12', '--dev', 'god,onehit'],
     {'CYBERWORLD_AUTOPILOT': '1'}, None, 2640, 2840),
    # (0.12.0: the soft reset, issue #113: the second screen's fight on seed
    # 7's layer 2, its Cannon countering the Mettaur, then A+B+SELECT+START
    # held; Cyberworld Endless's title, START, CONTINUE on Layer 2)
    ('soft-reset', SECOND_7, {}, SECOND_HERO + '70:,6:A+B+SELECT+START,90:,6:START,100:', 2270, 2620),
    # (0.12.0's one net, issue #82: BN5's Powie and Marina named on BN6's
    # Custom screen, UNITE with ShadowMan's Soul, the unison, ShadowSoul's
    # charged Sword behind Powie V3; DarkInvs dealt at the hand's end while
    # MegaMan worries, the game fighting KnightMan for him, grey)
    ('bn5-one-net', ONE_NET + ['--dev', 'quiet,god,souls=2,folder=71'], {}, ONE_SOUL, 504, 1190),
    ('bn5-darkinvs', ONE_NET + ['--dev', 'quiet,worried,hp=999/200'], {}, ONE_DARKINVS, 1240, 1950),
    # (0.13.0's: Trade Central's spares list, issue #145, with BN5's ROM)
    ('trade-spares', ['--scene', 'emu', '--run-depth', '2', '--seed', '3', '--dev', 'quiet'], {}, TRADE_SPARES, 252, 860),
    # (0.15.0's liberation: a dark panel's battle from TURN 1 START! to 1 TURN
    # LIBERATE!; the Darkloids' phase, its banner, CircusMan's reach lit red,
    # his line and his blow)
    ('bn5-lib-battle', LIB_BOARD, {'CYBERWORLD_AUTOPILOT': '1'}, None, 1600, 1860),
    ('bn5-lib-darkloid', LIB_BOARD, {'CYBERWORLD_AUTOPILOT': '1'}, None, 13550, 13900),
    # (0.16.0's: a Virus Battle's first round, ErthDrgn and Mettaur against
    # Mettaur and Gunner, to YOU WIN!; ChargeMan's BombLiberation, his
    # move onto the dark and his command in its window)
    ('vb-fight', VB_HELD, {}, VB_SERIES, 2290, 2770),
    ('bn5-lib-bomb', LIB_CHARGE, {'CYBERWORLD_AUTOPILOT': '1'}, None, 3590, 3800),
    # (0.17.0's: the courier in Lan's HP after act 2, seed 2, naming the
    # BBS and each offer with its pay, a box each)
    ('courier-offers', HP_2, {}, COURIER_OFFERS, 1220, 1700),
    # (0.17.0's: the exit pad holding a layer's battle, from MegaMan's walk
    # onto it to the Custom screen)
    ('floor-pad', FLOOR_PAD, {}, FLOOR_PAD_INPUT, 800, 1090),
    # (0.19.0's: Options' Display on the title, its scene changing with the
    # line and the whole screen with Colour correction; a GBA's screen's
    # battle, MegaMan stepping and firing, Off beside LCD ghosting)
    ('options-live', ['--scene', 'title'], {}, OPTIONS_LIVE, 200, 1100),
    ('gba-blend', GBA_SCREEN, {}, None, 1200, 1420,
     'beside', [('Off', []), ('Frame blending: LCD', ['--interframe-blending', 'lcd_ghosting'])]),
    # (0.20.0's: the tournament's door, BN4's bracket with every face, round
    # 1's card and GutsMan's battle starting; Roll's summon bringing BN4's
    # Mettaur; BN4's ACDC Area walked; the Glitch warp switching looks under
    # BN6's mosaic; a Glitch layer's floor flipping through other areas'; a
    # copy's first form glitching out and the next arriving on its panel)
    ('bn4-tourney', TOURNEY, {'CYBERWORLD_AUTOPILOT': '1'}, None, 1000, 1560),
    ('bn4-roll-summon', BN4_NAVI, {'CYBERWORLD_AUTOPILOT': '1'}, '460:,0:foes 380@52', 2290, 2550),
    ('acdc-bn4', BN4_AREA + ['--net-biome', 'x15'], {'CYBERWORLD_AUTOPILOT': 'weak'}, None, 300, 700),
    ('glitch-warp', GLITCH_WARP, {}, None, 500, 900),
    ('glitch-layer', GLITCH_LAYER, {}, None, 600, 1300),
    ('glitch-copy', ['--scene', 'emu', '--net-biome', '0', '--seed', '15', '--dev', 'god,onehit,quiet'],
     {'CYBERWORLD_AUTOPILOT': '1'}, '300:,0:swap 101/107/10D', 1360, 1660),
    # 0.21.0 (B)
    # (BN4's HeatSprd from its forming to its burst; Meteors1's five
    # meteors; TopMan's spinning top crossing the field)
    ('bn4-heatsprd', B_CHIP + ['--dev', 'quiet,god'], {}, B_HEATSPRD, 1450, 1830),
    ('bn4-meteors', B_CHIP + ['--dev', 'quiet,god,folder=674'], B_AP, '460:,' + B_METT, 880, 1140),
    ('bn4-topman-top', BN4_NAVI, B_AP, '460:,0:foes 3B0@52', 1950, 2086),
    # 0.21.0 (A)
    ('lab-won', LAB, {}, LAB_WON, 700, 1800),
    # (the constellation's battle, act 2's third layer: Armadill, Gunner and
    # Piranha from the Custom screen into the first turn, the autopilot fighting)
    ('groups-constellation', GROUPS + ['--run-depth', '6'], {'CYBERWORLD_AUTOPILOT': '1'}, '460:battle', 560, 1080),
    # 0.21.0 (A) ends: clips
]


# 0.22.0's notes: words and fixes
# (stills of the lines and fixed screens, each a scripted run on a fresh data
# folder. N22_IN: seed 4's first layer in Central Area, its arrival's words
# paged, MegaMan free from frame 400; N22_CONTINUE: the title's START, then
# A on CONTINUE; N22_SERVER: SERVER_8's talk paged to its question, Yes, the
# Custom screen's START and OK, the buster down each row twice over, then A
# through RESULT, the Server's words, the pick and its prize)
N22_L1 = ['--scene', 'emu', '--net-biome', '0', '--run-depth', '1', '--seed', '4', '--dev', 'quiet']
N22_IN = '100:,' + '4:A,6:,' * 30
N22_CONTINUE = '200:,6:START,100:,6:A,'
N22_ROWS = '6:B,6:,12:,' * 6
N22_SERVER = ('1:,300:,' + '6:A,6:,70:,' * 3 + '6:A,6:,90:,6:LEFT,6:,10:,6:A,6:,400:,6:START,6:,10:,6:A,6:,120:,' +
              (N22_ROWS + '8:UP,15:,' + N22_ROWS + '8:DOWN,15:,' + N22_ROWS + '8:DOWN,15:,' + N22_ROWS + '8:UP,15:,') * 2 + '6:A,60:,' * 22)
# (the runs before a CONTINUE, quit as their frames end: L's words open on
# seed 4's layer 2; a random battle won on layer 1, quit on its RESULT; seed
# 5's layer 2 reached at 60 of 140 HP, Chaud's call paged)
SCREENSHOT_PREP.update({
    'quit-talk': (['--scene', 'emu', '--run-depth', '2', '--seed', '4', '--dev', 'quiet', '--input', '150:,' + '6:A,4:,' * 30 + '40:,6:L,144:'], {}, 630),
    'quit-won': (['--scene', 'emu', '--run-depth', '1', '--seed', '4', '--net-biome', '0', '--dev', 'quiet,god,onehit', '--input', '460:battle'],
                 {'CYBERWORLD_AUTOPILOT': '1'}, 1101),
    'heal-continue': (['--scene', 'emu', '--run-depth', '2', '--seed', '5', '--dev', 'quiet,hp=140/60', '--input', '150:,' + '6:A,14:,' * 60], {}, 1500),
})
# (the ROMs screen with BN4's slot open, its line on what BN4 brings)
NO_BN4_A += ('launcher-bn4-line',)
SCREENSHOTS += [
    # (Shorter stops on the way: a Net Dealer's one box before his list; L's
    # first press, ProtoMan first of the two things worth a detour)
    ('dealer-one-box', ['--scene', 'emu', '--run-depth', '4', '--seed', '3', '--dev', 'quiet', '--talk', 'shop:430'], [(560, 'dealer-one-box')], {}),
    ('l-two-things', ['--scene', 'emu', '--run-depth', '2', '--seed', '4', '--dev', 'quiet', '--input', '150:,' + '6:A,4:,' * 30 + '40:,6:L,144:'],
     [(620, 'l-two-things')], {}),
    # (Chaud's call and ProtoMan's race: Lan's surprise right after Chaud's
    # name; the race's question, MegaMan's, on No)
    ('chaud-race', ['--scene', 'emu', '--run-depth', '2', '--seed', '3', '--net-biome', '0', '--dev', 'quiet', '--talk', 'duel:1500',
                    '--input', '700:,' + '6:A,54:,' * 12 + '100:,' + '6:A,74:,' * 9], [(750, 'chaud-surprise'), (2150, 'race-question')], {}),
    # (Lines that say what they mean: the Recovery Mr.Prog's patch at 40 of
    # 100 HP, the HP it gave. A word after a battle: MegaMan's on the first
    # virus from an older Net, in BN4's ACDC Area, once the fade is over, and
    # Lan's answer there)
    ('heal-says-hp', ['--scene', 'emu', '--run-depth', '2', '--seed', '5', '--dev', 'quiet,hp=100/40', '--talk', 'heal:1100',
                      '--input', '660:,' + '6:A,14:,' * 20 + '120:,' + '6:A,74:,' * 5], [(1400, 'heal-says-hp')], {}),
    ('bn4-after-battle', ['--scene', 'emu', '--run-depth', '2', '--seed', '4', '--net-biome', 'x15', '--dev', 'quiet,god,onehit', '--input', '460:battle'],
     [(1268, 'bn4-word-after-battle'), (1300, 'bn4-old-viruses')], {'CYBERWORLD_AUTOPILOT': '1'}),
    # (What the run tells you: Dad's first call on what a dive leaves behind;
    # MegaMan's word a few panels from a guardian's arena, its second box)
    ('dad-keeps', ['--scene', 'town', '--seed', '2', '--input', '200:,' + '6:A,44:,' * 14], [(800, 'dad-keeps')], {}),
    ('arena-ready', ['--scene', 'emu', '--run-depth', '3', '--seed', '4', '--dev', 'quiet', '--input', '200:,0:place 264 -31 1,100:,6:A,114:,6:A,120:'],
     [(400, 'arena-ready')], {}),
    # (A quit with a text box open: the CONTINUE after SCREENSHOT_PREP's quit
    # in L's words. A quit after a won battle: the CONTINUE after its quit on
    # RESULT, MegaMan where the battle met him)
    ('quit-talk', ['--scene', 'title', '--input', N22_CONTINUE + '400:'], [(400, 'quit-talk')], {}),
    ('quit-won', ['--scene', 'title', '--input', N22_CONTINUE + '400:'], [(400, 'quit-won')], {}),
    # (The PET's Save and your next A: its own box. Blank chips in the
    # Standard folder: FOLDER EDIT on a new run's folder)
    ('pet-save', N22_L1 + ['--input', N22_IN + '30:,6:START,40:,' + '6:DOWN,6:,' * 7 + '6:A,200:'], [(650, 'pet-save')], {}),
    ('folder-named', N22_L1 + ['--input', N22_IN + '30:,6:START,40:,6:A,80:,6:A,60:,6:A,120:'], [(720, 'folder-named')], {}),
    # (A prize before the chatter: the Server's pick taken, its prize's box.
    # Mr.Prog's gift, taken unseen: the gift's screen as A is pressed through
    # his lines, its cursor still and faded)
    ('server-prize', SERVER_8 + ['--input', N22_SERVER], [(3160, 'server-prize')], {}),
    ('gift-cursor', N22_L1 + ['--talk', 'gift:450', '--input', N22_IN + '80:,' + '6:A,34:,' * 12], [(780, 'gift-cursor')], {}),
    # (RESULT's virus count, and RECORDS' closest goal as a sentence)
    ('result-count', ['--scene', 'summary', '--run-depth', '5', '--seed', '3', '--input', '320:,4:A,36:'],
     [(200, 'result-count'), (400, 'records-goal')], {}),
    # (Two menu lines made whole: Options' Fast forward note; the ROMs
    # screen's line on BN4, its slot open and chosen)
    ('options-fast-note', ['--scene', 'title', '--input', '60:,4:SELECT,100:,4:L,40:,4:DOWN,36:'], [(240, 'options-fast-note')], {}),
    ('launcher-bn4-line', ['--scene', 'launcher', '--size', '1280x720', '--input', '40:,6:UP,10:,6:RIGHT,10:,6:RIGHT,40:'],
     [(120, 'launcher-bn4-line')], {}, 2),
    # (Mr.Prog's patch after a CONTINUE: SCREENSHOT_PREP's run continued at 60
    # of 140 HP, the patch's Yes, half of max HP)
    ('heal-continue', ['--scene', 'title', '--talk', 'heal:1300', '--input', N22_CONTINUE + '100:,' + '6:A,14:,' * 50 + '500:,' + '6:A,74:,' * 8],
     [(2130, 'heal-continue')], {}),
    # (First meetings: a fresh profile's first dive into an older Net's
    # area, BN5's ACDC Area, MegaMan finding it old)
    ('bn5-first-dive', ['--scene', 'emu', '--run-depth', '1', '--net-biome', 'x0', '--seed', '4', '--input', '200:,' + '6:A,44:,' * 14],
     [(600, 'bn5-first-dive')], {}),
    # (A stray A at a DarkChip: A pressed through the flame's words, the
    # question on No)
    ('dark-no', ['--scene', 'emu', '--run-depth', '2', '--seed', '4', '--dev', 'quiet,darkchips=0x2', '--talk', 'dark:900',
                 '--input', '200:,' + '6:A,14:,' * 32 + '100:,' + '6:A,34:,' * 22], [(1920, 'dark-no')], {}),
    # (SAVES on a first start: no save yet, THIS DEVICE "No runs yet")
    ('saves-first', ['--scene', 'saves', '--size', '240x160'], [(120, 'saves-first')], {}),
]
# 0.22.0's notes: Aquarium Comp's tanks (the autopilot's seed 1 in Aquarium
# Comp alone, passing the tank that stands on its third layer)
SCREENSHOTS += [
    ('aquarium-tank', ['--scene', 'emu', '--seed', '1', '--net-biome', '12', '--run-depth', '3', '--dev', 'quiet'],
     [(550, 'aquarium-tank')], {'CYBERWORLD_AUTOPILOT': '1'}),
]


# the clips the README or a release's notes show as GIFs too (GitHub plays no
# video from the repository): name, then its first seconds and frames a second
# (a walk's scrolling floor makes a GIF of all of it 2 MB)
# 0.22.1's notes, the first half: Return to title, a quit at a layer's exit, the Chip Trader, a liberation's
# rules after a deletion in one (#669), a new run's count of viruses, the Library's chip just found
# (a run's first layer as tools/cw_routes.py's `layer` reaches it: the arrival paged, the opening gift's first
# offer picked, MegaMan free at 616)
N221_LAYER = '1:,150:,' + '4:A,6:,' * 30 + '20:,6:A,6:,60:,6:A,6:,60:,'
# (a layer's arrival paged as the scenarios page it, MegaMan free at 531)
N221_ARRIVE = '150:,' + '4:A,6:,' * 30 + '20:,60:,'
# (the run before the CONTINUE: the autopilot's seed 3 from layer 2, quit in its exit's warp, layer 3's
# checkpoint not written yet; tests/scenarios/quit_as_a_layer_is_left.py)
SCREENSHOT_PREP.update({
    'quit-layer-exit': (['--scene', 'emu', '--run-depth', '2', '--seed', '3'], {'CYBERWORLD_AUTOPILOT': '1'}, 6062),
})
# (the Chip Trader of seed 5's layer 4, six spare chips in the Pack: its talk opened at 541, Yes, the first
# three chips put in, "Add these 3?" at 940, Yes, the chip it gives paged, "Try again?" Yes: the Pack after
# the trade at 1530; tests/scenarios/layer_gates.py's trader-trades-three-for-one, by --talk as v0.22.0 has it)
N221_TRADER = ['--scene', 'emu', '--seed', '5', '--run-depth', '4', '--dev', 'quiet,pack=6', '--talk', 'trader:541', '--input',
               '1:,' + N221_ARRIVE + '10:,' + '6:A,6:,30:,' * 3 + '120:,' + '6:A,6:,40:,' * 3 + '6:A,6:,90:,' + '6:A,6:,30:,' * 4 +
               '6:A,6:,150:,6:A,6:,150:']
# (a liberation's rules after a deletion in one, issue #669: the autopilot on seed 5's dark layer, deleted by the
# first hit of its board's first battle (fragile), the summary's pages, START, NEW GAME, the setup's START, and
# the next run walked to its first battle, whose start is at 4840; tests/scenarios/liberation_rules_end.py)
N221_LIBERATION = ['--scene', 'emu', '--seed', '5', '--run-depth', '5', '--dev', 'dark,fragile', '--input',
                   '2341:,' + '6:A,46:,' * 3 + '6:START,46:,6:A,46:,6:START,66:,3000:']
# (a new run's count of viruses: on seed 1's layer 2 three Mettaurs, one deleted by a buster shot (onehit), then
# MegaMan by the first hit (fragile), GAME OVER paged to RESULT at 1471; its pages, START, NEW GAME, the setup's
# START, Lan's room (Dad's call paged), R at his PC, Lan's HP's pink pad, layer 1, and the same battle again:
# the second run's RESULT from 4622; tests/scenarios/battle_deleted.py)
N221_DELETED = '0:foes 01@52/01@41/01@63,240:,6:START,6:,10:,6:A,6:,60:,' + '4:B,6:,' * 9 + '300:,' + '6:A,6:,60:,' * 3
N221_NEXT_RUN = ('6:A,6:,40:,' * 4 + '6:START,6:,40:,6:A,6:,40:,6:START,6:,60:,' + '100:,' + '4:A,6:,' * 46 +
                 '30:,0:place -40 -8 7,10:,6:R,6:,500:,' + '4:A,6:,' * 6 + '30:,0:place 171 0 1,10:,25:UP+RIGHT,60:,')
N221_TWO_RUNS = ['--scene', 'emu', '--seed', '1', '--run-depth', '2', '--dev', 'onehit,fragile', '--input',
                 '1:,' + N221_ARRIVE + N221_DELETED + N221_NEXT_RUN + N221_ARRIVE + N221_DELETED + '120:']
# (the Library after a chip just found, where 0.22.0 missed it: a new game, Lan's room (Dad's call paged), out
# of the house and into AsterLand, its Chip Trader given the Pack's first three (twelve spare chips, pack=12),
# "Lan got: GoingRd *" at 1640, "Try again?" No; then the PET, the Library, and its row 173, GoingRd's;
# tests/scenarios/home_town.py's aster-chip-trader and xrom_library.py)
N221_LIBRARY = ['--scene', 'town', '--seed', '2', '--dev', 'pack=12', '--input',
                '1:,40:,' + '4:A,6:,' * 46 + '30:,0:place -62 80 3,10:,24:DOWN,120:,0:place -78 55 1,10:,30:DOWN,120:,' +
                '0:place -156 -40 7,10:,62:UP+LEFT,120:,0:place 94 46 7,10:,' + '6:A,6:,40:,' * 2 + '30:,' + '6:A,6:,30:,' * 3 +
                '6:A,6:,40:,' * 2 + '6:A,6:,90:,' + '6:A,6:,40:,' * 4 + '6:RIGHT,6:,20:,6:A,6:,120:,' +
                '30:,6:START,6:,40:,6:DOWN,6:,6:,6:DOWN,6:,6:,6:A,6:,100:,' + '6:R,6:,10:,' * 24 + '6:DOWN,6:,' * 4 + '30:']
SCREENSHOTS += [
    # (Return to title: the PET's Options in a run, the Game page's last line, and its question, the cursor on No)
    ('options-return-to-title', ['--scene', 'emu', '--seed', '4', '--dev', 'quiet', '--input',
                                 N221_LAYER + '6:START,6:,40:,' + '6:DOWN,6:,' * 6 + '6:A,6:,60:,6:L,6:,20:,6:UP,6:,10:,6:A,6:,60:'],
     [(860, 'options-return-to-title'), (930, 'options-return-ask')], {}),
    # (A quit as a layer's exit was taken: after SCREENSHOT_PREP's quit in the warp, the title's CONTINUE row,
    # and the CONTINUE on layer 3)
    ('quit-layer-exit', ['--scene', 'title', '--input', N22_CONTINUE + '400:'],
     [(260, 'quit-layer-exit-title'), (520, 'quit-layer-exit')], {}),
    # (The Chip Trader takes the three: Cannon, HiCannon and M-Cannon put in, and the Pack after the trade,
    # the three gone and its chip there; tools/before_after.py's trader-pack is the Pack in 0.22.0 beside it)
    ('trader-three-in', N221_TRADER, [(940, 'trader-three-in'), (1530, 'trader-pack-after')], {}),
    # (A new run's count: the second run's RESULT, one Mettaur deleted in it; the first run's before it, one too;
    # tools/before_after.py's result-next-run is 0.22.0's 2 beside it)
    ('result-next-run', N221_TWO_RUNS, [(1500, 'result-first-run'), (4740, 'result-next-run')], {}),
    # (The Library's chip just found: AsterLand's Chip Trader's chip, and the Library's row of it right after;
    # tools/before_after.py's library-found is 0.22.0's page beside it. And on a layer: seed 3's second, its purple
    # Mystery Data opened with an Unlocker, the chip it gave, then the PET's Library on that chip's row)
    ('library-found', N221_LIBRARY, [(1640, 'library-found-trade'), (2715, 'library-found')], {}),
    ('library-data', ['--scene', 'emu', '--seed', '3', '--run-depth', '2', '--dev', 'quiet', '--talk', 'keys:10', '--input',
                      '1:,' + N221_ARRIVE + '0:place -148 -156 5,10:,' + '6:A,6:,30:,' * 7 +
                      '30:,6:START,6:,40:,6:DOWN,6:,6:,6:DOWN,6:,6:,6:A,6:,60:,' + '6:R,6:,10:,' * 23 + '6:DOWN,6:,' * 4 + '30:'],
     [(790, 'library-data-got'), (1600, 'library-data-found')], {}),
]
# 0.22.1's notes, the second half: a dark layer's panels with BN6's ROM alone, a won liberation battle and a quit,
# START on the team's pick, a Glitch layer settled, LCD 3x kept, the JACK-IN SETUP after a first win
# (the dark layer's panels are tools/before_after.py's dark-panels, a pair against 0.22.0.
# N221_WIN: the dev menu's "Win this battle", its fifth row, as the scenarios take it
# (docs/devtools/scenarios.md, A dark layer's board); N221_LCD: SELECT on the title, Options' Display, down to LCD 3x)
N221_WIN = '4:SELECT,4:SELECT+R,6:,' + '6:DOWN,6:,' * 4
N221_LCD = '50:,6:SELECT,46:,' + '6:DOWN,12:,' * 5
# (the runs before a restart, quit as their frames end. lib-quit-words: seed 7's own dark layer with BN6's ROM
# alone, Lan's briefing paged, MegaMan beside a dark panel, Liberation, the Custom screen's OK, the battle won
# in its first turn, RESULT paged to his words, the quit under them (tests/scenarios/liberation_board.py,
# continue/under-the-words). settled-glitch: seed 3's Glitch layer, the copy's Yes, its forms deleted one by
# one, its words and its pick of Navi chips taken (tests/scenarios/extras_glitch.py, copy-deleted). lcd3x-kept:
# LCD 3x turned on with A, Options left with B. setup-first-win: a profile's first run begun, NEW GAME with no
# setup, quit in Lan's room)
SCREENSHOT_PREP.update({
    'lib-quit-words': (['--scene', 'emu', '--seed', '7', '--run-depth', '5', '--dev', 'quiet', '--input',
                        '121:,' + '6:A,26:,' * 13 + '0:board dark,4:,' + '6:A,16:,' * 2 + '300:,' + '6:START,16:,6:A,56:,' * 2 +
                        N221_WIN + '6:A,12:,' + '6:A,14:,' * 16 + '100:'], {}, 1481),
    'settled-glitch': (['--scene', 'emu', '--seed', '3', '--run-depth', '4', '--dev', 'glitch,quiet,side=4', '--talk', 'copy:800', '--input',
                        '150:,' + '4:A,6:,' * 30 + '380:,' + '6:LEFT,6:,6:A,26:,' * 10 + '210:,6:START,16:,6:A,186:,' +
                        N221_WIN + '6:A,66:,' + '4:SELECT,4:SELECT+R,6:,6:A,126:,' * 5 + '60:,' + '6:A,46:,' * 7 + '120:'], {}, 3100),
    'lcd3x-kept': (['--scene', 'title', '--dev', 'settings', '--size', '720x480', '--input', N221_LCD + '6:A,12:,6:B,66:'], {}, 260),
    'setup-first-win': (['--scene', 'title', '--input', N22_CONTINUE + '300:'], {}, 560),
})
SCREENSHOTS += [
    # (A won liberation battle and a quit under MegaMan's words: the CONTINUE after SCREENSHOT_PREP's quit, his
    # words and the Darkloids' phase paged: the board in its second phase, the panels the one-turn win freed
    # round him, the dark ones beyond)
    ('lib-quit-words', ['--scene', 'title', '--input', N22_CONTINUE + '300:,' + '6:A,26:,' * 20 + '260:'], [(1150, 'lib-quit-words')], {}),
    # (START on the team's pick, BN5's ROM beside BN6's: seed 5's dark layer with Colonel's and ShadowMan's Souls
    # held and a place for one, the pick as it opens; START, and Lan's briefing with no PET over it)
    ('bn5-pick-start', ['--scene', 'emu', '--seed', '5', '--run-depth', '5', '--dev', 'dark,souls=3,quiet', '--input', '300:,10:START,300:'],
     [(280, 'bn5-pick-team'), (520, 'bn5-pick-start')], {}),
    # (A Glitch layer settled: the CONTINUE after SCREENSHOT_PREP's quit, its card, then the layer as it stands)
    ('settled-glitch', ['--scene', 'title', '--dev', 'quiet', '--input', N22_CONTINUE + '1100:'],
     [(470, 'settled-glitch-continue'), (1200, 'settled-glitch')], {}),
    # (LCD 3x kept: the game started again on SCREENSHOT_PREP's settings.ini (--dev settings: read as a player's
    # start reads it), Options' Display down to LCD 3x; the whole screen at 720x480, where the stripes show)
    ('lcd3x-kept', ['--scene', 'title', '--dev', 'settings', '--size', '720x480', '--input', N221_LCD + '200:'], [(280, 'lcd3x-kept')], {}, 1),
    # (The JACK-IN SETUP after a first win: over SCREENSHOT_PREP's first run, seed 3's short net won on its last
    # layer by the autopilot, RESULT and RECORDS paged, START, NEW GAME: the setup as it opens, on the short net;
    # then Right, the endless one)
    ('setup-first-win', ['--scene', 'emu', '--seed', '3', '--setup', 'short', '--run-depth', '10', '--dev', 'onehit,god', '--input',
                         '4420:,6:A,36:,6:A,80:,6:START,60:,6:A,160:,6:RIGHT,100:'],
     [(4760, 'setup-first-win'), (4860, 'setup-first-win-endless')], {'CYBERWORLD_AUTOPILOT': '1'}),
]
README_GIFS = {'guardian': (None, 15), 'sky': (4, 12), 'weather': (4, 12), 'acdc-bn5': (4, 12),
               'rush': (None, 15), 'teleport': (None, 15), 'arrows': (None, 15), 'hidden': (None, 15), 'obstacle': (None, 15),
               'bn5-battle': (None, 15), 'bn5-unite': (None, 15), 'dark-chip': (None, 15),
               'intro': (None, 15), 'two-screens': (None, 12), 'home-visit': (None, 12), 'identity': (4, 12),
               'bass-arrival': (None, 12), 'cybeast-arrival': (None, 12), 'guardian-logout': (None, 15),
               'soft-reset': (None, 15), 'bn5-one-net': (None, 15), 'bn5-darkinvs': (None, 12), 'home-sort': (None, 12),
               'online-square': (None, 15), 'online-trade': (None, 15), 'online-battle': (None, 12),
               'online-netpass': (None, 15), 'trade-spares': (None, 15), 'bn5-lib-battle': (None, 15),
               'bn5-lib-darkloid': (None, 15), 'vb-fight': (None, 15), 'bn5-lib-bomb': (None, 15), 'courier-offers': (None, 15),
               'floor-pad': (None, 15), 'options-live': (None, 15), 'gba-blend': (None, 15),
               'bn4-tourney': (None, 15), 'bn4-roll-summon': (None, 15), 'acdc-bn4': (4, 12), 'glitch-warp': (None, 15),
               'glitch-layer': (None, 10), 'glitch-copy': (None, 15),
               # 0.21.0 (A)
               'lab-won': (None, 12), 'groups-constellation': (None, 12), 'online-pet': (None, 12), 'online-netpass-bn4': (None, 15)}
# 0.21.0 (B)
README_GIFS.update({'bn4-heatsprd': (None, 15), 'bn4-meteors': (None, 15), 'bn4-topman-top': (None, 15)})
# 0.21.1: the floors before and after, two pictures side by side (tools/before_after.py --clips)
README_GIFS.update({name: (None, 10) for name in ('compare-bn4-park', 'compare-bn4-countries', 'compare-bn5-acdc', 'compare-sky',
                                                   'compare-seaside', 'compare-undernet')})
# 0.21.2: AquaNdl1 before and after at BN5's and BN4's viruses (tools/before_after.py --clips)
README_GIFS['compare-bn5-aquandl'] = (None, 15)
# 0.22.0's notes: saves, the dive, systems (tools/online_pictures.py's DIVE_CLIPS)
README_GIFS['online-dive'] = (None, 15)
# 0.22.0's notes: the net
# (a layer's arrival paged, MegaMan free at 470, as tools/trailers/v0_22_0_footage.py's ARRIVE; an area's
# layer walked by the weak autopilot in god mode, so no battle moves its frames)
N22_ARRIVE = '150:,' + '6:A,4:,' * 30 + '20:,'
N22_WEAK = {'CYBERWORLD_AUTOPILOT': 'weak'}
N22_AUTO = {'CYBERWORLD_AUTOPILOT': '1'}


def n22_walk(biome, seed, depth=1, dev='quiet,god'):
    return ['--scene', 'emu', '--run-depth', str(depth), '--seed', str(seed), '--net-biome', str(biome), '--dev', dev]


# (L, its boxes paged, and the arrow it leaves)
N22_L = N22_ARRIVE + '20:,6:L,144:,' + '6:A,144:,' * 3 + '60:,200:'
# (ProtoMan's terms opened at 610 and paged, Yes, OK with no chips: the race's first turn)
N22_RACE = '300:,' + '4:A,6:,' * 30 + '6:A,114:,' * 6 + '120:,6:LEFT,14:,6:A,400:,6:START,4:,6:A,600:'
# (Lan's room, Dad's call paged: Lan free at 530)
N22_ROOM = '40:,' + '4:A,6:,' * 46 + '30:,'
# (seed 7's layer 3 to its arena's door, a soft reset, START, CONTINUE)
N22_DOOR = '1500:,6:A+B+SELECT+START,260:,6:START,150:,6:A,500:'
N22_GUTSMAN = ['--scene', 'emu', '--run-depth', '3', '--seed', '4', '--net-biome', 'x15', '--guardian', '35', '--dev', 'quiet,god,onehit']
N22_GLITCH = ['--scene', 'emu', '--run-depth', '4', '--seed', '3', '--dev', 'quiet,god,glitch,side=4,glitchstage=5']
N22_COPYBOT = ['--scene', 'emu', '--run-depth', '2', '--seed', '2', '--net-biome', '15', '--dev', 'quiet']
SCREENSHOTS += [
    # (the net's locks: seed 4's second layer, the Security Cube beside the arrival, MegaMan's first box on it;
    # the footage's LOCK)
    ('lock-cube', ['--scene', 'emu', '--run-depth', '2', '--seed', '4', '--dev', 'quiet', '--input',
                   N22_ARRIVE + '24:UP+RIGHT,20:,60:DOWN+LEFT,10:,' + '6:A,116:,' * 4 + '6:A,60:'], [(660, 'lock-cube')], {}),
    # (what stands beside the floor, MegaMan by each: Robot Control Comp's figure behind seed 9's arrival pad
    # and its wires' red lights; the Nest's crystals; Judge Tree Comp's planters under a wall and tufts on the
    # rims; Mr. Weather Comp's cloud; the Undernet's fire bowls and Undernet Zero's; BN4's Undernet's horns;
    # BN5's Undernet's skull totem; End Area's pagoda, "piece in a hole at world 68,-84" of seed 9's layer in
    # CYBERWORLD_EMU_DEBUG's lines)
    ('robot-figure', n22_walk(11, 9, dev='quiet') + ['--input', '110:,' + '6:A,24:,' * 8 + '50:UP+LEFT,150:'], [(520, 'robot-figure')], {}),
    ('robot-wires', n22_walk(11, 9), [(620, 'robot-wires')], N22_WEAK),
    ('nest-crystals', n22_walk(7, 4, 2), [(852, 'nest-crystals')], N22_WEAK),
    ('judge-planters', n22_walk(13, 1), [(1380, 'judge-planters')], N22_WEAK),
    ('weather-cloud', n22_walk(14, 2), [(260, 'weather-cloud')], N22_WEAK),
    ('undernet-bowls', n22_walk(5, 5, 2), [(780, 'undernet-bowls')], N22_WEAK),
    ('undernet-zero-bowls', n22_walk(6, 5, 2), [(1800, 'undernet-zero-bowls')], N22_WEAK),
    ('bn4-horns-run', n22_walk('x19', 1), [(460, 'bn4-horns-run')], N22_WEAK),
    ('bn5-undernet-totem', n22_walk('x5', 2, 2), [(1140, 'bn5-undernet-totem')], N22_WEAK),
    ('bn5-end-pagoda', n22_walk('x2', 9) + ['--input', N22_ARRIVE + '0:place 100 -100 7,40:'], [(505, 'bn5-end-pagoda')], {}),
    # (ProtoMan's race on seed 3's second layer: a Mettaur and an OldStove, his time over the clock)
    ('race-start', n22_walk(0, 3, 2, 'quiet') + ['--talk', 'duel:610', '--input', N22_RACE], [(2030, 'race-start')], {}),
    # (SELECT's map on seed 2's second layer, MegaMan hurt and L's way to the Recovery Mr.Prog in the key's
    # Heal green, the key's ProtoMan on its second row)
    ('map-heal-way', ['--scene', 'emu', '--run-depth', '2', '--seed', '2', '--net-biome', '0', '--dev', 'quiet,mapall,hp=300/60', '--input',
                      N22_ARRIVE + '20:,6:L,144:,' + '6:A,144:,' * 6 + '60:,240:SELECT,40:'], [(1700, 'map-heal-way')], {}),
    # (the arrow pointing down, below MegaMan's feet: seed 52's second layer after L)
    ('arrow-below', ['--scene', 'emu', '--run-depth', '2', '--seed', '52', '--dev', 'quiet', '--input', N22_L], [(1170, 'arrow-below')], {}),
    # (fixed: BN4's GutsMan guarding BN4's ACDC Area, facing MegaMan on his left, and his Guardian Data once
    # he is deleted; Town Area's exit pad on its pale floor after FireMan, #624's start; the Glitch copy
    # standing as SpoutMan, #626's start; a CONTINUE from seed 7's arena's door and the guardian's card after)
    ('bn4-gutsman-data', N22_GUTSMAN, [(1800, 'bn4-gutsman-faces'), (4250, 'bn4-gutsman-data')], N22_AUTO),
    ('bn4-town-exit-pad', ['--scene', 'emu', '--run-depth', '3', '--seed', '1016', '--net-biome', 'x16', '--dev', 'quiet,god,onehit'],
     [(2640, 'bn4-town-exit-pad')], N22_AUTO),
    ('glitch-copy-spoutman', ['--scene', 'emu', '--run-depth', '7', '--seed', '1', '--dev', 'quiet,god,glitch,side=4', '--input',
                              N22_ARRIVE + '0:glitch,300:'], [(520, 'glitch-copy-spoutman')], {}),
    ('continue-door', ['--scene', 'emu', '--run-depth', '3', '--seed', '7', '--net-biome', '11', '--guardian', '12', '--input', N22_DOOR],
     [(2010, 'continue-door'), (2250, 'continue-door-card')], N22_WEAK),
]
CLIPS += [
    # (the net climbs: Green's layer 4 of seed 2, up the yellow stair to the terrace and back down; CopyBot
    # Comp's layer 2, up its railed ladder onto the raised room: the footage's CLIMB_A and CLIMB_B)
    ('net-climb', ['--scene', 'emu', '--run-depth', '4', '--seed', '2', '--net-biome', '3', '--dev', 'quiet'], {},
     N22_ARRIVE + '100:UP+RIGHT,30:,100:DOWN+LEFT,30:', 470, 730),
    ('net-climb-ladder', N22_COPYBOT, {}, N22_ARRIVE + '0:place 268 -124 7,20:,150:UP+LEFT,40:', 470, 680),
    # (the Glitch layer: MegaMan by the copy at its heart (0:glitch), its body changing between its forms'
    # and a flip into Seaside's look with Seaside's sky)
    ('glitch-copy-bodies', N22_GLITCH, {}, N22_ARRIVE + '0:glitch,400:', 480, 860),
    # (Lan a step short of his desk, R: he walks onto the PC's spot and jacks in)
    ('desk-jack-in', ['--scene', 'town', '--seed', '2'], {}, N22_ROOM + '0:place -24 8 7,30:,6:R,300:', 540, 800),
]
README_GIFS.update({'net-climb': (None, 10), 'net-climb-ladder': (None, 10), 'glitch-copy-bodies': (None, 12), 'desk-jack-in': (None, 15)})
# 0.24.0 (media a): MegaMan's colours (docs/game/looks.md), the setup's shelf of starting folders
# (docs/game/meta/shelf.md) and issue #865's folders
# (Lan's room, Dad's call paged, START, down the PET's left column to Colours, A: the Colours screen up by 712)
N24_COLOURS = N22_ROOM + '6:START,40:,' + '6:DOWN,6:,' * 5 + '10:,6:A,60:,'
N24_KEY = '6:%s,9:,'
# (a look picked a channel at a time, every palette owned (looks=4094): Armour, R to ProtoMan's page, up the
# grid from PLAIN and across to its red, A; Suit, R to the Undernet's, its indigo; Accent, L to Glitch's, its
# cyan; the look the pictures after it wear, 3.2,1.5,11.7)
N24_PICK = (N24_KEY % 'DOWN' + N24_KEY % 'A' + N24_KEY % 'R' * 3 + N24_KEY % 'UP' * 4 + N24_KEY % 'RIGHT' * 2 + '20:,' + N24_KEY % 'A' +
            N24_KEY % 'DOWN' + N24_KEY % 'A' + N24_KEY % 'R' + N24_KEY % 'UP' * 3 + N24_KEY % 'RIGHT' + '20:,' + N24_KEY % 'A' +
            N24_KEY % 'DOWN' + N24_KEY % 'A' + N24_KEY % 'L' + N24_KEY % 'UP' * 3 + N24_KEY % 'RIGHT' * 3 + '20:,' + N24_KEY % 'A' + '90:')
N24_TOWN = ['--scene', 'town', '--seed', '2']
N24_LOOK = '3.2,1.5,11.7'
# (seed 1's first layer in that look, its arrival paged; a Mettaur's battle at once (0:foes), the Custom screen
# OKed with no chip; with HeatCross brought (--setup's CROSS 1), Up and A on the Custom screen first)
N24_WORN = ['--scene', 'emu', '--seed', '1', '--run-depth', '1', '--dev', f'quiet,god,look={N24_LOOK}']
N24_FIGHT = '30:,0:foes 01@52,330:,%s6:START,20:,6:A,400:'
# (a first layer's arrival paged and the run's opening gift taken, its first offer: MegaMan free by 602)
N24_ARRIVE = '150:,' + '4:A,6:,' * 30 + '20:,6:A,60:,6:A,60:,'
# (a Practice NetBattle between two cores of one machine, both scripted (docs/devtools/arena.md): the first
# MegaMan in that look, the other in Hazard's yellow, black and red (arenalook's twelve hex digits a look, the
# three palettes then their colours); BN5's ROM beside BN6's, as every NetBattle's rules ask)
N24_ARENA = ['--scene', 'emu', '--dev', 'quiet,arena,arenaboth,arenaseed=7,arenalook=03010B020507/060606010B04']
WHOLE_ROMS_A += ('netbattle-looks',)
# (the JACK-IN SETUP at its Folder row, by 132; Right turns the shelf)
N24_SHELF = '60:,6:DOWN,20:,6:DOWN,40:,'


def n24_shelf(name, turns, dev=None):
    return (name, ['--scene', 'setup'] + (['--dev', dev] if dev else []) + ['--input', N24_SHELF + '6:RIGHT,10:,' * turns + '80:'],
            [(190 + 16 * turns, name)], {})


# (a run's first layer with a starting folder, --setup's FOLDER (9 Ember, 3 Z-Canon): the arrival paged, the
# opening gift's first offer taken, a Mettaur's battle at once, its first hand on the Custom screen)
def n24_hand(folder, seed, dev='god,quiet'):
    return ['--scene', 'emu', '--seed', str(seed), '--net-biome', '0', '--setup', f'short,{folder},0,0', '--dev', dev]


N24_HAND = '150:,' + '4:A,6:,' * 30 + '20:,6:A,60:,6:A,60:,30:,0:foes %s@52,330:,'
# (seed 1's Ember hand, AirShot *, GrasSeed F, FireHit1 D, FireBrn1 F, AreaGrab F: the second and the fourth, OK,
# then A for each)
N24_EMBER = '30:,6:RIGHT,14:,6:A,14:,6:RIGHT,10:,6:RIGHT,14:,6:A,20:,6:START,14:,6:A,200:,6:A,90:,6:A,200:'
# (a battle of one buster shot (onehit): OK with no chip, B until the Mettaur is gone, RESULT paged, and the
# next battle's first hand; 1136 frames)
N24_NEXT = '6:START,14:,6:A,200:,' + '4:B,8:,' * 10 + '4:A,6:,' * 34 + '120:,0:foes 01@52,330:,'
# (seed 3's Z-Canon run: its fifth battle's first hand is Atk+10 *, Cannon A, Cannon B, Atk+10 *, Cannon C, up
# by 5480 (a hand follows the battles before it, not the seed alone: four won with a buster shot first); the
# three Cannons picked in order, OK, GigaCan1 forms, A fires it)
N24_ZCANON = (N24_HAND % '01' + N24_NEXT * 4 + '30:,6:RIGHT,14:,6:A,20:,6:RIGHT,14:,6:A,20:,6:RIGHT,10:,6:RIGHT,14:,6:A,40:,'
              '6:START,14:,6:A,330:,6:A,200:')
SCREENSHOTS += [
    # (MegaMan's colours, issue #854: the Colours screen with the look picked; a palette not owned yet on a new
    # profile, Undernet's page and its colour's line; the look worn in a chat box, on the field, on the Custom
    # screen and in a battle; HeatCross in its own colours over it; a NetBattle's two MegaMen, each in his look)
    ('colours', N24_TOWN + ['--dev', 'looks=4094', '--input', N24_COLOURS + N24_PICK], [(1250, 'colours')], {}),
    ('colours-locked', N24_TOWN + ['--input', N24_COLOURS + N24_KEY % 'DOWN' + N24_KEY % 'R' + N24_KEY % 'A' + N24_KEY % 'UP' + '80:'],
     [(820, 'colours-locked')], {}),
    ('colours-worn', N24_WORN + ['--input', N24_ARRIVE + N24_FIGHT % ''],
     [(190, 'colours-chat'), (620, 'colours-field'), (900, 'colours-custom'), (1200, 'colours-battle')], {}),
    ('colours-cross', N24_WORN + ['--setup', 'short,0,0,0,1', '--input', N24_ARRIVE + N24_FIGHT % '6:UP,30:,6:A,60:,'],
     [(1400, 'colours-cross')], {}),
    ('netbattle-looks', N24_ARENA, [(510, 'netbattle-looks')], {}),
    # (the setup's shelf, issue #864: a new profile's FOLDER row on Standard, open, 1/10; Right, Blade locked
    # with its hint; Blade heard of (heard=2), its deed said plainly)
    n24_shelf('shelf-open', 0), n24_shelf('shelf-locked', 1), n24_shelf('shelf-heard', 1, 'heard=2'),
    # (issue #865's folders on the shelf, every one open (folders): Z-Canon rebuilt, Steady, Navi, Ember, Tide)
    n24_shelf('shelf-zcanon', 3, 'folders'), n24_shelf('shelf-steady', 6, 'folders'), n24_shelf('shelf-navi', 7, 'folders'),
    n24_shelf('shelf-ember', 8, 'folders'), n24_shelf('shelf-tide', 9, 'folders'),
    # (Ember's first hand, GrasSeed under the cursor; the grass laid under a Mettaur3; FireBrn1 on it. Z-Canon's
    # hand with Cannon A, B and C in it; GigaCan1 formed)
    ('folder-ember-hand', n24_hand(9, 1) + ['--input', N24_HAND % '03' + N24_EMBER],
     [(1010, 'folder-ember-hand'), (1392, 'folder-ember-grass'), (1432, 'folder-ember-fire')], {}),
    ('folder-zcanon-hand', n24_hand(3, 3, 'god,quiet,onehit') + ['--input', N24_ZCANON],
     [(5550, 'folder-zcanon-hand'), (5800, 'folder-zcanon-advance')], {}),
]
SCREENSHOT_SHEETS.update({
    'shelf-states': [[('shelf-open', 'Open'), ('shelf-locked', 'Locked: a hint'), ('shelf-heard', 'Heard in the Net')]],
    'shelf-folders': [[('shelf-steady', 'Steady'), ('shelf-navi', 'Navi')], [('shelf-ember', 'Ember'), ('shelf-tide', 'Tide')]],
})
CLIPS += [
    # (the Colours screen: the three channels picked, the preview following the cursor)
    ('colours-pick', N24_TOWN + ['--dev', 'looks=4094'], {}, N24_COLOURS + N24_PICK, 700, 1280),
    # (Ember's first hand at a Mettaur3: GrasSeed F and FireBrn1 F picked, OK; the seed lays grass under it, the
    # flame burns it for double)
    ('folder-ember', n24_hand(9, 1), {}, N24_HAND % '03' + N24_EMBER, 990, 1490),
    # (Z-Canon: Cannon A, B and C picked in order, OK, the Program Advance forming and fired)
    ('folder-zcanon', n24_hand(3, 3, 'god,quiet,onehit'), {}, N24_ZCANON, 5540, 6130),
]
README_GIFS.update({'colours-pick': (None, 15), 'folder-ember': (None, 15), 'folder-zcanon': (None, 15)})


# 0.24.0 (media b)
# 0.24.0's notes: Team ProtoMan and their Souls, Falzar's Crosses, Forms, the PET's menu, the link, the map, the
# requests in Lan's HP, the fixes. Each run is written as a playtest's batch is (tools/play.py's commands: press,
# hold, wait, mash and its dev steps), a `shot` at each picture.
def n24_script(commands):
    """tools/play.py's commands as --input's steps, and the frame of each `shot` among them."""
    steps, shots, at = [], [], 0
    for cmd in commands.split(';'):
        w = cmd.split()
        if not w:
            continue
        if w[0] == 'shot':
            shots.append(at)
            continue
        if w[0] == 'press':
            run = [(int(w[2]) if len(w) > 2 else 6, w[1]), (6, '')]
        elif w[0] == 'hold':
            run = [(int(w[2]), w[1])]
        elif w[0] == 'wait':
            run = [(int(w[1]), '')]
        elif w[0] == 'mash':
            run = [(4, w[1]), (6, '')] * max(1, int(w[2]) // 10)
        else:
            steps.append('0:' + ' '.join(w))   # (a dev step, as play.py sends it)
            continue
        for frames, buttons in run:
            steps.append(f'{frames}:{buttons}')
            at += frames
    return ','.join(steps), shots


def n24_run(names, start, commands, env=None):
    """A SCREENSHOTS entry: the game's options, the commands, a name for each `shot`."""
    script, shots = n24_script(commands)
    assert len(shots) == len(names), names
    return (names[0], start.split() + ['--input', script], list(zip(shots, names)), env or {})


# (a run's first layer, its arrival's words paged and the opening gift's first offer taken, as tools/cw_routes.py's
# LAYER; Lan's room at a new run, Dad's call paged, as its ROOM; Lan's HP after an act, its words paged, the
# courier spoken to from below him, as tests/scenarios/home_requests.py's COURIER)
N24_LAYER = 'wait 150; mash A 300; wait 20; press A; wait 60; press A; wait 60; '
N24_ROOM = 'wait 40; mash A 460; wait 30; '
N24_COURIER = 'wait 300; mash A 900; wait 60; place -84 54 7; wait 10; press A; wait 40; '
N24_FIRST = '--scene emu --seed 15 --net-biome 0 --dev god,quiet,hp=2000'
N24_SECOND = '--scene emu --seed 3 --run-depth 2 --dev quiet'
N24_FORMS = '--scene emu --seed 1 --run-depth 2 --setup short,0,0,0,1 --dev quiet,god'
N24_TO_FORMS = 'press START; wait 60; ' + 'press DOWN; wait 6; ' * 4 + 'press A; wait 60; '
N24_UNITE = 'wait 300; press A; wait 10; press START; wait 10; press DOWN; wait 10; '
N24_FOLDER_EDIT = 'wait 5; press A; wait 60; press A; wait 60; press A; wait 40; '
# (a dark layer's board, BN5's with ProtoSoul held (souls=64): Lan's briefing and the phase's banner paged, R
# passes play to ProtoMan and his word is paged, the dev step stands him by a dark panel, A opens his window
# (Liberation, WideSwrd, Pass), A takes Liberation: its battle's first turn, ProtoMan in person)
N24_BOARD = ('wait 242; ' + 'press A; wait 20; ' * 8 + 'wait 30; ' + 'press A; wait 20; ' * 12 + 'press R; wait 10; '
             + 'press A; wait 20; ' * 6 + 'wait 30; shot; board dark; wait 4; press A; wait 10; shot; wait 30; press A; wait 300; '
             'press START; wait 10; press A; wait 206; shot')
# (a liberation's battle by MegaMan alone, its three turns left by OK with no chip, to TIME UP! and LIBERATE FAILED!)
N24_TIME_UP = ('wait 150; mash A 700; wait 60; board dark; wait 4; press A; wait 40; press A; wait 270; '
               + 'press START; wait 10; press A; wait 700; ' * 2 + 'press START; wait 10; press A; wait 674; shot; wait 30; shot')
N24B_SHOTS = [
    # (Team ProtoMan guarding: DarkProto's card in BN5's Undernet and his slash, Meddy in a HeelNavi's body on
    # BN5's ACDC Area and her capsules in the fight, the autopilot's walk to each)
    n24_run(['team-darkproto-card', 'team-darkproto'], '--scene emu --seed 3 --run-depth 3 --net-biome x5 --guardian 59 --dev god,quiet,hp=600',
            'wait 1400; shot; wait 1400; shot', N22_AUTO),
    n24_run(['team-meddy-map', 'team-meddy'], '--scene emu --seed 3 --run-depth 3 --net-biome x0 --guardian 64 --dev god,quiet',
            'wait 1000; shot; wait 1800; shot', N22_AUTO),
    # (their Souls: ProtoSoul held (souls=0x40) and a folder of Sword, UNITE under OK and its picture, MegaMan
    # united, his charged WideSword; MeddySoul (0x800) by Recov10, united, and her two capsules on the second
    # Custom screen, the cursor on the first)
    n24_run(['soul-proto-unite', 'soul-proto', 'soul-proto-slash'], N24_FIRST + ',souls=0x40,folder=71',
            N24_LAYER + 'foes 01@63; ' + N24_UNITE + 'shot; press A; wait 60; press START; wait 10; press A; wait 260; shot; wait 20; '
            'hold B 100; wait 16; shot'),
    n24_run(['soul-meddy', 'soul-meddy-capsules'], N24_FIRST + ',souls=0x800,folder=154',
            N24_LAYER + 'foes 205@52; ' + N24_UNITE + 'press A; wait 60; press START; wait 10; press A; wait 220; shot; wait 40; '
            'hold B 90; wait 520; press L; wait 150; ' + 'press RIGHT; wait 8; ' * 3 + 'press DOWN; wait 20; shot'),
    n24_run(['team-board-protoman', 'team-board-command', 'team-board-battle'], '--scene emu --seed 5 --run-depth 5 --dev dark,souls=64,god',
            N24_BOARD),
    # (Falzar's five Crosses held by their flags, 0xE7-0xEB: CROSSSELECT's list of them, the cursor on TenguCross;
    # MegaMan in it, his charged shot)
    n24_run(['falzar-crossselect', 'falzar-tengucross'], N24_SECOND + ',god,hp=400',
            N24_LAYER + 'flags 231 235 1; wait 4; foes 01@52/01@63; wait 300; press UP; wait 40; press DOWN; wait 20; press DOWN; wait 20; '
            'shot; press A; wait 60; press START; wait 10; press A; wait 260; hold B 80; wait 20; shot'),
    # (Forms from the PET's menu: HeatCross brought and six more held by their flags, CROSSSELECT's five and two
    # spares; ElecCross taken up, the cursor among the spares; swapped. Its Souls' page with every Soul of BN5's held)
    n24_run(['forms-screen', 'forms-taken', 'forms-swap'], N24_FORMS,
            N24_LAYER + 'flags 227 232 1; wait 4; ' + N24_TO_FORMS + 'press DOWN; wait 30; shot; wait 20; press A; wait 30; shot; wait 20; '
            'press A; wait 30; shot; wait 20'),
    n24_run(['forms-souls'], N24_FORMS + ',souls=0xFFF', N24_LAYER + 'flags 227 232 1; wait 4; ' + N24_TO_FORMS + 'press R; wait 40; shot; wait 20'),
    # (the PET's menu over a layer's field; SELECT pressed once and MegaMan walked under the map)
    n24_run(['pet-menu-columns'], N24_SECOND, N24_LAYER + 'press START; wait 60; shot; wait 20'),
    n24_run(['map-stays-open'], N24_SECOND, N24_LAYER + 'press SELECT; wait 60; hold RIGHT 40; wait 4; shot; wait 20'),
    # (fast forward's mark while it runs; a dropped link's card over a battle and the band as it is back, drawn by
    # the dev step, docs/devtools/net-square.md: the Square's words, no NetBattle under them)
    n24_run(['fast-forward-mark'], N24_SECOND + ',god', N24_LAYER + 'hold FAST+UP+RIGHT 40; shot; hold FAST+UP+RIGHT 30'),
    n24_run(['link-dropped', 'link-back'], N24_SECOND + ',god',
            N24_LAYER + 'foes 01@52/01@63; wait 300; press START; wait 10; press A; wait 240; reconnect 12; wait 60; shot; wait 200; '
            'reconnect back; wait 20; shot'),
    # (a party's field as seat 1 sees it, the dev's stood-in party of three against layer 3's guardian: each
    # partner's HP under his mark)
    n24_run(['party-partner-hp'], '--scene emu --seed 3 --run-depth 3 --dev quiet,partycore=3,partybot=1,partyview=1,partyhp=60',
            'wait 1950; shot', N22_AUTO),
    # (Lan's HP after act 3 of seed 7, a request done (job=3) or a Bring request with two Wood chips in the Pack
    # (job=4, pack=38): the courier's pay in the asker's name, and BN6's Chip Trader screen over the two)
    n24_run(['courier-pays', 'courier-pays-zenny'], '--scene home --seed 7 --run-depth 10 --dev job=3,jobstate=2',
            N24_COURIER + 'press A; wait 40; ' * 3 + 'shot; ' + 'press A; wait 40; ' * 2 + 'shot'),
    n24_run(['bring-choice-asked', 'bring-choice'], '--scene home --seed 7 --run-depth 10 --dev job=4,jobstate=2,pack=38',
            N24_COURIER + 'press A; wait 40; ' * 4 + 'shot; press A; wait 40; shot'),
    # (fixed: GolmHit1 on a Surrounded field, MegaMan turned left with L, the fist on the foe ahead of him (#838);
    # a liberation's battle standing still under TIME UP! and LIBERATE FAILED! (#794); CosmoMn SP's card in Folder
    # Edit, 80 (#835); ColonlDS's row in the Pack (#732); HubBatc's card, BN6's blank (#731))
    n24_run(['golmhit-turned-left', 'golmhit-turned'], N24_FIRST,
            N24_LAYER + 'chip 120 I 29; foes 06@12/06@22/06@52/06@62/sides@33/mm@32; wait 300; press A; wait 10; press START; wait 10; '
            'press A; wait 260; status paralysis 900; wait 30; hold L 4; wait 30; shot; press A; wait 20; shot'),
    n24_run(['liberation-time-up', 'liberation-time-up-failed'], '--scene emu --seed 5 --run-depth 5 --dev dark,god', N24_TIME_UP),
    n24_run(['card-cosmoman-sp'], '--scene town --seed 2', N24_ROOM + 'press START; wait 40; chip 516 C 1; ' + N24_FOLDER_EDIT + 'shot'),
    n24_run(['pack-colonlds'], '--scene town --seed 2',
            N24_ROOM + 'press START; wait 40; chip 492 C 1; ' + N24_FOLDER_EDIT + 'press RIGHT; wait 40; press DOWN; wait 30; shot'),
    # (seed 14's fourth layer, its vault open (library=1) and a Pack to trade from: three chips into the Chip Trader,
    # BN6's save after the trade, and "Try again?" with no reward pick over it (#734))
    n24_run(['trader-after-trade'], '--scene emu --seed 14 --run-depth 4 --dev quiet,library=1,pack=120',
            'wait 151; mash A 300; wait 20; mash A 60; wait 30; mash A 60; wait 30; by trader; wait 10; ' + 'press A; wait 30; ' * 6
            + 'press A; wait 40; ' * 4 + 'press A; wait 130; press A; wait 60; shot'),
    n24_run(['card-hubbatc'], '--scene town --seed 2', N24_ROOM + 'press START; wait 40; chip 309 J 1; ' + N24_FOLDER_EDIT + 'shot'),
]
SCREENSHOTS += N24B_SHOTS
N24B = {entry[0]: entry for entry in N24B_SHOTS}   # (tools/before_after.py's 0.24.0 scenes run these by the last release too)
# (BN5's Navis, Souls, board and chips: the whole ROM folder)
WHOLE_ROMS_A += ('team-darkproto-card', 'team-meddy-map', 'soul-proto-unite', 'soul-meddy', 'team-board-protoman', 'forms-souls',
                 'liberation-time-up', 'card-cosmoman-sp', 'pack-colonlds')
# (the shared dive's pictures, docs/screenshots/dive-*.png and docs/clips/dive-crew.gif, are no entries here: two or
# three games on a local server at the clock's pace, tests/scenarios/online_week.py's and online_crew_view.py's
# courses played by tools/scenarios_online.py's cast (alpha, beta and gamma from Lan's HP, seed 120's Square, the
# full NetPass), a picture taken at the moment wanted, as tools/online_pictures.py's dive takes are; a retake
# draws another net)
# (the map opened by a press, MegaMan walked under it, the next press closing it; the courier's pay, his boxes paged)
N24_MAP_CLIP = n24_script(N24_LAYER + 'wait 20; press SELECT; wait 50; hold RIGHT 40; hold UP+RIGHT 40; hold LEFT 40; wait 20; press SELECT; wait 40')[0]
N24_PAY_CLIP = n24_script(N24_COURIER + 'press A; wait 40; ' * 5 + 'press A; wait 60')[0]
CLIPS += [
    ('map-stays-open', N24_SECOND.split(), {}, N24_MAP_CLIP, 620, 890),
    ('courier-pays', '--scene home --seed 7 --run-depth 10 --dev job=3,jobstate=2'.split(), {}, N24_PAY_CLIP, 1264, 1650),
]
README_GIFS.update({'map-stays-open': (None, 15), 'courier-pays': (None, 15)})


def clips(only=None):
    """docs/clips/NAME.webm, .mp4 and .png: frames of scripted runs, 4x, 30 fps (and
    NAME.gif, 2x, at the rate README_GIFS gives, for those in it); a clip
    marked '3ds' as a New 3DS shows it, the second screen under the
    picture (two_screens), 2x and its GIF 1x."""
    from PIL import Image
    keep_pil_nearest()
    out = os.path.join(ROOT, 'docs', 'clips')
    tmp = os.path.join(ROOT, '.build', 'clips')
    os.makedirs(out, exist_ok=True)
    for name, args, env, script, first, last, *mode in CLIPS:
        if only and name not in only:
            continue
        dual = mode == ['3ds']
        # (a clip 'beside': the same run once a (label, options) each, side by side under the labels)
        beside = mode[1] if mode[:1] == ['beside'] else [(None, [])]
        shutil.rmtree(tmp, ignore_errors=True)
        os.makedirs(os.path.join(tmp, 'data'))
        os.makedirs(os.path.join(tmp, 'png'))
        saved = {k: os.environ.get(k) for k in env}
        os.environ.update(env)
        extra = ['--input', script] if script else []
        if dual:
            extra += ['--second-shot-range', f'{first}:{last}:/src/.build/clips/s']
        code = 0
        for i, (_, more) in enumerate(beside):
            shutil.rmtree(os.path.join(tmp, 'data'), ignore_errors=True)
            os.makedirs(os.path.join(tmp, 'data'))
            code = code or docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/clips/data',
                                  *args, *more, *extra, '--frames', str(last + 1), '--shot-range', f'{first}:{last}:/src/.build/clips/f{i}_',
                                  mounts=docs_rom_mounts(bn5=whole_rom_folder(name) or name.startswith('trade-')))
        for k, v in saved.items():
            if v is None:
                os.environ.pop(k, None)
            else:
                os.environ[k] = v
        if code:
            return code
        n = 0
        for f in range(first, last + 1, 2):
            ims = []
            for i in range(len(beside)):
                im = Image.open(os.path.join(tmp, f'f{i}_{f:05d}.bmp')).convert('RGB')
                w, h = im.size   # the canvas: the game's 240 x 160 in the middle
                ims.append(im.crop(((w - 240) // 2, (h - 160) // 2, (w + 240) // 2, (h + 160) // 2)))
            im = ims[0] if len(ims) == 1 else sheet([list(zip(ims, (label for label, _ in beside)))])
            if dual:
                im = two_screens(im, Image.open(os.path.join(tmp, f's{f:05d}.bmp')).convert('RGB'))
            if n == 0:
                im.save(os.path.join(out, f'{name}.png'), optimize=True)
            im.save(os.path.join(tmp, 'png', f'{n:05d}.png'))
            n += 1
        # (side by side the sheet is 2x already: encoded as two screens are)
        if encode_clip(name, tmp, out, n, im.size, dual or len(beside) > 1):
            return 1
    return 0


def encode_clip(name, tmp, out, n, size, dual=False):
    """docs/clips/NAME.webm and .mp4 from the n frames in tmp/png
    (00000.png on), 30 a second, 4x with whole pixels (and the colour
    planes' 2x2 blocks inside them; two screens 2x), and NAME.gif, 2x (two
    screens 1x), where README_GIFS names it: nonzero if ffmpeg failed."""
    w, h = size
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
        cmd = docker_run() + [
            '--rm', '-u', f'{os.getuid()}:{os.getgid()}', '--entrypoint', 'ffmpeg', '-v', f'{tmp}:/work', '-v', f'{out}:/out',
            FFMPEG_IMAGE, *enc]
        if subprocess.call(cmd):
            return 1
    exts = ('webm', 'mp4', 'gif') if name in README_GIFS else ('webm', 'mp4')
    sizes = ', '.join(f'{ext} {os.path.getsize(os.path.join(out, name + "." + ext)) // 1024} KB' for ext in exts)
    print(f'clip {name}: {n} frames, {sizes}')
    return 0


FLATPAK_ID = 'io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless'


def online_pictures(kind, rest):
    """The pictures of online play (tools/online_pictures.py), where none or
    one of them is named: games in a Square of their own on a local server,
    the Linux build and the server made first. --keep: the scenes already
    captured."""
    names = [n for n in rest if not n.startswith('--')]
    if names and not any(n.startswith('online-') for n in names):
        return 0
    build('linux')
    if kind == 'screenshots' and (not names or 'online-browser' in names):
        build('web')   # (the browser's picture: the site's player, tools/online_browser_picture.py)
    if subprocess.call([sys.executable, os.path.join(ROOT, 'server', 'build.py')], cwd=ROOT):
        return 1
    sys.path.insert(0, os.path.join(ROOT, 'tools'))
    import online_pictures as pictures
    return pictures.main(rest, kinds=(kind,), encode=encode_clip)


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


# bn6.h and the headers it includes for the chips', the virus families', the PET's, the panels' and MegaMan's forms' addresses (src/emu/), and the map's buffers' (bn6_map.h)
BN6_HEADERS = ('bn6.h', 'bn6_attacks.h', 'bn6_chips.h', 'bn6_families.h', 'bn6_pet.h', 'bn6_panels.h', 'bn6_forms.h', 'bn6_navis.h', 'bn6_shop.h', 'bn6_expression.h', 'bn6_sound.h', 'bn6_map.h', 'bn6_battler.h', 'bn6_look.h')


def lint_rom_data(update):
    """Every address of src/emu/bn6.h (and bn6_pet.h) and every RomLayout field named in docs/rom/."""
    from collections import Counter
    doc = ''
    for name in sorted(os.listdir(os.path.join(ROOT, 'docs', 'rom'))):
        if name.endswith('.md'):
            with open(os.path.join(ROOT, 'docs', 'rom', name)) as f:
                doc += f.read().lower()
    bn6 = ''
    for name in BN6_HEADERS:
        with open(os.path.join(ROOT, 'src', 'emu', name)) as f:
            bn6 += f.read()
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
    return lint_compare('ROM offsets without their note in docs/rom/', 'rom_data.txt',
                        'ROM offsets docs/rom/ does not name (AGENTS.md: a new one needs its note);\n'
                        'this list only shrinks: build.py lint --update after naming one', Counter(missing), update)


# the knowledge base (AGENTS.md, The knowledge base): the okf skill's validator, and every docs/ path the tree names
DOC_PATH = re.compile(r'(?<![\w/.\-])docs/[\w./\-]*[\w/]')
# (docs/itch/description.html is the live itch page's copy, changed only with the page: docs/project/itch.md)
DOC_PATH_SKIP = ('docs/project/moved.md', 'docs/releases/v', 'docs/itch/description.html', 'docs/symbols/', 'docs/voice/catalog.json', 'docs/re/state.json',
                 'docs/re/dashboard.html', '.claude/skills/okf/', '.claude/skills/reverse-engineering/',
                 '.claude/skills/game-design/', 'CHANGELOG.md')


def lint_docs():
    """docs/ as an OKF bundle (okf-validate.mjs --strict: every concept typed, none orphaned, no broken
    link inside it; a link leaving it for a file of the repository is not broken), and every docs/ path
    named in the tree there (a path pinned to a commit, `at 7b4b8ef`, or a placeholder, NAME, TAG, X.Y.Z,
    excepted)."""
    bad = []
    node = shutil.which('node')
    if node is None:
        print('docs: node not found (the okf validator needs it)')
        return False
    out = subprocess.run([node, os.path.join(ROOT, '.claude', 'skills', 'okf', 'okf-validate.mjs'), 'docs', '--strict'],
                         cwd=ROOT, capture_output=True, text=True)
    for line in (out.stdout + out.stderr).splitlines():
        m = re.match(r'(error|gate)\s+(\S+): (.*)$', line)
        if not m:
            continue
        b = re.match(r'broken concept link -> (\S+) \(no such concept\)', m.group(3))
        if b:
            target = os.path.normpath(os.path.join('docs', os.path.dirname(m.group(2)), b.group(1).split('#')[0]))
            if not target.startswith('docs' + os.sep) and os.path.isfile(os.path.join(ROOT, target)):
                continue
        bad.append(f'{m.group(2)}: {m.group(3)}')
    # the files git keeps and any new one not yet added (a new concept is checked before its commit)
    names = subprocess.run(['git', 'ls-files', '-z', '--cached', '--others', '--exclude-standard'], cwd=ROOT,
                           capture_output=True, check=True).stdout.decode().split('\0')
    for n in filter(None, names):
        if n.startswith(DOC_PATH_SKIP) or not os.path.isfile(os.path.join(ROOT, n)):
            continue
        try:
            with open(os.path.join(ROOT, n), encoding='utf-8') as f:
                text = f.read()
        except (UnicodeDecodeError, OSError):
            continue
        for m in DOC_PATH.finditer(text):
            p = m.group(0).rstrip('.')
            if re.search(r'[A-Z]{3,}|X\.Y\.Z|\bvX', p.split('/')[-1]) and not p.endswith('.md'):
                continue
            if re.search(r'/(NAME|TAG|vX\.Y\.Z|vX_Y_Z)', p) or re.match(r'`?\s+at\s+`?[0-9a-f]{7}', text[m.end():]):
                continue
            if not os.path.exists(os.path.join(ROOT, p)):
                if p.endswith('.md') or p.endswith('/'):
                    bad.append(f'{n}: names {p}, which is not there')
    for b in bad:
        print(f'  docs: {b}')
    print(f'docs: {len(bad)} problems')
    return not bad


def lizard(*args):
    """lizard over src/ in the build image (its output)."""
    out = subprocess.run(docker_run() +
                         ['--rm', '-v', f'{ROOT}:/src', '-w', '/src', IMAGE, 'lizard', *args, 'src'],
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
    # (each file on its own, side by side, its findings in a file of its own: the counts read a file's findings after the
    # line naming their function, so files' lines must not interleave)
    script = ('flags="-std=c11 -O2 -fanalyzer -fdiagnostics-plain-output -D_DEFAULT_SOURCE -DCW_DESKTOP -Ishared/online -isystem third_party/monocypher '
              '$(for d in src/*/; do printf -- "-I%s " "$d"; done) -Ibuild/host/gen $(pkg-config --cflags sdl2 | sed "s/-I/-isystem /g") '
              '-isystem /opt/mgba/host/include"; out=$(mktemp -d); export flags out; '
              'printf "%s\\n" src/*/*.c | xargs -P "$(nproc)" -I{} sh -c \'gcc $flags -c -o /dev/null "$1" > "$out/$(echo "$1" | tr / _)" 2>&1\' _ {}; '
              'cat "$out"/*')
    build('host')   # (its version.h)
    out = subprocess.run(docker_run() +
                         ['--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src', IMAGE,
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
        run = subprocess.run(docker_run() +
                             ['--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src', image,
                              'sh', '-c', f'rm -f {out}/{binary}; make TARGET={target} OUT={out} VERSION=lint -j{os.cpu_count() or 4} '
                              f'"{extra[0]}" "{extra[1]}" {out}/{binary} 2>&1 >/dev/null; '
                              f'{nm} -A -g --defined-only {out}/obj/*/*.o'], capture_output=True, text=True)
        links[target] = run.stdout
    ensure_image()
    test_bins = ' '.join('build/lint/' + name for name in
                         ('test_core', 'test_emu', 'test_online_transport', 'test_online'))
    test = subprocess.run(docker_run() +
                          ['--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src', IMAGE,
                           'sh', '-c', f'rm -f {test_bins} && '
                           f'make -j{os.cpu_count() or 4} {test_bins} 2>&1 >/dev/null && '
                           f'nm -A -g --defined-only build/lint/test/src/*/*.o {test_bins}'], capture_output=True, text=True)
    if test.returncode:
        sys.exit(f'the test links for build.py lint failed:\n{(test.stdout + test.stderr)[-1200:]}')
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
        if target == 'tests':
            # A shared object can be dropped by one test and reached by
            # another. The union of discarded sections loses that fact;
            # the final executables say which public functions survive.
            linked = set(re.findall(r'^build/lint/test_\w+:[0-9a-f]* T (\w+)$', text, re.MULTILINE))
            reached |= {(f, n) for f, n in here if n in linked}
        else:
            reached |= here - dropped
    dead = Counter(f'{f}: {n}' for f, n in present - reached if n != 'main')
    return lint_compare('functions no build reaches', 'dead.txt',
                        'functions no build reaches (the host and handheld builds and the tests, build.py lint):\n'
                        'remove one, or make it static where its own file uses it; build.py lint --update', dead, update)


def lint_offsets(update):
    """No field of the game's structures as a bare offset outside bn6.h (and bn6_pet.h, issue #34): BN6_PLAYER
    + 0x1C is BN6_PLAYER_X there. Event flags (BN6_FLAG_ + n) and indexes (+ n * size) are numbers, not fields."""
    import glob
    from collections import Counter
    found = []
    for path in sorted(glob.glob(os.path.join(ROOT, 'src', '*', '*.[ch]'))):
        if os.path.basename(path) in BN6_HEADERS:
            continue
        with open(path) as f:
            for n, line in enumerate(f, 1):
                for m in re.finditer(r'\b(BN6_\w+)\s*\+\s*(0x[0-9A-Fa-f]+|\d+)\b(?!\s*\*)', line):
                    if not m.group(1).startswith('BN6_FLAG_'):
                        found.append(f'{os.path.relpath(path, ROOT)}: {m.group(0)}')
    return lint_compare('fields of the game\'s structures as bare offsets, outside src/emu/bn6.h', 'offsets.txt',
                        'bare offsets into bn6.h\'s structures (BN6_X + 0x..): name the field in bn6.h instead;\n'
                        'this list only shrinks: build.py lint --update after naming one', Counter(found), update)


def lint_layers(update):
    """The code's layers (tools/layers.py, docs/code/layers.md): no include of
    src/ that tests/lint/layers.txt does not allow, but those in
    tests/lint/layer_violations.txt, which only shrinks."""
    sys.stdout.flush()
    return subprocess.call([sys.executable, os.path.join(ROOT, 'tools', 'layers.py'), '--update' if update else '--lint']) == 0


def lint_generated():
    """The files tools write from the tree (tools/generated.py: docs/symbols and
    docs/rom/symbols.md's numbers, docs/re's status, the voice catalog): their
    sources sound (every name in bn6.h and bn5.h described, the ledger's shape,
    the words files read through), and the files themselves left as dev has
    them, where its workflow writes them after each push
    (docs/project/merging.md). On main and a tag, in step with their sources."""
    sys.stdout.flush()
    return subprocess.call([sys.executable, os.path.join(ROOT, 'tools', 'generated.py'), 'check']) == 0


def lint_shards():
    """The registries kept a file an entry (tools/shards.py: docs/re/ledger,
    tests/xdiff/mutations, tests/xdiff/legit; docs/project/merging.md,
    Registries): each reads whole, is written as the tool writes it, and
    none of the old single files is back."""
    sys.stdout.flush()
    return subprocess.call([sys.executable, os.path.join(ROOT, 'tools', 'shards.py'), 'check']) == 0


def lint_markers():
    """No conflict marker left in a file git tracks: a `<<<<<<< ` line, a bare
    `=======` after it and a `>>>>>>> ` line after that. A merge resolved in
    part, or a tool that wrote a conflict back (tools/generated.py restore
    did, into a page already resolved: tests/test_generated.py), passed every
    other check with the markers committed."""
    names = subprocess.run(['git', 'ls-files', '-z'], cwd=ROOT, capture_output=True, check=True).stdout.decode().split('\0')
    bad = []
    for n in filter(None, names):
        path = os.path.join(ROOT, n)
        try:
            if not os.path.isfile(path) or os.path.getsize(path) > 8 << 20:
                continue
            with open(path, encoding='utf-8') as f:
                lines = f.read().split('\n')
        except (UnicodeDecodeError, OSError):
            continue   # (not text)
        state, at = 0, 0
        for i, line in enumerate(lines, 1):
            if line.startswith('<<<<<<< ') or line == '<<<<<<<':
                state, at = 1, i
            elif state == 1 and line == '=======':
                state = 2
            elif state == 2 and (line.startswith('>>>>>>> ') or line == '>>>>>>>'):
                bad.append(f'{n}:{at}')
                state = 0
    for b in bad:
        print(f'lint: {b}: a conflict\'s markers are left in the file: resolve it')
    print(f'lint: conflict markers left in tracked files: {len(bad)}')
    return not bad


def lint_claims():
    """The claims of the branches in flight (tools/claims.py, docs/project/claims.md):
    each file well formed, no two meeting in a space or a unit, at most one
    changing Run."""
    sys.stdout.flush()
    return subprocess.call([sys.executable, os.path.join(ROOT, 'tools', 'claims.py'), '--lint']) == 0


def lint_free_space():
    """src/emu/free_space.h and docs/engine/emulation/memory.md's and
    free-space.md's tables as tools/free_space.py writes them from the free
    space's plan, and the plan whole (docs/engine/emulation/free-space.md)."""
    sys.stdout.flush()
    return subprocess.call([sys.executable, os.path.join(ROOT, 'tools', 'free_space.py'), '--check']) == 0


def lint_dispatch_numbers():
    """The dispatch's numbers BN4 takes of the tables every game shares
    (tools/xchiprecipe_bn4.py: its Navis' third-kind TYPES3_NAVIS, the
    fourth kind's run TYPES4, first to end) clear of every number a
    track of BN5's moves and every one a Navi of BN5's not yet moved will
    (tools/bn5_twins.py dispatch_reserved): the MOVED table is one for
    every game (docs/devtools/chip-recipe-generator.md, The dispatch's
    numbers). Needs no ROM."""
    import ast
    sys.path.insert(0, os.path.join(ROOT, 'tools'))
    try:
        import bn5_twins
    finally:
        sys.path.pop(0)
    with open(os.path.join(ROOT, 'tools', 'xchiprecipe_bn4.py'), encoding='utf-8') as f:
        tree = ast.parse(f.read())
    bad, seen = [], set()
    for node in tree.body:
        name = getattr(node.targets[0], 'id', '') if isinstance(node, ast.Assign) else ''
        if name not in ('TYPES3_NAVIS', 'TYPES4'):
            continue
        seen.add(name)
        value = ast.literal_eval(node.value)
        held = bn5_twins.dispatch_reserved(4 if name == 'TYPES4' else 3)
        for n in range(*value) if name == 'TYPES4' else value:
            if n in held:
                bad.append(f"{name}'s {n:#04x} is {held[n]}")
    for name in sorted({'TYPES3_NAVIS', 'TYPES4'} - seen):
        bad.append(f'{name} is not there to check')
    for b in bad:
        print(f'lint: tools/xchiprecipe_bn4.py: {b}')
    print(f"lint: the dispatch's numbers of BN4's: {len(bad)} of BN5's own")
    return not bad


def lint_itch():
    """The itch.io page in step with the repository (docs/project/itch.md, Keeping it
    in step): its description names no version, docs/itch/'s files are the
    ones applied on itch, and every release but the newest has its devlog
    post there (tools/itch.py lint, offline)."""
    sys.stdout.flush()
    return subprocess.call([sys.executable, os.path.join(ROOT, 'tools', 'itch.py'), 'lint']) == 0


def lint_ps5_param():
    """The PS5 app's sce_sys/param.json, which Prospero Store's catalog reads
    at each release's tag (ps5/README.md, Submitting to Prospero Store): its
    contentVersion is the CHANGELOG's newest version's, its major plus one,
    minor and patch (0.18.0 as 01.018.000), so a release raises it."""
    import json
    with open(os.path.join(ROOT, 'CHANGELOG.md')) as f:
        newest = re.search(r'^## (\d+)\.(\d+)\.(\d+) \(', f.read(), re.M)
    with open(os.path.join(ROOT, 'sce_sys', 'param.json')) as f:
        have = json.load(f)['contentVersion']
    want = f'{int(newest.group(1)) + 1:02d}.{int(newest.group(2)):03d}.{int(newest.group(3)):03d}'
    print(f'ps5 param.json: contentVersion {have}, {want} for the CHANGELOG\'s newest version')
    if have != want:
        print(f'  refused: sce_sys/param.json\'s contentVersion is {have}; set it to {want} with the release')
    return have == want


# ---- the code's smells (AGENTS.md, Code structure) ----

SMELL_LINES = {'.c': 1000, '.m': 1000, '.h': 400}   # a file past this many lines holds more than one thing
SMELL_INCLUDES = 20     # a .c file including more of src/'s headers reaches into too many others
SMELL_PARAMS = 7        # a function taking more parameters wants a struct
SMELL_NESTING = 5       # a function's body indented deeper than this many levels wants a function of its own
SMELL_DUPLICATE = 10    # lines of a block repeated elsewhere (lizard -Eduplicate) want one function
# what characters say lives in its own files, apart from what decides when (docs/voice/rules.md)
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
    script = ('for h in src/*/*.h shared/online/*.h; do gcc -std=c11 -fsyntax-only -D_DEFAULT_SOURCE -DCW_DESKTOP -Ishared/online '
              '$(for d in src/*/; do printf -- "-I%s " "$d"; done) -Ibuild/host/gen $(pkg-config --cflags sdl2 | sed "s/-I/-isystem /g") '
              '-isystem /opt/mgba/host/include -Wno-pragma-once-outside-header -x c "$h" 2>&1 | grep -q "error" && echo "$h"; done; true')
    out = subprocess.run(docker_run() +
                         ['--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src', IMAGE,
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
    .build/symbols and docs/rom/symbols.md's counts. bn6f's names stay out of git
    (docs/rom/symbols.md)."""
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
    ok = lint_layers(update) and ok
    ok = lint_generated() and ok
    ok = lint_shards() and ok
    ok = lint_claims() and ok
    ok = lint_markers() and ok
    ok = lint_free_space() and ok
    ok = lint_dispatch_numbers() and ok
    ok = lint_docs() and ok
    ok = lint_itch() and ok
    ok = lint_ps5_param() and ok
    ok = lint_complexity(update) and ok
    ok = lint_smells(update) and ok
    ok = lint_headers() and ok
    ok = lint_analyzer(update) and ok
    ok = lint_dead(update) and ok
    return 0 if ok else 1


def online_pair(rest):
    """BN6's Practice on one linked pair of cores, with a snapshot restored
    and replayed midway (tools/online_pair_run.c, docs/online/link.md)."""
    ap = argparse.ArgumentParser(prog='build.py online-pair')
    ap.add_argument('--rom', required=True, help="the player's BN6 Gregar USA ROM")
    ap.add_argument('--state', required=True, help='a boot-4.state from a data folder')
    ap.add_argument('--at', type=int, default=1150, help='the frame the snapshot is taken at')
    ap.add_argument('--horizon', type=int, default=8, help='the frames replayed after the restore')
    args = ap.parse_args(rest)
    for path in (args.rom, args.state):
        if not os.path.isfile(path):
            ap.error(f'not a file: {path}')
    ensure_image()
    code = docker('make', 'online-pair', *(['WERROR=1'] if os.environ.get('CI') else []))
    if code:
        return code
    out = os.path.join(ROOT, '.build', 'online-pair')
    os.makedirs(out, exist_ok=True)
    return docker('build/host/online-pair', '/pair/rom.gba', '/pair/boot.state', '/src/.build/online-pair/trace',
                  str(args.at), str(args.horizon),
                  mounts=[(os.path.abspath(args.rom), '/pair/rom.gba:ro'), (os.path.abspath(args.state), '/pair/boot.state:ro')])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('action', nargs='?', default='all', choices=['all', 'host', 'device', 'linux', 'windows', 'android', 'macos', 'ios', 'flatpak', '3ds', 'switch', 'vita', 'psp', 'ps4', 'ps5', 'run', 'web', 'serve', 'release', 'package', 'shot', 'asan', 'test', 'lint', 'clean', 'atlas', 'tiles', 'sizes', 'tour', 'pacing', 'economy', 'npc-anims', 'xdiff', 'xrecords', 'scenarios', 'screenshots', 'clips', 'town', 'world', 'symbols', 'server', 'server-run', 'server-test', 'online-pair'])
    ap.add_argument('rest', nargs=argparse.REMAINDER)
    a = ap.parse_args()
    if a.action == 'server-test':
        sys.exit(subprocess.call([sys.executable, '-m', 'unittest', 'discover', '-s', 'server/tests', *a.rest], cwd=ROOT))
    if a.action in ('server', 'server-run'):
        code = subprocess.call([sys.executable, os.path.join(ROOT, 'server', 'build.py')], cwd=ROOT)
        if code == 0 and a.action == 'server-run':
            code = subprocess.call([sys.executable, os.path.join(ROOT, 'build', 'server', 'cyberworld-server.pyz'), *a.rest], cwd=ROOT)
        sys.exit(code)
    if a.action == 'online-pair':
        sys.exit(online_pair(a.rest))
    if a.action == 'clean':
        shutil.rmtree(os.path.join(ROOT, 'build'), ignore_errors=True)
        return
    if a.action == 'test':
        ensure_image()
        code = docker('make', 'test', f'-j{os.cpu_count() or 4}', *(['WERROR=1'] if os.environ.get('CI') else []))
        # (linux/steam/add-to-steam.py, the site's pages and docs/re's status, on the host's Python)
        if code == 0:
            code = subprocess.call([sys.executable, os.path.join(ROOT, 'tests', 'test_add_to_steam.py')])
        if code == 0:   # port/update.py, the PortMaster port's own update
            code = subprocess.call([sys.executable, os.path.join(ROOT, 'tests', 'test_port_update.py')])
        if code == 0:   # tools/update_manifest.py's notes, as the update box shows them
            code = subprocess.call([sys.executable, os.path.join(ROOT, 'tests', 'test_update_manifest.py')])
        if code == 0:   # tools/discord_release.py's post in the Discord server, every release's, unsent
            code = subprocess.call([sys.executable, os.path.join(ROOT, 'tests', 'test_discord_release.py')])
        if code == 0:
            code = subprocess.call([sys.executable, os.path.join(ROOT, 'tests', 'test_site.py')])
        if code == 0:   # docs/re's ledger sound (its written files are dev's workflow's: tools/generated.py)
            code = subprocess.call([sys.executable, os.path.join(ROOT, 'tools', 're_status.py'), '--lint'])
        if code == 0:   # the voice catalog's reading of C (tools/voice_catalog.py)
            code = subprocess.call([sys.executable, os.path.join(ROOT, 'tests', 'test_voice_catalog.py')])
        if code == 0:   # tools/generated.py restore in a merge, on a repository of its own
            code = subprocess.call([sys.executable, os.path.join(ROOT, 'tests', 'test_generated.py')])
        if code == 0:   # the agents' tools (tools/cw.py and its MCP server)
            code = subprocess.call([sys.executable, os.path.join(ROOT, 'tests', 'test_cw.py')])
        if code == 0:
            code = subprocess.call([sys.executable, '-m', 'unittest', 'discover', '-s', 'server/tests'], cwd=ROOT)
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
    if a.action == 'switch':
        # the Nintendo Switch (switch/README.md): build/switch/cyberworld-endless.nro
        build('switch')
        print('built', os.path.join(ROOT, 'build', 'switch', 'cyberworld-endless.nro'))
        return
    if a.action == 'psp':
        # the PSP (psp/README.md): build/psp/EBOOT.PBP, and the release's
        # build/psp/cyberworld-endless-psp.zip
        build('psp')
        print('built', os.path.join(ROOT, 'build', 'psp', 'EBOOT.PBP'))
        psp_release()
        return
    if a.action == 'vita':
        # the PlayStation Vita (vita/README.md): build/vita/cyberworld-endless.vpk
        build('vita')
        print('built', os.path.join(ROOT, 'build', 'vita', 'cyberworld-endless.vpk'))
        return
    if a.action == 'ps4':
        # the PS4, and a PS5 through it (ps4/README.md): build/ps4/cyberworld-endless.pkg
        build('ps4')
        print('built', os.path.join(ROOT, 'build', 'ps4', 'cyberworld-endless.pkg'))
        return
    if a.action == 'ps5':
        # the PS5's own build (ps5/README.md): build/ps5/cyberworld-endless-ps5.zip, its app folder zipped
        build('ps5')
        print('built', os.path.join(ROOT, 'build', 'ps5', 'cyberworld-endless-ps5.zip'))
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
    if a.action in ('clips', 'screenshots'):
        names = [n for n in a.rest if not n.startswith('--')]
        code = 0
        if not names or any(not n.startswith('online-') for n in names):
            build('host')
            code = (clips if a.action == 'clips' else screenshots)(names or None)
        sys.exit(code or online_pictures(a.action, a.rest))
    if a.action == 'xdiff':
        # (local only: both ROMs; the tool builds the host binary and runs it once a spec)
        sys.exit(subprocess.call([sys.executable, os.path.join(ROOT, 'tools', 'xdiff.py'), *a.rest]))
    if a.action == 'scenarios':
        # (local only: the ROM; the game played through scripted courses, its state checked on the way, tests/scenarios)
        sys.exit(subprocess.call([sys.executable, os.path.join(ROOT, 'tools', 'scenarios.py'), *a.rest]))
    if a.action == 'xrecords':
        # (local only: both ROMs; BN5's records in BN6's engine, each fightable or why not)
        sys.exit(subprocess.call([sys.executable, os.path.join(ROOT, 'tools', 'xrecords.py'), *a.rest]))
    if a.action == 'pacing':
        build('host')
        rom_dir = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
        os.makedirs(os.path.join(ROOT, '.build', 'data'), exist_ok=True)
        r = docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/data',
                   '--pacing', '/src/.build/pacing.txt', mounts=[(rom_dir, '/rom:ro')])
        # (then the battles per layer and act, played: the battle floor's bands)
        sys.exit(r or subprocess.call([sys.executable, os.path.join(ROOT, 'tools', 'battle_floor.py'), '--out',
                                       os.path.join(ROOT, '.build', 'pacing.txt'), *a.rest]))
    if a.action == 'economy':
        # (local only: the ROM; what a run hands out by act against its bands, docs/game/economy/; RUNS after the action)
        build('host')
        rom_dir = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
        os.makedirs(os.path.join(ROOT, '.build', 'data'), exist_ok=True)
        runs = next((r for r in a.rest if r.isdigit()), '300')
        # ("noarc" after the action: the acts' arcs off, into .build/economy-noarc.txt, to hold the arcs' drops against)
        noarc = 'noarc' in a.rest
        sys.exit(docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/data',
                        '--economy', f'/src/.build/economy{"-noarc" if noarc else ""}.txt:{runs}{":noarc" if noarc else ""}',
                        mounts=[(rom_dir, '/rom:ro')]))
    if a.action == 'npc-anims':
        # (local only: the ROMs, BN5's and BN4's read; each animation an NPC in another game's sprite is told to play, issue #614)
        build('host')
        rom_dir = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
        os.makedirs(os.path.join(ROOT, '.build', 'data'), exist_ok=True)
        sys.exit(docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/data',
                        '--npc-anims', '/src/.build/npc-anims.txt', mounts=[(rom_dir, '/rom:ro')]))
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
    if a.action == 'sizes':
        build('host')
        sys.exit(layer_sizes(*a.rest[:1]))
    if a.action in ('all', 'host', 'shot'):
        build('host')
    if a.action == 'all':
        subprocess.run([sys.executable, os.path.join(ROOT, 'server', 'build.py')], check=True)
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
