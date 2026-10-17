"""Compare delayed frame-input Practice battles with the pristine local cable.

Developer-only, Linux/POSIX. All evidence stays in .build/online-frame.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from tools.online_frame_compare import Mismatch, accept_case, accept_fault, compare_game  # noqa: E402

PINS = {
    'tools/online_link_bn6.c': '3cf188845d4d7885d8aa1dbb6abf0ec6fc53f9e59cf3bef31b43431f3c52eceb',
    'tools/online_link_script.c': 'bbca6b2079acc64f1fdd5b1f78fbca328f222b9df0fe9e358529d638c359eaf8',
    'tools/online_frame_compare.py': '155c12abbb3e7de93f724b0572aec22a59fda1d31a2aa5676c9cdc5a724e292b',
    'tests/test_online_frame_compare.py': 'b6dabbd20a054995701ef5e3fe5a3a57135e06a0c3527d6e1e4a5f896e9fdb62',
    'docs/re/arena-frame-goal.md': 'c711930e689bc5f4d215d72d46ca7619eb3e13acd1724f1e6ad27f78972a62cf',
}
ROM_SHA1 = '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6'


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def verify_inputs(rom, state):
    if hashlib.sha1(rom.read_bytes()).hexdigest() != ROM_SHA1:
        raise RuntimeError('The proof requires the unmodified BN6 Gregar USA ROM')
    if not state.is_file() or state.name != 'boot-3.state':
        raise RuntimeError('The proof requires pre-director boot-3.state, not a run checkpoint')
    for name, expected in PINS.items():
        if sha256(ROOT / name) != expected:
            raise RuntimeError(f'Reviewed oracle changed: {name}; obtain separate review before repinning')
    return dict(rom_sha256=sha256(rom), boot_sha256=sha256(state))


def run_original(rom, state, prefix):
    with prefix.with_suffix('.log').open('wb') as log:
        subprocess.run(['build/host/online-link-bn6', '--practice', str(rom), str(state), str(prefix)],
                       stdout=log, stderr=log, check=True, timeout=90)
    report = json.loads(Path(str(prefix) + '-report.json').read_text())
    if not report.get('passed'):
        raise Mismatch('Pristine local Practice did not complete')
    return Path(str(prefix) + '-trace.csv')


def inside(rom, state, work, delays, faults):
    from tools.online_frame_relay import run_case
    before = verify_inputs(rom, state)
    manifest = dict(**before, oracle_pins=PINS, core='mGBA 0.10.5 Release/NDEBUG',
                    core_source_sha256='91d6fbd32abcbdf030d58d3f562de25ebbc9d56040d513ff8e5c19bee9dacf14',
                    linked_library_sha256=sha256('/opt/mgba/host/lib/libmgba.a'),
                    oracle_binary_sha256=sha256('build/host/online-link-bn6'),
                    worker_binary_sha256=sha256('build/host/online-frame-worker'),
                    source_hashes={str(path.relative_to(ROOT)): sha256(path)
                                   for path in (ROOT / 'tools').glob('online_frame_*') if path.is_file()})
    (work / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    reference = run_original(rom, state, work / 'original-a')
    repeat = run_original(rom, state, work / 'original-b')
    original_frames = compare_game(reference, repeat)
    results = [dict(case='original-control', verdict='PASS', active_timers=original_frames)]
    for delay in delays:
        run_case(rom, state, work, delay)
        results.append(accept_case(reference, work, f'delay{delay}', delay))
        if before != verify_inputs(rom, state):
            raise Mismatch('A read-only source changed after a battle')
    if faults:
        for fault in ('corruption', 'disconnect'):
            run_case(rom, state, work, 50, fault)
            results.append(accept_fault(work, fault))
            if before != verify_inputs(rom, state):
                raise Mismatch('A read-only source changed after a fault')
    if before != verify_inputs(rom, state):
        raise Mismatch('A read-only source changed')
    results.append(dict(case='source-preservation', verdict='PASS'))
    report = dict(verdict='PASS', scope='starter Practice; fixed one-way input delay; two local cores per worker',
                  results=results, source_files_unchanged=True, production_arena_enabled=False,
                  complete_spec=sorted(set(delays)) == [0, 50, 100, 150] and faults,
                  rollback=False, **manifest)
    (work / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom', required=True, type=Path)
    parser.add_argument('--state', required=True, type=Path)
    parser.add_argument('--delays', nargs='+', type=int, choices=(0, 50, 100, 150), default=[0, 50, 100, 150])
    parser.add_argument('--no-faults', action='store_true', help='partial research run; omit fault cases')
    parser.add_argument('--inside', action='store_true', help=argparse.SUPPRESS)
    parser.add_argument('--work', type=Path, help=argparse.SUPPRESS)
    args = parser.parse_args()
    rom, state = args.rom.resolve(), args.state.resolve()
    try:
        before = verify_inputs(rom, state)
        if args.inside:
            return inside(rom, state, args.work, args.delays, not args.no_faults)
        import build
        build.ensure_image()
        if build.docker('make', 'online-frame', 'online-link-bn6', 'WERROR=1'):
            return 1
        parent = ROOT / '.build/online-frame'
        parent.mkdir(parents=True, exist_ok=True)
        work = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
        command = ['python3', 'tools/online_frame_proof.py', '--inside', '--work',
                   '/src/' + str(work.relative_to(ROOT)), '--rom', '/probe/bn6.gba',
                   '--state', '/probe/boot-3.state', '--delays', *map(str, args.delays)]
        if args.no_faults:
            command.append('--no-faults')
        code = build.docker(*command, mounts=[(str(rom), '/probe/bn6.gba:ro'),
                                             (str(state), '/probe/boot-3.state:ro')])
        if before != verify_inputs(rom, state):
            raise Mismatch('A read-only source changed')
        print(f'Local evidence: {work}')
        return code
    except (RuntimeError, OSError, subprocess.SubprocessError) as error:
        parser.exit(1, str(error) + '\n')


if __name__ == '__main__':
    raise SystemExit(main())
