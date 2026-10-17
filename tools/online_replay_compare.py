"""Guarded comparator for complete local cable snapshots (Arena unit 2).

This file defines acceptance; restore code must not edit it to obtain a pass.
No ROM, raw state, flash content or process address belongs in its evidence.
"""
import json
from pathlib import Path
import re

from tools.online_frame_compare import Mismatch, compare_game

TOTAL = 2822
ENTRY = 958
HORIZONS = (1, 4, 8)
POINTS = ('custom', 'held-charge', 'before-release', 'after-damage', 'nonidle-cable', 'flash-change')
MUTATIONS = ('omit-cable', 'omit-flash', 'credit', 'receive-word', 'event-delay')
SHA256 = re.compile(r'[0-9a-f]{64}')
TARGET = {
    'rom_sha256': '572e113eeb53bb29cd9ff8acb9db265cfd48c5e509c8d0e6420b58e71e442cf2',
    'boot_sha256': '4b921c9cf48c22085605714797373114de206a32b8bba821606dc348f73f95b6',
    'linked_library_sha256': '5b02195c77b74ffa6622952706025bc7e5a77bd5d14d9306e0bbc84ab789f0cd',
}


class Unreached(Mismatch):
    """The original did not exercise a required snapshot condition."""


def require(condition, message):
    if not condition:
        raise Mismatch(message)


def integer(value, low=0, high=0xffffffff):
    return type(value) is int and low <= value <= high


def members(value, names, label):
    require(type(value) is dict and set(value) == set(names.split()), f'{label}: wrong fields')


def words(value, count, high=0xffffffff):
    return type(value) is list and len(value) == count and all(integer(v, high=high) for v in value)


def validate_blob(blob, state):
    members(blob, 'bytes sha256', 'component')
    require(integer(blob['bytes'], 1 if state else 0, 16 * 1024 * 1024), 'component: invalid byte length')
    require(type(blob['sha256']) is str and SHA256.fullmatch(blob['sha256']), 'component: invalid SHA-256')


def validate_cable(cable):
    members(cable, 'attached phase cycles attached_multi attached_normal waiting transfers running multi_recv normal_recv nodes', 'cable')
    require(cable['attached'] == 2 and type(cable['attached']) is int, 'cable: not two cores')
    require(integer(cable['phase'], high=4), 'cable: invalid phase')
    require(integer(cable['cycles'], -0x80000000, 0x7fffffff), 'cable: invalid cycles')
    for field in ('attached_multi', 'attached_normal', 'waiting', 'transfers', 'running'):
        require(integer(cable[field]), f'cable: invalid {field}')
    require(cable['running'] < 2 and cable['waiting'] < 4, 'cable: invalid scheduler owner/mask')
    require(words(cable['multi_recv'], 4, 65535) and words(cable['normal_recv'], 4), 'cable: invalid receive words')
    require(type(cable['nodes']) is list and len(cable['nodes']) == 2, 'cable: missing node')
    for node in cable['nodes']:
        members(node, 'credit next_event event_diff mode normal_so transfer_finished siocnt rcnt scheduled event_delay event_priority', 'node')
        for name in ('credit', 'next_event', 'event_diff', 'event_delay', 'event_priority'):
            require(integer(node[name], -0x80000000, 0x7fffffff), f'node: invalid {name}')
        for name in ('mode', 'siocnt', 'rcnt'):
            require(integer(node[name]), f'node: invalid {name}')
        for name in ('normal_so', 'transfer_finished', 'scheduled'):
            require(type(node[name]) is bool, f'node: invalid {name}')


def validate_record(row):
    members(row, 'frame hash_algorithm cores cable pair samples inputs', 'record')
    require(integer(row['frame'], high=TOTAL), 'record: invalid frame')
    require(row['hash_algorithm'] == 'sha256', 'record: wrong hash algorithm')
    require(type(row['cores']) is list and len(row['cores']) == 2, 'record: missing core')
    for core in row['cores']:
        members(core, 'state flash', 'core')
        validate_blob(core['state'], True)
        validate_blob(core['flash'], False)
    validate_cable(row['cable'])
    pair = row['pair']
    members(pair, 'frame start_frames traced_frames entry_step entry_frame', 'pair')
    require(pair['frame'] == row['frame'] and type(pair['frame']) is int, 'pair: frame mismatch')
    for name in ('start_frames', 'traced_frames'):
        require(words(pair[name], 2), f'pair: invalid {name}')
    for name in ('entry_step', 'entry_frame'):
        require(integer(pair[name]), f'pair: invalid {name}')
    require(type(row['samples']) is list and len(row['samples']) == 2 and
            all(words(sample, 8) for sample in row['samples']), 'record: malformed game samples')
    require(type(row['inputs']) is list and len(row['inputs']) == 4 and
            all(words(pair, 2, 1023) for pair in row['inputs']), 'record: invalid next inputs')


def read_trace(path, first=0, last=TOTAL):
    try:
        rows = [json.loads(line) for line in Path(path).read_text().splitlines()]
    except (OSError, ValueError) as error:
        raise Mismatch(f'Cannot read complete trace: {path}') from error
    require(len(rows) == last - first + 1, f'{path}: incomplete trace')
    for frame, row in enumerate(rows, first):
        validate_record(row)
        require(row['frame'] == frame, f'{path}: non-contiguous frame {frame}')
    return rows


def verify_manifest(manifest, reviewed_sources):
    """Caller pins the separately reviewed observer/comparator before subjects."""
    members(manifest, 'before after source_hashes binary_hashes', 'manifest')
    require(manifest['before'] == TARGET and manifest['after'] == TARGET, 'Target hashes changed or wrong build')
    require(type(reviewed_sources) is dict and reviewed_sources, 'No reviewed observer source pins')
    for digest in reviewed_sources.values():
        require(type(digest) is str and SHA256.fullmatch(digest), 'Malformed reviewed pin')
    require(manifest['source_hashes'] == reviewed_sources, 'Reviewed source changed')
    members(manifest['binary_hashes'], 'oracle subject', 'binaries')
    for digest in manifest['binary_hashes'].values():
        require(type(digest) is str and SHA256.fullmatch(digest), 'Malformed binary hash')


def active(row):
    return all(s[2] > 0 and s[7] & 65535 == 0x0c04 for s in row['samples'])


def read_cable(path, original, exercised=True):
    try:
        record = json.loads(Path(path).read_text())
    except FileNotFoundError as error:
        raise Unreached('No nonidle instruction-boundary observation') from error
    except (OSError, ValueError) as error:
        raise Mismatch('Malformed instruction-boundary observation') from error
    members(record, 'cursor state', 'instruction boundary')
    row, cursor = record['state'], record['cursor']
    validate_record(row)
    members(cursor, 'stage frame round targets relative battle keys', 'executor cursor')
    require(cursor['stage'] == 'check-targets', 'Wrong suspended executor stage')
    require(integer(cursor['frame']) and cursor['frame'] == row['frame'] and row['frame'] <= TOTAL - max(HORIZONS),
            'Wrong suspended executor frame')
    require(integer(cursor['round'], 1, 4000000), 'Wrong suspended executor round')
    battle = row['frame'] >= ENTRY
    relative = row['frame'] - ENTRY if battle else row['pair']['entry_frame']
    require(type(cursor['battle']) is bool and cursor['battle'] == battle and
            integer(cursor['relative']) and cursor['relative'] == relative, 'Wrong suspended script position')
    require(words(cursor['targets'], 2) and cursor['targets'] == [f + relative + 1 for f in row['pair']['start_frames']],
            'Wrong suspended frame targets')
    require(cursor['keys'] == row['inputs'] == original[row['frame']]['inputs'], 'Wrong suspended input lookahead')
    baseline_pair = original[row['frame']]['pair']
    require(all(row['pair'][key] == baseline_pair[key] for key in ('frame', 'start_frames', 'entry_step', 'entry_frame')),
            'Wrong suspended pair bookkeeping')
    require(all(old <= current <= sample[5] for old, current, sample in
                zip(baseline_pair['traced_frames'], row['pair']['traced_frames'], row['samples'])),
            'Wrong suspended per-core observation cursor')
    if exercised and not (active(row) and row['cable']['phase'] and any(n['scheduled'] for n in row['cable']['nodes'])):
        raise Unreached('No active nonidle scheduled cable transfer at instruction boundary')
    return record


def select_points(rows, cable=None):
    """Choose from original evidence before subject execution, never its report."""
    require(len(rows) == TOTAL + 1, 'Original trace is incomplete')
    found = {}
    for frame in range(1, TOTAL - max(HORIZONS) + 1):
        row, previous, following = rows[frame], rows[frame - 1], rows[frame + 1]
        candidates = {
            'custom': all((sample[7] >> 8) & 255 == 8 for sample in row['samples']),
            'held-charge': active(row) and previous['inputs'][0][0] & 2 and row['inputs'][0][0] & 2,
            'before-release': active(row) and previous['inputs'][0][0] & 2 and not row['inputs'][0][0] & 2,
            'after-damage': active(row) and all((p[3] >> 16) > (s[3] >> 16) for p, s in zip(previous['samples'], row['samples'])),
            'nonidle-cable': active(row) and row['cable']['phase'] != 0 and any(n['scheduled'] for n in row['cable']['nodes']),
            'flash-change': any(c['flash'] != n['flash'] for c, n in zip(row['cores'], following['cores'])),
        }
        for name, exercised in candidates.items():
            if exercised and name not in found:
                found[name] = frame
    missing = set(POINTS) - set(found)
    if cable is not None:
        found['nonidle-cable'] = cable['state']['frame']
        missing.discard('nonidle-cable')
    if missing:
        raise Unreached('Original did not exercise: ' + ', '.join(sorted(missing)))
    return found


def compare_rows(reference, subject, label):
    require(len(reference) == len(subject), f'{label}: wrong trace length')
    for expected, actual in zip(reference, subject):
        require(expected == actual, f'{label}: first divergence at frame {expected["frame"]}')


def accept_observer(original_csv, observer_csv, first, second, cable_first=None, cable_second=None, baseline=None):
    logical = compare_game(original_csv, observer_csv)
    a, b = read_trace(first), read_trace(second)
    compare_rows(a, b, 'original control')
    cable = read_cable(cable_first, a) if cable_first is not None else None
    if cable is not None:
        require(baseline is not None and cable_second is not None, 'Micro observer needs the controlled original full trace')
        compare_rows(read_trace(baseline), a, 'original versus instruction executor')
        require(cable == read_cable(cable_second, b), 'Original instruction boundary does not repeat')
    return dict(verdict='PASS', original_active_timers=logical, full_records=len(a), points=select_points(a, cable))


def accept_replays(original, noop, directory, cable_path=None):
    rows = read_trace(original)
    compare_rows(rows, read_trace(noop), 'snapshot noop')
    cable = read_cable(cable_path, rows) if cable_path is not None else None
    points, accepted = select_points(rows, cable), []
    for name, frame in points.items():
        for horizon in HORIZONS:
            for repeat in (1, 2):
                label = f'{name}-{horizon}-{repeat}'
                first = frame
                if name == 'nonidle-cable' and cable is not None:
                    immediate = read_cable(Path(directory) / f'{label}-immediate.json', rows, exercised=False)
                    require(immediate == cable, f'{label}: immediate instruction-boundary state/cursor differs')
                    first += 1
                replay = read_trace(Path(directory) / f'{label}.jsonl', first, frame + horizon)
                # The first record is immediately after restore, before any CPU step.
                compare_rows(rows[first:frame + horizon + 1], replay, label)
                accepted.append(label)
    return dict(verdict='PASS', points=points, replay_cases=accepted, masks=[])


def accept_mutation(original, mutation, path, cable_path=None, immediate_path=None):
    require(mutation in MUTATIONS, 'Unknown replay mutation')
    rows = read_trace(original)
    point = 'flash-change' if mutation == 'omit-flash' else 'nonidle-cable'
    cable = read_cable(cable_path, rows) if cable_path is not None else None
    frame = select_points(rows, cable)[point]
    if point == 'nonidle-cable' and cable is not None:
        immediate = read_cable(immediate_path, rows, exercised=False)
        require(immediate['cursor'] == cable['cursor'], 'Mutation changed suspended executor cursor')
        changed = [immediate['state']] + read_trace(path, frame + 1, frame + max(HORIZONS))
        expected = [cable['state']] + rows[frame + 1:frame + max(HORIZONS) + 1]
    else:
        changed = read_trace(path, frame, frame + max(HORIZONS))
        expected = rows[frame:frame + max(HORIZONS) + 1]
    differences = [a['frame'] for a, b in zip(expected, changed) if a != b]
    require(differences, f'{mutation}: mutation did not produce a divergence')
    # A changed input, frame or sample-only fabrication is not a state-restoration mutation.
    for original_row, actual in zip(expected, changed):
        require(original_row['inputs'] == actual['inputs'], 'Mutation changed the controller script')
    before, after = expected[0]['cable'], changed[0]['cable']
    component_changed = {
        'omit-cable': before != after,
        'omit-flash': any(a['flash'] != b['flash'] for a, b in zip(rows[frame]['cores'], changed[0]['cores'])),
        'credit': any(a['credit'] != b['credit'] for a, b in zip(before['nodes'], after['nodes'])),
        'receive-word': before['multi_recv'] != after['multi_recv'] or before['normal_recv'] != after['normal_recv'],
        'event-delay': any(a['scheduled'] and b['scheduled'] and a['event_delay'] != b['event_delay']
                           for a, b in zip(before['nodes'], after['nodes'])),
    }
    require(component_changed[mutation], f'{mutation}: claimed component did not change immediately after restore')
    if mutation == 'omit-flash':
        require(any(a['flash'] != b['flash'] for a, b in zip(rows[frame]['cores'], rows[frame + 1]['cores'])),
                'Flash mutation did not cross a native flash change')
        require(any(a['flash'] != b['flash'] for a, b in zip(rows[frame]['cores'], changed[0]['cores'])),
                'Flash mutation did not leave changed flash after restore')
    return dict(verdict='PASS', mutation=mutation, expected_failure=True, first_divergent_frame=differences[0])
