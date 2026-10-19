"""Bounded, network-byte-order presence messages. No emulator state enters here."""

import json
import re
import struct
import secrets
from importlib.resources import files
from pathlib import Path


try:
    _spec_text = files(__package__).joinpath("protocol.json").read_text()
except FileNotFoundError:
    _spec_text = (Path(__file__).resolve().parents[2] / "shared/online/protocol.json").read_text()
SPEC = json.loads(_spec_text)
HELLO = struct.Struct("!BHHI4s20s4s20s16sHB")
STATE = struct.Struct("!BiiiB")
PEER = struct.Struct("!I16siiiB16sB")
PROFILE = struct.Struct("!HHBBBHHHBHI49s")
WELCOME = struct.Struct("!BIIHBH16sHIH")
TRADE = struct.Struct("!BBIHBHB")
TRADE_STATE = struct.Struct("!BBII16sHBHB")
ARENA = struct.Struct("!BBIHB")
ARENA_STATE = struct.Struct("!BBII16sBBB")
ARENA_INPUT = struct.Struct("!BIIB")
ARENA_HASH = struct.Struct("!BIIQ")
ARENA_LOADOUT = struct.Struct("!BIH")
OFFER = struct.Struct("!BHBB")
BOARD_ENTRY = struct.Struct("!IHBB")
DEFAULT_PROFILE = PROFILE.pack(100, 100, 1, 1, 1, 0, 0, 0, 0, 0, 0, bytes(49))


class ProtocolError(Exception):
    """A rejected connection, with an actionable, non-sensitive reason."""

    def __init__(self, reason, code=1):
        super().__init__(reason)
        self.code = code


def read_hello(data):
    """HELLO checked: its name, guest key, channel and the ROMs its game has
    (protocol.json's roms_bn5 bit where BN5 Team Colonel's identity came)."""
    if len(data) < HELLO.size:
        raise ProtocolError("Expected HELLO")
    kind, version, make, square_hash, rom, digest, rom5, digest5, key, channel, length = HELLO.unpack_from(data)
    if kind != SPEC["hello"] or len(data) != HELLO.size + length:
        raise ProtocolError("Malformed HELLO")
    if version != SPEC["version"]:
        raise ProtocolError("Protocol version mismatch", 2)
    if make != SPEC["square_make"] or square_hash != SPEC["square_hash"]:
        raise ProtocolError("Square generation mismatch", 3)
    if rom != SPEC["rom_code"].encode() or digest.hex() != SPEC["rom_sha1"]:
        raise ProtocolError("BN6 identity mismatch", 4)
    # (BN5's: none, all zeros, or Team Colonel's exactly as BN6's is checked)
    bn5 = any(rom5) or any(digest5)
    if bn5 and (rom5 != SPEC["bn5_code"].encode() or digest5.hex() != SPEC["bn5_sha1"]):
        raise ProtocolError("BN5 identity mismatch", 15)
    if not any(key):
        raise ProtocolError("A random guest key is required", 9)
    name = data[HELLO.size:]
    if not re.fullmatch(rb"[A-Za-z0-9][A-Za-z0-9 _-]{0,15}", name):
        raise ProtocolError("Use 1-16 ASCII letters, digits, spaces, _ or -", 5)
    return name.decode("ascii").rstrip(), key, channel, SPEC["roms_bn5"] if bn5 else 0


def hello(name, key=None, channel=0, bn5=False, **overrides):
    spec = dict(SPEC, **overrides)
    name = name.encode("ascii")
    rom5 = (spec["bn5_code"].encode(), bytes.fromhex(spec["bn5_sha1"])) if bn5 else (bytes(4), bytes(20))
    return HELLO.pack(spec["hello"], spec["version"], spec["square_make"],
                      spec["square_hash"], spec["rom_code"].encode(),
                      bytes.fromhex(spec["rom_sha1"]), *rom5, key or secrets.token_bytes(16), channel, len(name)) + name


def read_state(data):
    if len(data) != STATE.size or data[0] != SPEC["state"]:
        raise ProtocolError("Malformed STATE")
    _, x, y, z, facing = STATE.unpack(data)
    if any(abs(value) > SPEC["coordinate_limit"] for value in (x, y, z)) or facing > 7:
        raise ProtocolError("Position or facing outside limits", 6)
    return x, y, z, facing


def snapshot(players, recipient):
    room = players[recipient].channel
    peers = [player for player in players.values() if player.id != recipient and player.channel == room]
    data = bytes((SPEC["snapshot"], len(peers)))
    return data + b"".join(PEER.pack(p.id, p.identity, *p.position, p.name.encode(), p.roms) for p in peers)


def read_profile(data):
    if len(data) != 1 + PROFILE.size or data[0] != SPEC["profile"]:
        raise ProtocolError("Malformed PROFILE")
    hp, maximum, attack, speed, charge, souls, crosses, depth, act, best, marks, board = PROFILE.unpack(data[1:])
    if not 1 <= maximum <= 9999 or not 0 <= hp <= maximum:
        raise ProtocolError("Invalid profile HP")
    if max(attack, speed, charge) > 5 or souls > 63 or crosses > 31 or marks > 65535 or max(board) > 6:
        raise ProtocolError("Invalid profile values")
    return data[1:]


def read_social(data):
    if len(data) != 3 or data[0] != SPEC["social"]:
        raise ProtocolError("Malformed SOCIAL")
    kind, choice = data[1:]
    count = SPEC["phrase_count"] if kind == 0 else SPEC["emote_count"]
    if kind > 1 or choice >= count:
        raise ProtocolError("Invalid phrase or emote")
    return kind, choice


def error(exc):
    return struct.pack("!BH", SPEC["error"], exc.code) + str(exc).encode("ascii")[:96]


def chip_valid(chip, code):
    """A chip id of BN6's tables as a game with BN5 grows them (to 511) and
    a code (A..Z, * as 26); whose chip it is, chip_bn5 says."""
    return 1 <= chip <= SPEC["chip_last"] and 0 <= code <= SPEC["code_last"]


def chip_bn5(chip):
    """A chip only a game with BN5's ROM holds: past BN6's own ids, or one of
    BN6's blank records, where BN5's chips are installed (src/emu/bn6_chips.h)."""
    return chip > SPEC["bn6_chip_last"] or any(low <= chip <= high for low, high in SPEC["bn6_blank"])


def read_trade(data):
    if len(data) != TRADE.size or data[0] != SPEC["trade"]:
        raise ProtocolError("Malformed TRADE")
    _, action, ref, give, give_code, get, get_code = TRADE.unpack(data)
    if not 1 <= action <= 5 or not ref:
        raise ProtocolError("Invalid trade action")
    if action in (1, 2, 3, 5) and not chip_valid(give, give_code):
        raise ProtocolError("Invalid trade chip")
    if action in (2, 3, 5) and not chip_valid(get, get_code):
        raise ProtocolError("Invalid trade chip")
    return action, ref, (give, give_code), (get, get_code)


def trade_state(stage, trade_id, other, identity, mine, theirs):
    return TRADE_STATE.pack(SPEC["trade_state"], stage, trade_id, other, identity, mine[0], mine[1], theirs[0], theirs[1])


def read_offer(data):
    """OFFER: the chip a player has up on the board and the kind he wants
    for it (0..want_last, the game's words: anything, an element, a kind);
    all zeros take it down."""
    if len(data) != OFFER.size or data[0] != SPEC["offer"]:
        raise ProtocolError("Malformed OFFER")
    _, chip, code, want = OFFER.unpack(data)
    if (chip or code or want) and (not chip_valid(chip, code) or want > SPEC["want_last"]):
        raise ProtocolError("Invalid offer")
    return (chip, code), want


def board(entries):
    """BOARD: a channel's offers as one recipient may see them, (session, chip, want) each."""
    return bytes((SPEC["board"], len(entries))) + b"".join(
        BOARD_ENTRY.pack(player_id, chip[0], chip[1], want) for player_id, chip, want in entries)


def read_arena(data):
    if len(data) != ARENA.size or data[0] != SPEC["arena"]:
        raise ProtocolError("Malformed ARENA")
    _, action, ref, latency, rules = ARENA.unpack(data)
    if not 1 <= action <= 4 or not ref or latency > 5000 or rules > SPEC["format_full"]:
        raise ProtocolError("Invalid arena action")
    return action, ref, latency, rules


def arena_state(stage, match_id, other, identity, side=0, delay=0, rules=0):
    return ARENA_STATE.pack(SPEC["arena_state"], stage, match_id, other, identity, side, delay, rules)


def arena_relay(data):
    """An arena message for the opponent: its match id, its shape checked."""
    kind = data[0] if data else 0
    if kind == SPEC["arena_input"] and len(data) >= ARENA_INPUT.size:
        _, match, frame, count = ARENA_INPUT.unpack_from(data)
        if 1 <= count <= SPEC["arena_input_max"] and len(data) == ARENA_INPUT.size + 2 * count:
            return match
    elif kind == SPEC["arena_hash"] and len(data) == ARENA_HASH.size:
        return ARENA_HASH.unpack(data)[1]
    elif kind == SPEC["arena_loadout"] and len(data) >= ARENA_LOADOUT.size:
        _, match, length = ARENA_LOADOUT.unpack_from(data)
        if 1 <= length <= SPEC["arena_loadout_max"] and len(data) == ARENA_LOADOUT.size + length:
            return match
    raise ProtocolError("Malformed arena relay")
