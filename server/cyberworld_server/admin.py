"""Local-only moderation. Filesystem access to the private state is authority."""

import argparse
import contextlib
import json
import os
from pathlib import Path
import time

from .identity import public_id, read_json, write_json

ACTIONS = ("list", "ban", "unban", "mute", "unmute", "kick")


@contextlib.contextmanager
def locked(directory):
    descriptor = os.open(directory / "moderation.lock", os.O_RDWR | os.O_CREAT, 0o600)
    with os.fdopen(descriptor, "r+b") as stream:
        if os.name == "nt":
            import msvcrt
            if not stream.read(1):
                stream.write(b"0")
                stream.flush()
            stream.seek(0)
            msvcrt.locking(stream.fileno(), msvcrt.LK_LOCK, 1)
        else:
            import fcntl
            fcntl.flock(stream, fcntl.LOCK_EX)
        yield


def perform(action, directory, identity=None, reason=""):
    directory = Path(directory)
    if not (directory / "identity.key").is_file():
        raise ValueError("Use the running server's private state directory")
    if action == "list":
        return {"roster": read_json(directory / "players.json", {}),
                "moderation": read_json(directory / "moderation.json", {})}
    if not identity:
        raise ValueError("An identity from the local player roster is required")
    identity = public_id(identity)
    if len(reason) > 160 or any(ord(c) < 32 for c in reason):
        raise ValueError("Reason must be at most 160 characters with no control characters")
    with locked(directory):
        rules = read_json(directory / "moderation.json", {"bans": {}, "mutes": {}, "kicks": {}})
        target = "bans" if action in ("ban", "unban") else "mutes" if action in ("mute", "unmute") else "kicks"
        entries = rules.setdefault(target, {})
        if action.startswith("un"):
            entries.pop(identity, None)
        else:
            entries[identity] = {"time": time.time(), "reason": reason}
        if len(entries) > 4096:
            raise ValueError("Moderation list is full (4096 entries)")
        write_json(directory / "moderation.json", rules)
    return {"action": action, "identity": identity, "applies_within_seconds": 1}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=ACTIONS)
    parser.add_argument("identity", nargs="?")
    parser.add_argument("--state-dir", required=True)
    parser.add_argument("--reason", default="")
    args = parser.parse_args()
    try:
        print(json.dumps(perform(args.action, args.state_dir, args.identity, args.reason), indent=2))
    except (ValueError, OSError) as exc:
        parser.exit(1, str(exc) + "\n")
