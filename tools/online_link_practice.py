"""Run BN6's own Practice battle through two linked cores; keep evidence local."""
import csv
import hashlib
import json
from pathlib import Path
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def compare_frames(path):
    observations = [{}, {}]
    with path.open() as stream:
        for row in csv.DictReader(stream):
            timer = int(row['battle_timer'])
            if timer and int(row['battle_state']) & 0xffff == 0x0c04:
                observations[int(row['core'])][timer] = tuple(int(row[key]) for key in ('hp0', 'hp1', 'positions'))
    common = sorted(observations[0].keys() & observations[1].keys())
    if not common or common != list(range(1, common[-1] + 1)):
        raise RuntimeError('The trace does not cover every active battle frame')
    if observations[0].keys() != observations[1].keys():
        raise RuntimeError('One core has missing battle frames')
    mismatches = [frame for frame in common if observations[0][frame] != observations[1][frame]]
    if mismatches:
        raise RuntimeError(f'Battle HP or positions disagree at timer {mismatches[0]}')
    damage = []
    for frame in common:
        hp = observations[0][frame][:2]
        if not damage or damage[-1]['hp'] != list(hp):
            damage.append(dict(timer=frame, hp=list(hp)))
    return dict(active_frames_compared=len(common), disagreements=0, damage=damage)


def run(rom, state=None):
    import build
    rom = rom.resolve()
    if not rom.is_file():
        raise RuntimeError(f'ROM not found: {rom}')
    if hashlib.sha1(rom.read_bytes()).hexdigest() != '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6':
        raise RuntimeError('Practice needs the unmodified BN6 Cybeast Gregar USA ROM')
    build.ensure_image()
    if build.docker('make', 'online-link-bn6', 'WERROR=1'):
        return 1
    parent = ROOT / '.build/online-battle'
    parent.mkdir(parents=True, exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    inside = '/src/' + str(work.relative_to(ROOT))
    mounts = [(str(rom), '/probe/bn6.gba:ro')]
    if state is None:
        # Use the project's existing deterministic boot, before its director
        # patches a run map. The experiment itself loads the plain ROM.
        build.build('host')
        data = work / 'bootstrap'
        data.mkdir()
        if build.docker('build/host/cyberworld', '--headless', '--rom-dir', '/probe',
                        '--data-dir', inside + '/bootstrap', '--scene', 'emu', '--frames', '1',
                        '--seed', '120', mounts=mounts):
            return 1
        state = data / 'boot-3.state'
    else:
        state = state.resolve()
    if not state.is_file() or state.name != 'boot-3.state':
        raise RuntimeError('Practice needs the pre-director boot-3.state; a run checkpoint cannot execute with an unpatched ROM')
    before = dict(rom_sha256=digest(rom), boot_sha256=digest(state))
    mounts.append((str(state), '/probe/boot-3.state:ro'))
    code = build.docker('build/host/online-link-bn6', '--practice', '/probe/bn6.gba',
                        '/probe/boot-3.state', inside + '/practice', mounts=mounts)
    if code:
        raise RuntimeError(f'Practice battle failed; local evidence: {work}')
    report = json.loads((work / 'practice-report.json').read_text())
    report.update(compare_frames(work / 'practice-trace.csv'))
    after = dict(rom_sha256=digest(rom), boot_sha256=digest(state))
    if before != after:
        raise RuntimeError('A read-only ROM or boot-state input changed')
    report.update(before, source_files_unchanged=True, output_directory=str(work))
    if not report['passed']:
        raise RuntimeError('Both games did not agree on the battle result')
    (work / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))
    return 0
