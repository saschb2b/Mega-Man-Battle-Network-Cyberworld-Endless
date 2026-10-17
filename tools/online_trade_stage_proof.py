#!/usr/bin/env python3
"""Reproduce the pinned original-routine controls; this is not a staged-swap pass."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import online_trade_stage_compare as compare

ROOT = Path(__file__).resolve().parents[1]
LIBRARY = "/opt/mgba/host/lib/libmgba.a"
# Fixed at observer review. Never regenerate these from candidate runs.
REVIEWED = {
    "tools/online_trade_reload.c": "ed0b8ba29a4ef71bc1cbc19a75b930564b98ded427c313204edcbf28b2ab4377",
    "tools/online_trade_stage_compare.py": "a5333ea68048981275d9e5c61c64388886488f7ff33d196a33bcda7e6e314ad4",
    "tools/online_trade_routines.c": "cbee8a49abe7fb3fb6272617eddf7e95cad7f088311aeeb40c54473a6cc132d7",
    "tools/online_trade_runtime.c": "d715803772265a42a9d84ec4c0352fd6e74b4f0f4c812879261ec447c96a6663",
    "tools/online_trade_runtime.h": "5f3b9866da8fa14c080fd51dcaa6b64286746307c1c28ededfe5634da20f8ade",
    "src/online/online_trade_capture.c": "e837f9a4f00665647a6ae591d33541a89403be63daafffc6acd88c67eb19cd04",
    "src/online/online_trade_capture.h": "44420f421e173f7cfc99648aed4ad576644b817dda84351fa099bcfa21304467",
    "src/online/online_trade_inventory.c": "704329812eb143b10a1f205d10d8917cbab784f226ead19ede09e148af8b3a35",
    "src/online/online_trade_inventory.h": "546bcecba8f9e891e03b05b177054d9766ade964f118268b7b0d00cdc1356bbd",
    "src/online/online_snapshot.c": "9a17dab9dc5bfac9b754c087200c0f60c171ffd0251f8a5048c43212492bb44a",
    "src/online/online_snapshot.h": "77e47a3d9b35147b650346273c6717e7110cfe3c8cfc1c314fbc8ec9b24ae29c",
    "src/emu/bn6.h": "8681082989ce5520e8da66478acee46ea0dec403b68179b300ef3fd1a2f02036",
    "src/emu/bn6_chips.h": "cb98fd2d2f2859c4ed5cbd0af3dd2fa9838bfb8b9b2c1504635ddb874f26e4ba",
    "src/emu/bn6_pet.h": "b3606cfe7fbc8e131befd04369f5ab9980333894f06692386443740fad3d65f1"
}


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def verify_sources():
    current = {name: sha(ROOT / name) for name in REVIEWED}
    compare.require(current == REVIEWED, "Reviewed original observer/comparator changed")


def docker(command, rom=None, state=None):
    args = ["docker", "run", "--rm", "--network", "none", "-v", f"{ROOT}:/src", "-w", "/src"]
    if rom is not None:
        args += ["-v", f"{rom}:/input/game.gba:ro", "-v", f"{state}:/input/boot.state:ro"]
    return subprocess.run(args + ["cyberworld-build", *command], check=True, text=True, capture_output=True)


def controls(rom, state, output):
    verify_sources()
    before = dict(rom_sha256=sha(rom), state_sha256=sha(state),
                  library_sha256=docker(["sha256sum", LIBRARY]).stdout.split()[0])
    compare.require(before == compare.TARGET, "Wrong original target or library")
    # A fresh directory prevents missing observer output from reusing old files.
    output.mkdir(parents=True, exist_ok=False)
    binary = output / "original-routines"
    relative = str(binary.relative_to(ROOT))
    docker(["make", "WERROR=1", f"TRADE_ROUTINE_PROBE={relative}", relative])
    report = dict(oracle="original-routines", native_peer_exchange="INCONCLUSIVE",
                  target_before=before, sources=REVIEWED, binary_sha256=sha(binary), cases={})
    for index, name in enumerate(compare.CASES):
        for repeat in ("original-a", "original-b"):
            lane = output / name / repeat
            lane.mkdir(parents=True)
            docker([relative, "/input/game.gba", "/input/boot.state", str(lane.relative_to(ROOT)), str(index)], rom, state)
        source, result, fields, complete = compare.oracle(output / name / "original-a", output / name / "original-b", name)
        a = compare.editor(output / name / "original-a/editor.counts")
        compare.require(a == compare.editor(output / name / "original-b/editor.counts"), "Original editor controls differ")
        changed = compare.differing(source, result)
        report["cases"][name] = dict(verdict="PASS", source_sha256=compare.digest(source),
            original_sha256=compare.digest(result), editor_sha256=compare.digest(a),
            changed_serialized_bytes=changed,
            inventory_fields={key: dict(offset=at, size=size, before=list(source[at:at+size]), after=list(result[at:at+size]))
                              for key, (at, size) in fields.items()})
        print(f"{name}: original routine controls PASS", flush=True)
    report["target_after"] = dict(rom_sha256=sha(rom), state_sha256=sha(state), library_sha256=docker(["sha256sum", LIBRARY]).stdout.split()[0])
    compare.require(report["target_after"] == before, "Original input changed")
    verify_sources()
    (output / "controls.json").write_text(json.dumps(report, indent=2) + "\n")
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", type=Path, default=Path.home() / ".cache/mmbn-ref/roms/bn6-gregar-us.gba")
    parser.add_argument("--state", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True, help="new directory under .build")
    args = parser.parse_args()
    output = args.output.resolve()
    if not output.is_relative_to(ROOT / ".build"):
        parser.error("all evidence must remain under .build")
    try:
        controls(args.rom.resolve(), args.state.resolve(), output)
    except (OSError, subprocess.CalledProcessError, compare.Mismatch) as error:
        print(f"INCONCLUSIVE: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
