#!/usr/bin/env python3
"""Run the pinned native folder editor against the immutable trade reader.

Requires the player's Gregar USA ROM, a local pristine map-side boot state,
and the repository's cyberworld-build Docker image. All evidence stays local.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path
import subprocess
import sys

from online_trade_compare import CASES, ROM_SHA1, compare

ROOT = Path(__file__).resolve().parents[1]
IMAGE = "cyberworld-build"
LIBRARY = "/opt/mgba/host/lib/libmgba.a"
MUTATIONS = {
    "equipped-as-spares": ("first-folder", "memset(out->equipped, 255, sizeof out->equipped);"),
    "omit-second-folder": ("second-folder", "memset(out->equipped + BN6_FOLDER_ENTRIES, 255, 2 * BN6_FOLDER_ENTRIES);"),
    "omit-extra-folder": ("extra-folder", "memset(out->equipped + 2 * BN6_FOLDER_ENTRIES, 255, 2 * BN6_FOLDER_ENTRIES);"),
    "collapse-codes": ("first-folder", "out->chips[1].codes[1] = 255;"),
    "write-observed-ram": ("first-folder", "emu_write8(BN6_EWRAM_END - 1, emu_read8(BN6_EWRAM_END - 1) ^ 1);"),
    "live-backed-snapshot": ("first-folder", None),
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        while chunk := stream.read(1024 * 1024):
            digest.update(chunk)
    return digest.hexdigest()


def docker(*command: str, rom: Path | None = None, state: Path | None = None, **kwargs):
    args = ["docker", "run", "--rm", "--network", "none", "-v", f"{ROOT}:/src", "-w", "/src"]
    if rom:
        args += ["-v", f"{rom}:/input/game.gba:ro", "-v", f"{state}:/input/boot.state:ro"]
    return subprocess.run(args + [IMAGE, *command], check=True, **kwargs)


def build_probe(output: Path, source: Path | None = None) -> None:
    args = ["make", "WERROR=1", f"TRADE_PROBE={output.relative_to(ROOT)}"]
    if source:
        args.append(f"TRADE_CAPTURE_SOURCE={source.relative_to(ROOT)}")
    args.append(str(output.relative_to(ROOT)))
    docker(*args, stdout=subprocess.DEVNULL)


def lane(probe: Path, path: Path, name: str, capture: bool, rom: Path, state: Path,
         evidence: Path, live_mutation: bool = False) -> dict:
    path.mkdir(parents=True, exist_ok=True)
    count, active = CASES[name]
    docker(str(probe.relative_to(ROOT)), "/input/game.gba", "/input/boot.state",
           "/src/" + str(path.relative_to(ROOT)), str(count), str(active),
           str(int(name == "count-boundaries")), str(int(capture)), str(int(live_mutation)),
           rom=rom, state=state)
    observed = json.loads((path / "observation.json").read_text())
    files = {"ram": "ram.bin", "counts": "counts.bin", "prepared_before": "prepared-before.bin",
             "prepared_after": "prepared-after.bin", "retained_counts": "retained-counts.bin"}
    observed.update({key: str((path / file).relative_to(evidence)) for key, file in files.items()})
    return observed


def save_report(path: Path, report: dict) -> dict:
    path.write_text(json.dumps(report, indent=2) + "\n")
    outcome = compare(path.parent, report)
    path.with_suffix(".verdict.json").write_text(json.dumps(outcome, indent=2) + "\n")
    return outcome


def mutate(report: dict, evidence: Path, probe: Path, rom: Path, state: Path) -> dict:
    source = (ROOT / "src/online/online_trade_capture.c").read_text()
    outcomes = {}
    for mutation, (name, injection) in MUTATIONS.items():
        variant = evidence / mutation
        variant.mkdir(exist_ok=True)
        selected = probe
        if injection:
            marker = "\tout->ready = true;"
            if source.count(marker) != 1:
                raise ValueError("capture source changed: mutation insertion must be reviewed")
            target = variant / "online_trade_capture.c"
            target.write_text(source.replace(marker, "\t" + injection + "\n" + marker))
            selected = variant / "probe"
            build_probe(selected, target)
        broken = copy.deepcopy(report)
        broken["probe_sha256"] = sha256(selected)
        broken["cases"][name]["subject"] = lane(selected, variant / name, name, True, rom, state,
                                                  evidence, live_mutation=injection is None)
        outcome = save_report(evidence / f"{mutation}.json", broken)
        outcomes[mutation] = {"verdict": outcome["verdict"], "case": name,
                              "detail": outcome.get("cases", {}).get(name, {})}
        print(f"{mutation}: {outcome['verdict']}", flush=True)
    return outcomes


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", type=Path, default=Path.home() / ".cache/mmbn-ref/roms/bn6-gregar-us.gba")
    parser.add_argument("--state", type=Path, required=True)
    parser.add_argument("--output", type=Path, default=ROOT / ".build/trade-proof")
    parser.add_argument("--mutations", action="store_true")
    args = parser.parse_args()
    rom, state, evidence = args.rom.resolve(), args.state.resolve(), args.output.resolve()
    if hashlib.sha1(rom.read_bytes()).hexdigest() != ROM_SHA1:
        parser.error("wrong target: expected the pinned Gregar USA ROM")
    if not evidence.is_relative_to(ROOT / ".build"):
        parser.error("evidence must remain under the ignored .build directory")
    evidence.mkdir(parents=True, exist_ok=True)
    probe = evidence / "probe"
    build_probe(probe)
    before = {"rom": sha256(rom), "state": sha256(state)}
    report = {"schema": "online-trade-proof/1", "rom_sha1": ROM_SHA1, "state_sha256": before["state"],
              "mgba": "0.10.5", "probe_sha256": sha256(probe),
              "mgba_library_sha256": docker("sha256sum", LIBRARY, text=True, capture_output=True).stdout.split()[0],
              "comparator_sha256": sha256(ROOT / "tools/online_trade_compare.py"),
              "input_hashes_before": before, "cases": {},
              "fixture": {"source": "tools/online_trade_probe.c", "sha256": sha256(ROOT / "tools/online_trade_probe.c"),
                          "construction": "native first folder copied; slot 1 and 2 replace Cannon A by native first-code IDs 2 and 3; folder kinds 0/1/2; names copied locally; totals include all folders plus 1/2/3 spares; boundary total 99",
                          "inputs": "native START, ChipFolder, EQUIP each required active slot, exit to map, capture, START, ChipFolder, EDIT first folder"}}
    for name in CASES:
        report["cases"][name] = {key: lane(probe, evidence / name / key, name, key == "subject",
                                          rom, state, evidence) for key in ("reference", "control", "subject")}
        print(f"observed {name}", flush=True)
    report["input_hashes_after"] = {"rom": sha256(rom), "state": sha256(state)}
    outcome = save_report(evidence / "report.json", report)
    print(json.dumps(outcome, indent=2))
    if outcome["verdict"] != "PASS":
        return 1
    if args.mutations:
        mutations = mutate(report, evidence, probe, rom, state)
        (evidence / "mutations.json").write_text(json.dumps(mutations, indent=2) + "\n")
        if any(item["verdict"] != "FAIL" for item in mutations.values()):
            return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
