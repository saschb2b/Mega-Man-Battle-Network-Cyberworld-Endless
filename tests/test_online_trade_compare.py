#!/usr/bin/env python3
"""Comparator guards over invented bytes; never evidence about BN6 itself."""
import copy
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from online_trade_compare import CASES, COUNTS_BYTES, RAM_BYTES, ROM_SHA1, compare


class TradeComparatorTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / "counts").write_bytes(bytes(COUNTS_BYTES))
        (self.root / "ram").write_bytes(bytes(RAM_BYTES))
        (self.root / "snapshot").write_bytes(b"invented immutable snapshot")
        self.report = {"schema": "online-trade-proof/1", "rom_sha1": ROM_SHA1,
                       "state_sha256": "a" * 64, "mgba": "0.10.5",
                       "input_hashes_before": {"rom": "b" * 64},
                       "input_hashes_after": {"rom": "b" * 64}, "cases": {}}
        for name, (count, active) in CASES.items():
            lane = {"folder_count": count, "active_folder": active, "map_mode": 4,
                    "editor_mode": 40, "editor_state": [32, 4], "frames": 200,
                    "sample_frames": [200], "counts": "counts", "ram": "ram",
                    "capture_ok": True, "prepared_before": "snapshot",
                    "prepared_after": "snapshot", "live_pack_changed": True,
                    "capacity_full": True, "retained_counts": "counts"}
            self.report["cases"][name] = {key: copy.deepcopy(lane)
                                           for key in ("reference", "control", "subject")}

    def verdict(self):
        return compare(self.root, self.report)["verdict"]

    def test_complete_control(self):
        self.assertEqual(self.verdict(), "PASS")

    def test_missing_or_unreached_case_is_inconclusive(self):
        for name in CASES:
            with self.subTest(name=name):
                lane = self.report["cases"][name]["subject"]
                lane["editor_state"] = [0, 4]
                self.assertEqual(self.verdict(), "INCONCLUSIVE")
                lane["editor_state"] = [32, 4]

    def test_first_count_difference_is_precise(self):
        counts = bytearray(COUNTS_BYTES)
        counts[27 * 5 + 2] = 3
        (self.root / "wrong-counts").write_bytes(counts)
        self.report["cases"]["extra-folder"]["subject"]["counts"] = "wrong-counts"
        outcome = compare(self.root, self.report)
        self.assertEqual(outcome["verdict"], "FAIL")
        self.assertIn("chip 5, code 2: expected 0, got 3", outcome["cases"]["extra-folder"]["detail"])

    def test_whole_ram_difference_has_no_mask(self):
        ram = bytearray(RAM_BYTES)
        ram[-1] = 1
        (self.root / "wrong-ram").write_bytes(ram)
        self.report["cases"]["first-folder"]["subject"]["ram"] = "wrong-ram"
        self.assertEqual(self.verdict(), "FAIL")
        self.report["cases"]["first-folder"]["control"]["ram"] = "wrong-ram"
        self.assertEqual(self.verdict(), "INCONCLUSIVE")

    def test_capture_and_lifetime(self):
        lane = self.report["cases"]["first-folder"]["subject"]
        lane["capture_ok"] = False
        self.assertEqual(self.verdict(), "FAIL")
        lane["capture_ok"] = True
        lane["live_pack_changed"] = False
        self.assertEqual(self.verdict(), "INCONCLUSIVE")
        lane["live_pack_changed"] = True
        (self.root / "changed").write_bytes(b"invented mutable!! snapshot")
        lane["prepared_after"] = "changed"
        self.assertEqual(self.verdict(), "FAIL")

    def test_truncation_and_frame_alignment(self):
        lane = self.report["cases"]["first-folder"]["subject"]
        lane["counts"] = "snapshot"
        self.assertEqual(self.verdict(), "INCONCLUSIVE")
        lane["counts"] = "counts"
        lane["sample_frames"] = [201]
        self.assertEqual(self.verdict(), "INCONCLUSIVE")

    def test_capacity_and_file_mutation(self):
        self.report["cases"]["count-boundaries"]["subject"]["capacity_full"] = False
        self.assertEqual(self.verdict(), "FAIL")
        self.report["cases"]["count-boundaries"]["subject"]["capacity_full"] = True
        self.report["input_hashes_after"]["rom"] = "c" * 64
        self.assertEqual(self.verdict(), "FAIL")


if __name__ == "__main__":
    unittest.main()
