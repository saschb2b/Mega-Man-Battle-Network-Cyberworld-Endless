"""Mutation checks for the separately reviewed frame-battle comparator."""
import csv
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools.online_frame_compare import (HASH_ALGORITHM, Mismatch, accept_case, accept_fault,
                                        compare_game, compare_replicas)  # noqa: E402


class ComparatorTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.rows = []
        for side in (0, 1):
            for timer in range(1, 4):
                self.rows.append(dict(core=side, frame=timer, battle_timer=timer, result=0,
                                      hp0=100, hp1=100 - timer * 10, positions=257,
                                      menu=0, battle_state=0x0C04))
            self.rows.append(dict(core=side, frame=4, battle_timer=3, result=side + 1,
                                  hp0=100, hp1=0, positions=257, menu=781, battle_state=0))
        self.reference = self.write('reference.csv', self.rows)

    def write(self, name, rows):
        path = self.root / name
        with path.open('w') as stream:
            writer = csv.DictWriter(stream, fieldnames=self.rows[0])
            writer.writeheader()
            writer.writerows(rows)
        return path

    def test_unmodified_control(self):
        self.assertEqual(compare_game(self.reference, self.reference), 3)

    def test_game_mutations_fail(self):
        for field in ('hp0', 'hp1', 'positions', 'result', 'menu', 'battle_state', 'battle_timer'):
            with self.subTest(field=field):
                rows = [dict(row) for row in self.rows]
                rows[-1][field] += 1
                with self.assertRaises(Mismatch):
                    compare_game(self.reference, self.write('mutated.csv', rows))

    def test_matching_replica_error_does_not_replace_original(self):
        rows = [dict(row) for row in self.rows]
        for row in rows:
            if row['battle_timer'] == 2:
                row['hp1'] += 1
        with self.assertRaises(Mismatch):
            compare_game(self.reference, self.write('both-wrong.csv', rows))

    def test_missing_active_timer_fails(self):
        rows = [row for row in self.rows if row['battle_timer'] != 2]
        with self.assertRaises(Mismatch):
            compare_game(self.reference, self.write('missing.csv', rows))

    def test_incomplete_result_fails(self):
        rows = [row for row in self.rows if not row['result']]
        with self.assertRaises(Mismatch):
            compare_game(self.reference, self.write('unfinished.csv', rows))

    def test_replica_hash_and_coverage_mutations(self):
        rows = [dict(worker=side, frame=frame, hash='0123456789abcdef',
                     hash_algorithm=HASH_ALGORITHM, samples=[[0] * 8, [0] * 8])
                for side in (0, 1) for frame in range(3)]
        path = self.root / 'replicas.jsonl'
        def save(values):
            path.write_text(''.join(json.dumps(row) + '\n' for row in values))
        save(rows)
        self.assertEqual(compare_replicas(path, 3), 3)
        for mutation in ('hash', 'samples', 'truncated', 'duplicate', 'empty', 'short-hash', 'algorithm'):
            with self.subTest(mutation=mutation):
                changed = [dict(row) for row in rows]
                if mutation == 'truncated':
                    changed.pop()
                elif mutation == 'duplicate':
                    changed.append(changed[-1])
                elif mutation == 'empty':
                    for row in changed:
                        row['samples'] = []
                elif mutation == 'short-hash':
                    for row in changed:
                        row['hash'] = '1234'
                elif mutation == 'algorithm':
                    for row in changed:
                        row['hash_algorithm'] = 'unchecked'
                elif mutation == 'hash':
                    changed[-1]['hash'] = 'fedcba9876543210'
                else:
                    changed[-1][mutation] = 'wrong'
                save(changed)
                with self.assertRaises(Mismatch):
                    compare_replicas(path, 3)

    def test_transport_metadata_mutations(self):
        valid = dict(status='complete', exit_codes=[0, 0], pids=[101, 102],
                     delivered_inputs=5644, delay_ms=50, minimum_delivery_ms=50.01)
        path = self.root / 'case-transport.json'
        path.write_text(json.dumps(valid))
        (self.root / 'case-inputs.jsonl').write_text(''.join(json.dumps(row) + '\n' for row in self.input_rows(2822)))
        # This unit isolates acceptance of the relay's measured metadata;
        # the independent game/hash comparisons have their own mutations above.
        with patch('tools.online_frame_compare.compare_game', return_value=3), patch(
                'tools.online_frame_compare.compare_replicas', return_value=2822):
            self.assertEqual(accept_case(self.reference, self.root, 'case', 50)['verdict'], 'PASS')
            mutations = [('status', 'deadline'), ('exit_codes', [0, 1]), ('pids', [101, 101]),
                         ('delivered_inputs', 5643), ('delay_ms', 0),
                         ('minimum_delivery_ms', 49.99), ('minimum_delivery_ms', '50'),
                         ('minimum_delivery_ms', None)]
            for field, value in mutations:
                with self.subTest(field=field, value=value):
                    changed = dict(valid)
                    if value is None:
                        del changed[field]
                    else:
                        changed[field] = value
                    path.write_text(json.dumps(changed))
                    with self.assertRaises(Mismatch):
                        accept_case(self.reference, self.root, 'case', 50)

    def input_rows(self, frames):
        return [dict(owner=side, frame=frame, source_keys=2, delivered_keys=2, delivery_ms=50.01)
                for side in (0, 1) for frame in range(frames)]

    def fault_fixture(self, name):
        rows = [dict(worker=side, frame=frame, hash='0123456789abcdef',
                     hash_algorithm=HASH_ALGORITHM, samples=[[0, 0, 1, 0, 0, 0, 0, 0x0C04]] * 2)
                for side in (0, 1) for frame in range(1402)]
        corrupt = name == 'corruption'
        if corrupt:
            rows[-1]['hash'] = 'fedcba9876543210'
        report = dict(status='desync' if corrupt else 'disconnected', fault_injected=True,
                      fault_detection_ms=25, first_divergent_frame=1401 if corrupt else None,
                      pids=[10, 11], exit_codes=[0, 0] if corrupt else [0, -9])
        inputs = self.input_rows(1402)
        if corrupt:
            inputs[1400]['delivered_keys'] = 18
        return rows, report, inputs

    def save_fault(self, name, rows, report, inputs):
        (self.root / f'{name}-replicas.jsonl').write_text(''.join(json.dumps(row) + '\n' for row in rows))
        (self.root / f'{name}-transport.json').write_text(json.dumps(report))
        (self.root / f'{name}-inputs.jsonl').write_text(''.join(json.dumps(row) + '\n' for row in inputs))

    def test_fault_evidence_mutations(self):
        for name in ('corruption', 'disconnect'):
            with self.subTest(name=name):
                values = self.fault_fixture(name)
                self.save_fault(name, *values)
                self.assertEqual(accept_fault(self.root, name)['verdict'], 'PASS')
                for mutation in ('status', 'deadline', 'not-injected', 'no-evidence', 'early', 'different-first',
                                 'empty-inputs', 'truncated-inputs', 'duplicate-input', 'not-active', 'invalid-delay', 'invalid-keys'):
                    with self.subTest(mutation=mutation):
                        rows, report, inputs = self.fault_fixture(name)
                        if mutation == 'status':
                            report['status'] = 'complete'
                        elif mutation == 'deadline':
                            report['fault_detection_ms'] = 5000
                        elif mutation == 'not-injected':
                            report['fault_injected'] = False
                        elif mutation == 'early':
                            rows = [row for row in rows if row['frame'] < 10]
                        elif mutation == 'no-evidence':
                            if name == 'corruption':
                                inputs[1400]['delivered_keys'] = inputs[1400]['source_keys']
                            else:
                                report['exit_codes'] = [0, 0]
                        elif mutation == 'empty-inputs':
                            inputs = []
                        elif mutation == 'truncated-inputs':
                            inputs.pop()
                        elif mutation == 'duplicate-input':
                            inputs.append(inputs[-1])
                        elif mutation == 'not-active':
                            rows[-1]['samples'] = [[0] * 8, [0] * 8]
                        elif mutation == 'invalid-delay':
                            inputs[0]['delivery_ms'] = float('nan')
                        elif mutation == 'invalid-keys':
                            inputs[0]['source_keys'] = 1024
                        elif name == 'corruption':
                            report['first_divergent_frame'] = 1400
                        else:
                            rows[-1]['hash'] = 'fedcba9876543210'
                        self.save_fault(name, rows, report, inputs)
                        with self.assertRaises(Mismatch):
                            accept_fault(self.root, name)


if __name__ == '__main__':
    unittest.main()
