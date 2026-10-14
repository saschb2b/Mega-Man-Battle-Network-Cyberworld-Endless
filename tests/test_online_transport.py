#!/usr/bin/env python3
"""Run the actual native transport against sockets, TLS and hostile peers.

No ROM is read. Supply the platform's Makefile online-transport executable.
--public additionally checks the official WSS deployment without trust overrides.
"""
import argparse
import asyncio
import contextlib
import json
import os
from pathlib import Path
import platform
import shutil
import ssl
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path[:0] = [str(ROOT / "server"), str(ROOT / "server/tests")]
from cyberworld_server import protocol, websocket
from cyberworld_server.app import Server
from test_server import Client


class ObservedServer(Server):
    def __init__(self, disconnect=False):
        super().__init__()
        self.disconnect = disconnect
        self.messages = set()
        self.positions = 0

    async def welcome(self, player):
        await super().welcome(player)
        if self.disconnect:
            asyncio.get_running_loop().call_later(0.4, player.writer.close)

    async def message(self, player, data, now):
        await super().message(player, data, now)
        if player.name == "TransportProbe":
            self.messages.add(data[0])
            if data[0] == protocol.SPEC["state"]:
                assert player.position == (123, -234, 12, 3)
                self.positions += 1


async def run_probe(binary, url, mode, report, label):
    process = await asyncio.create_subprocess_exec(
        str(binary), url, mode, protocol.SPEC["rom_code"], protocol.SPEC["rom_sha1"],
        stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE)
    try:
        stdout, stderr = await asyncio.wait_for(process.communicate(), 35)
    except BaseException:
        process.kill()
        await process.wait()
        raise
    results = [json.loads(line) for line in stdout.decode().splitlines() if line.startswith("{")]
    report[label] = {"sessions": results, "exit_code": process.returncode,
                     "diagnostic": stderr.decode(errors="replace")}
    print(label, json.dumps(report[label]), flush=True)
    assert process.returncode == 0 and len(results) == 2 and all(x["passed"] for x in results), label


async def drain_peer(peer):
    while True:
        await peer.receive()


async def normal_exchange(binary, report):
    server = ObservedServer()
    port = await server.start("127.0.0.1", 0)
    peer = Client()
    await peer.open(port, "ProbePeer")
    await peer.send(protocol.STATE.pack(protocol.SPEC["state"], 321, -123, 0, 7))
    drain = asyncio.create_task(drain_peer(peer))
    try:
        await run_probe(binary, f"ws://127.0.0.1:{port}/presence", "exchange", report, "exchange")
        wanted = {protocol.SPEC[k] for k in ("state", "profile", "inspect", "social", "channel", "ping")}
        assert wanted <= server.messages and server.positions >= 40
        report["exchange"]["positions_verified"] = server.positions
    finally:
        drain.cancel()
        with contextlib.suppress(asyncio.CancelledError):
            await drain
        await peer.close()
        await server.close()


async def disconnect(binary, report):
    server = ObservedServer(disconnect=True)
    port = await server.start("127.0.0.1", 0)
    try:
        await run_probe(binary, f"ws://127.0.0.1:{port}/presence", "disconnect", report, "disconnect")
    finally:
        await server.close()


async def raw_server_case(binary, report, handler, label, mode="disconnect", tls=None):
    connections = set()
    errors = []

    async def tracked(reader, writer):
        task = asyncio.current_task()
        connections.add(task)
        try:
            await handler(reader, writer)
        except (ConnectionError, asyncio.IncompleteReadError):
            pass
        except Exception as error:
            errors.append(str(error) or type(error).__name__)
        finally:
            writer.close()
            with contextlib.suppress(ConnectionError):
                await writer.wait_closed()
            connections.discard(task)

    listener = await asyncio.start_server(tracked, "127.0.0.1", 0, ssl=tls)
    port = listener.sockets[0].getsockname()[1]
    try:
        scheme = "wss" if tls else "ws"
        await run_probe(binary, f"{scheme}://127.0.0.1:{port}/presence", mode, report, label)
    finally:
        listener.close()
        tasks = list(connections)
        for task in tasks:
            task.cancel()
        await asyncio.gather(*tasks, return_exceptions=True)
        await listener.wait_closed()
    assert not errors, (label, errors)


async def malicious(reader, writer, frame):
    await websocket.upgrade(reader, writer, ())
    opcode, hello = await websocket.receive(reader)
    assert opcode == 2 and hello[0] == protocol.SPEC["hello"]
    writer.write(websocket.frame(protocol.WELCOME.pack(protocol.SPEC["welcome"], 120, 1, 20, 8, 1, b"t" * 16, 8)))
    await writer.drain()
    await asyncio.sleep(0.1)
    writer.write(frame)
    await writer.drain()
    # The client must close on the malicious frame, not a server EOF.
    async def wait_closed():
        while await reader.read(8192):
            pass
    await asyncio.wait_for(wait_closed(), 5)


def untrusted_context(directory):
    openssl = shutil.which("openssl")
    if not openssl:
        raise RuntimeError("openssl is required for the untrusted TLS test")
    key, cert = directory / "key.pem", directory / "cert.pem"
    subprocess.run([openssl, "req", "-x509", "-newkey", "rsa:2048", "-nodes", "-days", "1",
                    "-subj", "/CN=localhost", "-addext", "subjectAltName=DNS:localhost,IP:127.0.0.1",
                    "-keyout", str(key), "-out", str(cert)], check=True, capture_output=True)
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(cert, key)
    return context


async def main(args):
    binary = args.binary.resolve()
    if not binary.is_file():
        raise FileNotFoundError(binary)
    report = {"platform": platform.platform(), "machine": platform.machine(), "passed": False}
    try:
        await normal_exchange(binary, report)
        await disconnect(binary, report)
        for label, frame in (("oversized", b"\x82\x7e\x02\x01" + bytes(513)),
                             ("text_message", websocket.frame(b"not binary", 1)),
                             ("bad_protocol", websocket.frame(b"\xff"))):
            async def handler(reader, writer, data=frame):
                await malicious(reader, writer, data)
            await raw_server_case(binary, report, handler, label)

        async def stall(reader, writer):
            await reader.read()
        await raw_server_case(binary, report, stall, "cancel_handshake", "cancel")
        tls_requests = []

        async def should_not_connect(reader, writer):
            tls_requests.append(await reader.read(8192))
        with tempfile.TemporaryDirectory(prefix="cw-transport-tls-") as directory:
            await raw_server_case(binary, report, should_not_connect, "untrusted_tls", "refuse",
                                  untrusted_context(Path(directory)))
        assert not any(tls_requests), "client sent application data to an untrusted TLS server"
        if args.public:
            await run_probe(binary, "wss://cyberworld.saschb2b.com/presence", "public", report, "public_wss")
        report["passed"] = True
    finally:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, indent=2) + "\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, default=ROOT / "build/host/online-transport-probe")
    parser.add_argument("--report", type=Path, default=ROOT / ".build/online-transport/report.json")
    parser.add_argument("--public", action="store_true", default=os.environ.get("CYBERWORLD_TEST_PUBLIC_WSS") == "true")
    asyncio.run(main(parser.parse_args()))
