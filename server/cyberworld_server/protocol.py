"""Bounded, network-byte-order presence messages. No emulator state enters here."""

import json
import re
import struct
from importlib.resources import files
from pathlib import Path


try:
    _spec_text = files(__package__).joinpath("protocol.json").read_text()
except FileNotFoundError:
    _spec_text = (Path(__file__).resolve().parents[2] / "shared/online/protocol.json").read_text()
SPEC = json.loads(_spec_text)
HELLO = struct.Struct("!BHHI4s20sB")
STATE = struct.Struct("!BiiiB")
PEER = struct.Struct("!IiiiB16s")


class ProtocolError(Exception):
    """A rejected connection, with an actionable, non-sensitive reason."""

    def __init__(self, reason, code=1):
        super().__init__(reason)
        self.code = code


def read_hello(data):
    if len(data) < HELLO.size:
        raise ProtocolError("Expected HELLO")
    kind, version, make, layer_hash, rom, digest, length = HELLO.unpack_from(data)
    if kind != SPEC["hello"] or len(data) != HELLO.size + length:
        raise ProtocolError("Malformed HELLO")
    if version != SPEC["version"]:
        raise ProtocolError("Protocol version mismatch", 2)
    if make != SPEC["layer_make"] or layer_hash != SPEC["layer_hash"]:
        raise ProtocolError("Layer generation mismatch", 3)
    if rom != SPEC["rom_code"].encode() or digest.hex() != SPEC["rom_sha1"]:
        raise ProtocolError("BN6 identity mismatch", 4)
    name = data[HELLO.size:]
    if not re.fullmatch(rb"[A-Za-z0-9][A-Za-z0-9 _-]{0,15}", name):
        raise ProtocolError("Use 1-16 ASCII letters, digits, spaces, _ or -", 5)
    return name.decode("ascii").rstrip()


def hello(name, **overrides):
    spec = dict(SPEC, **overrides)
    name = name.encode("ascii")
    return HELLO.pack(spec["hello"], spec["version"], spec["layer_make"],
                      spec["layer_hash"], spec["rom_code"].encode(),
                      bytes.fromhex(spec["rom_sha1"]), len(name)) + name


def read_state(data):
    if len(data) != STATE.size or data[0] != SPEC["state"]:
        raise ProtocolError("Malformed STATE")
    _, x, y, z, facing = STATE.unpack(data)
    if any(abs(value) > SPEC["coordinate_limit"] for value in (x, y, z)) or facing > 7:
        raise ProtocolError("Position or facing outside limits", 6)
    return x, y, z, facing


def snapshot(players, recipient):
    peers = [player for player in players.values() if player.id != recipient]
    data = bytes((SPEC["snapshot"], len(peers)))
    return data + b"".join(PEER.pack(p.id, *p.position, p.name.encode()) for p in peers)


def error(exc):
    return struct.pack("!BH", SPEC["error"], exc.code) + str(exc).encode("ascii")[:96]
