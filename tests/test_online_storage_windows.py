"""Run the disposable Win32 storage API probe; never infer durability."""
from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import tempfile


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=pathlib.Path, required=True)
    parser.add_argument("--wine", action="store_true", help="API compatibility observations only; not Windows runtime evidence")
    parser.add_argument("--report", type=pathlib.Path, default=pathlib.Path(".build/online-storage-windows/report.json"))
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="cw-storage-") as temporary:
        root = pathlib.Path(temporary) / "日本語-checkpoint"
        destination = str(root)
        command = [str(args.binary.resolve())]
        if args.wine:
            destination = subprocess.run(["winepath", "-w", str(root)], text=True, capture_output=True, check=True, timeout=30).stdout.strip()
            command.insert(0, "wine")
        result = subprocess.run([*command, destination], text=True, capture_output=True, timeout=30)
        rows = [json.loads(line) for line in result.stdout.splitlines() if line.startswith("{")]
        report = {"runtime": "Wine" if args.wine else "Windows", "exit_code": result.returncode,
                  "observations": rows, "stderr": result.stderr, "durability_certified": False}
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        assert result.returncode == 0, report
        assert rows and rows[-1] == {"summary": True, "failed_checks": 0, "durability_certified": False}, report
        assert len([row for row in rows if row.get("operation")]) == 17, report
        assert not root.exists(), "probe left disposable files behind"
        assert all(row["ok"] for row in rows if row.get("required")), report
    print(f"Win32 storage API observations passed; durability remains uncertified. Report: {args.report}")


if __name__ == "__main__":
    main()
