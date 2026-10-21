#!/usr/bin/env python3
"""A TCP relay between a game and a local Net Square server that can drop
the link on cue (issue #153, docs/ONLINE.md, A dropped link): for testing
the reconnect without touching a real network, and never against the
official server.

  tools/link_relay.py --listen 8931 --to 127.0.0.1:8930 --control 8932

The game's network.ini (or --dev online=ws://127.0.0.1:8931/presence) names
the relay. Commands, a line each, on the control port (`echo "cut 5" | nc
127.0.0.1 8932`):

  cut N     every byte held, both ways, for N seconds, then let through in
            order (a stall: the sockets stay open, as a Wi-Fi gap leaves
            them); a connection made meanwhile hangs too
  close N   every connection closed now, and new ones refused for N
            seconds (a dropped link: the far side sees it at once)
  status    the connections and what is in force

Each event is printed with the time, so a test's log says when the link
went and came back.
"""
import argparse
import asyncio
import time

STATE = {"held_until": 0.0, "refused_until": 0.0, "links": set()}


def log(*words):
    print(time.strftime("%H:%M:%S"), *words, flush=True)


async def pump(reader, writer):
    try:
        while data := await reader.read(65536):
            # (a cut in force: the bytes wait, in order, until it ends)
            while time.monotonic() < STATE["held_until"]:
                await asyncio.sleep(0.05)
            writer.write(data)
            await writer.drain()
    except (ConnectionError, asyncio.CancelledError):
        pass
    finally:
        writer.close()


async def link(client_reader, client_writer, target):
    if time.monotonic() < STATE["refused_until"]:
        log("refused a connection (the link is down)")
        client_writer.close()
        return
    try:
        server_reader, server_writer = await asyncio.open_connection(*target)
    except OSError as exc:
        log("the server did not answer:", exc)
        client_writer.close()
        return
    pair = (client_writer, server_writer)
    STATE["links"].add(pair)
    log(f"link open ({len(STATE['links'])} up)")
    try:
        await asyncio.gather(pump(client_reader, server_writer), pump(server_reader, client_writer))
    finally:
        STATE["links"].discard(pair)
        log(f"link closed ({len(STATE['links'])} up)")


async def control(reader, writer):
    line = (await reader.readline()).decode().split()
    now = time.monotonic()
    if line[:1] == ["cut"] and len(line) == 2:
        STATE["held_until"] = now + float(line[1])
        log(f"cut: everything held for {line[1]} s")
        answer = "held"
    elif line[:1] == ["close"] and len(line) == 2:
        STATE["refused_until"] = now + float(line[1])
        for pair in list(STATE["links"]):
            for side in pair:
                side.close()
        log(f"close: every link closed, new ones refused for {line[1]} s")
        answer = "closed"
    else:
        answer = (f"links {len(STATE['links'])}, held {max(0, STATE['held_until'] - now):.1f} s, "
                  f"refused {max(0, STATE['refused_until'] - now):.1f} s")
    writer.write((answer + "\n").encode())
    await writer.drain()
    writer.close()


async def main(args):
    host, port = args.to.rsplit(":", 1)
    target = (host, int(port))
    relay = await asyncio.start_server(lambda r, w: link(r, w, target), "127.0.0.1", args.listen)
    panel = await asyncio.start_server(control, "127.0.0.1", args.control)
    log(f"relaying 127.0.0.1:{args.listen} to {args.to}, control on {args.control}")
    async with relay, panel:
        await asyncio.gather(relay.serve_forever(), panel.serve_forever())


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--listen", type=int, required=True, help="the port the game connects to")
    parser.add_argument("--to", required=True, help="the server's HOST:PORT (a local one)")
    parser.add_argument("--control", type=int, required=True, help="the port that takes cut and close")
    args = parser.parse_args()
    if not args.to.startswith(("127.", "localhost")):
        parser.error("the relay is for a local server only")
    asyncio.run(main(args))
