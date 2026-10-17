"""Independent acceptance oracle for the starter Practice frame experiment.

Change only in a reviewed oracle commit, separate from the worker/relay.
"""
import csv
import json
import math
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


def read_replicas(path):
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
    for side in (0, 1):
        if set(records[side]) != set(range(len(records[side]))):
            raise Mismatch('Replica trace has a missing frame')
    return records


def compare_replicas(path, frames=FRAME_COUNT):
    records = read_replicas(path)
    expected = set(range(frames))
    if any(set(rows) != expected for rows in records):
        raise Mismatch('Replica trace is incomplete')
    for frame in range(frames):
        if records[0][frame] != records[1][frame]:
            raise Mismatch(f'Replicas first differ at simulation frame {frame}')
    return frames


def read_inputs(path):
    records = [[], []]
    with Path(path).open() as stream:
        for line in stream:
            row = json.loads(line)
            owner, frame = row.get('owner'), row.get('frame')
            if type(owner) is not int or owner not in (0, 1) or type(frame) is not int or frame != len(records[owner]):
                raise Mismatch('Delivered inputs are not unique contiguous owner prefixes')
            if any(type(row.get(key)) is not int or not 0 <= row[key] <= 0x3FF
                   for key in ('source_keys', 'delivered_keys')):
                raise Mismatch('Invalid delivered controller word')
            delay = row.get('delivery_ms')
            if type(delay) not in (int, float) or not math.isfinite(delay) or not 0 <= delay < 5000:
                raise Mismatch('Missing or invalid measured input delay')
            records[owner].append(row)
    return records


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
    inputs = read_inputs(work / f'{name}-inputs.jsonl')
    if any(len(rows) != FRAME_COUNT for rows in inputs):
        raise Mismatch(f'{name}: delivered input evidence is incomplete')
    if any(row['source_keys'] != row['delivered_keys'] for rows in inputs for row in rows):
        raise Mismatch(f'{name}: unrequested controller mutation')
    measured_minimum = min(row['delivery_ms'] for rows in inputs for row in rows)
    minimum = report.get('minimum_delivery_ms')
    if report.get('delay_ms') != delay_ms or type(minimum) not in (int, float) or minimum != measured_minimum or not delay_ms <= minimum < 5000:
        raise Mismatch(f'{name}: configured delivery delay not measured')
    frames = compare_replicas(work / f'{name}-replicas.jsonl')
    active = [compare_game(reference, work / f'{name}-worker{side}.csv') for side in (0, 1)]
    return dict(case=name, verdict='PASS', replica_frames=frames,
                active_frames_compared=active, delay_ms=delay_ms)


def accept_fault(work, name):
    """Fault acceptance requires the recorded input mutation/OS exit and trace."""
    work = Path(work)
    report = json.loads((work / f'{name}-transport.json').read_text())
    expected = {'corruption': 'desync', 'disconnect': 'disconnected'}[name]
    elapsed = report.get('fault_detection_ms')
    if report.get('status') != expected or report.get('fault_injected') is not True or type(elapsed) not in (int, float) or not 0 <= elapsed < 5000:
        raise Mismatch(f'{name}: missing bounded fault detection')
    pids = report.get('pids', [])
    if len(pids) != 2 or any(type(pid) is not int or pid <= 0 for pid in pids) or len(set(pids)) != 2:
        raise Mismatch(f'{name}: independent workers missing')
    records = read_replicas(work / f'{name}-replicas.jsonl')
    common = sorted(set(records[0]) & set(records[1]))
    if len(common) < 1400 or max(map(len, records)) >= FRAME_COUNT:
        raise Mismatch(f'{name}: fault did not interrupt active play')
    differences = [frame for frame in common if records[0][frame] != records[1][frame]]
    deliveries = read_inputs(work / f'{name}-inputs.jsonl')
    if any(len(deliveries[side]) < len(records[side ^ 1]) for side in (0, 1)):
        raise Mismatch('Delivered input evidence does not cover consumed replica frames')
    if any(not (sample[2] > 0 and sample[7] & 0xFFFF == ACTIVE_PHASE)
           for side in (0, 1) for sample in records[side][common[-1]][1]):
        raise Mismatch('Fault evidence does not reach active battle play')
    changed = [row for rows in deliveries for row in rows if row['source_keys'] != row['delivered_keys']]
    if name == 'corruption':
        if len(changed) != 1 or changed[0]['owner'] != 0 or changed[0]['frame'] != 1400 or changed[0]['delivered_keys'] != changed[0]['source_keys'] ^ 16:
            raise Mismatch('The promised controller corruption was not delivered')
        if not differences or differences[0] < 1400 or differences[0] != report.get('first_divergent_frame'):
            raise Mismatch('Corruption did not produce the reported first replica difference')
    else:
        if differences or changed or report.get('exit_codes') != [0, -9]:
            raise Mismatch('Peer loss must be an actual killed worker without a preceding desync')
    return dict(case=name, verdict='PASS', expected_failure=expected,
                first_divergent_frame=differences[0] if differences else None, detection_ms=elapsed)
