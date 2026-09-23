#!/usr/bin/env python3
"""Build, test and package Cyberworld Endless.

Compilation runs inside the `cyberworld-build` Docker image (Debian trixie),
which matches the glibc and SDL2 that current ROCKNIX ships.

  python3 build.py              host and device binaries
  python3 build.py host         host binary only
  python3 build.py device       aarch64 binary only
  python3 build.py shot ...     run the host binary headlessly (options below)
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
CONTEXT = os.environ.get('DOCKER_CONTEXT_NAME', 'desktop-linux')


def docker(*cmd, mounts=()):
    args = ['docker']
    if CONTEXT:
        args += ['--context', CONTEXT]
    args += ['run', '--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src']
    for host, guest in mounts:
        args += ['-v', f'{host}:{guest}']
    for var in ('CYBERWORLD_AUDIO_DUMP', 'CYBERWORLD_SFX_LOG', 'CYBERWORLD_AUDIO_OFFLINE'):
        if os.environ.get(var):
            args += ['-e', f'{var}={os.environ[var]}']
    args += [IMAGE, *cmd]
    return subprocess.call(args)


def ensure_image():
    probe = ['docker'] + (['--context', CONTEXT] if CONTEXT else []) + ['image', 'inspect', IMAGE]
    if subprocess.call(probe, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL) != 0:
        build = ['docker'] + (['--context', CONTEXT] if CONTEXT else []) + ['build', '-t', IMAGE, os.path.join(ROOT, 'docker')]
        if subprocess.call(build) != 0:
            sys.exit('docker build failed')


def build(target):
    ensure_image()
    if docker('make', f'TARGET={target}', f'-j{os.cpu_count() or 4}') != 0:
        sys.exit(f'{target} build failed')


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
    with open(os.path.join(game, 'rom', 'PUT_YOUR_ROM_HERE.txt'), 'w') as f:
        f.write('Copy your own Mega Man Battle Network 6: Cybeast Gregar (USA) .gba file into this folder.\n')
    shutil.copy2(os.path.join(ROOT, 'port', 'Cyberworld Endless.sh'), out)
    print('packaged', out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('action', nargs='?', default='all', choices=['all', 'host', 'device', 'package', 'shot', 'asan', 'test', 'clean'])
    ap.add_argument('rest', nargs=argparse.REMAINDER)
    a = ap.parse_args()
    if a.action == 'clean':
        shutil.rmtree(os.path.join(ROOT, 'build'), ignore_errors=True)
        return
    if a.action == 'test':
        ensure_image()
        sys.exit(docker('make', 'test'))
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
