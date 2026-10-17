"""The cable replay oracle rejects empty, partial and self-asserted evidence."""
import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools.online_replay_compare import (HORIZONS, MUTATIONS, TARGET, TOTAL, Mismatch, Unreached, accept_cable_stop,
                                        accept_mutation, accept_observer, accept_replays, read_cable, read_trace, select_points, verify_manifest)


def record(frame):
    blob = dict(bytes=1024, sha256='a' * 64)
    node = dict(credit=0, next_event=0, event_diff=0, mode=0, normal_so=False,
                transfer_finished=False, siocnt=0, rcnt=0, scheduled=True, event_delay=8, event_priority=0)
    sample = [0, 0, 0, 100 << 16 | 100, 0, frame, 0, 0x0804]
    return dict(frame=frame, hash_algorithm='sha256', cores=[dict(state=blob.copy(), flash=blob.copy()) for _ in range(2)],
                cable=dict(attached=2, phase=0, cycles=0, attached_multi=2, attached_normal=0, waiting=0,
                           transfers=0, running=0, multi_recv=[0]*4, normal_recv=[0]*4, nodes=[node.copy(), node.copy()]),
                pair=dict(frame=frame, start_frames=[0, 0], traced_frames=[frame, frame], entry_step=0, entry_frame=0),
                samples=[sample.copy(), sample.copy()], inputs=[[0, 0] for _ in range(4)])


def fixture():
    rows = [record(frame) for frame in range(TOTAL + 1)]
    for row in rows[10:]:
        for sample in row['samples']:
            sample[2], sample[7] = row['frame'] - 9, 0x0c04
    for row in rows[10:15]:
        row['inputs'][0][0] = 2
    for row in rows[20:]:
        for sample in row['samples']:
            sample[3] = 90 << 16 | 100
    rows[25]['cable']['phase'] = 2
    for row in rows[31:]:
        row['cores'][0]['flash']['sha256'] = 'b' * 64
    return rows


class ReplayComparatorTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.path = Path(self.temp.name)
        self.rows = fixture()
        self.original = self.write('original.jsonl', self.rows)

    def write(self, name, rows):
        path = self.path / name
        path.write_text(''.join(json.dumps(row) + '\n' for row in rows))
        return path

    def replays(self):
        for name, frame in select_points(self.rows).items():
            for horizon in HORIZONS:
                for repeat in (1, 2):
                    self.write(f'{name}-{horizon}-{repeat}.jsonl', self.rows[frame:frame + horizon + 1])

    def cable_fixture(self):
        row = copy.deepcopy(self.rows[25])
        record = dict(state=row, cursor=dict(stage='check-targets', frame=25, round=4, targets=[1, 1],
                                             relative=0, battle=False, keys=copy.deepcopy(row['inputs'])))
        path = self.path / 'cable.json'
        path.write_text(json.dumps(record))
        return path, record

    def test_complete_control_and_all_replays_pass(self):
        self.replays()
        result = accept_replays(self.original, self.original, self.path)
        self.assertEqual(len(result['replay_cases']), 36)
        self.assertEqual(result['points'], dict(custom=1, **{'held-charge':11, 'before-release':15,
                         'after-damage':20, 'nonidle-cable':25, 'flash-change':30}))

    def test_parser_rejects_empty_partial_duplicate_and_malformed(self):
        for rows in ([], self.rows[:-1], self.rows[:1] * (TOTAL + 1)):
            with self.subTest(length=len(rows)), self.assertRaises(Mismatch):
                read_trace(self.write('bad.jsonl', rows))
        for mutate in (lambda r: r.update(frame=True), lambda r: r.update(samples=[]),
                       lambda r: r.update(hash_algorithm='md5'),
                       lambda r: r['cores'][0]['state'].update(bytes=0),
                       lambda r: r['cores'][1]['flash'].update(sha256='not a hash'),
                       lambda r: r['cable']['nodes'][0].update(scheduled=1),
                       lambda r: r['inputs'][0].__setitem__(0, 1024)):
            rows = copy.deepcopy(self.rows)
            mutate(rows[0])
            with self.assertRaises(Mismatch):
                read_trace(self.write('bad.jsonl', rows))

    def test_changed_core_flash_cable_sample_input_or_missing_repeat_fails(self):
        self.replays()
        frame = select_points(self.rows)['nonidle-cable']
        name = 'nonidle-cable-8-2.jsonl'
        for mutate in (lambda r: r['cores'][0]['state'].update(sha256='c'*64),
                       lambda r: r['cores'][1]['flash'].update(sha256='c'*64),
                       lambda r: r['cable']['nodes'][0].update(event_delay=9),
                       lambda r: r['samples'][0].__setitem__(3, 42),
                       lambda r: r['inputs'][0].__setitem__(0, 1)):
            rows = copy.deepcopy(self.rows[frame:frame+9])
            mutate(rows[0])
            self.write(name, rows)
            with self.assertRaises(Mismatch):
                accept_replays(self.original, self.original, self.path)
        (self.path / name).unlink()
        with self.assertRaises(Mismatch):
            accept_replays(self.original, self.original, self.path)

    def test_idle_cable_and_unchanged_flash_are_unreached(self):
        rows = copy.deepcopy(self.rows)
        rows[25]['cable']['phase'] = 0
        with self.assertRaisesRegex(Unreached, 'nonidle-cable'):
            select_points(rows)
        rows = copy.deepcopy(self.rows)
        for row in rows:
            row['cores'][0]['flash']['sha256'] = 'a'*64
        with self.assertRaisesRegex(Unreached, 'flash-change'):
            select_points(rows)

    def test_real_component_mutations_caught_and_false_mutations_refused(self):
        points = select_points(self.rows)
        for mutation in MUTATIONS:
            frame = points['flash-change' if mutation == 'omit-flash' else 'nonidle-cable']
            rows = copy.deepcopy(self.rows[frame:frame+9])
            path = self.write('mutation.jsonl', rows)
            with self.subTest(mutation=mutation), self.assertRaises(Mismatch):
                accept_mutation(self.original, mutation, path)
            if mutation == 'omit-flash':
                rows[0]['cores'][0]['flash']['sha256'] = 'b'*64
            elif mutation == 'receive-word':
                rows[0]['cable']['multi_recv'][0] ^= 1
            elif mutation == 'event-delay':
                rows[0]['cable']['nodes'][0]['event_delay'] += 1
            else:
                rows[0]['cable']['nodes'][0]['credit'] += 1
            path = self.write('mutation.jsonl', rows)
            self.assertTrue(accept_mutation(self.original, mutation, path)['expected_failure'])
            rows[0]['inputs'][0][0] ^= 16
            with self.assertRaisesRegex(Mismatch, 'controller'):
                accept_mutation(self.original, mutation, self.write('mutation.jsonl', rows))

    def test_source_pin_and_target_preservation_guards(self):
        pins = {'reviewed-observer.c': 'c'*64, 'reviewed-comparator.py': 'd'*64}
        manifest = dict(before=TARGET.copy(), after=TARGET.copy(), source_hashes=pins.copy(),
                        binary_hashes=dict(oracle='e'*64, subject='f'*64))
        verify_manifest(manifest, pins)
        for mutate in (lambda m: m['after'].update(boot_sha256='0'*64),
                       lambda m: m['before'].update(rom_sha256='0'*64),
                       lambda m: m['source_hashes'].update({'reviewed-observer.c':'0'*64}),
                       lambda m: m['binary_hashes'].update(subject='')):
            changed = copy.deepcopy(manifest)
            mutate(changed)
            with self.assertRaises(Mismatch):
                verify_manifest(changed, pins)

    def test_instruction_boundary_replay_requires_exact_cursor_and_immediate_state(self):
        self.replays()
        path, cable = self.cable_fixture()
        for horizon in HORIZONS:
            for repeat in (1, 2):
                label = f'nonidle-cable-{horizon}-{repeat}'
                (self.path / f'{label}-immediate.json').write_text(json.dumps(cable))
                self.write(f'{label}.jsonl', self.rows[26:26+horizon])
        self.assertEqual(len(accept_replays(self.original, self.original, self.path, path)['replay_cases']), 36)
        target = self.path / 'nonidle-cable-8-2-immediate.json'
        for mutate in (lambda c: c['cursor'].update(stage='frame-start'),
                       lambda c: c['cursor'].update(round=0),
                       lambda c: c['cursor'].update(round=5),
                       lambda c: c['cursor'].update(targets=[2, 1]),
                       lambda c: c['state']['cores'][0]['state'].update(sha256='f'*64)):
            changed = copy.deepcopy(cable)
            mutate(changed)
            target.write_text(json.dumps(changed))
            with self.assertRaises(Mismatch):
                accept_replays(self.original, self.original, self.path, path)
        target.unlink()
        with self.assertRaises(Mismatch):
            accept_replays(self.original, self.original, self.path, path)

    def test_instruction_boundary_must_be_active_nonidle_and_scheduled(self):
        path, cable = self.cable_fixture()
        self.assertEqual(read_cable(path, self.rows), cable)
        for mutate in (lambda c: c['state']['cable'].update(phase=0),
                       lambda c: [n.update(scheduled=False) for n in c['state']['cable']['nodes']],
                       lambda c: c['state']['samples'][0].__setitem__(7, 0x0804)):
            changed = copy.deepcopy(cable)
            mutate(changed)
            path.write_text(json.dumps(changed))
            with self.assertRaises(Unreached):
                read_cable(path, self.rows)

    def test_instruction_boundary_mutation_preserves_cursor_and_frame_continuation(self):
        reference, cable = self.cable_fixture()
        changed = copy.deepcopy(cable)
        changed['state']['cable']['nodes'][0]['credit'] += 1
        immediate = self.path / 'changed-immediate.json'
        immediate.write_text(json.dumps(changed))
        continuation = self.write('changed.jsonl', self.rows[26:34])
        result = accept_mutation(self.original, 'credit', continuation, reference, immediate)
        self.assertEqual(result['first_divergent_frame'], 25)
        with self.assertRaises(Mismatch):
            accept_mutation(self.original, 'event-delay', continuation, reference, immediate)
        changed['cursor']['round'] += 1
        immediate.write_text(json.dumps(changed))
        with self.assertRaisesRegex(Mismatch, 'cursor'):
            accept_mutation(self.original, 'credit', continuation, reference, immediate)

    def test_repeating_micro_observer_must_match_controlled_original_full_state(self):
        cable, _ = self.cable_fixture()
        # Equal logical CSV alone must not excuse a common full-state error.
        with patch('tools.online_replay_compare.compare_game', return_value=1433):
            with self.assertRaisesRegex(Mismatch, 'controlled original'):
                accept_observer('old.csv', 'new.csv', self.original, self.original, cable, cable)
            self.assertEqual(accept_observer('old.csv', 'new.csv', self.original, self.original,
                                            cable, cable, self.original)['verdict'], 'PASS')
            for mutate in (lambda r: r['cores'][0]['state'].update(sha256='e'*64),
                           lambda r: r['cable']['nodes'][0].update(credit=42)):
                rows = copy.deepcopy(self.rows)
                mutate(rows[700])
                repeated = self.write('repeated.jsonl', rows)
                with self.assertRaisesRegex(Mismatch, 'original versus instruction executor'):
                    accept_observer('old.csv', 'new.csv', repeated, repeated, cable, cable, self.original)

    def stop_fixture(self):
        self.rows[25]['cable']['phase'] = 0
        self.rows[1150]['cable']['phase'] = 1
        for row in self.rows[958:]:
            row['pair']['start_frames'] = [2000, 2000]
            row['pair']['traced_frames'] = [2000 + row['frame'] - 958] * 2
            for sample in row['samples']:
                sample[5] = 2000 + row['frame'] - 958
        for name in ('micro-a', 'micro-b', 'baseline', 'noop'):
            self.write(f'{name}.jsonl', self.rows)
        cable = dict(state=copy.deepcopy(self.rows[1150]),
                     cursor=dict(stage='check-targets', frame=1150, round=4, targets=[2193, 2193],
                                 relative=192, battle=True, keys=copy.deepcopy(self.rows[1150]['inputs'])))
        for name in ('micro-a-cable', 'micro-b-cable'):
            (self.path / f'{name}.json').write_text(json.dumps(cable))
        for name, frame in select_points(self.rows, cable).items():
            for horizon in HORIZONS:
                for repeat in (1, 2):
                    label = f'{name}-{horizon}-{repeat}'
                    first = frame
                    if name == 'nonidle-cable':
                        first += 1
                        (self.path / f'{label}-immediate.json').write_text(json.dumps(cable))
                    self.write(f'{label}.jsonl', self.rows[first:frame + horizon + 1])
        immediate = copy.deepcopy(cable)
        for node in immediate['state']['cable']['nodes']:
            node['scheduled'] = False
        (self.path / 'omit-cable-immediate.json').write_text(json.dumps(immediate))
        self.write('omit-cable.jsonl', self.rows[1151:1155])
        row = copy.deepcopy(self.rows[1154])
        row['samples'][1][5] += 4
        row['pair']['traced_frames'][1] += 3
        row['cable']['running'] = 1
        stop = dict(reason='frame-lead', state=row,
                    cursor=dict(stage='before-step', frame=1154, round=32, targets=[2197, 2197],
                                relative=196, battle=True, keys=copy.deepcopy(row['inputs']), next_side=1))
        (self.path / 'omit-cable-stop.json').write_text(json.dumps(stop))
        pins = {'tools/online_replay_snapshot.c':'c'*64, 'tools/online_replay_subject.c':'d'*64}
        manifest = dict(before=TARGET.copy(), after=TARGET.copy(), source_hashes=pins.copy(),
                        binary_hashes=dict(oracle='e'*64, subject='f'*64))
        termination = dict(returncode=3, elapsed_seconds=1.5, binary_sha256='f'*64)
        return pins, manifest, termination

    @patch('tools.online_replay_compare.compare_game', return_value=1433)
    def test_omitted_cable_stop_needs_all_controls_and_an_observed_bound(self, _logical):
        pins, manifest, termination = self.stop_fixture()
        result = accept_cable_stop(self.path, manifest, pins, termination)
        self.assertEqual((result['first_divergent_frame'], result['completed_prefix_frames']), (1150, 4))
        path = self.path / 'omit-cable-stop.json'
        original = json.loads(path.read_text())
        for mutate in (lambda s: s.update(reason='crash'),
                       lambda s: s['cursor'].update(stage='check-targets'),
                       lambda s: s['cursor'].update(frame=1150),
                       lambda s: s['cursor'].update(targets=[1, 1]),
                       lambda s: s['state']['samples'][1].__setitem__(5, 2199),
                       lambda s: s['state']['inputs'][0].__setitem__(0, 42)):
            changed = copy.deepcopy(original)
            mutate(changed)
            path.write_text(json.dumps(changed))
            with self.assertRaises(Mismatch):
                accept_cable_stop(self.path, manifest, pins, termination)
        path.write_text(json.dumps(original))
        for mutate in (lambda m: m.update(returncode=-9), lambda m: m.update(returncode=1),
                       lambda m: m.update(binary_sha256='0'*64),
                       lambda m: m.update(elapsed_seconds=float('nan')),
                       lambda m: m.update(elapsed_seconds=180)):
            changed = copy.deepcopy(termination)
            mutate(changed)
            with self.assertRaises(Mismatch):
                accept_cable_stop(self.path, manifest, pins, changed)
        manifest['source_hashes']['tools/online_replay_snapshot.c'] = '0'*64
        with self.assertRaises(Mismatch):
            accept_cable_stop(self.path, manifest, pins, termination)

    @patch('tools.online_replay_compare.compare_game', return_value=1433)
    def test_omission_partial_logs_and_self_agreement_never_pass(self, _logical):
        pins, manifest, termination = self.stop_fixture()
        for prefix in ([], self.rows[1151:1154], self.rows[1151:1152] * 4):
            self.write('omit-cable.jsonl', prefix)
            with self.assertRaises(Unreached):
                accept_cable_stop(self.path, manifest, pins, termination)
        self.write('omit-cable.jsonl', self.rows[1151:1155])
        path = self.path / 'omit-cable-immediate.json'
        original = json.loads(path.read_text())
        for mutate in (lambda s: s['state']['cores'][0]['state'].update(sha256='b'*64),
                       lambda s: s['cursor'].update(round=5),
                       lambda s: [n.update(scheduled=True) for n in s['state']['cable']['nodes']]):
            changed = copy.deepcopy(original)
            mutate(changed)
            path.write_text(json.dumps(changed))
            with self.assertRaises(Mismatch):
                accept_cable_stop(self.path, manifest, pins, termination)
        path.write_text(json.dumps(original))
        self.write('custom-1-1.jsonl', [])
        with self.assertRaises(Mismatch):
            accept_cable_stop(self.path, manifest, pins, termination)
        self.stop_fixture()
        changed = copy.deepcopy(self.rows)
        changed[700]['cores'][0]['state']['sha256'] = 'b'*64
        self.write('micro-a.jsonl', changed)
        self.write('micro-b.jsonl', changed)
        with self.assertRaisesRegex(Mismatch, 'original versus instruction executor'):
            accept_cable_stop(self.path, manifest, pins, termination)


if __name__ == '__main__':
    unittest.main()
