"""Command-line configuration for a local spike or a TLS endpoint."""

import argparse
import asyncio
import configparser
import contextlib
import logging
import signal
import ssl
import json
import sys

from .app import Server
from . import admin, backup
from .identity import public_id
from .retention import Retention


def configured(args):
    """The INI's [server] section and the state directory it and the arguments name."""
    config = configparser.ConfigParser()
    if args.config and not config.read(args.config):
        raise ValueError("Could not read server config: " + args.config)
    if not config.has_section("server"):
        config.add_section("server")
    settings = config["server"]
    return settings, args.state_dir or settings.get("state_dir", "server-data")


def backups(args):
    """--backup (an archive a day, or --once now) and --restore (backup.py)."""
    settings, state_dir = configured(args)
    if args.restore:
        restored = backup.restore(args.restore, state_dir)
        print(json.dumps({"restored": restored, "from": args.restore, "then": "restart the server"}, indent=2))
        return
    days = Retention.from_settings(settings).backup_days
    # (as a container's first process, SIGTERM ends it only through a handler)
    signal.signal(signal.SIGTERM, lambda *_: sys.exit(0))
    path = backup.run(state_dir, args.backup, days, once=args.once)
    if args.once:
        print(json.dumps({"archive": str(path)}, indent=2))


async def run(args):
    settings, state_dir = configured(args)
    host = args.host or settings.get("host", "127.0.0.1")
    port = args.port if args.port is not None else settings.getint("port", 8765)
    seed = args.seed if args.seed is not None else settings.getint("seed", 120)
    if args.admin:
        print(json.dumps(admin.perform(args.admin, state_dir, args.identity, args.reason, args.minutes, args.week), indent=2))
        return
    if settings.getboolean("free_text", False):
        raise ValueError("free_text is unavailable: quick phrases and emotes only")
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
                        tuple(settings.get("origins", "").split()), events,
                        player_cap=settings.getint("player_cap", 64), state_dir=state_dir,
                        name=settings.get("name", "Cyberworld Endless"), motd=settings.get("motd", "Welcome to the Net Square"),
                        whitelist=tuple(public_id(value) for value in settings.get("whitelist", "").split()),
                        trading=settings.getboolean("trading", True), arena=settings.getboolean("arena", True),
                        policy=Retention.from_settings(settings))
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
    parser = argparse.ArgumentParser(description="Cyberworld Endless Net Square server (ROM-free)")
    parser.add_argument("--config")
    parser.add_argument("--host")
    parser.add_argument("--port", type=int)
    parser.add_argument("--seed", type=int)
    parser.add_argument("--event-log", help="Optional JSONL connection/position evidence; contains display names")
    parser.add_argument("--state-dir", help="Private directory for identity salt and moderation; preserve across upgrades")
    parser.add_argument("--admin", choices=admin.ACTIONS, help="Local-only administrative action instead of starting the server")
    parser.add_argument("--identity", help="Public profile identity from --admin list")
    parser.add_argument("--reason", default="", help="Local moderation note, not sent to players")
    parser.add_argument("--minutes", type=int, help="How long a kick refuses the profile (default 10)")
    parser.add_argument("--week", type=int, help="With --admin hide: the week whose Board entry is taken down (default this week)")
    parser.add_argument("--backup", metavar="DIR", help="Archive the state directory into DIR once a UTC day, keeping backup_days days, instead of starting the server")
    parser.add_argument("--once", action="store_true", help="With --backup: one archive now, then exit")
    parser.add_argument("--restore", metavar="ARCHIVE", help="Put the state directory back as a backup's ARCHIVE holds it, then restart the server")
    args = parser.parse_args()
    logging.basicConfig(level=logging.INFO, format="%(message)s")
    if args.once and not args.backup:
        parser.error("--once applies only to --backup")
    try:
        if args.backup or args.restore:
            backups(args)
            return
        asyncio.run(run(args))
    except (OSError, ValueError) as exc:
        parser.exit(1, str(exc) + "\n")
    except KeyboardInterrupt:
        pass
