"""Command-line configuration: a Square on this computer with nothing set,
or a TLS endpoint behind its INI (server/README.md, Host your own Net Square)."""

import argparse
import asyncio
import configparser
import contextlib
import logging
import os
from pathlib import Path
import signal
import ssl
import json
import sys

from .app import Server
from . import admin, backup, hosting
from .identity import public_id
from .retention import Retention

# (the published browser build's page: its players come to any Square that
# serves wss://; server.ini's `origins =` empty lets native games alone in)
ORIGINS = "https://saschb2b.github.io"


def config_path(args):
    """The INI read: --config's, else a server.ini in the folder the server
    starts in or beside its program, else none (the defaults)."""
    if args.config:
        return Path(args.config)
    return hosting.find_config((Path.cwd(), hosting.program_folder()))


def configured(args, environ=os.environ):
    """The [server] section, from the INI with the environment's
    CYBERWORLD_SERVER_* over it, and the state directory it and the
    arguments name."""
    config = configparser.ConfigParser()
    path = config_path(args)
    if path and not config.read(path):
        raise ValueError("Could not read server config: " + str(path))
    if not config.has_section("server"):
        config.add_section("server")
    settings = config["server"]
    settings.update(hosting.environment_settings(environ))
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
    # (empty: every interface, IPv4 and IPv6, so other computers can come)
    host = args.host or settings.get("host") or None
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
                        tuple(settings.get("origins", ORIGINS).split()), events,
                        player_cap=settings.getint("player_cap", 64), state_dir=state_dir,
                        name=settings.get("name", "Cyberworld Endless"), motd=settings.get("motd", "Welcome to the Net Square"),
                        whitelist=tuple(public_id(value) for value in settings.get("whitelist", "").split()),
                        trading=settings.getboolean("trading", True), arena=settings.getboolean("arena", True),
                        policy=Retention.from_settings(settings), maintenance=settings.get("maintenance", ""))
        port = await server.start(host, port, tls)
        await announce(server, host, port, tls is not None, state_dir, config_path(args))
        stop = asyncio.Event()
        for sig in (signal.SIGINT, signal.SIGTERM):
            with contextlib.suppress(NotImplementedError):
                asyncio.get_running_loop().add_signal_handler(sig, stop.set)
        try:
            await stop.wait()
        finally:
            await server.close()


async def announce(server, host, port, secure, state_dir, config):
    """The console's word to the host as the Square opens: where players
    come in from (hosting.py); this computer's name's addresses looked up
    off the loop, and given up after two seconds."""
    try:
        addresses = await asyncio.wait_for(asyncio.to_thread(hosting.local_addresses), 2)
    except asyncio.TimeoutError:
        addresses = hosting.local_addresses(named=False)
    hosting.announce(hosting.banner(server.name, host, port, secure, addresses, state_dir, config,
                                    hosting.contained(), os.name == "nt"))


def main():
    parser = argparse.ArgumentParser(description="Cyberworld Endless Net Square server (ROM-free)")
    parser.add_argument("--config", help="The INI to read (default: server.ini in this folder or beside the server, if there is one)")
    parser.add_argument("--host", help="Where to listen (default: every interface; 127.0.0.1 this computer alone)")
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
