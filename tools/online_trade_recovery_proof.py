#!/usr/bin/env python3
"""Exercise title recovery against isolated copies of a real checkpoint.

The pending transaction is a storage fixture with unchanged staged state,
not a native chip swap. No server is contacted. All files stay in .build.
"""
import argparse
import contextlib
import hashlib
import io
import json
from pathlib import Path
import shutil
import subprocess
import tempfile

from PIL import Image

import play
from online_proof import ROOT
from online_social_proof import Script


def protected(directory):
    # Opening the advisory lock creates an empty file, not saved progress.
    # Every journal, receipt, intent and checkpoint byte remains protected.
    return {str(p.relative_to(directory)): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in directory.rglob('*') if p.is_file() and
            p.name != 'trade.lock' and
            (p.relative_to(directory).parts[0].startswith('savedata') or
             p.relative_to(directory).parts[0].startswith('trade.'))}


def route(mode):
    script = Script()
    script.wait(45); script.shot('initial')
    script.press('A'); script.shot('first-action')
    if mode == 'discard':
        script.press('DOWN'); script.press('A')
    elif mode == 'unbound':
        script.press('A'); script.shot('kept')
        script.press('A'); script.press('B')
    else:
        for key in ('B', 'START', 'SELECT', 'L', 'R'):
            script.press(key)
    script.wait(20); script.shot('final')
    return script


def case(args, evidence, mode):
    data = evidence / mode
    data.mkdir()
    shutil.copytree(args.checkpoint / 'savedata', data / 'savedata')
    if mode == 'import-blocked':
        # A folder cannot replace this file during interrupted import
        # recovery. Preserve both it and the previous checkpoint.
        (data / 'savedata').rename(data / 'savedata.old')
        (data / 'savedata').write_bytes(b'blocked import destination')
    else:
        subprocess.run([str(args.fixture.resolve()), str(data),
                        'unbound' if mode == 'discard' else mode], check=True)
    before = protected(data)
    script = route(mode)
    shots = ','.join(f'{frame}:{data / (name + ".bmp")}' for name, frame in script.shots.items())
    command = [str(args.binary.resolve()), '--headless', '--scene', 'title',
               '--rom-dir', str(args.rom_dir.resolve()), '--data-dir', str(data),
               '--size', '240x160', '--frames', str(script.frames + 1),
               '--input', ','.join(script.steps), '--shot', shots]
    with (data / 'game.log').open('w') as log:
        subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT,
                       check=True, timeout=45)
    for name in script.shots:
        with Image.open(data / (name + '.bmp')) as picture:
            assert picture.size == (240, 160)
            picture.save(data / (name + '.png'))
    after = protected(data)
    if mode != 'discard':
        return {mode + '_keeps_all_files': before == after}
    run_files = ('run.sav', 'run.state', 'battle.sav', 'run.trade', 'run.make',
                 'run.area', 'run.seen', 'run.folder', 'run.act', 'run.dark',
                 'run.souls', 'run.board', 'transfer.manifest')
    removed = {'savedata/' + name for name in run_files} | {'trade.newrun'}
    # Optional per-run records need not occur in every input checkpoint.
    removed &= before.keys()
    return {'discard_removes_only_run_and_intent': set(before) - set(after) == removed,
            'discard_preserves_permanent_progress_and_history':
                all(before.get(key) == value for key, value in after.items())}


def failed_continue(args, evidence):
    session = evidence / 'failed-continue'
    session.mkdir()
    data = session / 'data'
    shutil.copytree(args.checkpoint / 'savedata', data / 'savedata')
    play.home = lambda name: str(session)
    play.BINARY = str(args.binary.resolve())
    play.ROM_DIR = str(args.rom_dir.resolve())
    with contextlib.redirect_stdout(io.StringIO()):
        try:
            play.cmd_start('recovery', ['--', '--scene', 'title', '--size', '240x160'])
            play.cmd_do('recovery', ['wait 45; press START; wait 60'])
            # The menu has already seen Continue. Simulate its file
            # becoming unreadable before activation, only in this copy.
            (data / 'savedata/run.sav').write_bytes(b'damaged test checkpoint')
            before = protected(data)
            play.cmd_do('recovery', ['press A; wait 90'])
            state = (session / 'state.txt').read_text()
            play.talk(str(session), f'shot {session / "failed.bmp"}')
            after = protected(data)
            with Image.open(session / 'failed.bmp') as picture:
                picture.save(session / 'failed.png')
            return {'failed_continue_stays_at_title': 'scene title' in state,
                    'failed_continue_preserves_files': before == after,
                    'failed_continue_creates_no_new_intent': not (data / 'trade.newrun').exists()}
        finally:
            play.cmd_stop('recovery')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--checkpoint', type=Path, required=True, help='data directory containing a real savedata checkpoint')
    parser.add_argument('--rom-dir', type=Path, required=True)
    parser.add_argument('--binary', type=Path, default=ROOT / 'build/linux/cyberworld')
    parser.add_argument('--fixture', type=Path, default=ROOT / 'build/host/online-trade-recovery-fixture')
    args = parser.parse_args()
    base = ROOT / '.build/title-recovery'
    base.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='proof-', dir=base))
    checks = {}
    for mode in ('unbound', 'pending', 'blocked', 'import-blocked', 'discard'):
        checks.update(case(args, evidence, mode))
    checks.update(failed_continue(args, evidence))
    report = {'checks': checks, 'pass': all(checks.values()), 'visual_review_required': True}
    (evidence / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(dict(report, evidence=str(evidence)), indent=2))
    return 0 if report['pass'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
