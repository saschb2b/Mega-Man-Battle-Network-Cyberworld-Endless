"""Reviewed oracle for the bounded native carried-Cannon experiment.

This file and its fixtures change separately from the loadout installer.
"""
import json
from pathlib import Path
import re

from tools.online_frame_compare import Mismatch, compare_replicas, read_inputs

SEED = [2622147170, 2129837214]
ENTRY_STEPS = ([(30, 0, 0), (4, 8, 8), (40, 0, 0), (4, 64, 64),
                (12, 0, 0), (4, 64, 64), (12, 0, 0), (4, 1, 1), (100, 0, 0)]
               + [(4, 1, 1), (80, 0, 0)] * 6 + [(4, 1, 1), (240, 0, 0)])
BATTLE_STEPS = ([(4, 1, 0), (12, 0, 0)] + [(4, 16, 16), (8, 0, 0)] * 5
                + [(4, 1, 1), (200, 0, 0), (4, 1, 0), (60, 0, 0)]
                + [(110, 2, 0), (30, 0, 0)] * 6 + [(300, 0, 0)])
SCRIPT = ENTRY_STEPS + BATTLE_STEPS
ENTRY_FRAMES = sum(step[0] for step in ENTRY_STEPS)
FRAME_COUNT = sum(step[0] for step in SCRIPT)
FIELDS = {'core', 'frame', 'ordinal', 'input', 'main', 'menu', 'state', 'timer',
          'result', 'side', 'objects', 'custom', 'hand', 'rng'}
OBJECT = {'id', 'hp', 'alliance', 'x', 'y', 'chip'}
CUSTOM = {'mode', 'cursor', 'size', 'window', 'picked', 'picks', 'deck'}
HAND = {'at', 'ids', 'powers', 'bonuses'}
SCALARS = ('hp', 'max_hp', 'base_hp', 'attack', 'speed', 'charge', 'regular_mb',
           'mega_limit', 'giga_limit', 'regular', 'crosses', 'souls', 'beast')
LOADOUT = {'folder', 'pack', 'board', 'grid', 'tag', 'record', *SCALARS}


class Unreached(Mismatch):
    """A native control did not exercise a required path; INCONCLUSIVE."""


def reached(condition, message):
    if not condition:
        raise Unreached(message)


def require(condition, message):
    if not condition:
        raise Mismatch(message)


def uint(value, maximum=0xFFFFFFFF):
    return type(value) is int and 0 <= value <= maximum


def words(value, size, maximum=0xFFFF):
    return isinstance(value, list) and len(value) == size and all(uint(v, maximum) for v in value)


def blob(value, size=None):
    return (isinstance(value, dict) and set(value) == {'bytes', 'sha256'}
            and uint(value['bytes']) and value['bytes'] > 0
            and (size is None or value['bytes'] == size)
            and isinstance(value['sha256'], str)
            and re.fullmatch('[0-9a-f]{64}', value['sha256']) is not None)


def key_at(frame, side):
    for count, left, right in SCRIPT:
        if frame < count:
            return (left, right)[side]
        frame -= count
    return 0


def validate_row(row):
    require(isinstance(row, dict) and set(row) == FIELDS, 'Unexpected frame fields')
    require(all(uint(row[k]) for k in FIELDS - {'objects', 'custom', 'hand', 'rng'}), 'Invalid frame integer')
    require(row['core'] in (0, 1) and row['input'] <= 0x3FF, 'Invalid core/controller')
    require(words(row['rng'], 2, 0xFFFFFFFF), 'Invalid RNG observation')
    objects = row['objects']
    require(isinstance(objects, list) and len(objects) == 2, 'Missing player objects')
    require(all(isinstance(obj, dict) and set(obj) == OBJECT
                and all(uint(v, 0xFFFF) for v in obj.values()) for obj in objects), 'Invalid player object')
    custom, hand = row['custom'], row['hand']
    require(isinstance(custom, dict) and set(custom) == CUSTOM, 'Invalid Custom fields')
    require(all(uint(custom[k], 255) for k in CUSTOM - {'picks', 'deck'})
            and words(custom['picks'], 6, 255) and words(custom['deck'], 30), 'Invalid Custom observation')
    require(isinstance(hand, dict) and set(hand) == HAND and uint(hand['at'], 0xFFFF)
            and all(words(hand[k], 6) for k in ('ids', 'powers', 'bonuses')), 'Invalid hand observation')


def ready(row):
    c = row['custom']
    return (row['state'] & 0xFFFF == 0x0804 and c['mode'] == 4
            and c['window'] == 120 and 1 <= c['size'] <= 10 and c['cursor'] == 0 and c['picked'] == 0)


def read_trace(path):
    cores = [[], []]
    with Path(path).open() as stream:
        for line in stream:
            try:
                row = json.loads(line)
                validate_row(row)
            except (ValueError, TypeError, KeyError) as error:
                raise Mismatch(f'Malformed frame: {error}') from error
            rows = cores[row['core']]
            require(row['ordinal'] == len(rows), 'Missing/duplicate/out-of-order core frame')
            require(not rows or row['frame'] == rows[-1]['frame'] + 1, 'Noncontiguous native frame clock')
            rows.append(row)
    require(all(cores), 'Empty player trace')
    return cores


def aligned(rows):
    first = next((i for i, row in enumerate(rows) if ready(row)), None)
    reached(first is not None and first < ENTRY_FRAMES, 'Ready Custom state was not reached within entry')
    return [{k: v for k, v in row.items() if k not in ('frame', 'ordinal')}
            for row in rows[first:]]


def chip_evidence(cores):
    """Observe the native transition; agreement alone cannot establish use."""
    left = aligned(cores[0])
    cutoff = next((r['timer'] for r in left if r['input'] & 2 and r['state'] & 0xFFFF == 0x0C04), None)
    reached(cutoff is not None and cutoff > 0, 'No active buster input boundary')
    evidence = []
    for side, raw in enumerate(cores):
        rows = aligned(raw)
        selection = next((i for i, r in enumerate(rows) if r['custom']['picked'] == 1
                          and r['custom']['picks'][0] == 0 and r['custom']['deck'][0] == 1), None)
        if side == 0:
            reached(selection is not None, 'Cannon A slot zero was not selected')
        else:
            require(all(r['custom']['picked'] == 0 for r in rows), 'Passive peer selected a chip')
        hand = next((i for i, r in enumerate(rows) if r['hand']['at'] == 0
                     and r['hand']['ids'][0] == 1 and r['hand']['powers'][0] == 40
                     and r['hand']['bonuses'][0] == 0), None)
        used = next((i for i, r in enumerate(rows) if hand is not None and i > hand
                     and r['hand']['at'] == 1 and r['hand']['ids'][0] == 1
                     and r['state'] & 0xFFFF == 0x0C04 and r['timer'] > 0), None)
        reached(hand is not None and used is not None, 'Native hand did not consume Cannon 40')
        require(side != 0 or selection < hand, 'Hand transfer preceded selection')
        buster = next((i for i, r in enumerate(rows) if r['input'] & 2), len(rows))
        damage = next((i for i, r in enumerate(rows) if used <= i < buster and r['timer'] < cutoff
                       and any(o['alliance'] == 1 and o['hp'] == 60 for o in r['objects'])
                       and any(o['alliance'] == 0 and o['hp'] == 100 for o in r['objects'])), None)
        require(damage is not None, 'No 40-point Cannon damage before buster input')
        require(any(any(o['alliance'] == 0 and o['chip'] == 1 for o in r['objects'])
                    for r in rows[hand:damage + 1]), 'Native player object never held Cannon')
        require(all(any(o['alliance'] == 1 and o['hp'] == 100 for o in r['objects'])
                    for r in rows[hand:used]), 'HP changed before chip consumption')
        final = rows[-1]
        reached(final['result'] != 0, 'Native Practice result was not reached')
        require(final['result'] == side + 1 and final['timer'] > 0
                and final['main'] == 0x28 and final['menu'] & 255 == 0x18,
                'Practice did not finish in original Comm result')
        require(any(o['alliance'] == 1 and o['hp'] == 0 for o in final['objects'])
                and any(o['alliance'] == 0 and o['hp'] == 100 for o in final['objects']), 'Wrong final HP')
        evidence.append(dict(core=side, selection=selection, hand=hand, consumption=used,
                             damage=damage, compared_frames=len(rows), final_timer=final['timer']))
    require(evidence[0]['final_timer'] == evidence[1]['final_timer'], 'Original peers have different outcomes')
    return evidence


def compare_game(reference, subject):
    expected, actual = read_trace(reference), read_trace(subject)
    evidence = chip_evidence(expected)
    try:
        chip_evidence(actual)
    except Unreached as error:
        raise Mismatch(f'Subject missed an exercised native path: {error}') from error
    for side in (0, 1):
        left, right = aligned(expected[side]), aligned(actual[side])
        require(len(left) == len(right), f'Core {side}: incomplete native frame coverage')
        for frame, (a, b) in enumerate(zip(left, right)):
            require(a == b, f'Core {side}: first original divergence at Custom-relative frame {frame}')
    return evidence


def compare_controls(first, second, no_seed):
    a, b, c = (read_trace(p) for p in (first, second, no_seed))
    require(all(FRAME_COUNT <= len(rows) <= FRAME_COUNT + 64 for rows in a), 'Incomplete native control duration')
    require(a == b == c, 'Native repeat or same-value seed changed the original trace')
    return chip_evidence(a)


def read_loadout(path):
    record = json.loads(Path(path).read_text())
    require(isinstance(record, dict) and set(record) == LOADOUT, 'Unexpected carried fields')
    require(words(record['folder'], 30) and words(record['tag'], 2, 255), 'Malformed folder/Tag')
    require(all(uint(record[k], 0xFFFF if 'hp' in k else 255) for k in SCALARS), 'Malformed carried stats')
    require(all(blob(record[k], n) for k, n in [('pack', 6144), ('board', 392), ('grid', 49)])
            and blob(record['record'], 6664), 'Malformed complete carried-record hash')
    return record


def compare_loadout(reference, installed, starter):
    ref, subject, baseline = map(read_loadout, (reference, installed, starter))
    require(ref == subject, 'Installed carried record differs from native')
    changed = [i for i, (a, b) in enumerate(zip(ref['folder'], baseline['folder'])) if a != b]
    require(changed == [9] and ref['folder'][9] == 1 and baseline['folder'][9] == 0x0236,
            'Native fixture did not carry the bounded nondefault folder')
    return ref['record']


def compare_preserved(before, after):
    require(isinstance(before, dict) and set(before) == {'rom', 'source_state', 'carried_record'}
            and all(blob(v) for v in before.values()), 'Missing preservation evidence')
    require(before == after, 'ROM, source checkpoint or full carried record changed')


def check_seed(record, no_write=False):
    require(isinstance(record, list) and len(record) == 2, 'Missing independent map boundaries')
    for side, r in enumerate(record):
        require(isinstance(r, dict) and set(r) == {'core', 'main', 'frame', 'before', 'after', 'writes'},
                'Invalid map-boundary observation')
        require(type(r['core']) is int and r['core'] == side and type(r['main']) is int and r['main'] == 4
                and uint(r['frame']) and type(r['writes']) is int
                and words(r['before'], 2, 0xFFFFFFFF) and r['after'] == SEED
                and r['writes'] == (0 if no_write else 1), 'Wrong RNG alignment boundary')
        if no_write:
            require(r['before'] == SEED, 'No-write control did not preserve native seed')


def accept_delay(reference, work, name, delay):
    work = Path(work)
    report = json.loads((work / f'{name}-transport.json').read_text())
    require(report.get('status') == 'complete' and report.get('exit_codes') == [0, 0], 'Incomplete workers')
    pids = report.get('pids')
    require(isinstance(pids, list) and len(pids) == 2 and all(uint(p) and p > 0 for p in pids)
            and len(set(pids)) == 2, 'Missing independent workers')
    inputs = read_inputs(work / f'{name}-inputs.jsonl')
    require(report.get('delivered_inputs') == FRAME_COUNT * 2
            and all(len(rows) == FRAME_COUNT for rows in inputs), 'Incomplete delivered inputs')
    require(all(r['source_keys'] == r['delivered_keys'] == key_at(r['frame'], side)
                for side, rows in enumerate(inputs) for r in rows), 'Changed input sequence')
    minimum = min(r['delivery_ms'] for rows in inputs for r in rows)
    require(report.get('delay_ms') == delay and report.get('minimum_delivery_ms') == minimum
            and delay <= minimum < 5000, 'Missing measured one-way delay')
    compare_replicas(work / f'{name}-replicas.jsonl', FRAME_COUNT)
    evidence = [compare_game(reference, work / f'{name}-worker{side}.jsonl') for side in (0, 1)]
    return dict(verdict='PASS', delay_ms=delay, replica_frames=FRAME_COUNT, selected_chip=evidence)
