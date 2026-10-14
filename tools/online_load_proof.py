#!/usr/bin/env python3
"""Run 2-16 native games through the built server and verify checkpoint returns.

Each game uses its own private data directory under .build/online-load-proof.
Requires the player's ROM; no ROM or save is copied into a release.
"""
import argparse
import contextlib
import json
import os
from pathlib import Path
import shutil
import statistics
import subprocess
import sys
import tempfile

from online_proof import ROOT, hashes, initial_checkpoint, records, stop, validate_return, wait_for


def ready(evidence, names):
    for name in names:
        rows = records(evidence / f'{name}.jsonl')
        if not rows or rows[-1]['frame'] < 540 or not rows[-1]['connected']:
            return False
    return True


def validate(name, rows, saved, evidence, expected_peers):
    full = [row for row in rows if row['connected'] and len(row['peers']) == expected_peers]
    if len(full) < 20:
        raise RuntimeError(f'{name}: fewer than 20 samples in its full channel')
    local = {(row['local']['x'], row['local']['y']) for row in full}
    if len(local) < 3:
        raise RuntimeError(f'{name}: game movement was not observed')
    npcs = [peer for row in full for peer in row['peers'] if peer['npc_match']]
    if expected_peers and len(npcs) < 20:
        raise RuntimeError(f'{name}: real game peer NPCs were not observed')
    for peer in npcs:
        if any(peer['npc_' + axis] != peer['rendered_' + axis] for axis in ('x', 'y', 'z')):
            raise RuntimeError(f'{name}: real NPC differs from rendered peer position')
    if hashes(evidence / name / 'savedata') != saved:
        raise RuntimeError(f'{name}: checkpoint files changed during the visit')
    latency = [row['latency_ms'] for row in full if row.get('latency_ms')]
    return dict(full_channel_samples=len(full), expected_peers=expected_peers,
                local_positions=len(local), matched_npc_samples=len(npcs),
                round_trip_median_ms=statistics.median(latency) if latency else None,
                checkpoint_files_unchanged=len(saved), carried_checkpoint=validate_return(name, evidence))


def run(args, evidence):
    binary = args.binary.resolve()
    server_binary = ROOT / 'build/server/cyberworld-server.pyz'
    if not binary.is_file() or not server_binary.is_file():
        raise RuntimeError('Build the native game and server before running this proof')
    template = evidence / 'template'
    saved = initial_checkpoint(binary, args.rom_dir, template, 120, evidence / 'checkpoint.log')
    names = [f'Navi{i + 1:02d}' for i in range(args.clients)]
    for name in names:
        shutil.copytree(template, evidence / name)
    config = evidence / 'server.ini'
    capacity = min(8, args.clients)
    config.write_text(f'[server]\ncapacity = {capacity}\nplayer_cap = {args.clients}\n')
    events = evidence / 'server.jsonl'
    with contextlib.ExitStack() as stack:
        log = stack.enter_context((evidence / 'server.log').open('w'))
        server = subprocess.Popen([sys.executable, str(server_binary), '--host', '127.0.0.1',
                                   '--port', '0', '--seed', '120', '--config', str(config),
                                   '--state-dir', str(evidence / 'server-data'), '--event-log', str(events)],
                                  cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
        stack.callback(stop, server)
        listening = wait_for(lambda: next((row for row in records(events) if row['event'] == 'listening'), None),
                             10, 'Server did not start; see server.log')
        url = f'ws://127.0.0.1:{listening["port"]}/presence'
        clients = {}
        script = '180:,30:UP,45:,30:DOWN,45:,24:LEFT,45:,24:RIGHT,1400:'
        for name in names:
            log = stack.enter_context((evidence / f'{name}.log').open('w'))
            env = dict(os.environ, CYBERWORLD_ONLINE_TRACE=str(evidence / f'{name}.jsonl'))
            command = [str(binary), '--headless', '--rom-dir', str(args.rom_dir),
                       '--data-dir', str(evidence / name), '--online', url, '--online-name', name,
                       '--frames', '1800', '--input', script]
            clients[name] = subprocess.Popen(command, cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT)
            stack.callback(stop, clients[name])
        print(f'{len(clients)} native games running through {url}; evidence: {evidence}', flush=True)
        wait_for(lambda: len([row for row in records(events) if row['event'] == 'join']) == args.clients,
                 30, 'The server did not accept every native game')
        wait_for(lambda: ready(evidence, names), 60, 'Some native games did not reach 540 online frames')
        # Every game has finished its movement before connection loss, so
        # remaining scripted buttons cannot resume offline play after return.
        stop(server)
        for name, process in clients.items():
            process.wait(timeout=40)
            if process.returncode:
                raise RuntimeError(f'{name} exited unsuccessfully; see its log')
    joined = [row for row in records(events) if row['event'] == 'join']
    if len(joined) != args.clients:
        raise RuntimeError('The server accepted an unexpected number of game connections')
    channels = {}
    for row in joined:
        channels[row['channel']] = channels.get(row['channel'], 0) + 1
    report = dict(result='passed', native_processes=args.clients, channel_players=channels, clients={})
    worlds = set()
    for name in names:
        rows = records(evidence / f'{name}.jsonl')
        joined_client = next(row for row in joined if row['name'] == name)
        report['clients'][name] = validate(name, rows, saved, evidence, channels[joined_client['channel']] - 1)
        worlds.update((row['seed'], row['world_hash']) for row in rows)
    if len(worlds) != 1:
        raise RuntimeError('Native games generated different shared worlds')
    report['world_seed'], report['world_hash'] = next(iter(worlds))
    (evidence / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, default=ROOT / 'build/linux/cyberworld')
    parser.add_argument('--rom-dir', type=Path,
                        default=Path(os.environ.get('CYBERWORLD_ROM_DIR', '~/.cache/mmbn-ref/roms')).expanduser())
    parser.add_argument('--clients', type=int, choices=range(2, 17), default=16)
    args = parser.parse_args()
    parent = ROOT / '.build/online-load-proof'
    parent.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    try:
        run(args, evidence)
    except (RuntimeError, OSError, subprocess.SubprocessError) as exc:
        (evidence / 'failure.txt').write_text(str(exc) + '\n')
        parser.exit(1, f'{exc}\nEvidence retained in {evidence}\n')


if __name__ == '__main__':
    main()
