#!/usr/bin/env python3
"""Exercise a physical ROCKNIX game and a desktop peer through a chosen server.

Uses an already authenticated SSH control socket and a temporary PortMaster
launcher. Installed ROMs remain on the device; installed saves are copied
only to its temporary test folder and hashed before/after. Evidence is local.
Optional resolver libraries affect these two test processes only.
"""
import argparse
import json
import os
from pathlib import Path
import re
import shlex
import socket
import ssl
import statistics
import subprocess
import tempfile
import threading
import time

from online_proof import ROOT, hashes, initial_checkpoint, position, records, stop


def motion(rows, other):
    peer_id = next(row['player_id'] for row in other if row['connected'])
    selected = [(row, peer) for row in rows if row['connected']
                for peer in row['peers'] if peer['id'] == peer_id]
    local = {position(row['local']) for row, _ in selected}
    remote = {position(peer) for _, peer in selected}
    other_local = {position(row['local']) for row in other}
    matching = [peer for _, peer in selected if peer['npc_match'] and
                all(peer['npc_' + axis] == peer['rendered_' + axis] for axis in ('x', 'y', 'z'))]
    latency = [row['latency_ms'] for row, _ in selected if row['latency_ms']]
    if min(len(local), len(remote), len(remote & other_local)) < 3 or len(matching) < 10:
        raise RuntimeError('Reciprocal movement and actual NPC interpolation were not established')
    return dict(samples=len(selected), local_positions=len(local), peer_positions=len(remote),
                shared_positions=len(remote & other_local), matching_actual_npc_samples=len(matching),
                round_trip_median_ms=statistics.median(latency), round_trip_max_ms=max(latency))


class Device:
    def __init__(self, args, evidence):
        self.ssh = ['ssh', '-S', args.control_path, '-o', 'BatchMode=yes', '-o', 'ConnectTimeout=10', args.host]
        self.installed = args.device_game
        self.folder = '/tmp/cw-online-' + evidence.name
        self.launcher = str(Path(self.installed).parent / ('zz-cw-online-' + evidence.name + '.sh'))

    def run(self, command, **kwargs):
        return subprocess.run(self.ssh + [command], capture_output=True, check=True, timeout=30, **kwargs).stdout

    def write(self, path, data):
        self.run('cat > ' + shlex.quote(path), input=data)

    def saved(self, folder):
        lines = self.run('find ' + shlex.quote(folder) + ' -type f -exec sha256sum {} \\;').decode().splitlines()
        return {name.removeprefix(folder + '/'): digest for digest, name in (line.split('  ', 1) for line in lines)}

    def fetch(self, remote, local):
        local.write_bytes(self.run('cat ' + shlex.quote(remote)))

    def prepare(self, args):
        q = shlex.quote
        if b'NO GAME' not in self.run('curl -s localhost:1234/runningGame'):
            raise RuntimeError('Device is busy; leave its current game first')
        self.run('mkdir -p ' + q(self.folder + '/data') + '; cp -a ' + q(self.installed + '/savedata') +
                 ' ' + q(self.folder + '/data/') + '; ln -s ' + q(self.installed + '/rom') + ' ' + q(self.folder + '/rom'))
        for name in ('boot-3.state', 'settings.ini', 'pad.ini', 'keys.ini'):
            self.run('cp ' + q(self.installed + '/' + name) + ' ' + q(self.folder + '/data/') + ' 2>/dev/null || true')
        self.write(self.folder + '/cyberworld.aarch64', args.device_binary.read_bytes())
        self.run('chmod +x ' + q(self.folder + '/cyberworld.aarch64'))
        if args.device_resolver:
            self.write(self.folder + '/resolver.so', args.device_resolver.read_bytes())

    def launch(self, args, route):
        q = shlex.quote
        original = self.run('cat ' + q(str(Path(self.installed).parent / 'Cyberworld Endless.sh'))).decode()
        original = re.sub(r'^GAMEDIR=.*$', 'GAMEDIR=' + q(self.folder), original, flags=re.M)
        env = ['CYBERWORLD_ONLINE_TRACE=' + q(self.folder + '/Nova.jsonl'), 'CYBERWORLD_FRAME_LOG=1']
        if args.device_resolver:
            env.append('LD_PRELOAD=' + q(self.folder + '/resolver.so'))
        command = ['env', *env, q(self.folder + '/cyberworld.aarch64'), '--rom-dir', q(self.folder + '/rom'),
                   '--data-dir', q(self.folder + '/data'), '--online', q(args.endpoint), '--online-name', 'NovaProof',
                   '--frames', '1800', '--input', q(route), '--shot', q('600:' + self.folder + '/Nova.bmp')]
        original = re.sub(r'^\./cyberworld\.aarch64 .*$', ' '.join(command) + '\ntouch ' + q(self.folder + '/done'),
                          original, flags=re.M)
        self.write(self.launcher, original.encode())
        self.run('chmod +x ' + q(self.launcher) + '; curl -s localhost:1234/reloadgames >/dev/null')
        time.sleep(2)
        self.run('curl -s -X POST -d ' + q(self.launcher) + ' localhost:1234/launch')

    def cleanup(self):
        q = shlex.quote
        self.run('rm -f ' + q(self.launcher) + '; curl -s localhost:1234/reloadgames >/dev/null; rm -rf ' + q(self.folder))


def tls_refusal(device, evidence):
    """Forward an untrusted local TLS fixture to device loopback, never a LAN listener."""
    cert, key = evidence / 'untrusted.pem', evidence / 'untrusted-key.pem'
    subprocess.run(['openssl', 'req', '-x509', '-newkey', 'rsa:2048', '-nodes', '-days', '1',
                    '-subj', '/CN=localhost', '-addext', 'subjectAltName=DNS:localhost',
                    '-keyout', str(key), '-out', str(cert)], check=True, capture_output=True)
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(cert, key)
    result = {'tls_client_hello_received': False, 'application_bytes': 0}
    with socket.socket() as listener:
        listener.bind(('127.0.0.1', 0))
        listener.listen()
        listener.settimeout(20)
        def accept():
            try:
                peer, _ = listener.accept()
                with peer:
                    peer.settimeout(10)
                    first = peer.recv(1, socket.MSG_PEEK)
                    result['first_record_type'] = first.hex()
                    result['tls_client_hello_received'] = first == b'\x16'
                    try:
                        with context.wrap_socket(peer, server_side=True) as secure:
                            result['application_bytes'] = len(secure.recv(1024))
                    except OSError as error:
                        result['server_error'] = str(error)
            except OSError as error:
                result['server_error'] = str(error)
        thread = threading.Thread(target=accept, daemon=True)
        thread.start()
        forward = f'127.0.0.1:0:127.0.0.1:{listener.getsockname()[1]}'
        command = device.ssh[:-1] + ['-O', 'forward', '-R', forward, device.ssh[-1]]
        port = int(subprocess.check_output(command, timeout=15).strip())
        try:
            before = device.saved(device.folder + '/data/savedata')
            argv = ['env', 'SDL_AUDIODRIVER=dummy', device.folder + '/cyberworld.aarch64', '--headless',
                    '--rom-dir', device.folder + '/rom', '--data-dir', device.folder + '/data',
                    '--online', f'wss://localhost:{port}/presence', '--online-name', 'NovaTLS', '--frames', '500']
            command = shlex.join(argv) + ' > ' + shlex.quote(device.folder + '/tls.log') + ' 2>&1'
            device.run(command)
            device.fetch(device.folder + '/tls.log', evidence / 'Nova-tls.log')
            log = (evidence / 'Nova-tls.log').read_text()
            result['game_reported_certificate_failure'] = 'Connect failed:' in log and 'certificate' in log.lower()
            result['returned_loadout_identical'] = 'checkpoint return=restored carried=identical' in log
            result['checkpoint_unchanged'] = device.saved(device.folder + '/data/savedata') == before
        finally:
            cancel = f'127.0.0.1:{port}:127.0.0.1:{listener.getsockname()[1]}'
            subprocess.run(device.ssh[:-1] + ['-O', 'cancel', '-R', cancel, device.ssh[-1]],
                           capture_output=True, timeout=15, check=False)
        thread.join(timeout=12)
    if not all(result[name] for name in ('tls_client_hello_received', 'game_reported_certificate_failure',
                                        'returned_loadout_identical', 'checkpoint_unchanged')) or result['application_bytes']:
        raise RuntimeError('Device TLS refusal did not satisfy all checks: ' + str(result))
    return result


def run(args, evidence):
    device = Device(args, evidence)
    installed = device.saved(device.installed + '/savedata')
    peer_data = evidence / 'Desktop'
    peer_saved = initial_checkpoint(args.peer_binary, args.rom_dir, peer_data, 122, evidence / 'Desktop-checkpoint.log')
    leave = '4:START,8:,' + '4:DOWN,8:,' * 4 + '4:A,220:'
    walk = '30:UP,45:,30:DOWN,45:,24:LEFT,45:,24:RIGHT,90:,'
    device_route = '300:,' + walk * 2 + '120:,' + leave
    peer_route = '300:,' + walk * 5 + '120:,' + leave
    report = dict(platform='ROCKNIX physical handheld', endpoint=args.endpoint,
                  resolver_override=bool(args.device_resolver), installed_checkpoint_files=len(installed))
    peer = None
    try:
        device.prepare(args)
        borrowed = device.saved(device.folder + '/data/savedata')
        env = dict(os.environ, CYBERWORLD_ONLINE_TRACE=str(evidence / 'Desktop.jsonl'))
        if args.peer_resolver:
            env['LD_PRELOAD'] = str(args.peer_resolver.resolve())
        with (evidence / 'Desktop.log').open('w') as log:
            peer = subprocess.Popen([str(args.peer_binary), '--headless', '--size', '720x480', '--rom-dir', str(args.rom_dir),
                                     '--data-dir', str(peer_data), '--online', args.endpoint, '--online-name', 'NovaDesktop',
                                     '--frames', '2600', '--input', peer_route, '--shot', f'700:{evidence / "Desktop.bmp"}'],
                                    cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT)
            device.launch(args, device_route)
            print('Physical handheld and desktop proof started: ' + str(evidence), flush=True)
            started = time.monotonic()
            pictured = False
            while time.monotonic() - started < 120:
                if not pictured and time.monotonic() - started > 13:
                    device.run('source /etc/profile; grim ' + shlex.quote(device.folder + '/Nova-screen.png'))
                    device.fetch(device.folder + '/Nova-screen.png', evidence / 'Nova-screen.png')
                    pictured = True
                if device.run('test -f ' + shlex.quote(device.folder + '/done') + ' && echo done || true').strip():
                    break
                time.sleep(1)
            else:
                raise RuntimeError('Handheld did not finish within 120 seconds')
            peer.wait(timeout=60)
        for name in ('Nova.jsonl', 'Nova.bmp'):
            device.fetch(device.folder + '/' + name, evidence / name)
        device.fetch(device.folder + '/log.txt', evidence / 'Nova.log')
        rows = {name: records(evidence / (name + '.jsonl')) for name in ('Nova', 'Desktop')}
        report['clients'] = {name: motion(rows[name], rows[other])
                             for name, other in (('Nova', 'Desktop'), ('Desktop', 'Nova'))}
        worlds = {(row['seed'], row['world_hash']) for stream in rows.values() for row in stream}
        if len(worlds) != 1:
            raise RuntimeError('Device and desktop world hashes differ')
        report['world'] = list(next(iter(worlds)))
        report['borrowed_checkpoint_unchanged'] = device.saved(device.folder + '/data/savedata') == borrowed
        report['desktop_checkpoint_unchanged'] = hashes(peer_data / 'savedata') == peer_saved
        report['installed_checkpoint_unchanged'] = device.saved(device.installed + '/savedata') == installed
        report['loadouts_restored'] = all('checkpoint return=restored carried=identical' in
                                          (evidence / (name + '.log')).read_text() for name in ('Nova', 'Desktop'))
        if not all(report[key] for key in ('borrowed_checkpoint_unchanged', 'desktop_checkpoint_unchanged',
                                           'installed_checkpoint_unchanged', 'loadouts_restored')):
            raise RuntimeError('Checkpoint preservation or return failed')
        report['tls_refusal'] = tls_refusal(device, evidence)
        report['result'] = 'passed'
    except Exception as error:
        report.update(result='failed', error=str(error), remote_evidence=device.folder)
        try:
            device.fetch(device.folder + '/log.txt', evidence / 'Nova.log')
        except subprocess.SubprocessError:
            pass
        raise
    finally:
        if peer:
            stop(peer)
        (evidence / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        if report.get('result') == 'passed':
            device.cleanup()
    print(json.dumps(report, indent=2), flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('host')
    parser.add_argument('--control-path', required=True)
    parser.add_argument('--endpoint', default='wss://cyberworld.saschb2b.com/presence')
    parser.add_argument('--device-game', default='/storage/roms/ports/cyberworld')
    parser.add_argument('--device-binary', type=Path, default=ROOT / 'build/aarch64/cyberworld.aarch64')
    parser.add_argument('--peer-binary', type=Path, default=ROOT / 'build/linux/cyberworld')
    parser.add_argument('--rom-dir', type=Path, required=True)
    parser.add_argument('--device-resolver', type=Path)
    parser.add_argument('--peer-resolver', type=Path)
    args = parser.parse_args()
    base = ROOT / '.build/online-platform-rocknix'
    base.mkdir(exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='run-', dir=base))
    run(args, evidence)


if __name__ == '__main__':
    main()
