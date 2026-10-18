"""ROM-free mutation guards for the native carried-chip oracle."""
import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools.online_carried_compare import (FRAME_COUNT, SEED, Mismatch, Unreached, check_seed, chip_evidence,
    accept_delay, compare_controls, compare_game, compare_loadout, compare_preserved, key_at, read_trace)


def blob(size, value='a'):
    return dict(bytes=size, sha256=value * 64)


def loadout(native=True):
    fields = dict(hp=100, max_hp=100, base_hp=100, attack=1, speed=1, charge=1,
                  regular_mb=4, mega_limit=5, giga_limit=1, regular=255, crosses=0, souls=0, beast=0)
    folder = [1] * 30
    folder[9] = 1 if native else 0x0236
    return dict(folder=folder, pack=blob(6144), board=blob(392), grid=blob(49), tag=[255, 255],
                record=blob(6664), **fields)


def fixture():
    rows = []
    for frame in range(FRAME_COUNT):
        for side in (0, 1):
            r = dict(core=side, frame=1528 + frame, ordinal=frame, input=0, main=0x28, menu=0x18,
                     state=0x0804, timer=0, result=0, side=side,
                     objects=[dict(id=416, hp=100, alliance=1, x=5, y=2, chip=65535),
                              dict(id=416, hp=100, alliance=0, x=2, y=2, chip=65535)],
                     custom=dict(mode=4, cursor=0, size=5, window=120, picked=0, picks=[0]*6, deck=[1]*30),
                     hand=dict(at=0, ids=[65535]*6, powers=[0]*6, bonuses=[0]*6), rng=SEED.copy())
            if frame >= 1 and side == 0:
                r['custom']['picked'] = 1
            if frame >= 2:
                r['hand']['ids'][0], r['hand']['powers'][0] = 1, 40
                r['objects'][1]['chip'] = 1
            if frame >= 3:
                r['state'], r['timer'] = 0x0C04, frame - 2
            if frame >= 4:
                r['hand']['at'], r['objects'][0]['hp'] = 1, 60
            if frame >= 6 and side == 0:
                r['input'] = 2
            if frame == FRAME_COUNT - 1:
                r['state'], r['result'], r['objects'][0]['hp'] = 0x0408, side + 1, 0
            rows.append(r)
    return rows


class CarriedComparatorTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.directory = Path(self.temp.name)
        self.rows = fixture()
        self.original = self.trace('original', self.rows)

    def trace(self, name, rows):
        path = self.directory / f'{name}.jsonl'
        path.write_text(''.join(json.dumps(r) + '\n' for r in rows))
        return path

    def obj(self, name, value):
        path = self.directory / f'{name}.json'
        path.write_text(json.dumps(value))
        return path

    def reject(self, change):
        rows = copy.deepcopy(self.rows)
        change(rows)
        with self.assertRaises(Mismatch):
            compare_game(self.original, self.trace('mutated', rows))

    def test_original_repeat_seed_noop_and_subject_pass(self):
        self.assertEqual(len(compare_controls(self.original, self.original, self.original)), 2)
        self.assertEqual(len(compare_game(self.original, self.original)), 2)

    def test_missing_duplicate_empty_and_malformed_frames_fail(self):
        for rows in ([], self.rows[2:], self.rows[:8] + self.rows[6:8] + self.rows[8:], self.rows[:-1]):
            with self.subTest(length=len(rows)), self.assertRaises(Mismatch):
                compare_game(self.original, self.trace('bad', rows))
        rows = copy.deepcopy(self.rows)
        rows[3]['rng'] = []
        with self.assertRaises(Mismatch):
            read_trace(self.trace('bad', rows))

    def test_changed_chip_code_slot_power_hp_rng_and_input_fail(self):
        changes = [(4, 'custom', 'deck', 0, 0x0201), (4, 'custom', 'picks', 0, 1),
                   (4, 'hand', 'powers', 0, 41), (4, 'rng', None, 0, 1)]
        for index, field, member, slot, value in changes:
            def change(rows):
                destination = rows[index][field] if member is None else rows[index][field][member]
                destination[slot] = value
            with self.subTest(field=field, member=member):
                self.reject(change)
        self.reject(lambda rows: rows[8]['objects'][0].update(hp=59))
        self.reject(lambda rows: rows[8].update(input=1))

    def test_missing_consumption_and_native_no_chip_fail(self):
        for operation in ('consumption', 'selection', 'damage', 'object'):
            rows = copy.deepcopy(self.rows)
            for row in rows:
                if operation == 'consumption': row['hand']['at'] = 0
                if operation == 'selection': row['custom']['picked'] = 0
                if operation == 'damage' and row['objects'][0]['hp'] == 60: row['objects'][0]['hp'] = 100
                if operation == 'object': row['objects'][1]['chip'] = 65535
            with self.subTest(operation=operation), self.assertRaises(Mismatch):
                chip_evidence(read_trace(self.trace('no-chip', rows)))

    def test_duplicate_active_timer_does_not_hide_transient(self):
        rows = copy.deepcopy(self.rows)
        for r in rows[12:16]: r['timer'] = 3
        original = self.trace('repeated-timer', rows)
        rows[12]['rng'][1] ^= 1
        with self.assertRaises(Mismatch):
            compare_game(original, self.trace('transient', rows))

    def test_wrong_control_duration_or_seed_write_fails(self):
        short = self.trace('short', self.rows[-8:])
        with self.assertRaises(Mismatch): compare_controls(short, short, short)
        records = [dict(core=i, main=4, frame=100, before=SEED.copy(), after=SEED.copy(), writes=1) for i in (0, 1)]
        check_seed(records)
        for r in records: r['writes'] = 0
        check_seed(records, no_write=True)
        for field, value in [('main', 0x28), ('before', [1, 2]), ('after', [1, 2]), ('writes', 2)]:
            changed = copy.deepcopy(records); changed[0][field] = value
            with self.subTest(field=field), self.assertRaises(Mismatch): check_seed(changed, no_write=True)

    def test_unreached_original_is_inconclusive_subject_is_failure(self):
        for field in ('ready', 'consumption', 'result'):
            rows = copy.deepcopy(self.rows)
            for r in rows:
                if field == 'ready': r['custom']['window'] = 0
                if field == 'consumption': r['hand']['at'] = 0
                if field == 'result': r['result'] = 0
            path = self.trace('unreached', rows)
            with self.subTest(field=field), self.assertRaises(Unreached):
                chip_evidence(read_trace(path))
            try:
                compare_game(self.original, path)
            except Mismatch as error:
                self.assertNotIsInstance(error, Unreached)
            else:
                self.fail('Subject missed a known exercised path without failure')

    def test_complete_loadout_and_source_preservation(self):
        native = self.obj('native', loadout())
        starter = self.obj('starter', loadout(False))
        compare_loadout(native, native, starter)
        with self.assertRaises(Mismatch): compare_loadout(native, starter, starter)
        for field in ('pack', 'board', 'grid', 'record'):
            bad = loadout(); bad[field]['sha256'] = 'b' * 64
            with self.subTest(field=field), self.assertRaises(Mismatch):
                compare_loadout(native, self.obj('bad', bad), starter)
        record = dict(rom=blob(8 * 1024 * 1024), source_state=blob(100), carried_record=blob(6664))
        compare_preserved(record, record)
        for field in record:
            bad = copy.deepcopy(record); bad[field]['sha256'] = 'b' * 64
            with self.subTest(field=field), self.assertRaises(Mismatch): compare_preserved(record, bad)
        bad = copy.deepcopy(record); bad['rom']['sha256'] = ''
        with self.assertRaises(Mismatch): compare_preserved(bad, bad)


    def transport_fixture(self):
        self.report = dict(status='complete', exit_codes=[0, 0], pids=[100, 101],
                           delivered_inputs=FRAME_COUNT * 2, minimum_delivery_ms=50, delay_ms=50)
        self.inputs = [dict(owner=side, frame=frame, source_keys=key_at(frame, side),
                            delivered_keys=key_at(frame, side), delivery_ms=50)
                       for frame in range(FRAME_COUNT) for side in (0, 1)]
        replicas = [dict(worker=side, frame=frame, hash_algorithm='fnv1a64-core-savedata-cable-v1',
                         hash='1234567890abcdef', samples=[[0, 0, 0, 100, 0, frame, 0x1828, 0x0804]] * 2)
                    for frame in range(FRAME_COUNT) for side in (0, 1)]
        self.trace('case-replicas', replicas)
        self.trace('case-worker0', self.rows)
        self.trace('case-worker1', self.rows)
        self.write_transport()

    def write_transport(self):
        self.obj('case-transport', self.report)
        self.trace('case-inputs', self.inputs)

    def test_measured_delays_and_delivered_script_required(self):
        self.transport_fixture()
        self.assertEqual(accept_delay(self.original, self.directory, 'case', 50)['replica_frames'], FRAME_COUNT)
        for field, value in [('delay_ms', 0), ('minimum_delivery_ms', None), ('pids', [100, 100]),
                             ('exit_codes', [0, 1]), ('delivered_inputs', FRAME_COUNT)]:
            before = self.report[field]; self.report[field] = value; self.write_transport()
            with self.subTest(field=field), self.assertRaises(Mismatch):
                accept_delay(self.original, self.directory, 'case', 50)
            self.report[field] = before
        for operation in ('empty', 'truncated', 'duplicate', 'keys', 'measurement'):
            before = copy.deepcopy(self.inputs)
            if operation == 'empty': self.inputs = []
            if operation == 'truncated': self.inputs.pop()
            if operation == 'duplicate': self.inputs[2] = self.inputs[0].copy()
            if operation == 'keys': self.inputs[200]['delivered_keys'] ^= 1
            if operation == 'measurement': self.inputs[200]['delivery_ms'] = float('nan')
            self.write_transport()
            with self.subTest(operation=operation), self.assertRaises(Mismatch):
                accept_delay(self.original, self.directory, 'case', 50)
            self.inputs = before


if __name__ == '__main__':
    unittest.main()
