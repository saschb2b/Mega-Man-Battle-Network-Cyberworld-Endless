"""Bounded local relay for the developer frame-input experiment.

Only fixed-size numeric messages cross each socket. Artificial delay is a
one-way hold on a controller input, not RTT, jitter or an Internet model.
"""
import heapq
import json
from pathlib import Path
import selectors
import socket
import struct
import subprocess
import time

from tools.online_frame_compare import FRAME_COUNT, HASH_ALGORITHM

PACKET = struct.Struct('<24I')
MAGIC = 0x31465743
HELLO, TICK, INPUT, STATE, STOP = range(1, 6)
FRAME_SECONDS = 280896 / 16777216  # pinned mGBA video timing, not rounded 60 Hz
FAULT_FRAME = 1400


def packet(kind, frame, side, value=0):
    return PACKET.pack(MAGIC, kind, frame, side, value, *([0] * 19))


class Relay:
    def __init__(self, rom, state, work, name, delay_ms, fault=None):
        self.work, self.name, self.delay_ms, self.fault = Path(work), name, delay_ms, fault
        self.selector = selectors.DefaultSelector()
        self.sockets, self.processes, self.logs = [], [], []
        self.buffers = [bytearray(), bytearray()]
        self.sources, self.frames, self.pids = [0, 0], [0, 0], [0, 0]
        self.lookahead = [0, 0]
        self.pending, self.unmatched = [], {}
        self.deliveries, self.matched, self.delivered = [], 0, 0
        self.status, self.failure_frame = 'running', None
        self.tick, self.start, self.last_progress = 0, None, time.monotonic()
        self.injected, self.fault_at = False, None
        self.replica_file = (self.work / f'{name}-replicas.jsonl').open('w')
        self.input_file = (self.work / f'{name}-inputs.jsonl').open('w')
        for side in (0, 1):
            parent, child = socket.socketpair()
            parent.settimeout(5)
            log = (self.work / f'{name}-worker{side}.log').open('wb')
            prefix = self.work / f'{name}-worker{side}'
            proc = subprocess.Popen(['build/host/online-frame-worker', str(rom), str(state),
                                     str(side), str(child.fileno()), str(prefix)],
                                    pass_fds=(child.fileno(),), stdout=log, stderr=log)
            child.close()
            self.sockets.append(parent)
            self.processes.append(proc)
            self.logs.append(log)
            self.selector.register(parent, selectors.EVENT_READ, side)

    def send(self, side, data):
        self.sockets[side].sendall(data)

    def receive(self, side):
        chunk = self.sockets[side].recv(8192)
        if not chunk:
            if self.frames[side] == FRAME_COUNT:
                self.selector.unregister(self.sockets[side])
                return
            self.status = 'disconnected'
            return
        self.buffers[side].extend(chunk)
        if len(self.buffers[side]) > PACKET.size * 64:
            raise RuntimeError('Worker output exceeded its bounded frame queue')
        while len(self.buffers[side]) >= PACKET.size and self.status == 'running':
            words = PACKET.unpack(self.buffers[side][:PACKET.size])
            del self.buffers[side][:PACKET.size]
            self.accept(side, words)
        self.last_progress = time.monotonic()

    def accept(self, side, words):
        magic, kind, frame, owner, value = words[:5]
        if magic != MAGIC or owner != side:
            raise RuntimeError('Invalid worker framing or ownership')
        if kind == HELLO:
            if self.pids[side] or frame != FRAME_COUNT or value <= 0:
                raise RuntimeError('Invalid worker handshake')
            self.pids[side] = value
            self.lookahead[side] = words[5]
        elif kind == INPUT:
            if frame != self.sources[side] or frame >= self.tick or value > 0x3FF:
                raise RuntimeError('Invalid worker input sequence')
            self.sources[side] += 1
            now = time.monotonic()
            delivered_value = value
            if self.fault == 'corruption' and not side and frame == FAULT_FRAME:
                delivered_value ^= 16  # wrong direction reaches one replica only
                self.injected, self.fault_at = True, now
            heapq.heappush(self.pending, (now + self.delay_ms / 1000, side, frame, delivered_value, now, value))
        elif kind == STATE:
            self.accept_state(side, frame, words)
        else:
            raise RuntimeError('Unexpected worker packet kind')

    def accept_state(self, side, frame, words):
        if frame != self.frames[side] or frame >= FRAME_COUNT:
            raise RuntimeError('Invalid worker state sequence')
        self.frames[side] += 1
        digest = f'{words[6]:08x}{words[5]:08x}'
        samples = [list(words[7:15]), list(words[15:23])]
        row = dict(worker=side, frame=frame, hash=digest, hash_algorithm=HASH_ALGORITHM, samples=samples)
        self.replica_file.write(json.dumps(row, separators=(',', ':')) + '\n')
        observation = (digest, samples)
        if frame in self.unmatched:
            if self.unmatched.pop(frame) != observation:
                self.status, self.failure_frame = 'desync', frame
            else:
                self.matched += 1
        else:
            self.unmatched[frame] = observation
        if len(self.unmatched) > 64:
            raise RuntimeError('Replica comparison exceeded its bounded frame queue')

    def schedule(self):
        now = time.monotonic()
        if self.start is None and all(self.pids):
            self.start = now
        if self.start is not None and self.tick < FRAME_COUNT and self.tick - min(self.frames) < 60:
            if now >= self.start + self.tick * FRAME_SECONDS:
                for side in (0, 1):
                    self.send(side, packet(TICK, self.tick, side))
                self.tick += 1
        while self.pending and self.pending[0][0] <= now:
            _, owner, frame, keys, sent, source_keys = heapq.heappop(self.pending)
            self.send(owner ^ 1, packet(INPUT, frame, owner, keys))
            delivery_ms = (time.monotonic() - sent) * 1000
            self.deliveries.append(delivery_ms)
            self.input_file.write(json.dumps(dict(owner=owner, frame=frame, source_keys=source_keys,
                                                 delivered_keys=keys, delivery_ms=delivery_ms)) + '\n')
            self.delivered += 1
        if self.fault == 'disconnect' and not self.injected and min(self.frames) >= FAULT_FRAME:
            self.injected, self.fault_at = True, time.monotonic()
            self.processes[1].kill()

    def run(self, wall_seconds=90):
        entered = time.monotonic()
        try:
            while self.status == 'running':
                if min(self.frames) == FRAME_COUNT:
                    self.status = 'complete'
                    break
                now = time.monotonic()
                if now - entered >= wall_seconds or now - self.last_progress > 5:
                    self.status = 'timeout'
                    break
                self.schedule()
                for key, _ in self.selector.select(0.001):
                    self.receive(key.data)
        except (BrokenPipeError, ConnectionResetError):
            self.status = 'disconnected'
        finally:
            stopped = time.monotonic()
            self.close()
        return dict(case=self.name, status=self.status, pids=self.pids, frames=self.frames,
                    exit_codes=[p.returncode for p in self.processes], delay_ms=self.delay_ms,
                    delivered_inputs=self.delivered, matched_replica_frames=self.matched,
                    minimum_delivery_ms=min(self.deliveries) if self.deliveries else None,
                    maximum_delivery_ms=max(self.deliveries) if self.deliveries else None,
                    elapsed_ms=(stopped - entered) * 1000, simulated_ms=FRAME_COUNT * FRAME_SECONDS * 1000,
                    fault_injected=self.injected, first_divergent_frame=self.failure_frame,
                    fault_detection_ms=(stopped - self.fault_at) * 1000 if self.fault_at else None,
                    input_buffer_frames=[max(0, value - 1) for value in self.lookahead],
                    input_sampling_interval_ms=FRAME_SECONDS * 1000,
                    hash_algorithm=HASH_ALGORITHM, rollback=False, cores_per_worker=2)

    def close(self):
        for side, proc in enumerate(self.processes):
            if self.status != 'complete':
                try:
                    self.send(side, packet(STOP, 0, side))
                except (OSError, TimeoutError):
                    pass
            try:
                proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=5)
        for stream in self.sockets + self.logs + [self.replica_file, self.input_file]:
            stream.close()
        self.selector.close()


def run_case(rom, state, work, delay_ms, fault=None):
    name = fault or f'delay{delay_ms}'
    relay = Relay(rom, state, work, name, delay_ms, fault)
    report = relay.run()
    (Path(work) / f'{name}-transport.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report), flush=True)
    return report
