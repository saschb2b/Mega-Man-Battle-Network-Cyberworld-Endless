"""Exact binary codecs for the opt-in trade extension, currently unconnected."""

from importlib.resources import files
import json
from pathlib import Path
import struct

from .protocol import ProtocolError
from .trade_lobby import Carried
from .trade_store import TradeError

try:
    _spec_text = files(__package__).joinpath("trade.json").read_text()
except FileNotFoundError:
    _spec_text = (Path(__file__).resolve().parents[2] / "shared/online/trade.json").read_text()
SPEC = json.loads(_spec_text)
SUBPROTOCOL = "cyberworld.presence.v2.trade.v1"
SIDE = struct.Struct("!16s16sI32sHB")
READY = struct.Struct("!16sI32sB")
LISTING = struct.Struct("!16s16sHHB")


def chip(entry, optional=False):
    if type(entry) is not int or not 0 <= entry <= 0xffff:
        raise ProtocolError("Invalid trade chip")
    if entry == 0 and optional:
        return entry
    if not entry & 0x1ff or entry >> 9 > 26:
        raise ProtocolError("Invalid trade chip or code")
    return entry


def side_bytes(side):
    side.validate()
    return SIDE.pack(side.identity, side.run_id, side.revision, side.checkpoint, side.chip, side.rules)


def terms_bytes(decision):
    return decision.server + decision.transaction + side_bytes(decision.left) + side_bytes(decision.right)


def decision_bytes(decision):
    return bytes((SPEC["decision"],)) + terms_bytes(decision) + bytes((decision.state, decision.prepared, decision.applied))


def caps(server_identity, enabled):
    if not isinstance(server_identity, bytes) or len(server_identity) != 32 or not any(server_identity):
        raise TradeError("Invalid server identity")
    return struct.pack("!BH32sB", SPEC["caps"], SPEC["wire_version"], server_identity, bool(enabled))


def board(offers):
    if len(offers) > SPEC["listings_max"]:
        raise TradeError("Trade room exceeds its listing limit")
    result = bytes((SPEC["board"], len(offers)))
    for offer in offers:
        if not offer.carried or not offer.have:
            raise TradeError("Only live offers belong on the board")
        side = offer.carried.side(offer.identity, offer.have)
        result += LISTING.pack(offer.identity, offer.name.encode("ascii"), chip(offer.have), chip(offer.want, True), side.rules)
    return result


def read(data):
    """Return a validated command and values; no player identity comes from it."""
    if not isinstance(data, bytes) or not data:
        raise ProtocolError("Missing trade command")
    kind, payload = data[0], data[1:]
    try:
        if kind == SPEC["ready"] and len(payload) == READY.size:
            run, revision, checkpoint, rules = READY.unpack(payload)
            if rules & ~3:
                raise ProtocolError("Invalid carried rules")
            carried = Carried(run, revision, checkpoint, bool(rules & 1), bool(rules & 2))
            carried.side(b"v" * 16, (26 << 9 | 1) if carried.all_star else 1)
            return "ready", (carried,)
        if kind == SPEC["offer"] and len(payload) == 4:
            have, want = struct.unpack("!HH", payload)
            chip(have, True)
            chip(want, True)
            if (not have and want) or (have and have == want):
                raise ProtocolError("Invalid have/want offer")
            return "advertise", (have, want)
        if kind == SPEC["board_request"] and not payload:
            return "board", ()
        if kind == SPEC["request"] and len(payload) == 20:
            target, give, receive = struct.unpack("!16sHH", payload)
            if not any(target) or chip(give) == chip(receive):
                raise ProtocolError("Invalid trade selection")
            return "request", (target, give, receive)
        if kind == SPEC["prepare"] and len(payload) == 48 and any(payload[:16]) and any(payload[16:]):
            return "prepare", (payload[:16], payload[16:])
        for name, method in (("cancel", "cancel"), ("status", "status"), ("ack", "acknowledge")):
            if kind == SPEC[name] and len(payload) == 16 and any(payload):
                return method, (payload,)
    except TradeError as exc:
        raise ProtocolError(str(exc)) from exc
    raise ProtocolError("Malformed or out-of-order trade command")
