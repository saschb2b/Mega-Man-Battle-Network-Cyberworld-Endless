#!/usr/bin/env python3
"""Run a build on a ROCKNIX device without touching the installed port.

Copies build/aarch64/cyberworld.aarch64 to /tmp/cwtest on the device, starts
it through a temporary launcher beside the real one (data in
/tmp/cwtest/data, the installed boot state reused), takes screenshots with
grim at the given seconds, waits for the run to end, prints its log, and
removes the launcher and /tmp/cwtest again. The player's saves are hashed
before and after.

    tools/device_run.py root@device --control-path /tmp/sock \\
        --frames 3600 --shots 20,40 --env CYBERWORLD_AUTOPILOT=1 -- --scene emu
"""
import argparse
import os
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PORTS = '/storage/roms/ports'
GAME = PORTS + '/cyberworld'
TEST = '/tmp/cwtest'
LAUNCHER = PORTS + '/zz-cwtest.sh'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('host')
    ap.add_argument('--control-path', required=True)
    ap.add_argument('--frames', type=int, default=3600)
    ap.add_argument('--shots', default='', help='seconds after start, comma separated')
    ap.add_argument('--env', action='append', default=[], help='NAME=VALUE for the game')
    ap.add_argument('--out', default=os.path.join(ROOT, '.build', 'device'))
    argv = sys.argv[1:]
    cut = argv.index('--') if '--' in argv else len(argv)
    a = ap.parse_args(argv[:cut])
    game_args = argv[cut + 1:]
    ssh = ['ssh', '-S', a.control_path, '-o', 'ConnectTimeout=10', a.host]

    def sh(cmd, **kw):
        return subprocess.run(ssh + [cmd], capture_output=True, text=True, **kw)

    if 'NO GAME' not in sh('curl -s localhost:1234/runningGame').stdout:
        sys.exit('the device is busy')
    before = sh(f'sha256sum {GAME}/savedata/*').stdout
    binary = os.path.join(ROOT, 'build', 'aarch64', 'cyberworld.aarch64')
    with open(binary, 'rb') as f:
        subprocess.run(ssh + [f'rm -rf {TEST}; mkdir -p {TEST}/data; cat > {TEST}/cyberworld.aarch64; '
                              f'chmod +x {TEST}/cyberworld.aarch64; cp {GAME}/boot-3.state {TEST}/data/ 2>/dev/null'],
                       stdin=f, check=True)
    env = ' '.join(a.env)
    run = (f'date +%s > {TEST}/t0; {env} {TEST}/cyberworld.aarch64 --rom-dir "$GAMEDIR/rom" '
           f'--data-dir {TEST}/data --frames {a.frames} {" ".join(game_args)}; date +%s > {TEST}/t1')
    # the real launcher, with its log, binary and data redirected
    make = (f'sed -e \'s#> "$GAMEDIR/log.txt" && exec > >(tee "$GAMEDIR/log.txt") 2>&1#exec > {TEST}/log.txt 2>\\&1#\' '
            f'-e \'s#^pm_platform_helper .*#pm_platform_helper {TEST}/cyberworld.aarch64#\' '
            f'-e \'s#^./cyberworld.aarch64 .*#{run}#\' "{PORTS}/Cyberworld Endless.sh" > {LAUNCHER}; chmod +x {LAUNCHER}')
    sh(make, check=True)
    try:
        sh('curl -s localhost:1234/reloadgames >/dev/null; sleep 2; '
           f'curl -s -X POST -d "{LAUNCHER}" localhost:1234/launch')
        sh(f'for i in $(seq 60); do [ -f {TEST}/t0 ] && break; sleep 1; done')
        start = time.time()
        os.makedirs(a.out, exist_ok=True)
        for s in [int(x) for x in a.shots.split(',') if x]:
            time.sleep(max(0, s - (time.time() - start)))
            sh(f'source /etc/profile; grim {TEST}/shot.png')
            with open(os.path.join(a.out, f'shot_{s}.png'), 'wb') as f:
                f.write(subprocess.run(ssh + [f'cat {TEST}/shot.png'], capture_output=True).stdout)
        sh('for i in $(seq 600); do curl -s localhost:1234/runningGame | grep -q "NO GAME" && break; sleep 1; done')
        out = sh(f'echo elapsed $(( $(cat {TEST}/t1) - $(cat {TEST}/t0) ))s; cat {TEST}/log.txt').stdout
        print(out)
    finally:
        sh(f'rm -f {LAUNCHER}; curl -s localhost:1234/reloadgames >/dev/null; rm -rf {TEST}')
    after = sh(f'sha256sum {GAME}/savedata/*').stdout
    print('saves unchanged' if before == after else 'SAVES CHANGED:\n' + before + after)


if __name__ == '__main__':
    main()
