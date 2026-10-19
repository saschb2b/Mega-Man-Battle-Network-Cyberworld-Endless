"""Local-only moderation. Filesystem access to the private state is authority."""

import argparse
import contextlib
import json
import os
from pathlib import Path
import time

from .week import week_of
from .identity import KICK_MINUTES, kick_expires, prune_kicks, public_id, read_json, write_json

ACTIONS = ("list", "ban", "unban", "mute", "unmute", "kick", "unkick", "hide", "unhide")
KICK_MAX_MINUTES = 7 * 24 * 60


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


def perform(action, directory, identity=None, reason="", minutes=None, week=None):
    directory = Path(directory)
    if not (directory / "identity.key").is_file():
        raise ValueError("Use the running server's private state directory")
    if minutes is not None and action != "kick":
        raise ValueError("--minutes applies only to kick")
    if week is not None and action != "hide":
        raise ValueError("--week applies only to hide")
    if action == "list":
        return {"roster": read_json(directory / "players.json", {}),
                "moderation": prune_kicks(read_json(directory / "moderation.json", {}), time.time())}
    if not identity:
        raise ValueError("An identity from the local player roster is required")
    identity = public_id(identity)
    if len(reason) > 160 or any(ord(c) < 32 for c in reason):
        raise ValueError("Reason must be at most 160 characters with no control characters")
    minutes = KICK_MINUTES if minutes is None else minutes
    if not 1 <= minutes <= KICK_MAX_MINUTES:
        raise ValueError(f"A kick lasts 1 to {KICK_MAX_MINUTES} minutes; ban for longer")
    with locked(directory):
        now = time.time()
        rules = read_json(directory / "moderation.json", {"bans": {}, "mutes": {}, "kicks": {}, "hidden": {}})
        prune_kicks(rules, now)
        target = {"ban": "bans", "mute": "mutes", "kick": "kicks", "hide": "hidden"}[action.removeprefix("un")]
        entries = rules.setdefault(target, {})
        if action.startswith("un"):
            entries.pop(identity, None)
        else:
            entries[identity] = {"time": now, "reason": reason}
            if action == "kick":
                entries[identity]["expires"] = now + minutes * 60
            if action == "hide":
                # (one week's entry of this identity off the Board, and its
                # posts for that week refused: the current week by default)
                entries[identity]["week"] = week_of(now) if week is None else week
        if len(entries) > 4096:
            raise ValueError("Moderation list is full (4096 entries)")
        write_json(directory / "moderation.json", rules)
    result = {"action": action, "identity": identity, "applies_within_seconds": 1}
    if action == "kick":
        result["expires"] = kick_expires(entries[identity])
    if action == "hide":
        result["week"] = entries[identity]["week"]
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=ACTIONS)
    parser.add_argument("identity", nargs="?")
    parser.add_argument("--state-dir", required=True)
    parser.add_argument("--reason", default="")
    parser.add_argument("--minutes", type=int, help=f"How long a kick refuses the profile (default {KICK_MINUTES})")
    parser.add_argument("--week", type=int, help="The week whose Board entry hide takes down (default this week)")
    args = parser.parse_args()
    try:
        print(json.dumps(perform(args.action, args.state_dir, args.identity, args.reason, args.minutes, args.week), indent=2))
    except (ValueError, OSError) as exc:
        parser.exit(1, str(exc) + "\n")
