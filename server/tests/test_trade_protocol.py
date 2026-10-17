"""Trade command bounds and immutable confirmation bytes shared with C."""

import hashlib
import json
from pathlib import Path
import re
import struct
import tempfile
import unittest

from test_server import ROOT
from cyberworld_server.protocol import ProtocolError
from cyberworld_server import trade_protocol as wire
from cyberworld_server.trade_lobby import Carried, Offer
from cyberworld_server.trade_store import Decision, Side, TradeStore


class TradeProtocolTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.store = TradeStore(Path(self.temp.name), b"s" * 32)
        self.addCleanup(self.store.close)
        self.left = Side(b"a" * 16, b"A" * 32, 1, b"1" * 16, 7)
        self.right = Side(b"b" * 16, b"B" * 32, 2, b"2" * 16, 9, 1)
        self.trade = self.store.create(self.left, self.right)

    def test_shared_constants_and_confirmation_bytes(self):
        header = (ROOT / "shared/online/trade.h").read_text()
        constants = dict(re.findall(r"#define ONLINE_TRADE_(\w+) (\d+)", header))
        self.assertEqual({key.lower(): int(value) for key, value in constants.items()}, wire.SPEC)
        self.assertEqual(wire.SIDE.size, wire.SPEC["side_bytes"])
        self.assertEqual(wire.LISTING.size, wire.SPEC["listing_bytes"])
        terms = wire.terms_bytes(self.trade)
        self.assertEqual(len(terms), wire.SPEC["terms_bytes"])
        self.assertEqual(hashlib.sha256(b"Cyberworld trade 1\0" + terms).digest(), self.trade.digest)
        message = wire.decision_bytes(self.trade)
        self.assertEqual(len(message), wire.SPEC["decision_bytes"])
        self.assertEqual(message[1:-3], terms)
        self.assertEqual(message[-3:], bytes(3))
        negotiation = (ROOT / "shared/online/negotiation.h").read_text()
        selected = re.search(r'#define ONLINE_TRADE_SUBPROTOCOL "([^"]+)"', negotiation)
        self.assertEqual(selected.group(1), wire.SUBPROTOCOL)

    def test_fixed_c_and_python_confirmation_vector(self):
        vector = json.loads((ROOT / "shared/online/trade-vector.json").read_text())
        left = Side(bytes(range(1, 17)), bytes(range(65, 97)), 3 << 9 | 1, bytes(range(33, 49)), 0x01020304, 1)
        right = Side(bytes(range(17, 33)), bytes(range(97, 129)), 26 << 9 | 291, bytes(range(49, 65)), 0x05060708)
        decision = Decision(bytes(range(129, 145)), left, right, 0, 0, 0, bytes(range(145, 177)))
        self.assertEqual(wire.terms_bytes(decision).hex(), vector["terms_hex"])
        self.assertEqual(decision.digest.hex(), vector["digest"])

    def test_all_commands_have_exact_lengths_and_typed_payloads(self):
        tx = self.trade.transaction
        commands = {
            "ready": wire.READY.pack(b"1" * 16, 7, b"A" * 32, 1),
            "offer": struct.pack("!HH", 1, 2), "board_request": b"",
            "request": struct.pack("!16sHH", b"b" * 16, 1, 2),
            "prepare": tx + self.trade.digest, "cancel": tx, "status": tx, "ack": tx,
        }
        for name, payload in commands.items():
            message = bytes((wire.SPEC[name],)) + payload
            with self.subTest(name=name):
                method, arguments = wire.read(message)
                self.assertEqual(method, {"offer": "advertise", "board_request": "board", "ack": "acknowledge"}.get(name, name))
                self.assertIsInstance(arguments, tuple)
                for size in range(len(message)):
                    with self.assertRaises(ProtocolError):
                        wire.read(message[:size])
                with self.assertRaises(ProtocolError):
                    wire.read(message + b"\0")
        self.assertEqual(wire.read(bytes((wire.SPEC["offer"],)) + bytes(4)), ("advertise", (0, 0)))

    def test_chip_boundaries_and_forged_readiness(self):
        for entry in range(65536):
            valid = bool(entry & 511) and entry >> 9 <= 26
            if valid:
                self.assertEqual(wire.chip(entry), entry)
            else:
                with self.assertRaises(ProtocolError):
                    wire.chip(entry)
        for run, revision, checkpoint, rules in ((bytes(16), 0, b"A" * 32, 0),
                (b"1" * 16, 0xffffffff, b"A" * 32, 0), (b"1" * 16, 0, bytes(32), 0),
                (b"1" * 16, 0, b"A" * 32, 4)):
            with self.subTest(rules=rules), self.assertRaises(ProtocolError):
                wire.read(bytes((wire.SPEC["ready"],)) + wire.READY.pack(run, revision, checkpoint, rules))
        for have, want in ((0, 1), (1, 1), (1, 512), (27 << 9 | 1, 0)):
            with self.assertRaises(ProtocolError):
                wire.read(bytes((wire.SPEC["offer"],)) + struct.pack("!HH", have, want))

    def test_board_contains_only_bounded_public_offer_fields(self):
        carried = Carried(b"1" * 16, 7, b"A" * 32, True, False)
        offers = tuple(Offer(bytes((i,)) * 16, f"Player-{i}", 1, carried, 1, 2) for i in range(1, 9))
        data = wire.board(offers)
        self.assertEqual(len(data), 2 + 8 * wire.LISTING.size)
        self.assertLessEqual(len(data), 512)
        self.assertEqual(data[:2], bytes((wire.SPEC["board"], 8)))
        identity, name, have, want, rules = wire.LISTING.unpack_from(data, 2)
        self.assertEqual((identity, name.rstrip(b"\0"), have, want, rules), (b"\1" * 16, b"Player-1", 1, 2, 1))
        self.assertNotIn(carried.checkpoint, data)
        self.assertEqual(wire.board(()), bytes((wire.SPEC["board"], 0)))

    def test_responses_cannot_be_sent_as_client_commands(self):
        for response in (wire.caps(self.store.server_identity, True), wire.board(()), wire.decision_bytes(self.trade)):
            with self.assertRaises(ProtocolError):
                wire.read(response)
        for kind in ("prepare", "cancel", "status", "ack"):
            with self.assertRaises(ProtocolError):
                wire.read(bytes((wire.SPEC[kind],)) + bytes(48 if kind == "prepare" else 16))


if __name__ == "__main__":
    unittest.main()
