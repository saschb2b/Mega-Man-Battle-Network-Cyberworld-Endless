"""Reproduce the bounded linked-pair snapshot proof using the player's ROM.

Raw states and flash never leave memory; local traces contain only numbers and
hashes. The separately reviewed comparator is the sole acceptance authority.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from tools.online_replay_compare import (  # noqa: E402
    Mismatch, Unreached, TARGET, POINTS, MUTATIONS, accept_observer,
    accept_replays, accept_mutation, accept_cable_stop, compare_game, read_trace, read_cable,
    select_points, verify_manifest,
)

# Reviewed original controls; changes need their own review before repinning.
PINS = {
    "docs/re/arena-cable-boundary.md": "ca8d1481c452e5d3872d444de40ef3d32bdafeff5ef6b2ab4fe4f78d235a7563",
    "docs/re/arena-cable-omission.md": "d35af67c21dcade459746fd7233f110cc5b6bc8c0cad3a01044e4c58f546322a",
    "docs/re/arena-next-goal.md": "f1ba9d8b3664b617f095c9fce8daa929b1ebeb1ad92197edbc9e3f848af95636",
    "src/core/sha256.c": "a8501a17522241e931d01dcec699170373db50339a51c701c09abbf5a8b94b24",
    "src/core/sha256.h": "d82bef532dd873452a24414e098a036c0d43b3661d68fea9a9f01dd7da44ad73",
    "src/emu/bn6.h": "8681082989ce5520e8da66478acee46ea0dec403b68179b300ef3fd1a2f02036",
    "src/emu/bn6_attacks.h": "d0c3c53ac4ea169632cc4da0e376c82bef24e41af391864a9fff32230fe390d8",
    "src/emu/bn6_chips.h": "cb98fd2d2f2859c4ed5cbd0af3dd2fa9838bfb8b9b2c1504635ddb874f26e4ba",
    "src/emu/bn6_families.h": "c2390d4c1ca0301bb7a3a364ce17474674cf506e434811f9ccafe622540a508e",
    "src/emu/bn6_forms.h": "ef8f5a2691f7e3c664801ddb4cab5bd90cf255708db64403a6d8bb4bd77f6a5a",
    "src/emu/bn6_navis.h": "0562b8f1fa04c08ea4ea5c3da2a9ba45daeb3df6f323cb1817bfdf3f92ff9c50",
    "src/emu/bn6_panels.h": "a94707aeed415b27d54d6c35690bf4a1e8b5404cb68db4072651d2fa125bb9a7",
    "src/emu/bn6_pet.h": "b3606cfe7fbc8e131befd04369f5ab9980333894f06692386443740fad3d65f1",
    "tests/test_online_replay_compare.py": "36bfc11cc523457c766497921fdb2c9b22172f24034a89496eb435448c8803c4",
    "tools/online_frame_compare.py": "155c12abbb3e7de93f724b0572aec22a59fda1d31a2aa5676c9cdc5a724e292b",
    "tools/online_frame_pair.c": "eb483a3a0dbe5d78a4717dbbcfa730b0e5438c0ce2fcf136f9cdce8bc0ec9ed4",
    "tools/online_frame_pair.h": "fa5fe070e4043ff130eba017ab53e4d74af3474588f2676f79b711c3108c6a71",
    "tools/online_frame_pair_internal.h": "eb146b21b9b62b5bc5f6045c8ddd922dc57a8ff8359671ae6c9bf39f67750432",
    "tools/online_frame_wire.c": "c29d8d1ca06ccaa586e10939432710e77b8ce02d05e3dd0313cb8612b04097f2",
    "tools/online_frame_wire.h": "910d89a73e57320ef57e8790b58520054ec9ccda348aaa14ac6e70c90ad711ab",
    "tools/online_link_bn6.c": "3cf188845d4d7885d8aa1dbb6abf0ec6fc53f9e59cf3bef31b43431f3c52eceb",
    "tools/online_link_ipc.c": "bc5c01f2dcb1ce86d6ff88a7077d5a81b3eb841d47cb56e7802a6011116e98d4",
    "tools/online_link_ipc.h": "111d7de1654e747ee445481ad717a2232d7641ae98ebe01fd7e402351fbfbd40",
    "tools/online_link_ipc_test.c": "2d675ac78631c6f16620231dbf6f9dbbc32118435106f13db9e362c329a25809",
    "tools/online_link_ipc_wire.c": "0904f5a34b192387b3ae2f973b5cfeb563610559c120eb87e1f1542862cb3440",
    "tools/online_link_ipc_wire.h": "8798863f9486a4b52635581252b8542843ef7a75c5c57f013623a2418cdf1175",
    "tools/online_link_script.c": "bbca6b2079acc64f1fdd5b1f78fbca328f222b9df0fe9e358529d638c359eaf8",
    "tools/online_link_script.h": "5fdb06cc4b9d661e2b212ec97b06d870bd818b6f840b0072e4c2ad2b9f57dc5b",
    "tools/online_replay_compare.py": "12e1146aa806617fe4425de78fd4c86e3e419d3efb181adcf538547da8ca3dd3",
    "tools/online_replay_execute.c": "8fba507ff54395600011e4eee4af3679a847c98459b251e0e6709b3a0b8d4244",
    "tools/online_replay_execute.h": "56a9b89b3c1a2df077b66c88b5e71aa7e19db85eecf98016e6779056b9ff8745",
    "tools/online_replay_micro_observer.c": "062f4574cccf74e1a36f189ce764c5dbc2b1029e2eb2f49b768fdb9447fb7a65",
    "tools/online_replay_observe.c": "5c4dce38d643a230340c0fb6f57e56fad182cf36aecca84ca39b0afbe6add8a0",
    "tools/online_replay_observe.h": "0ce82f9e625ca9102344f3421eccfb69ce8913130c2c77430a8fc7d22b6ae9fe",
    "tools/online_replay_observer.c": "8feacf9840b1829de311057144a4a31e5e3fc79ffcd4052945bb269f05fecc35",
    "tools/online_replay_snapshot.c": "9a6fdd14de2f412daf83c3b57102633ef6367d22a35ae4d1969747eb23c6fabc",
    "tools/online_replay_snapshot.h": "316879309662ed7c905a60a075dab242b4fced72e98e5caa09b0c278b6c7476a",
    "tools/online_replay_subject.c": "3ff7dca0f4c5d6c91fb5594c31d8cd4b0366ee14373b1e838ad1623d6e452c41"
}

BASELINE_TRACE = '030a1976e81a4b730d378bf030da9dd465945b651779f980dec937f14ea761c0'
SUBJECT_SOURCES = ('tools/online_replay_snapshot.c', 'tools/online_replay_snapshot.h',
                   'tools/online_replay_subject.c', 'tools/online_replay_proof.py')
BINARIES = ('online-link-bn6', 'online-replay-observer', 'online-replay-micro-observer', 'online-replay-subject')
RECIPE_SHA256 = '756441497468af1bd420fd5655f090cb4279ba29198b337b84fa640d6d3d8f69'


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def pinned_sources():
    actual = {path: digest(ROOT / path) for path in PINS}
    if not PINS or actual != PINS:
        raise Mismatch('Reviewed observer/comparator changed; separate review required')
    return actual


def target(rom, state):
    return dict(rom_sha256=digest(rom), boot_sha256=digest(state),
                linked_library_sha256=digest('/opt/mgba/host/lib/libmgba.a'))


def build_recipe():
    result = subprocess.run(['make', '--no-print-directory', '-nB', *BINARIES, 'WERROR=1'],
                            cwd=ROOT, capture_output=True, text=True, check=True, timeout=30)
    return '\n'.join(' '.join(line.split()) for line in result.stdout.splitlines() if line.strip()) + '\n'


def verify_recipe():
    recipe = build_recipe()
    if hashlib.sha256(recipe.encode()).hexdigest() != RECIPE_SHA256:
        raise Mismatch('Effective research build recipes changed; separate review required')
    return recipe


def run_control(binary, rom, state, prefix):
    command = [f'build/host/{binary}']
    if binary == 'online-link-bn6':
        command.append('--practice')
    with prefix.with_suffix('.log').open('wb') as log:
        subprocess.run([*command, str(rom), str(state), str(prefix)],
                       stdout=log, stderr=log, check=True, timeout=180)


def controls(rom, state, work):
    jobs = [('online-link-bn6', 'original-a'), ('online-link-bn6', 'original-b'),
            ('online-replay-observer', 'baseline'),
            ('online-replay-micro-observer', 'micro-a'),
            ('online-replay-micro-observer', 'micro-b')]
    with ThreadPoolExecutor(max_workers=3) as pool:
        futures = [pool.submit(run_control, binary, rom, state, work / prefix) for binary, prefix in jobs]
        for future in futures:
            future.result()
    for name in ('original-a', 'original-b'):
        report = json.loads((work / f'{name}-report.json').read_text())
        if not report.get('passed'):
            raise Mismatch(f'{name}: original Practice did not complete')
    compare_game(work / 'original-a-trace.csv', work / 'original-b-trace.csv')
    if digest(work / 'baseline.jsonl') != BASELINE_TRACE:
        raise Mismatch('Fresh baseline differs from the reviewed original full trace')
    return accept_observer(work / 'original-a-trace.csv', work / 'micro-a.csv',
                           work / 'micro-a.jsonl', work / 'micro-b.jsonl',
                           work / 'micro-a-cable.json', work / 'micro-b-cable.json',
                           work / 'baseline.jsonl')


def run_subject(rom, state, work, manifest):
    rows = read_trace(work / 'micro-a.jsonl')
    cable = read_cable(work / 'micro-a-cable.json', rows)
    points = select_points(rows, cable)
    selection = work / 'selection.txt'
    selection.write_text(''.join(f'{name} {points[name]} '
                                 f'{cable["cursor"]["round"] if name == "nonidle-cable" else 0}\n'
                                 for name in POINTS))
    with (work / 'subject.log').open('wb') as log:
        started = time.monotonic()
        completed = subprocess.run(['build/host/online-replay-subject', str(rom), str(state),
                                    str(selection), str(work)], stdout=log, stderr=log,
                                   check=False, timeout=180)
        termination = dict(returncode=completed.returncode, elapsed_seconds=time.monotonic() - started,
                           binary_sha256=digest('build/host/online-replay-subject'))
    manifest['after'] = target(rom, state)
    verify_manifest(manifest, PINS)
    if completed.returncode not in (0, 3):
        raise Unreached(f'Subject did not complete or reach its validated bound: {completed.returncode}')
    result = accept_replays(work / 'micro-a.jsonl', work / 'noop.jsonl', work, work / 'micro-a-cable.json')
    mutations = []
    for name in MUTATIONS:
        if name == 'omit-cable' and completed.returncode == 3:
            mutations.append(accept_cable_stop(work, manifest, PINS, termination))
        else:
            mutations.append(accept_mutation(work / 'micro-a.jsonl', name, work / f'{name}.jsonl',
                                             work / 'micro-a-cable.json', work / f'{name}-immediate.json'))
    result['termination'] = termination
    return result, mutations


def inside(rom, state, work):
    recipe = verify_recipe()
    manifest = dict(before=target(rom, state), after=None, source_hashes=pinned_sources(),
                    binary_hashes=dict(oracle=digest('build/host/online-link-bn6'),
                                       subject=digest('build/host/online-replay-subject')))
    if manifest['before'] != TARGET:
        raise Mismatch('Wrong ROM, boot state or linked mGBA build')
    subject_sources = {name: digest(ROOT / name) for name in SUBJECT_SOURCES}
    binaries = {name: digest(ROOT / 'build/host' / name) for name in BINARIES}
    report = dict(verdict='INCONCLUSIVE', production_arena_enabled=False, masks=[],
                  scope='local starter Practice pair; exact 1/4/8-frame replay, twice at six exercised points',
                  library='mGBA 0.10.5 Release/NDEBUG',
                  core_source_sha256='91d6fbd32abcbdf030d58d3f562de25ebbc9d56040d513ff8e5c19bee9dacf14',
                  subject_sources=subject_sources, binaries=binaries, manifest=manifest,
                  build_recipe=recipe, build_recipe_sha256=RECIPE_SHA256)
    try:
        report['observer'] = controls(rom, state, work)
        report['replays'], report['mutations'] = run_subject(rom, state, work, manifest)
        report['statistics'] = json.loads((work / 'statistics.json').read_text())
        report['verdict'] = 'PASS'
    except Unreached as error:
        report['error'] = str(error)
    except (Mismatch, OSError, ValueError, subprocess.SubprocessError) as error:
        report['verdict'], report['error'] = 'FAIL', str(error)
    finally:
        manifest['after'] = target(rom, state)
        verify_manifest(manifest, PINS)
        pinned_sources()
        verify_recipe()
        if subject_sources != {name: digest(ROOT / name) for name in SUBJECT_SOURCES}:
            raise Mismatch('Subject sources changed during the proof')
        if binaries != {name: digest(ROOT / 'build/host' / name) for name in BINARIES}:
            raise Mismatch('A proof executable changed during the run')
        report['trace_sha256'] = {p.name: digest(p) for p in sorted(work.iterdir())
                                  if p.suffix in ('.jsonl', '.json', '.csv') and p.name != 'report.json'}
        (work / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))
    return 0 if report['verdict'] == 'PASS' else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom', required=True, type=Path)
    parser.add_argument('--state', required=True, type=Path)
    parser.add_argument('--inside', action='store_true', help=argparse.SUPPRESS)
    parser.add_argument('--work', type=Path, help=argparse.SUPPRESS)
    args = parser.parse_args()
    rom, state = args.rom.resolve(), args.state.resolve()
    try:
        pinned_sources()
        before = (digest(rom), digest(state))
        if before != (TARGET['rom_sha256'], TARGET['boot_sha256']):
            raise Mismatch('This bounded proof requires its reviewed original ROM and boot state')
        if args.inside:
            return inside(rom, state, args.work)
        import build
        build.ensure_image()
        if build.docker('python3', '-c', 'from tools.online_replay_proof import verify_recipe; verify_recipe()'):
            return 1
        if build.docker('make', *BINARIES, 'WERROR=1'):
            return 1
        parent = ROOT / '.build/online-replay'
        parent.mkdir(parents=True, exist_ok=True)
        work = Path(tempfile.mkdtemp(prefix='proof-', dir=parent))
        code = build.docker('python3', 'tools/online_replay_proof.py', '--inside', '--work',
                            '/src/' + str(work.relative_to(ROOT)), '--rom', '/probe/bn6.gba',
                            '--state', '/probe/boot-3.state',
                            mounts=[(str(rom), '/probe/bn6.gba:ro'), (str(state), '/probe/boot-3.state:ro')])
        if before != (digest(rom), digest(state)):
            raise Mismatch('Read-only source changed')
        print(f'Local evidence: {work}')
        return code
    except (Mismatch, OSError, ValueError, subprocess.SubprocessError) as error:
        parser.exit(1, str(error) + '\n')


if __name__ == '__main__':
    raise SystemExit(main())
