"""The cable replay oracle rejects empty, partial and self-asserted evidence."""
import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools.online_replay_compare import (HORIZONS, MUTATIONS, TARGET, TOTAL, Mismatch, Unreached,
                                        accept_mutation, accept_replays, read_trace, select_points, verify_manifest)


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


if __name__ == '__main__':
    unittest.main()
