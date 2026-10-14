"""Private local server state: a salted public identity and moderation rules."""

import hashlib
import hmac
import json
import os
from pathlib import Path
import secrets
import tempfile
import time

MAX_STATE_BYTES = 1024 * 1024


def write_json(path, value):
    encoded = json.dumps(value, indent=2, sort_keys=True) + "\n"
    if len(encoded) > MAX_STATE_BYTES:
        raise ValueError("Server state exceeds 1 MiB")
    descriptor, temporary = tempfile.mkstemp(prefix=".server-", dir=path.parent)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8") as stream:
            stream.write(encoded)
            stream.flush()
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


class IdentityStore:
    def __init__(self, directory=None):
        self.directory = Path(directory) if directory else None
        self.secret = secrets.token_bytes(32)
        self.rules = {"bans": {}, "mutes": {}, "kicks": {}}
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
            self.reload()

    def identity(self, key):
        return hmac.new(self.secret, key, hashlib.sha256).digest()[:16]

    def reload(self):
        if self.directory:
            rules = read_json(self.directory / "moderation.json", self.rules)
            for action in ("bans", "mutes", "kicks"):
                entries = rules.get(action, {})
                if not isinstance(entries, dict) or len(entries) > 4096:
                    raise ValueError("Invalid moderation list")
                for identity, entry in entries.items():
                    public_id(identity)
                    if not isinstance(entry, dict) or not isinstance(entry.get("time"), (float, int)):
                        raise ValueError("Invalid moderation entry")
            self.rules = rules

    def contains(self, action, identity):
        return identity.hex() in self.rules.get(action, {})

    def kicked(self, identity, joined):
        return self.rules.get("kicks", {}).get(identity.hex(), {}).get("time", 0) >= joined

    def roster(self, players):
        if self.directory:
            write_json(self.directory / "players.json", {"updated": time.time(), "players": [
                {"id": player.id, "identity": player.identity.hex(), "name": player.name, "channel": player.channel}
                for player in players.values()]})
