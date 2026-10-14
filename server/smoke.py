#!/usr/bin/env python3
"""Run an isolated packaged server, checking help, health and local administration."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time
import urllib.request


def wait_port(process, events):
    deadline = time.monotonic() + 15
    while time.monotonic() < deadline:
        if process.poll() is not None:
            raise RuntimeError("Packaged server exited before listening")
        if events.exists():
            for line in events.read_text(encoding="utf-8").splitlines():
                try:
                    event = json.loads(line)
                except json.JSONDecodeError:
                    continue
                if event.get("event") == "listening":
                    return event["port"]
        time.sleep(0.05)
    raise RuntimeError("Packaged server did not listen within 15 seconds")


def stop(process):
    if process.poll() is not None:
        return
    if os.name == "nt":
        # A one-file PyInstaller executable has a child. Kill only this test's
        # process tree, including the bootloader, before deleting its temp files.
        subprocess.run(["taskkill", "/PID", str(process.pid), "/T", "/F"],
                       check=True, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
    else:
        process.terminate()
    try:
        process.wait(timeout=10)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait(timeout=5)
        raise RuntimeError("Packaged server did not stop within 10 seconds")


def smoke(executable):
    executable = executable.resolve()
    command = [sys.executable, str(executable)] if executable.suffix == ".pyz" else [str(executable)]
    spec = json.loads((Path(__file__).resolve().parents[1] / "shared/online/protocol.json").read_text())
    with tempfile.TemporaryDirectory(prefix="cyberworld-package-") as directory:
        work = Path(directory)
        # A different cwd proves protocol.json was actually bundled.
        result = subprocess.run(command + ["--help"], cwd=work, capture_output=True, text=True, timeout=20, check=True)
        if "ROM-free" not in result.stdout:
            raise RuntimeError("Unexpected packaged server help")
        events, log = work / "events.jsonl", work / "process.log"
        with log.open("w", encoding="utf-8") as output:
            process = subprocess.Popen(command + ["--host", "127.0.0.1", "--port", "0", "--seed", "120",
                                                  "--state-dir", str(work / "private"), "--event-log", str(events)],
                                       cwd=work, stdout=output, stderr=subprocess.STDOUT)
            try:
                port = wait_port(process, events)
                opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))
                with opener.open(f"http://127.0.0.1:{port}/health", timeout=5) as response:
                    health = json.load(response)
                if health.get("protocol") != spec["version"] or health.get("players") != 0:
                    raise RuntimeError("Wrong protocol or occupancy in packaged health response: " + repr(health))
                admin = subprocess.run(command + ["--state-dir", str(work / "private"), "--admin", "list"],
                                       cwd=work, capture_output=True, text=True, timeout=20, check=True)
                if not isinstance(json.loads(admin.stdout), dict):
                    raise RuntimeError("Invalid packaged administration response")
            except Exception:
                output.flush()
                print(log.read_text(encoding="utf-8"), file=sys.stderr)
                raise
            finally:
                stop(process)
    print(f"Packaged server passed: {executable.name} (protocol {spec['version']})")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    smoke(parser.parse_args().executable)
