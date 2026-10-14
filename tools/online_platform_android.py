#!/usr/bin/env python3
"""Exercise an installed APK in a disposable, rooted Android emulator.

Requires a 2400x1080 landscape display, the app's own imported BN6 ROM and
an existing checkpoint, and the first-start questions already answered.
Never targets a physical phone. Network preferences are restored afterwards;
ROMs and checkpoint files are only read. Screenshots need visual review.
"""
import argparse
import base64
import contextlib
import json
import os
from pathlib import Path
import re
import socket
import ssl
import struct
import subprocess
import sys
import tempfile
import threading
import time
from urllib.parse import urlsplit

from online_proof import ROOT, hashes, initial_checkpoint, position, records, stop

sys.path.insert(0, str(ROOT / 'server'))
from cyberworld_server import protocol

PACKAGE = 'io.github.saschb2b.cyberworldendless'
DATA = '/data/user/0/' + PACKAGE + '/files/data'


class Android:
    def __init__(self, serial, evidence):
        self.adb, self.evidence = ['adb', '-s', serial], evidence
        if not serial.startswith('emulator-') or self.shell('getprop ro.kernel.qemu').strip() != b'1':
            raise RuntimeError('This scripted test only supports disposable emulators')
        if self.shell('id -u').strip() != b'0':
            raise RuntimeError('Use adb root in the disposable emulator first')

    def call(self, *args):
        return subprocess.check_output(self.adb + list(args), timeout=30)

    def shell(self, command):
        return self.call('shell', command)

    def saved(self):
        result = self.shell('cd ' + DATA + '/savedata && sha256sum *').decode()
        return dict((name, digest) for digest, name in (line.split('  ', 1) for line in result.splitlines())
                    if name != 'transfer.manifest')

    def prefs(self, content):
        command = 'cat > ' + DATA + '/network.ini; owner=$(stat -c %u:%g ' + DATA + '/settings.ini); '
        command += 'chown "$owner" ' + DATA + '/network.ini; chmod 600 ' + DATA + '/network.ini; restorecon ' + DATA + '/network.ini'
        subprocess.run(self.adb + ['shell', command], input=content, check=True, timeout=10, capture_output=True)

    def endpoint(self, address):
        content = self.shell('cat ' + DATA + '/network.ini').decode()
        content = re.sub(r'^(name|server|selected|consent)=.*\n?', '', content, flags=re.M)
        self.prefs((content + '\nname=AndroidProof\nserver=' + address + '\nselected=0\nconsent=0\n').encode())

    def tap(self, x, y, hold=80):
        self.shell(f'input swipe {x} {y} {x} {y} {hold}')
        time.sleep(.15)

    def picture(self, name):
        (self.evidence / (name + '.png')).write_bytes(self.call('exec-out', 'screencap', '-p'))

    def join(self):
        self.shell('am force-stop ' + PACKAGE)
        self.shell('am start -n ' + PACKAGE + '/.GameActivity')
        time.sleep(10)  # Both boot panels finish before the title receives Start.
        self.shell('wm user-rotation lock 1')
        time.sleep(.5)
        self.tap(2160, 960)  # Start, then title's Network row.
        time.sleep(.8)
        self.tap(240, 730)
        self.tap(2255, 487)
        self.picture('network-menu')
        self.tap(1200, 705)
        self.picture('privacy')
        self.tap(1780, 960)

    def leave(self):
        self.tap(2160, 960)
        for _ in range(4):
            self.tap(240, 730)
        self.tap(2255, 487)
        time.sleep(2)
        self.picture('checkpoint-return')


def masked(data):
    mask = os.urandom(4)
    header = bytes((0x82, 0x80 | len(data))) if len(data) < 126 else struct.pack('!BBH', 0x82, 254, len(data))
    return header + mask + bytes(value ^ mask[i % 4] for i, value in enumerate(data))


class Observer:
    def __init__(self, endpoint, connect_ip):
        url = urlsplit(endpoint)
        peer = socket.create_connection((connect_ip or url.hostname, url.port or 443), timeout=8)
        self.socket = ssl.create_default_context().wrap_socket(peer, server_hostname=url.hostname)
        key = base64.b64encode(os.urandom(16)).decode()
        request = (f'GET /presence HTTP/1.1\r\nHost: {url.netloc}\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n'
                   f'Sec-WebSocket-Key: {key}\r\nSec-WebSocket-Version: 13\r\n'
                   f'Sec-WebSocket-Protocol: {protocol.SPEC["subprotocol"]}\r\n\r\n')
        self.socket.sendall(request.encode())
        response = b''
        while not response.endswith(b'\r\n\r\n'):
            response += self.socket.recv(1)
        if not response.startswith(b'HTTP/1.1 101 '):
            raise RuntimeError('Observer WebSocket upgrade failed')
        self.socket.sendall(masked(protocol.hello('AndroidObserver')))
        self.rows, self.closed = [], False
        self.thread = threading.Thread(target=self.read, daemon=True)
        self.thread.start()

    def exact(self, size):
        result = b''
        while len(result) < size:
            part = self.socket.recv(size - len(result))
            if not part:
                raise EOFError()
            result += part
        return result

    def read(self):
        try:
            while not self.closed:
                opcode, size = self.exact(2)
                if opcode != 0x82:
                    break
                if size == 126:
                    size = struct.unpack('!H', self.exact(2))[0]
                message = self.exact(size)
                if message[0] != protocol.SPEC['snapshot']:
                    continue
                peers = []
                for offset in range(2, len(message), protocol.PEER.size):
                    value = protocol.PEER.unpack_from(message, offset)
                    peers.append(dict(id=value[0], x=value[2], y=value[3], z=value[4],
                                      name=value[6].rstrip(b'\0').decode()))
                self.rows.append(dict(time=time.monotonic(), peers=peers))
        except (OSError, EOFError):
            pass

    def close(self):
        self.closed = True
        self.socket.close()
        self.thread.join(timeout=1)


def refusal(android, evidence):
    cert, key = evidence / 'untrusted.pem', evidence / 'untrusted-key.pem'
    subprocess.run(['openssl', 'req', '-x509', '-newkey', 'rsa:2048', '-nodes', '-days', '1', '-subj', '/CN=localhost',
                    '-addext', 'subjectAltName=DNS:localhost', '-keyout', str(key), '-out', str(cert)],
                   check=True, capture_output=True)
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(cert, key)
    result = dict(tls_client_hello_received=False, application_bytes=0)
    with socket.socket() as listener:
        listener.bind(('127.0.0.1', 0)); listener.listen(); listener.settimeout(30)
        port = listener.getsockname()[1]
        def accept():
            try:
                peer, _ = listener.accept()
                with peer:
                    peer.settimeout(8)
                    result['tls_client_hello_received'] = peer.recv(1, socket.MSG_PEEK) == b'\x16'
                    try:
                        with context.wrap_socket(peer, server_side=True) as secure:
                            result['application_bytes'] = len(secure.recv(1024))
                    except OSError as error:
                        result['server_error'] = str(error)
                        result['certificate_rejected'] = isinstance(error, ssl.SSLError) and error.reason in (
                            'SSLV3_ALERT_CERTIFICATE_UNKNOWN', 'TLSV1_ALERT_UNKNOWN_CA', 'SSLV3_ALERT_BAD_CERTIFICATE')
            except OSError as error:
                result['server_error'] = str(error)
        thread = threading.Thread(target=accept, daemon=True); thread.start()
        android.call('reverse', 'tcp:43127', 'tcp:' + str(port))
        try:
            before = android.saved()
            android.shell('am force-stop ' + PACKAGE)
            android.endpoint('wss://localhost:43127/presence')
            android.join()
            time.sleep(.8); android.picture('tls-refused')
            time.sleep(3); android.picture('tls-checkpoint-return')
            thread.join(timeout=10)
            result['checkpoint_unchanged'] = android.saved() == before
        finally:
            android.call('reverse', '--remove', 'tcp:43127')
    if (not result['tls_client_hello_received'] or not result.get('certificate_rejected') or
            result['application_bytes'] or not result['checkpoint_unchanged']):
        raise RuntimeError('Android did not satisfy TLS refusal and checkpoint checks: ' + str(result))
    return result


def run(args, evidence):
    android = Android(args.serial, evidence)
    original_prefs = android.shell('cat ' + DATA + '/network.ini')
    saved = android.saved()
    report = dict(platform='Android emulator, actual release APK', physical_device=False, endpoint=args.endpoint)
    try:
        android.shell('am force-stop ' + PACKAGE)
        android.endpoint(args.endpoint)
        peer_data = evidence / 'Desktop'
        peer_saved = initial_checkpoint(args.binary, args.rom_dir, peer_data, 120, evidence / 'Desktop-checkpoint.log')
        with contextlib.ExitStack() as stack:
            observer = Observer(args.endpoint, args.connect_ip); stack.callback(observer.close)
            env = dict(os.environ, CYBERWORLD_ONLINE_TRACE=str(evidence / 'Desktop.jsonl'))
            if args.peer_resolver:
                env['LD_PRELOAD'] = str(args.peer_resolver.resolve())
            walk = '30:UP,45:,30:DOWN,45:,24:LEFT,45:,24:RIGHT,90:,'
            log = stack.enter_context((evidence / 'Desktop.log').open('w'))
            peer = subprocess.Popen([str(args.binary), '--headless', '--rom-dir', str(args.rom_dir), '--data-dir', str(peer_data),
                                     '--online', args.endpoint, '--online-name', 'AndroidDesktop', '--frames', '8000',
                                     '--input', '300:,' + walk * 18], env=env, stdout=log, stderr=subprocess.STDOUT)
            stack.callback(stop, peer)
            android.join(); time.sleep(3); android.picture('joined')
            for number, (x, y) in enumerate(((240, 435), (240, 730), (90, 580), (385, 580))):
                android.tap(x, y, 600); time.sleep(.7); android.picture('move-' + str(number))
            android.leave()
            rows = records(evidence / 'Desktop.jsonl')
            observed = [peer for row in observer.rows for peer in row['peers'] if peer['name'] == 'AndroidProof']
            ids = {peer['id'] for peer in observed}
            actual = [peer for row in rows for peer in row['peers'] if peer['id'] in ids]
            # A freshly admitted peer has no position until its first update.
            report['pending_initial_positions'] = sum(not any(position(peer)) for peer in actual)
            actual = [peer for peer in actual if any(position(peer))]
            matching = [peer for peer in actual if peer['npc_match'] and all(peer['npc_' + k] == peer['rendered_' + k] for k in ('x', 'y', 'z'))]
            report.update(android_positions=len({position(peer) for peer in observed}), desktop_actual_npc_matches=len(matching),
                          android_checkpoint_unchanged=android.saved() == saved,
                          desktop_checkpoint_unchanged=hashes(peer_data / 'savedata') == peer_saved)
            (evidence / 'observer.json').write_text(json.dumps(observer.rows))
            if (report['android_positions'] < 3 or len(matching) < 10 or len(matching) != len(actual) or
                    not report['android_checkpoint_unchanged'] or not report['desktop_checkpoint_unchanged']):
                raise RuntimeError('Android movement/replication/checkpoint assertions failed')
        report['desktop_checkpoint_unchanged'] = hashes(peer_data / 'savedata') == peer_saved
        if not report['desktop_checkpoint_unchanged']:
            raise RuntimeError('Desktop checkpoint changed during shutdown')
        report['tls_refusal'] = refusal(android, evidence)
        report['result'] = 'passed; screenshots require visual review'
    except Exception as error:
        report.update(result='failed', error=str(error))
        raise
    finally:
        android.shell('am force-stop ' + PACKAGE)
        android.prefs(original_prefs)
        (evidence / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--serial', required=True)
    parser.add_argument('--rom-dir', type=Path, required=True)
    parser.add_argument('--binary', type=Path, default=ROOT / 'build/linux/cyberworld')
    parser.add_argument('--endpoint', default='wss://cyberworld.saschb2b.com/presence')
    parser.add_argument('--connect-ip', help='Optional observer routing override; original TLS hostname remains verified')
    parser.add_argument('--peer-resolver', type=Path)
    args = parser.parse_args()
    base = ROOT / '.build/online-platform-android'; base.mkdir(exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='run-', dir=base))
    print('Android APK proof: ' + str(evidence), flush=True)
    run(args, evidence)


if __name__ == '__main__':
    main()
