#!/usr/bin/env python3
"""Measure the built presence server's bounded room traffic on loopback.

Uses the ROM-free wire test client, not a game performance simulation.
Application and WebSocket bytes exclude TCP/IP and TLS overhead.
"""
import argparse
import asyncio
import contextlib
import json
from pathlib import Path
import statistics
import struct
import subprocess
import sys
import tempfile
import time

from online_proof import records, stop, wait_for

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'server/tests'))
from test_server import Client, protocol


async def measure(port, players, seconds):
    clients, totals = [], []
    try:
        for index in range(players):
            client = Client()
            clients.append(client)
            welcome = await client.open(port, f'Probe{index + 1}')
            if welcome[0] != protocol.SPEC['welcome']:
                raise RuntimeError('Server rejected a probe client')
            totals.append(dict(sent_bytes=0, received_bytes=0, websocket_bytes=0,
                               snapshots=0, rtt=[]))
        start = time.monotonic()

        async def sender(client, total):
            tick = 0
            while time.monotonic() - start < seconds:
                messages = [protocol.STATE.pack(protocol.SPEC['state'], tick % 100, 50, 0, tick % 8)]
                if tick % 20 == 0:
                    stamp = int(time.monotonic() * 1000) & 0xffffffff
                    messages.append(struct.pack('!BI', protocol.SPEC['ping'], stamp))
                for data in messages:
                    await client.send(data)
                    total['sent_bytes'] += len(data)
                    total['websocket_bytes'] += len(data) + 6
                tick += 1
                await asyncio.sleep(max(0, start + tick / 20 - time.monotonic()))

        async def receiver(client, total):
            while True:
                data = await client.receive()
                total['received_bytes'] += len(data)
                total['websocket_bytes'] += len(data) + (2 if len(data) < 126 else 4)
                if data[0] == protocol.SPEC['snapshot']:
                    if data[1] != players - 1:
                        continue  # a snapshot queued while the room filled
                    total['snapshots'] += 1
                elif data[0] == protocol.SPEC['pong']:
                    stamp = struct.unpack('!I', data[1:])[0]
                    total['rtt'].append(((int(time.monotonic() * 1000) & 0xffffffff) - stamp) & 0xffffffff)
                else:
                    raise RuntimeError(f'Unexpected server message {data[0]}')

        readers = [asyncio.create_task(receiver(c, t)) for c, t in zip(clients, totals)]
        try:
            await asyncio.gather(*(sender(c, t) for c, t in zip(clients, totals)))
        finally:
            for task in readers:
                task.cancel()
            for task in readers:
                with contextlib.suppress(asyncio.CancelledError):
                    await task
        elapsed = time.monotonic() - start
        for total in totals:
            if total['snapshots'] < seconds * 15 or not total['rtt']:
                raise RuntimeError('A client did not sustain the room snapshot stream')
        latency = [value for total in totals for value in total['rtt']]
        return dict(result='passed', players=players, seconds=round(elapsed, 3),
                    snapshots_per_client=[t['snapshots'] for t in totals],
                    median_rtt_ms=statistics.median(latency), max_rtt_ms=max(latency),
                    application_bytes_per_second=round(sum(t['sent_bytes'] + t['received_bytes'] for t in totals) / elapsed),
                    websocket_bytes_per_second=round(sum(t['websocket_bytes'] for t in totals) / elapsed))
    finally:
        await asyncio.gather(*(client.close() for client in clients if hasattr(client, 'writer')))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--players', type=int, choices=range(1, 9), default=8)
    parser.add_argument('--seconds', type=int, choices=range(1, 61), default=10)
    args = parser.parse_args()
    parent = ROOT / '.build/online-bench'
    parent.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    events = evidence / 'server.jsonl'
    with (evidence / 'server.log').open('w') as output:
        server = subprocess.Popen([sys.executable, str(ROOT / 'build/server/cyberworld-server.pyz'),
                                   '--port', '0', '--event-log', str(events)], stdout=output, stderr=subprocess.STDOUT)
        try:
            listening = wait_for(lambda: next((r for r in records(events) if r['event'] == 'listening'), None),
                                 5, 'The built server did not start')
            report = asyncio.run(measure(listening['port'], args.players, args.seconds))
            (evidence / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
            print(json.dumps(report, indent=2))
            print(f'Evidence: {evidence}')
        finally:
            stop(server)


if __name__ == '__main__':
    main()
