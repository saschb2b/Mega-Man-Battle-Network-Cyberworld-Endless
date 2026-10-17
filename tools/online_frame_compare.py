"""Independent acceptance oracle for the starter Practice frame experiment.

Change only in a reviewed oracle commit, separate from the worker/relay.
"""
import csv
import json
from pathlib import Path
import re

FRAME_COUNT = 2822
ACTIVE_PHASE = 0x0C04
LOGICAL_FIELDS = ('hp0', 'hp1', 'positions')
FINAL_FIELDS = ('battle_timer', 'result', 'hp0', 'hp1', 'positions', 'menu', 'battle_state')
HASH_ALGORITHM = 'fnv1a64-core-savedata-cable-v1'


class Mismatch(RuntimeError):
    """A concrete acceptance difference; no successful partial traces."""


def read_trace(path):
    active, final = [{}, {}], [None, None]
    with Path(path).open() as stream:
        for raw in csv.DictReader(stream):
            row = {key: int(value) for key, value in raw.items()}
            side, timer = row['core'], row['battle_timer']
            if side not in (0, 1):
                raise Mismatch('Unknown core in game trace')
            final[side] = tuple(row[key] for key in FINAL_FIELDS)
            if timer and row['battle_state'] & 0xFFFF == ACTIVE_PHASE:
                active[side][timer] = tuple(row[key] for key in LOGICAL_FIELDS)
    for side in (0, 1):
        timers = sorted(active[side])
        if not timers or timers != list(range(1, timers[-1] + 1)):
            raise Mismatch(f'Core {side}: missing active battle timers')
        if final[side] is None or final[side][1] != side + 1:
            raise Mismatch(f'Core {side}: incomplete Practice result')
    if active[0] != active[1]:
        raise Mismatch('Game cores disagree on HP or panel positions')
    return active, final


def compare_game(reference, subject):
    expected, expected_final = read_trace(reference)
    actual, actual_final = read_trace(subject)
    for side in (0, 1):
        if expected[side].keys() != actual[side].keys():
            raise Mismatch(f'Core {side}: active timer coverage differs from the original')
        for timer, observation in expected[side].items():
            if actual[side][timer] != observation:
                raise Mismatch(f'Core {side}: original differs at battle timer {timer}')
        if expected_final[side] != actual_final[side]:
            raise Mismatch(f'Core {side}: final result/menu differs from the original')
    return len(expected[0])


def compare_replicas(path, frames=FRAME_COUNT):
    records = [{}, {}]
    with Path(path).open() as stream:
        for line in stream:
            row = json.loads(line)
            worker, frame = row['worker'], row['frame']
            if worker not in (0, 1) or frame in records[worker]:
                raise Mismatch('Invalid or duplicate replica frame')
            if row.get('hash_algorithm') != HASH_ALGORITHM or not isinstance(row.get('hash'), str) or not re.fullmatch('[0-9a-f]{16}', row['hash']):
                raise Mismatch('Invalid full-state hash representation')
            samples = row.get('samples')
            if not isinstance(samples, list) or len(samples) != 2 or any(
                    not isinstance(sample, list) or len(sample) != 8 or any(
                        type(value) is not int or not 0 <= value <= 0xFFFFFFFF for value in sample)
                    for sample in samples):
                raise Mismatch('Malformed replica observations')
            records[worker][frame] = (row['hash'], row['samples'])
    expected = set(range(frames))
    if any(set(rows) != expected for rows in records):
        raise Mismatch('Replica trace is incomplete')
    for frame in range(frames):
        if records[0][frame] != records[1][frame]:
            raise Mismatch(f'Replicas first differ at simulation frame {frame}')
    return frames


def accept_case(reference, work, name, delay_ms):
    work = Path(work)
    report = json.loads((work / f'{name}-transport.json').read_text())
    if report.get('status') != 'complete' or report.get('exit_codes') != [0, 0]:
        raise Mismatch(f'{name}: incomplete workers')
    pids = report.get('pids', [])
    if len(pids) != 2 or any(type(pid) is not int or pid <= 0 for pid in pids) or len(set(pids)) != 2:
        raise Mismatch(f'{name}: independent workers missing')
    if report.get('delivered_inputs') != FRAME_COUNT * 2:
        raise Mismatch(f'{name}: incomplete input stream')
    minimum = report.get('minimum_delivery_ms')
    if report.get('delay_ms') != delay_ms or type(minimum) not in (int, float) or not delay_ms <= minimum < 5000:
        raise Mismatch(f'{name}: configured delivery delay not measured')
    frames = compare_replicas(work / f'{name}-replicas.jsonl')
    active = [compare_game(reference, work / f'{name}-worker{side}.csv') for side in (0, 1)]
    return dict(case=name, verdict='PASS', replica_frames=frames,
                active_frames_compared=active, delay_ms=delay_ms)
