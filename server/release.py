#!/usr/bin/env python3
"""Package only the ROM-free server and its operator files for a release."""
import argparse
import importlib.metadata
import json
import os
from pathlib import Path
import platform
import subprocess
import sys
import zipfile

from build import build, zip_entry
from smoke import smoke

ROOT = Path(__file__).resolve().parents[1]
TARGETS = {("Linux", "x86_64"): "linux-x86_64", ("Linux", "aarch64"): "linux-arm64",
           ("Windows", "AMD64"): "windows-x64", ("Darwin", "x86_64"): "macos-x86_64",
           ("Darwin", "arm64"): "macos-arm64"}


def native(work):
    if importlib.metadata.version("pyinstaller") != "6.16.0":
        raise RuntimeError("Install the pinned server/requirements-build.txt first")
    name = "cyberworld-server"
    args = [sys.executable, "-m", "PyInstaller", "--noconfirm", "--clean", "--onefile", "--noupx",
            "--name", name, "--distpath", str(work / "dist"), "--workpath", str(work / "work"),
            "--specpath", str(work), "--paths", str(ROOT / "server"),
            "--add-data", str(ROOT / "shared/online/protocol.json") + ":cyberworld_server",
            "--add-data", str(ROOT / "shared/online/trade.json") + ":cyberworld_server",
            str(ROOT / "server/main.py")]
    environment = dict(os.environ, PYTHONHASHSEED="0", SOURCE_DATE_EPOCH="315532800",
                       PYINSTALLER_CONFIG_DIR=str(work / "cache"))
    subprocess.run(args, cwd=ROOT, env=environment, check=True)
    return work / "dist" / (name + (".exe" if os.name == "nt" else ""))


def archive(executable, target, output):
    output.mkdir(parents=True, exist_ok=True)
    destination = output / f"cyberworld-server-{target}.zip"
    with zipfile.ZipFile(destination, "w") as bundle:
        zip_entry(bundle, executable.name, executable.read_bytes(), executable=True)
        for source in (ROOT / "LICENSE", ROOT / "server/README.md", ROOT / "server/server.example.ini"):
            zip_entry(bundle, source.name, source.read_text(encoding="utf-8").encode())
        metadata = {"target": target, "python": platform.python_version(),
                    "protocol": json.loads((ROOT / "shared/online/protocol.json").read_text())["version"]}
        if target != "python":
            metadata["pyinstaller"] = importlib.metadata.version("pyinstaller")
            for source in sorted((ROOT / "server/licenses").iterdir()):
                zip_entry(bundle, "licenses/" + source.name, source.read_bytes())
            package = importlib.metadata.distribution("pyinstaller")
            notice = package.locate_file(next(path for path in package.files if str(path).endswith("/COPYING.txt")))
            zip_entry(bundle, "licenses/PyInstaller.txt", notice.read_bytes())
        zip_entry(bundle, "BUILD.json", (json.dumps(metadata, indent=2, sort_keys=True) + "\n").encode())
    print(destination)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--native", action="store_true", help="Freeze on this OS/architecture using pinned PyInstaller")
    parser.add_argument("--output", type=Path, default=ROOT / "build/release")
    args = parser.parse_args()
    target = TARGETS.get((platform.system(), platform.machine())) if args.native else "python"
    if target is None:
        parser.error("Unsupported server release OS/architecture")
    work = ROOT / "build/server-release" / target
    work.mkdir(parents=True, exist_ok=True)
    executable = native(work) if args.native else build(work / "cyberworld-server.pyz")
    smoke(executable)
    archive(executable, target, args.output)


if __name__ == "__main__":
    main()
