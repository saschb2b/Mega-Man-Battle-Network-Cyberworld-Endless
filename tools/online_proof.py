#!/usr/bin/env python3
"""Run two real game processes through the built server, keeping evidence locally.

Requires the player's BN6 ROM and the Linux game build. No ROM or save is
copied into a release. Each proof owns a new directory under .build/.
"""
import argparse
import contextlib
import hashlib
import json
import os
import statistics
from pathlib import Path
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]


def records(path):
    if not path.exists():
        return []
    result = []
    for line in path.read_text().splitlines():
        try:
            result.append(json.loads(line))
        except json.JSONDecodeError:
            # The last line may still be in flight while the server starts.
            continue
    return result


def hashes(path):
    return {str(p.relative_to(path)): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in sorted(path.rglob('*')) if p.is_file()}


def stop(process):
    if process.poll() is not None:
        return
    process.terminate()
    try:
        process.wait(timeout=10)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait(timeout=5)


def wait_for(predicate, timeout, explanation):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        value = predicate()
        if value:
            return value
        time.sleep(0.05)
    raise RuntimeError(explanation)


def initial_checkpoint(binary, roms, directory, seed, log):
    directory.mkdir()
    command = [str(binary), '--headless', '--rom-dir', str(roms), '--data-dir', str(directory),
               '--scene', 'emu', '--seed', str(seed), '--run-depth', '3', '--dev', 'quiet',
               '--frames', '1200', '--input', '180:,' + '4:A,8:,' * 75 + '120:']
    with log.open('w') as output:
        subprocess.run(command, cwd=ROOT, stdout=output, stderr=subprocess.STDOUT,
                       check=True, timeout=120)
    saved = hashes(directory / 'savedata')
    if not {'run.sav', 'run.state'} <= saved.keys():
        raise RuntimeError(f'No real checkpoint was created; see {log}')
    return saved


def position(record):
    return tuple(record[k] for k in ('x', 'y', 'z'))


def validate_client(name, rows, other, original, directory):
    connected = [row for row in rows if row['connected'] and row['peers']]
    if len(connected) < 10:
        raise RuntimeError(f'{name}: fewer than ten samples with a connected peer')
    local = {position(row['local']) for row in connected}
    remote = {position(peer) for row in connected for peer in row['peers']}
    real_npcs = {tuple(peer[k] for k in ('npc_x', 'npc_y', 'npc_z'))
                 for row in connected for peer in row['peers'] if peer['npc_match']}
    other_local = {position(row['local']) for row in other}
    if min(len(local), len(remote), len(real_npcs)) < 3:
        raise RuntimeError(f'{name}: local, remote and real NPC motion were not all observed')
    if len(remote & other_local) < 2 or len(real_npcs & other_local) < 2:
        raise RuntimeError(f'{name}: NPC positions do not match the other game process')
    if hashes(directory / 'savedata') != original:
        raise RuntimeError(f'{name}: visiting the Square changed checkpoint files')
    latency = [row['latency_ms'] for row in connected if row.get('latency_ms', 0)]
    return dict(samples=len(rows), local_positions=len(local), peer_positions=len(remote),
                real_npc_positions=len(real_npcs), matched_other_positions=len(real_npcs & other_local),
                checkpoint_files_unchanged=len(original),
                round_trip_median_ms=statistics.median(latency) if latency else None,
                round_trip_max_ms=max(latency, default=None))


def run(args, evidence):
    binary = args.binary.resolve()
    server_binary = ROOT / 'build/server/cyberworld-server.pyz'
    if not binary.is_file() or not server_binary.is_file():
        raise RuntimeError('Build first with python3 build.py online-proof')
    saved = {}
    for index, name in enumerate(('Alpha', 'Beta')):
        saved[name] = initial_checkpoint(binary, args.rom_dir, evidence / name,
                                         120 + index, evidence / f'{name}-checkpoint.log')
    event_file = evidence / 'server.jsonl'
    with contextlib.ExitStack() as stack:
        output = stack.enter_context((evidence / 'server.log').open('w'))
        server = subprocess.Popen([sys.executable, str(server_binary), '--host', '127.0.0.1',
                                   '--port', '0', '--seed', str(args.seed), '--event-log', str(event_file)],
                                  cwd=ROOT, stdout=output, stderr=subprocess.STDOUT)
        stack.callback(stop, server)
        listening = wait_for(lambda: next((row for row in records(event_file)
                                           if row.get('event') == 'listening'), None),
                             10, f'Server did not start; see {evidence / "server.log"}')
        url = f'ws://127.0.0.1:{listening["port"]}/presence'
        clients = {}
        scripts = {'Alpha': '180:,30:UP,45:,30:DOWN,45:,24:LEFT,45:,24:RIGHT,220:',
                   'Beta': '210:,24:RIGHT,45:,24:LEFT,45:,30:DOWN,45:,30:UP,400:'}
        for name, frames in (('Alpha', 660), ('Beta', 900)):
            output = stack.enter_context((evidence / f'{name}.log').open('w'))
            env = dict(os.environ, CYBERWORLD_ONLINE_TRACE=str(evidence / f'{name}.jsonl'))
            command = [str(binary), '--headless', '--size', '720x480', '--rom-dir', str(args.rom_dir),
                       '--data-dir', str(evidence / name), '--online', url, '--online-name', name,
                       '--frames', str(frames), '--input', scripts[name],
                       '--shot', f'480:{evidence / (name + ".bmp")}']
            clients[name] = subprocess.Popen(command, cwd=ROOT, env=env, stdout=output, stderr=subprocess.STDOUT)
            stack.callback(stop, clients[name])
        print(f'Two game instances connected through {url}; evidence: {evidence}', flush=True)
        clients['Alpha'].wait(timeout=90)
        if clients['Alpha'].returncode:
            raise RuntimeError('Alpha exited with an error; see Alpha.log')
        # A departing peer must disappear before the server itself goes away.
        wait_for(lambda: any(row['connected'] and not row['peers']
                             for row in records(evidence / 'Beta.jsonl')[-5:]),
                 5, 'The remaining game did not observe the departing peer')
        stop(server)
        clients['Beta'].wait(timeout=30)
        if clients['Beta'].returncode:
            raise RuntimeError('Beta exited with an error; see Beta.log')
    rows = {name: records(evidence / f'{name}.jsonl') for name in clients}
    report = dict(seed=args.seed, endpoint=url, clients={})
    for name, other in (('Alpha', 'Beta'), ('Beta', 'Alpha')):
        report['clients'][name] = validate_client(name, rows[name], rows[other], saved[name], evidence / name)
    worlds = {(row['seed'], row['world_hash']) for stream in rows.values() for row in stream}
    if len(worlds) != 1 or next(iter(worlds))[0] != args.seed:
        raise RuntimeError('The clients did not construct the same server-seeded world')
    if not any(not row['connected'] and not row['peers'] for row in rows['Beta']):
        raise RuntimeError('Server disconnect was not observed by the remaining client')
    events = records(event_file)
    if len([row for row in events if row['event'] == 'join']) != 2:
        raise RuntimeError('Server did not accept exactly two game clients')
    report.update(world_hash=next(iter(worlds))[1], peer_departure_observed=True,
                  server_disconnect_observed=True, result='passed')
    (evidence / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    for name in clients:
        try:
            from PIL import Image
            with Image.open(evidence / f'{name}.bmp') as picture:
                picture.save(evidence / f'{name}.png')
        except ImportError:
            pass
    print(json.dumps(report, indent=2))
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, default=ROOT / 'build/linux/cyberworld')
    parser.add_argument('--rom-dir', type=Path,
                        default=Path(os.environ.get('CYBERWORLD_ROM_DIR', '~/.cache/mmbn-ref/roms')).expanduser())
    parser.add_argument('--seed', type=int, default=120)
    args = parser.parse_args()
    parent = ROOT / '.build/online-proof'
    parent.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    try:
        run(args, evidence)
    except (RuntimeError, OSError, subprocess.SubprocessError) as exc:
        (evidence / 'failure.txt').write_text(str(exc) + '\n')
        parser.exit(1, f'{exc}\nEvidence retained in {evidence}\n')


if __name__ == '__main__':
    main()
