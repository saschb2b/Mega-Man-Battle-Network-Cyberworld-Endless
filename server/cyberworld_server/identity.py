"""Private local server state: a salted public identity and moderation rules."""

import hashlib
import hmac
import json
import logging
import os
from pathlib import Path
import secrets
import tempfile
import time

MAX_STATE_BYTES = 1024 * 1024
MODERATION = ("bans", "mutes", "kicks", "hidden")   # (hidden: a week's Board entry taken down, week.py)
KICK_MINUTES = 10
LOG = logging.getLogger("cyberworld.server")
_UNSET = object()


def write_json(path, value, durable=True):
    encoded = json.dumps(value, indent=2, sort_keys=True) + "\n"
    if len(encoded) > MAX_STATE_BYTES:
        raise ValueError("Server state exceeds 1 MiB")
    descriptor, temporary = tempfile.mkstemp(prefix=".server-", dir=path.parent)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8") as stream:
            stream.write(encoded)
            stream.flush()
            if durable:
                os.fsync(stream.fileno())
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)


def read_json(path, default):
    try:
        with path.open("r", encoding="utf-8") as stream:
            encoded = stream.read(MAX_STATE_BYTES + 1)
    except FileNotFoundError:
        return default
    if len(encoded) > MAX_STATE_BYTES:
        raise ValueError("Server state exceeds 1 MiB")
    value = json.loads(encoded)
    if not isinstance(value, dict):
        raise ValueError("Server state must be a JSON object")
    return value


def public_id(text):
    if len(text) != 32:
        raise ValueError("Public identity must be 32 hexadecimal characters")
    try:
        normalized = bytes.fromhex(text).hex()
        if normalized != text.lower():
            raise ValueError("Noncanonical public identity")
        return normalized
    except ValueError as exc:
        raise ValueError("Public identity must be 32 hexadecimal characters") from exc


def empty_rules():
    return {action: {} for action in MODERATION}


def checked_rules(rules):
    """moderation.json's lists, or ValueError naming what is wrong with them."""
    for action in MODERATION:
        entries = rules.setdefault(action, {})
        if not isinstance(entries, dict) or len(entries) > 4096:
            raise ValueError("Invalid moderation list")
        for identity, entry in entries.items():
            public_id(identity)
            if not isinstance(entry, dict) or not isinstance(entry.get("time"), (float, int)):
                raise ValueError("Invalid moderation entry")
            if not isinstance(entry.get("expires", 0), (float, int)):
                raise ValueError("Invalid kick expiry")
            if not isinstance(entry.get("week", 0), int):
                raise ValueError("Invalid hidden entry's week")
    return rules


def kick_expires(entry):
    """A kick lasts until its expiry; one written before kicks expired lasts the default."""
    return entry.get("expires", entry["time"] + KICK_MINUTES * 60)


def prune_kicks(rules, now):
    kicks = rules.get("kicks", {})
    for identity in [identity for identity, entry in kicks.items() if kick_expires(entry) <= now]:
        del kicks[identity]
    return rules


class IdentityStore:
    def __init__(self, directory=None):
        self.directory = Path(directory) if directory else None
        self.secret = secrets.token_bytes(32)
        self.rules = empty_rules()
        self.stamp = None       # moderation.json's stat when last read
        self.failed = _UNSET    # the stat of a file that would not load, logged once
        self.loads = 0
        if self.directory:
            self.directory.mkdir(mode=0o700, parents=True, exist_ok=True)
            secret_path = self.directory / "identity.key"
            try:
                descriptor = os.open(secret_path, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
            except FileExistsError:
                self.secret = secret_path.read_bytes()
            else:
                with os.fdopen(descriptor, "wb") as stream:
                    stream.write(self.secret)
                    stream.flush()
                    os.fsync(stream.fileno())
            if len(self.secret) != 32:
                raise ValueError("Invalid server identity.key; restore the server backup")
            # A bad file at startup refuses the start: there is no good copy to keep.
            self.reload()

    def identity(self, key):
        return hmac.new(self.secret, key, hashlib.sha256).digest()[:16]

    def _stamp(self):
        try:
            stat = os.stat(self.directory / "moderation.json")
        except FileNotFoundError:
            return None
        return stat.st_mtime_ns, stat.st_size, stat.st_ino, stat.st_dev

    def reload(self):
        """Read moderation.json now; ValueError or OSError if it cannot be used."""
        if not self.directory:
            return
        stamp = self._stamp()
        rules = checked_rules(read_json(self.directory / "moderation.json", empty_rules())) if stamp else empty_rules()
        self.rules, self.stamp, self.failed = rules, stamp, _UNSET
        self.loads += 1

    def refresh(self):
        """Re-read moderation.json only when its stat changed. Never raises: a
        file that will not load keeps the last good rules and is logged once."""
        if not self.directory:
            return
        try:
            stamp = self._stamp()
        except OSError as exc:
            stamp, error = "unreadable", exc
        else:
            if stamp == self.stamp or stamp == self.failed:
                return
            try:
                self.reload()
                return
            except (OSError, ValueError) as exc:
                error = exc
        if stamp != self.failed:
            self.failed = stamp
            LOG.error("moderation.json not loaded, keeping the last good rules: %s", error)

    def contains(self, action, identity):
        return identity.hex() in self.rules.get(action, {})

    def kick(self, identity, now):
        """The kick on this identity while it lasts, else None."""
        entry = self.rules.get("kicks", {}).get(identity.hex())
        return entry if entry and kick_expires(entry) > now else None

    def roster(self, entries):
        """The local roster for moderators: rewritten often, so not fsynced."""
        if self.directory:
            write_json(self.directory / "players.json", {"updated": time.time(), "players": list(entries)}, durable=False)
