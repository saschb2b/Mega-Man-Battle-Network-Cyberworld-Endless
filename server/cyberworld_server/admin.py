"""Local-only moderation. Filesystem access to the private state is authority.

`forget` answers a player's request to be removed (server/README.md, The
official server): his public identity out of every record that names it,
but a ban or a mute, which stays while it lasts."""

import argparse
import json
from pathlib import Path
import time

from .week import compact, week_of
from .identity import (KICK_MINUTES, kick_expires, prune_kicks, prune_moderation, public_id, read_json,
                       state_lock, write_json)
from .retention import PlayersLog
from .trade import prune_log

ACTIONS = ("list", "ban", "unban", "mute", "unmute", "kick", "unkick", "hide", "unhide", "forget")
KICK_MAX_MINUTES = 7 * 24 * 60


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
    if action == "forget":
        return forget(directory, identity)
    if len(reason) > 160 or any(ord(c) < 32 for c in reason):
        raise ValueError("Reason must be at most 160 characters with no control characters")
    minutes = KICK_MINUTES if minutes is None else minutes
    if not 1 <= minutes <= KICK_MAX_MINUTES:
        raise ValueError(f"A kick lasts 1 to {KICK_MAX_MINUTES} minutes; ban for longer")
    with state_lock(directory):
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


def forget(directory, identity):
    """A public identity taken out of the trades' log (its side of each
    trade blanked, the other's kept), the Board's log, the players' logs
    and moderation.json's kicks and hidden entries. The running server
    shows its entries on this week's Board until it restarts; the backups
    hold them until they age out."""
    _, trades = prune_log(directory, 0, forget=identity)
    _, board = compact(directory, 0, forget=identity)
    logs = PlayersLog(directory).forget(identity)
    moderation = prune_moderation(directory, time.time(), forget=identity)
    return {"action": "forget", "identity": identity, "trades": trades, "board": board, "logs": logs,
            "moderation": moderation, "then": "restart the server, which reads the Board only as it starts"}


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
