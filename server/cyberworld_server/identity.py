"""Private local server state: a salted public identity and moderation rules."""

import contextlib
import errno
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


@contextlib.contextmanager
def state_lock(directory):
    """The state directory's lock (its file `moderation.lock`): held by every
    write of moderation.json, by the trades' and the Board's logs as they
    are appended or pruned, and by a backup or a restore, so a moderator's
    command never meets a half-written file. One process or several."""
    descriptor = os.open(Path(directory) / "moderation.lock", os.O_RDWR | os.O_CREAT, 0o600)
    with os.fdopen(descriptor, "r+b") as stream:
        if os.name != "nt":
            import fcntl
            fcntl.flock(stream, fcntl.LOCK_EX)
            yield
            return
        import msvcrt
        # (Windows: the file's first byte, which may be locked past its end
        # and is never read, another holder's lock refusing a read; LK_LOCK
        # gives up after ten seconds, so it is asked again while one holds
        # it, and let go before the file closes)
        while True:
            try:
                msvcrt.locking(stream.fileno(), msvcrt.LK_LOCK, 1)
                break
            except OSError as error:
                if error.errno not in (errno.EACCES, getattr(errno, "EDEADLOCK", errno.EDEADLK)):
                    raise
        try:
            yield
        finally:
            stream.seek(0)
            msvcrt.locking(stream.fileno(), msvcrt.LK_UNLCK, 1)


def write_json(path, value, durable=True):
    encoded = json.dumps(value, indent=2, sort_keys=True) + "\n"
    if len(encoded) > MAX_STATE_BYTES:
        raise ValueError("Server state exceeds 1 MiB")
    write_text(path, encoded, durable)


def write_text(path, encoded, durable=True):
    """Replace a state file whole (text, or bytes): a temporary file beside
    it, readable by its owner alone, then a rename."""
    path = Path(path)
    descriptor, temporary = tempfile.mkstemp(prefix=".server-", dir=path.parent)
    try:
        with os.fdopen(descriptor, "wb" if isinstance(encoded, bytes) else "w",
                       encoding=None if isinstance(encoded, bytes) else "utf-8") as stream:
            stream.write(encoded)
            stream.flush()
            if durable:
                os.fsync(stream.fileno())
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)


def append_log(directory, name, entry):
    """One entry appended to a JSONL log of the state directory, under its lock."""
    with state_lock(directory):
        with open(Path(directory) / name, "a", encoding="utf-8") as log:
            log.write(json.dumps(entry, separators=(",", ":"), sort_keys=True) + "\n")


def rewrite_log(directory, name, edit):
    """A JSONL log of the state directory rewritten under its lock: `edit`
    takes its entries (a line that is not a JSON object is dropped) and
    returns those to keep, changed or not. The file is replaced only where
    that changed it. Returns the entries kept and how many lines went."""
    path = Path(directory) / name
    with state_lock(directory):
        try:
            with path.open("r", encoding="utf-8") as log:
                lines = log.read().splitlines()
        except FileNotFoundError:
            return [], 0
        entries = []
        for line in lines:
            try:
                entry = json.loads(line)
            except ValueError:
                continue
            if isinstance(entry, dict):
                entries.append(entry)
        kept = edit([dict(entry) for entry in entries])
        if kept != entries or len(entries) != len(lines):
            write_text(path, "".join(json.dumps(entry, separators=(",", ":"), sort_keys=True) + "\n" for entry in kept))
        return kept, len(lines) - len(kept)


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


def prune_moderation(directory, now, first_week=None, forget=None):
    """moderation.json without the kicks that ended and the Board entries
    hidden for a week before `first_week` (weeks.jsonl keeps none of them),
    and without `forget`'s kick and hidden entry (a ban or a mute stays
    while it lasts): how many entries went. A file that will not load is
    left as it is; the server keeps its last good rules and says so."""
    path = Path(directory) / "moderation.json"
    with state_lock(directory):
        if not path.exists():
            return 0
        try:
            rules = checked_rules(read_json(path, {}))
        except (OSError, ValueError):
            return 0
        count = sum(len(rules[action]) for action in MODERATION)
        prune_kicks(rules, now)
        hidden = rules["hidden"]
        for identity in [identity for identity, entry in hidden.items()
                         if first_week is not None and entry.get("week", 0) < first_week]:
            del hidden[identity]
        if forget:
            rules["kicks"].pop(forget, None)
            hidden.pop(forget, None)
        gone = count - sum(len(rules[action]) for action in MODERATION)
        if gone:
            write_json(path, rules)
        return gone


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
