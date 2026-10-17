"""Synthetic comparator fault checks. These are not original-ROM evidence."""
import copy
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import online_trade_stage_compare as c


class StageComparatorTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.addCleanup(self.temp.cleanup)

    def fixture(self, name='first-code'):
        case = c.CASES[name]
        before = bytearray(c.LAYOUT['core_bytes'] + 64)
        struct.pack_into('<I', before, 0, c.LAYOUT['magic'])
        struct.pack_into('<I', before, 8, c.LAYOUT['rom_crc'])
        struct.pack_into('<I', before, 28, c.LAYOUT['rom_id'])
        struct.pack_into('<I', before, c.offset(c.TOOLKIT + c.TOOLKIT_PACK, 4), 0x02002230)
        struct.pack_into('<I', before, c.offset(c.TOOLKIT + c.TOOLKIT_MARKS, 4), 0x02004c20)
        fields, complete = c.inventory(before, case)
        before[fields['outgoing-count'][0]] = case['outgoing']
        before[fields['incoming-count'][0]] = case['incoming']
        struct.pack_into('<H', before, fields['order-counter'][0], case['counter'])
        struct.pack_into('<H', before, fields['incoming-order'][0], 0xbeef)
        taken = bytearray(before)
        taken[fields['outgoing-count'][0]] -= 1
        given = bytearray(taken)
        given[fields['incoming-count'][0]] += 1
        struct.pack_into('<H', given, fields['order-counter'][0], (case['counter'] - 1) & 65535)
        struct.pack_into('<H', given, fields['incoming-order'][0], case['counter'])
        if not case['incoming']:
            given[fields['incoming-mark'][0]] = 1
            given[fields['incoming-library'][0]] = 0x80 >> ((c.LIBRARY_FLAG + case['receive']) & 7)
        regs = [0] * 16
        regs[10], regs[13], regs[15] = c.TOOLKIT, 0x03007e00, c.RETURN_PC + 2
        calls = {}
        for op in ('noop', 'take', 'give'):
            args = [case['receive'], case['codes'][1], 1] if op == 'give' else \
                   [case['give'], case['codes'][0], 0 if op == 'noop' else 1]
            calls[op] = dict(function=c.GIVE if op == 'give' else c.TAKE, arguments=args,
                             return_pc=c.RETURN_PC, steps=30, minimum_sp=regs[13] - 40,
                             before=regs[:], after=regs[:])
        self.states = dict(before=before, noop=bytearray(before), taken=taken, given=given)
        self.calls = calls
        for repeat in ('a', 'b'):
            path = self.root / repeat
            path.mkdir(exist_ok=True)
            (path / 'calls.json').write_text(json.dumps(calls))
            for stage, data in self.states.items():
                (path / f'{stage}.state').write_bytes(data)
        return fields, complete

    def check_oracle(self, name='first-code'):
        return c.oracle(self.root / 'a', self.root / 'b', name)

    def test_all_synthetic_cases_and_exact_candidate_pass(self):
        for name in c.CASES:
            self.fixture(name)
            before, original, fields, complete = self.check_oracle(name)
            result = c.accept_candidate(before, original, fields, complete, original, original, b'live', b'live')
            self.assertEqual(result['verdict'], 'PASS')
            self.assertEqual(result['oracle'], 'original-routines')
            self.assertEqual(result['masks'], [])

    def test_every_observed_inventory_omission_fails(self):
        self.fixture()
        before, original, fields, complete = self.check_oracle()
        for name, (at, size) in fields.items():
            candidate = bytearray(original)
            candidate[at:at + size] = before[at:at + size]
            with self.subTest(name=name), self.assertRaises(c.Mismatch):
                c.accept_candidate(before, original, fields, complete, candidate, candidate, b'a', b'a')

    def test_code_count_folder_cpu_flash_and_unrelated_byte_faults_fail(self):
        self.fixture()
        before, original, fields, complete = self.check_oracle()
        faults = [fields['outgoing-count'][0], fields['incoming-count'][0],
                  fields['outgoing-count'][0] + 1, fields['incoming-count'][0] + 1,
                  c.offset(0x02002178), c.offset(0x02001000), 32, c.LAYOUT['core_bytes']]
        for at in faults:
            candidate = bytearray(original)
            candidate[at] ^= 1
            with self.subTest(at=at), self.assertRaises(c.Mismatch):
                c.accept_candidate(before, original, fields, complete, candidate, candidate, b'a', b'a')
        with self.assertRaises(c.Mismatch):
            c.accept_candidate(before, original, fields, complete, before, before, b'a', b'a')
        with self.assertRaises(c.Mismatch):
            c.accept_candidate(before, original, fields, complete, original, before, b'a', b'a')
        with self.assertRaises(c.Mismatch):
            c.accept_candidate(before, original, fields, complete, original, original, b'a', b'b')

    def test_missing_truncated_wrong_target_or_nonrepeating_original_fails(self):
        self.fixture()
        path = self.root / 'b' / 'given.state'
        good = path.read_bytes()
        for invalid in (good[:100], bytes(len(good)), good[:-1], good + b'0'):
            path.write_bytes(invalid)
            with self.assertRaises(c.Mismatch):
                self.check_oracle()
        path.unlink()
        with self.assertRaises(c.Unreached):
            self.check_oracle()

    def test_original_missing_return_wrong_arguments_and_unclassified_effects_refused(self):
        for change in (lambda x: x['give'].update(return_pc=0),
                       lambda x: x['take'].update(arguments=[2, 11, 2]),
                       lambda x: x['give'].update(minimum_sp=0),
                       lambda x: x.pop('noop')):
            self.fixture()
            changed = copy.deepcopy(self.calls)
            change(changed)
            for repeat in ('a', 'b'):
                (self.root / repeat / 'calls.json').write_text(json.dumps(changed))
            with self.assertRaises(c.Mismatch):
                self.check_oracle()
        for at in (c.offset(0x02001000), c.LAYOUT['iwram'], c.LAYOUT['core_bytes']):
            self.fixture()
            changed = bytearray(self.states['given'])
            changed[at] ^= 1
            for repeat in ('a', 'b'):
                (self.root / repeat / 'given.state').write_bytes(changed)
            with self.assertRaises(c.Mismatch):
                self.check_oracle()

    def test_unchanged_original_effect_never_satisfies_its_mutation(self):
        fields, _ = self.fixture()
        for name in ('incoming-mark', 'incoming-library', 'incoming-order', 'order-counter'):
            changed = bytearray(self.states['given'])
            at, size = fields[name]
            changed[at:at + size] = self.states['before'][at:at + size]
            for repeat in ('a', 'b'):
                (self.root / repeat / 'given.state').write_bytes(changed)
            with self.subTest(name=name), self.assertRaises(c.Unreached):
                self.check_oracle()

    def test_manifest_requires_complete_nonmutation_and_build_pins(self):
        pins = {'comparator': '1' * 64, 'oracle': '2' * 64}
        valid = dict(before=c.TARGET, after=c.TARGET, layout=c.LAYOUT, sources=pins,
                     binaries=dict(oracle='3' * 64, candidate='4' * 64),
                     fixtures={name: '5' * 64 for name in c.CASES})
        c.verify_manifest(valid, pins)
        for field in valid:
            changed = copy.deepcopy(valid)
            del changed[field]
            with self.subTest(field=field), self.assertRaises(c.Mismatch):
                c.verify_manifest(changed, pins)
        for mutate in (lambda m: m['after'].pop('state_sha256'),
                       lambda m: m['after'].update(state_sha256='f' * 64),
                       lambda m: m['binaries'].update(oracle='mGBA 0.10.5'),
                       lambda m: m['fixtures'].pop('counter-wrap'),
                       lambda m: m['layout'].update(wram=0),
                       lambda m: m['sources'].update(comparator='f' * 64)):
            changed = copy.deepcopy(valid)
            mutate(changed)
            with self.assertRaises(c.Mismatch):
                c.verify_manifest(changed, pins)

    def test_missing_or_truncated_editor_evidence_is_unreached(self):
        path = self.root / 'editor.counts'
        with self.assertRaises(c.Unreached):
            c.editor(path)
        path.write_bytes(bytes(512 * 27 - 1))
        with self.assertRaises(c.Unreached):
            c.editor(path)
        path.write_bytes(bytes(512 * 27))
        self.assertEqual(len(c.editor(path)), 512 * 27)

    def test_complete_entry_checks_source_editor_and_live_evidence(self):
        self.fixture()
        for source, destination in (('a', 'original-a'), ('b', 'original-b')):
            (self.root / source).rename(self.root / destination)
            (self.root / destination / 'editor.counts').write_bytes(bytes(512 * 27))
        for name, data in {'source-after': self.states['before'], 'candidate': self.states['given'],
                           'reloaded': self.states['given'], 'live-before': self.states['before'],
                           'live-after': self.states['before']}.items():
            (self.root / f'{name}.state').write_bytes(data)
        (self.root / 'candidate-editor.counts').write_bytes(bytes(512 * 27))
        pins = {'comparator': '1' * 64}
        manifest = dict(before=c.TARGET, after=c.TARGET, layout=c.LAYOUT, sources=pins,
                        binaries=dict(oracle='2' * 64, candidate='3' * 64),
                        fixtures={name: c.digest(self.states['before']) for name in c.CASES})
        self.assertEqual(c.accept_case(self.root, 'first-code', manifest, pins)['verdict'], 'PASS')
        for name in ('source-after.state', 'live-after.state', 'candidate-editor.counts'):
            path = self.root / name
            old = path.read_bytes()
            changed = bytearray(old)
            changed[-1] ^= 1
            path.write_bytes(changed)
            with self.subTest(name=name), self.assertRaises(c.Mismatch):
                c.accept_case(self.root, 'first-code', manifest, pins)
            path.write_bytes(old)


if __name__ == '__main__':
    unittest.main()
