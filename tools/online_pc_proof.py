#!/usr/bin/env python3
"""Play the between-act PC entry, save failure, cancel, join and return locally.

Needs the Linux game and server builds and the player's own ROM directory.
Fresh saves, fake identities, logs and whole-frame captures stay in .build/.
"""
import argparse
import contextlib
import io
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

from PIL import Image

import play
from online_proof import ROOT, hashes, records, stop, wait_for


class Session:
    def __init__(self, directory):
        self.directory = directory
        self.data = directory / 'data'

    def do(self, command, picture):
        with contextlib.redirect_stdout(io.StringIO()):
            play.cmd_do('game', [command])
        state = (self.directory / 'state.txt').read_text()
        bmp = self.directory / (picture + '.bmp')
        play.talk(str(self.directory), f'shot {bmp}')
        with Image.open(bmp) as image:
            assert image.size == (240, 160)
            image.save(bmp.with_suffix('.png'))
        (self.directory / (picture + '.txt')).write_text(state)
        return state

    def log(self):
        return (self.directory / 'log.txt').read_text()


def run(args, evidence):
    session = Session(evidence / 'game')
    play.home = lambda name: str(session.directory)
    play.BINARY = str(args.binary.resolve())
    play.ROM_DIR = str(args.rom_dir.resolve())
    checks = {}
    with contextlib.ExitStack() as stack:
        events = evidence / 'server.jsonl'
        log = stack.enter_context((evidence / 'server.log').open('w'))
        server = subprocess.Popen([sys.executable, str(ROOT / 'build/server/cyberworld-server.pyz'),
                                   '--port', '0', '--event-log', str(events),
                                   '--state-dir', str(evidence / 'server-data')],
                                  cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
        stack.callback(stop, server)
        listener = wait_for(lambda: next((r for r in records(events) if r.get('event') == 'listening'), None),
                            10, 'Server did not start')
        endpoint = f'ws://127.0.0.1:{listener["port"]}/presence'
        trace = evidence / 'client.jsonl'
        previous = os.environ.get('CYBERWORLD_ONLINE_TRACE')
        os.environ['CYBERWORLD_ONLINE_TRACE'] = str(trace)
        try:
            with contextlib.redirect_stdout(io.StringIO()):
                play.cmd_start('game', ['--fresh', '--seed', '3', '--', '--scene', 'home', '--run-depth', '7',
                                        '--size', '240x160', '--dev', 'zenny=8000,job=2,jobstate=1'])
        finally:
            if previous is None:
                os.environ.pop('CYBERWORLD_ONLINE_TRACE', None)
            else:
                os.environ['CYBERWORLD_ONLINE_TRACE'] = previous
        stack.callback(play.cmd_stop, 'game')
        # Read the arrival and use BN6's own jack-out to stand at the PC.
        session.do('wait 600;' + 'press A; wait 94;' * 6 +
                   'wait 120; press R; wait 90; press LEFT; press A; wait 90; press A; wait 200; press R; wait 30',
                   'pc-menu')
        state = session.do('press DOWN; press A; wait 30', 'network')
        checks['pc_opens_network'] = 'scene network' in state
        checkpoint = hashes(session.data / 'savedata')
        state = session.do('press B; wait 200', 'cancel-return')
        checks['cancel_returns_to_pc'] = 'place Lan\'s Room' in state and 'chat closed' in state
        checks['cancel_checkpoint_identical'] = checkpoint == hashes(session.data / 'savedata')
        # Only this fake data directory is changed: an unwritable state
        # target must leave the still-live home core available for retry.
        blocked = session.data / 'savedata/run.state.tmp'
        blocked.mkdir()
        count = session.log().count('PC checkpoint saved; opening Network')
        state = session.do('press R; wait 20; press DOWN; press A; wait 30', 'save-failed')
        checks['failed_save_stays_at_pc'] = 'place Lan\'s Room' in state and 'zenny 8000' in state
        checks['failed_save_does_not_open_network'] = session.log().count('PC checkpoint saved; opening Network') == count
        checks['failed_write_reported'] = 'run.state could not be written' in session.log()
        blocked.rmdir()
        # The next opening reads this fixture exactly as a saved custom server.
        (session.data / 'network.ini').write_text(
            'identity=0123456789abcdef0123456789abcdef\nname=GuestNavi\nselected=1\nconsent=2\n'
            'server=wss://cyberworld.saschb2b.com/presence\n' + f'server={endpoint}\n')
        state = session.do('press A; wait 30', 'retry-network')
        checks['successful_retry_opens_network'] = 'scene network' in state
        checkpoint = hashes(session.data / 'savedata')
        session.do('press A; wait 240', 'joining')

        def connected():
            if any(row.get('connected') for row in records(trace)):
                return True
            session.do('wait 60', 'joining')
            return False

        wait_for(connected, 15, 'PC visit did not connect to the local server')
        session.do('wait 120', 'square')
        checks['local_server_joined'] = True
        state = session.do('press START;' + 'press DOWN;' * 4 + 'press A; wait 240', 'leave-return')
        checks['leave_returns_to_pc'] = 'place Lan\'s Room' in state and 'zenny 8000' in state
        checks['leave_carried_identical'] = 'checkpoint return=restored carried=identical' in session.log()
        checks['visit_checkpoint_identical'] = checkpoint == hashes(session.data / 'savedata')
        state = session.do('press R; wait 20; press A; wait 600', 'ordinary-jack-in')
        checks['ordinary_jack_in_preserved'] = 'place Lan\'s HP' in state
    report = {'result': 'passed' if all(checks.values()) else 'failed', 'checks': checks,
              'evidence': str(evidence), 'screenshots_require_visual_review': True}
    (evidence / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))
    return all(checks.values())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom-dir', required=True, type=Path)
    parser.add_argument('--binary', type=Path, default=ROOT / 'build/linux/cyberworld')
    args = parser.parse_args()
    base = ROOT / '.build/online-pc'
    base.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='run-', dir=base))
    print(f'PC proof: {evidence}', flush=True)
    raise SystemExit(0 if run(args, evidence) else 1)


if __name__ == '__main__':
    main()
