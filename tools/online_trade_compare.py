#!/usr/bin/env python3
"""Exact, ROM-free comparator for local native trade-inventory evidence.

The probe writes raw observations; this module owns required scenarios,
reachability, byte comparison and verdicts. No count subtraction lives here.
"""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

ROM_SHA1 = "89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6"
COUNTS_BYTES = 512 * 27  # byte count for every canonical id/code, including zero
RAM_BYTES = 0x40000 + 0x8000  # whole EWRAM and IWRAM, without masks
CASES = {
    "first-folder": (1, 0),
    "second-folder": (2, 0),
    "extra-folder": (3, 0),
    "active-second": (3, 1),
    "active-extra": (3, 2),
    "count-boundaries": (3, 0),
}


def result(verdict: str, detail: str) -> dict:
    return {"verdict": verdict, "detail": detail}


def read_blob(root: Path, path: str, size: int | None = None) -> bytes:
    data = (root / path).read_bytes()
    if size is not None and len(data) != size:
        raise ValueError(f"{path}: expected {size} bytes, got {len(data)}")
    return data


def first_difference(expected: bytes, actual: bytes) -> int | None:
    return next((i for i, (a, b) in enumerate(zip(expected, actual)) if a != b), None)


def compare_blob(expected: bytes, actual: bytes, label: str) -> dict | None:
    if len(expected) != len(actual):
        return result("INCONCLUSIVE", f"{label}: observation lengths differ")
    at = first_difference(expected, actual)
    if at is None:
        return None
    location = (f"chip {at // 27}, code {at % 27}" if label == "counts" else f"byte {at}")
    return result("FAIL", f"{label}: {location}: expected {expected[at]}, got {actual[at]}")


def reachable(observation: dict, count: int, active: int) -> bool:
    return (observation.get("folder_count") == count
            and observation.get("active_folder") == active
            and observation.get("map_mode") == 4
            and observation.get("editor_mode") == 40
            and observation.get("editor_state") == [32, 4]
            and observation.get("frames", 0) > 0)


def compare_case(root: Path, case: dict, expected_state: tuple[int, int], boundary: bool = False) -> dict:
    count, active = expected_state
    lanes = [case.get(name, {}) for name in ("reference", "control", "subject")]
    if not all(reachable(lane, count, active) for lane in lanes):
        return result("INCONCLUSIVE", "a lane never reached the specified native folder/editor state")
    reference, control, subject = lanes
    if not subject.get("capture_ok"):
        return result("FAIL", "reader refused a reached, supported map-side inventory")
    frames = reference.get("sample_frames")
    if not frames or sorted(set(frames)) != frames:
        return result("INCONCLUSIVE", "missing or invalid sampled-frame sequence")
    if any(lane.get("sample_frames") != frames for lane in lanes[1:]):
        return result("INCONCLUSIVE", "lanes sampled different frames")
    size = len(frames) * RAM_BYTES
    native_ram = read_blob(root, reference["ram"], size)
    repeat_ram = read_blob(root, control["ram"], size)
    observed_ram = read_blob(root, subject["ram"], size)
    difference = compare_blob(native_ram, repeat_ram, "native control RAM")
    if difference:
        return result("INCONCLUSIVE", "native repeat is not deterministic: " + difference["detail"])
    difference = compare_blob(native_ram, observed_ram, "observer RAM")
    if difference:
        return difference
    expected = read_blob(root, reference["counts"], COUNTS_BYTES)
    repeated = read_blob(root, control["counts"], COUNTS_BYTES)
    difference = compare_blob(expected, repeated, "counts")
    if difference:
        return result("INCONCLUSIVE", "native editor repeat differs: " + difference["detail"])
    actual = read_blob(root, subject["counts"], COUNTS_BYTES)
    difference = compare_blob(expected, actual, "counts")
    if difference:
        return difference
    before = read_blob(root, subject["prepared_before"])
    after = read_blob(root, subject["prepared_after"])
    if not before or not subject.get("live_pack_changed"):
        return result("INCONCLUSIVE", "snapshot lifetime challenge was not reached")
    difference = compare_blob(before, after, "prepared snapshot lifetime")
    if difference:
        return difference
    retained = read_blob(root, subject["retained_counts"], COUNTS_BYTES)
    difference = compare_blob(actual, retained, "retained counts")
    if difference:
        return difference
    if boundary and subject.get("capacity_full") is not True:
        return result("FAIL", "receive-capacity decision differs from the prescribed boundary")
    return result("PASS", "native counts, repeated control, whole RAM and snapshot lifetime agree")


def digest(value: object) -> bool:
    return isinstance(value, str) and re.fullmatch(r"[0-9a-f]{64}", value) is not None


def compare(root: Path, report: dict) -> dict:
    if (report.get("schema") != "online-trade-proof/1"
            or report.get("rom_sha1") != ROM_SHA1
            or report.get("mgba") != "0.10.5"
            or not all(digest(report.get(key)) for key in
                       ("state_sha256", "probe_sha256", "mgba_library_sha256"))):
        return result("INCONCLUSIVE", "missing or wrong pinned target identity")
    before, after = report.get("input_hashes_before", {}), report.get("input_hashes_after", {})
    if any(not isinstance(hashes, dict) or not all(digest(hashes.get(key)) for key in ("rom", "state"))
           for hashes in (before, after)):
        return result("INCONCLUSIVE", "ROM and state nonmutation hashes are required")
    if before["state"] != report["state_sha256"]:
        return result("INCONCLUSIVE", "state identity differs from the observed input file")
    if before != after:
        return result("FAIL", "a ROM, state, or existing save file changed")
    results = {}
    for name, state in CASES.items():
        try:
            results[name] = compare_case(root, report.get("cases", {}).get(name, {}), state, name == "count-boundaries")
        except (OSError, ValueError, KeyError, TypeError) as error:
            results[name] = result("INCONCLUSIVE", f"missing or malformed evidence: {error}")
    verdicts = [item["verdict"] for item in results.values()]
    verdict = "FAIL" if "FAIL" in verdicts else "INCONCLUSIVE" if "INCONCLUSIVE" in verdicts else "PASS"
    return {"verdict": verdict, "cases": results}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("report", type=Path)
    args = parser.parse_args()
    outcome = compare(args.report.parent, json.loads(args.report.read_text()))
    print(json.dumps(outcome, indent=2))
    return {"PASS": 0, "FAIL": 1, "INCONCLUSIVE": 2}[outcome["verdict"]]


if __name__ == "__main__":
    raise SystemExit(main())
