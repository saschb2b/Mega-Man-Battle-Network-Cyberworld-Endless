#!/usr/bin/env python3
"""Exercise Phase 1 social controls in two actual game processes, with local evidence."""
import argparse
import contextlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

from online_proof import ROOT, hashes, initial_checkpoint, records, stop, wait_for


class Script:
    def __init__(self):
        self.steps, self.shots, self.frames = [], {}, 0

    def wait(self, frames):
        self.steps.append(f'{frames}:')
        self.frames += frames

    def press(self, button):
        self.steps.extend((f'4:{button}', '12:'))
        self.frames += 16

    def shot(self, name):
        self.shots[name] = self.frames + 1
        self.wait(10)

    def menu(self, row):
        self.press('START')
        for _ in range(row):
            self.press('DOWN')
        self.press('A')


def route():
    script = Script()
    script.wait(240)
    script.press('A'); script.press('A'); script.wait(45); script.shot('card')
    script.press('A'); script.press('R'); script.wait(30); script.shot('blocked')
    script.press('R'); script.press('A')
    script.press('B'); script.press('B'); script.press('B')
    script.menu(1); script.press('A'); script.wait(60); script.shot('phrase')
    script.wait(90)
    script.menu(2); script.press('A'); script.wait(45); script.shot('emote')
    script.wait(90)
    script.menu(3); script.press('RIGHT'); script.press('A'); script.wait(90); script.shot('channel')
    script.menu(3); script.press('LEFT'); script.press('A'); script.wait(90)
    script.menu(4)
    script.wait(200)
    return script


def run(args, evidence):
    binary = args.binary.resolve()
    saved = {name: initial_checkpoint(binary, args.rom_dir, evidence / name, 120 + i,
                                     evidence / f'{name}-checkpoint.log') for i, name in enumerate(('Alpha', 'Beta'))}
    script = route()
    with contextlib.ExitStack() as stack:
        log = stack.enter_context((evidence / 'server.log').open('w'))
        events = evidence / 'server.jsonl'
        server = subprocess.Popen([sys.executable, str(ROOT / 'build/server/cyberworld-server.pyz'), '--port', '0',
                                   '--event-log', str(events), '--state-dir', str(evidence / 'server-data')],
                                  cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
        stack.callback(stop, server)
        listener = wait_for(lambda: next((r for r in records(events) if r.get('event') == 'listening'), None), 10, 'Server did not start')
        endpoint = f'ws://127.0.0.1:{listener["port"]}/presence'
        clients = {}
        for name in ('Alpha', 'Beta'):
            log = stack.enter_context((evidence / f'{name}.log').open('w'))
            env = dict(os.environ, CYBERWORLD_ONLINE_TRACE=str(evidence / f'{name}.jsonl'))
            pictures = ','.join(f'{frame}:{evidence / (name + "-" + label + ".bmp")}' for label, frame in script.shots.items())
            command = [str(binary), '--headless', '--size', '720x480', '--rom-dir', str(args.rom_dir),
                       '--data-dir', str(evidence / name), '--online', endpoint, '--online-name', name,
                       '--frames', str(script.frames + (180 if name == 'Beta' else 0)),
                       '--input', ','.join(script.steps) if name == 'Alpha' else f'{script.frames + 180}:', '--shot', pictures]
            clients[name] = subprocess.Popen(command, cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT)
            stack.callback(stop, clients[name])
        print(f'Social proof: {endpoint}; evidence {evidence}', flush=True)
        clients['Alpha'].wait(timeout=100)
        stop(server)
        clients['Beta'].wait(timeout=30)
    alpha, beta = (records(evidence / f'{name}.jsonl') for name in ('Alpha', 'Beta'))
    checks = dict(inspect=any(r['card_serial'] and r['card_hp'] == 100 for r in alpha),
                  blocked=any(r['peers'] and r['visible_peers'] == 0 for r in alpha),
                  phrase_and_emote_received=max((r['event_serial'] for r in beta), default=0) >= 2,
                  channel_switch=any(r['channel'] == 2 for r in alpha))
    for name in clients:
        checks[name + '_save_unchanged'] = hashes(evidence / name / 'savedata') == saved[name]
        checks[name + '_returned'] = 'checkpoint return=restored carried=identical' in (evidence / f'{name}.log').read_text()
    report = dict(result='passed' if all(checks.values()) else 'failed', checks=checks, endpoint=endpoint, screenshots=script.shots)
    (evidence / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    try:
        from PIL import Image
        for path in evidence.glob('*.bmp'):
            with Image.open(path) as picture:
                picture.save(path.with_suffix('.png'))
    except ImportError:
        pass
    print(json.dumps(report, indent=2), flush=True)
    return all(checks.values())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom-dir', required=True, type=Path)
    parser.add_argument('--binary', type=Path, default=ROOT / 'build/linux/cyberworld')
    args = parser.parse_args()
    base = ROOT / '.build/online-social'
    base.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='run-', dir=base))
    raise SystemExit(0 if run(args, evidence) else 1)


if __name__ == '__main__':
    main()
