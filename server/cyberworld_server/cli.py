"""Command-line configuration for a local spike or a TLS endpoint."""

import argparse
import asyncio
import configparser
import contextlib
import logging
import signal
import ssl

from .app import Server


async def run(args):
    config = configparser.ConfigParser()
    if args.config and not config.read(args.config):
        raise ValueError("Could not read server config: " + args.config)
    if not config.has_section("server"):
        config.add_section("server")
    settings = config["server"]
    host = args.host or settings.get("host", "127.0.0.1")
    port = args.port if args.port is not None else settings.getint("port", 8765)
    seed = args.seed if args.seed is not None else settings.getint("seed", 120)
    tls = None
    cert, key = settings.get("tls_certificate"), settings.get("tls_key")
    if bool(cert) != bool(key):
        raise ValueError("TLS needs both tls_certificate and tls_key")
    if cert:
        tls = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        tls.minimum_version = ssl.TLSVersion.TLSv1_2
        tls.load_cert_chain(cert, key)
    with contextlib.ExitStack() as stack:
        events = stack.enter_context(open(args.event_log, "a", encoding="utf-8")) if args.event_log else None
        server = Server(seed, settings.getint("capacity", 8), settings.getint("tick_hz", 20),
                        tuple(settings.get("origins", "").split()), events)
        await server.start(host, port, tls)
        stop = asyncio.Event()
        for sig in (signal.SIGINT, signal.SIGTERM):
            with contextlib.suppress(NotImplementedError):
                asyncio.get_running_loop().add_signal_handler(sig, stop.set)
        try:
            await stop.wait()
        finally:
            await server.close()


def main():
    parser = argparse.ArgumentParser(description="Cyberworld Endless Phase 0 presence server (ROM-free)")
    parser.add_argument("--config")
    parser.add_argument("--host")
    parser.add_argument("--port", type=int)
    parser.add_argument("--seed", type=int)
    parser.add_argument("--event-log", help="Optional JSONL connection/position evidence; contains display names")
    args = parser.parse_args()
    logging.basicConfig(level=logging.INFO, format="%(message)s")
    try:
        asyncio.run(run(args))
    except (OSError, ValueError) as exc:
        parser.exit(1, str(exc) + "\n")
    except KeyboardInterrupt:
        pass
