#!/usr/bin/env python3
"""Verify real no-checkpoint visits use remembered, unlocked title choices.

Build Linux first. All made-up profiles and numeric traces stay in .build;
the player's ROM is only read by the actual game. No original-game claim
is inferred from this project-behavior integration check.
"""
import argparse
import contextlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

from online_proof import ROOT, hashes, records, stop, wait_for


def fixture_tool(evidence):
    binary = evidence / 'fixture'
    subprocess.run(['cc', '-std=c11', '-D_DEFAULT_SOURCE', '-ffunction-sections', '-fdata-sections',
                    '-Wall', '-Wextra', '-Werror', *['-I' + str(p) for p in (ROOT / 'src').iterdir() if p.is_dir()],
                    str(ROOT / 'tools/online_fresh_fixture.c'), str(ROOT / 'src/core/meta.c'),
                    '-Wl,--gc-sections', '-o', str(binary)], check=True, cwd=ROOT)
    return binary


def visit(args, evidence, fixture, url, name, values):
    directory = evidence / name
    (directory / 'savedata').mkdir(parents=True)
    expected = json.loads(subprocess.check_output([str(fixture), str(directory / 'savedata/profile.sav'),
                                                  *map(str, values)], text=True))
    before = hashes(directory / 'savedata')
    trace = directory / 'loadout.json'
    motion = directory / 'presence.jsonl'
    env = dict(os.environ, CYBERWORLD_LOADOUT_TRACE=str(trace), CYBERWORLD_ONLINE_TRACE=str(motion))
    # Actual Start menu, fourth Down to Leave, then back to the title.
    route = '240:,4:START,8:,' + '4:DOWN,8:,' * 4 + '4:A,160:'
    with (directory / 'game.log').open('w') as log:
        subprocess.run([str(args.binary.resolve()), '--headless', '--rom-dir', str(args.rom_dir.resolve()),
                        '--data-dir', str(directory), '--online', url, '--online-name', 'FreshVisitor',
                        '--frames', '480', '--input', route], cwd=ROOT, env=env,
                       stdout=log, stderr=subprocess.STDOUT, check=True, timeout=45)
    observed = json.loads(trace.read_text())
    checks = {
        'fresh_inactive': not observed['checkpoint'] and not observed['run_active'],
        'connected': any(r.get('connected') for r in records(motion)),
        'returned_to_title': 'online: no checkpoint; returned to title' in (directory / 'game.log').read_text(),
        'profile_unchanged_no_checkpoint_created': hashes(directory / 'savedata') == before,
        'no_souls': observed['souls'] == 0,
    }
    return observed, expected, checks


def run(args, evidence):
    fixture = fixture_tool(evidence)
    with contextlib.ExitStack() as stack:
        events = evidence / 'server.jsonl'
        log = stack.enter_context((evidence / 'server.log').open('w'))
        server = subprocess.Popen([sys.executable, str(ROOT / 'build/server/cyberworld-server.pyz'),
                                   '--port', '0', '--event-log', str(events),
                                   '--state-dir', str(evidence / 'server-data')],
                                  cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
        stack.callback(stop, server)
        listener = wait_for(lambda: next((r for r in records(events) if r.get('event') == 'listening'), None),
                            10, 'Local server did not start')
        url = f'ws://127.0.0.1:{listener["port"]}/presence'
        # folder, Cross, helpers, folder unlock bits, Cross unlock bits, title marks.
        cases = {'standard': (0, 0, 0, 0, 0, 0), 'blade': (1, 3, 0, 2, 8, 0),
                 'storm-star': (2, 1, 24, 4, 2, 0), 'beast': (2, 5, 24, 4, 32, 256),
                 'locked': (1, 3, 16, 0, 0, 0), 'invalid': (255, 255, 255, 0, 0, 0)}
        observations, results = {}, {}
        for name, values in cases.items():
            observed, expected, checks = visit(args, evidence, fixture, url, name, values)
            if not expected or name == 'locked':
                expected = observations.get('standard', observed)['folder']
            helpers = values[2] & (31 if name == 'beast' else 15)
            if helpers & 8:
                expected = [(entry & 511) | 26 << 9 for entry in expected]
            checks.update(folder_matches=observed['folder'] == expected,
                          crosses_match=observed['crosses'] == ({'blade': 4, 'storm-star': 1, 'beast': 16}.get(name, 0)),
                          helpers_match=observed['helpers'] == helpers,
                          beast_match=observed['beast'] == int(name == 'beast'))
            if name != 'standard':
                checks['fresh_hp_unchanged'] = (observed['hp'], observed['max_hp']) == (
                    observations['standard']['hp'], observations['standard']['max_hp'])
            if name == 'locked':
                checks['locked_pack_unchanged'] = observed['pack_sha256'] == observations['standard']['pack_sha256']
            observations[name] = observed
            results[name] = checks
            print(name, 'PASS' if all(checks.values()) else 'FAIL', flush=True)
    report = {'result': 'PASS' if all(all(c.values()) for c in results.values()) else 'FAIL',
              'cases': results, 'observations': observations, 'evidence': str(evidence)}
    (evidence / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))
    return report['result'] == 'PASS'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom-dir', required=True, type=Path)
    parser.add_argument('--binary', type=Path, default=ROOT / 'build/linux/cyberworld')
    args = parser.parse_args()
    base = ROOT / '.build/online-fresh'
    base.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='run-', dir=base))
    print(f'Fresh visit proof: {evidence}', flush=True)
    raise SystemExit(0 if run(args, evidence) else 1)


if __name__ == '__main__':
    main()
