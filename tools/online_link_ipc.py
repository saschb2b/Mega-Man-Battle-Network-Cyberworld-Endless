"""Measure actual BN6 serial battles in separate processes; keep ROM evidence local."""
import argparse
import concurrent.futures
import csv
import hashlib
import json
from pathlib import Path
import shlex
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from tools.online_link_practice import compare_frames, digest  # noqa: E402


def compare_prefix(path):
    """Compare every completed active timer shared by a deadline's two traces."""
    observations = [{}, {}]
    with path.open() as stream:
        for row in csv.DictReader(stream):
            timer = int(row['battle_timer'])
            if timer and int(row['battle_state']) & 0xffff == 0x0c04:
                observations[int(row['core'])][timer] = tuple(int(row[k]) for k in ('hp0', 'hp1', 'positions'))
    common = sorted(observations[0].keys() & observations[1].keys())
    if common and common != list(range(1, common[-1] + 1)):
        raise RuntimeError('The partial trace has missing active battle frames')
    if any(observations[0][n] != observations[1][n] for n in common):
        raise RuntimeError('Partial battle HP or positions disagree')
    unmatched = [sorted(set(side) - set(common)) for side in observations]
    if any(len(side) > 1 for side in unmatched):
        raise RuntimeError('Deadline left more than one unmatched active frame')
    return dict(active_frames_compared=len(common), disagreements=0, trailing_frames=unmatched)


def run_case(case, work, mounts, wall_ms):
    import build
    name, delay, disconnect, corrupt, window = case
    inside = '/src/' + str(work.relative_to(ROOT))
    command = work / (name + '.txt')
    mode = 'ipcactive' if window == 'active' else 'ipcfull'
    command.write_text(f'{mode} {delay} {wall_ms} {disconnect} {corrupt}\n')
    log = work / (name + '.log')
    invocation = ' '.join(shlex.quote(part) for part in (
        'build/host/online-link-bn6', '/probe/bn6.gba', '/probe/boot-3.state'))
    invocation += ' < ' + shlex.quote(inside + '/' + command.name)
    invocation += ' > ' + shlex.quote(inside + '/' + log.name) + ' 2>&1'
    code = build.docker('sh', '-c', invocation, mounts=mounts,
                       env=[('ONLINE_LINK_IPC_TRACE', inside + '/' + name + '-trace.csv')])
    result = next((json.loads(line) for line in log.read_text().splitlines() if line.startswith('{"ipc":')), None)
    if result is None:
        raise RuntimeError(f'{name}: no structured IPC result; see {log}')
    if len(set(result['pids'])) != 2 or any(pid <= 0 for pid in result['pids']):
        raise RuntimeError(f'{name}: missing independent worker process IDs')
    result.update(case=name, exit_code=code, processes=2, delay_window=window)
    expected = 'disconnected' if disconnect else 'desync' if corrupt else None
    if expected:
        if result['status'] != expected or min(result['transfers']) < disconnect + corrupt:
            raise RuntimeError(f'{name}: expected injected {expected}, got {result["status"]}')
        result['fault_detected'] = True
    else:
        if result['status'] not in ('complete', 'deadline'):
            raise RuntimeError(f'{name}: unexpected {result["status"]}; see {log}')
        complete = result['status'] == 'complete'
        if complete and (not result['battle_agreement'] or code):
            raise RuntimeError(f'{name}: battle did not finish with matching results')
        if not delay and not complete:
            raise RuntimeError('Zero-delay baseline must complete the real battle')
        if delay and (not result['delayed_transfers'] or result['applied_delay_ms'] < result['delayed_transfers'] * delay - delay):
            raise RuntimeError(f'{name}: configured serial delay was not applied')
        if result['transfers'][0] == result['transfers'][1] and result['transcripts'][0] != result['transcripts'][1]:
            raise RuntimeError(f'{name}: serial transcripts disagree')
        result.update((compare_frames if complete else compare_prefix)(work / (name + '-trace.csv')))
        result['battle_completed'] = complete
        result['feasibility'] = 'completed' if complete else 'unfinished_at_deadline'
    (work / (name + '-report.json')).write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result), flush=True)
    return result


def run(rom, state, wall_ms=60000, workers=3, window='both'):
    import build
    rom, state = rom.resolve(), state.resolve()
    if not rom.is_file() or hashlib.sha1(rom.read_bytes()).hexdigest() != '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6':
        raise RuntimeError('IPC Practice needs the unmodified BN6 Cybeast Gregar USA ROM')
    if not state.is_file() or state.name != 'boot-3.state':
        raise RuntimeError('IPC Practice needs the pre-director boot-3.state')
    before = dict(rom_sha256=digest(rom), boot_sha256=digest(state))
    build.ensure_image()
    if build.docker('make', 'online-link-bn6', 'WERROR=1'):
        return 1
    parent = ROOT / '.build/online-battle'
    parent.mkdir(parents=True, exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix='ipc-', dir=parent))
    mounts = [(str(rom), '/probe/bn6.gba:ro'), (str(state), '/probe/boot-3.state:ro')]
    cases = [('delay0', 0, 0, 0, 'custom')]
    if window in ('custom', 'both'):
        cases += [(f'delay{n}', n, 0, 0, 'custom') for n in (50, 100, 150)]
    if window in ('active', 'both'):
        cases += [(f'active{n}', n, 0, 0, 'active') for n in (50, 100, 150)]
    cases += [('disconnect', 0, 20, 0, 'custom'), ('corruption', 0, 0, 20, 'custom')]
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
        futures = [pool.submit(run_case, case, work, mounts, wall_ms) for case in cases]
        results = [future.result() for future in futures]
    if before != dict(rom_sha256=digest(rom), boot_sha256=digest(state)):
        raise RuntimeError('A read-only input changed')
    baseline = results[0]['transfers'][0]
    for result in results:
        if not result['delay_ms']:
            continue
        remaining = max(0, baseline - min(result['transfers']))
        result['remaining_delay_lower_bound_ms'] = remaining * result['delay_ms']
    report = dict(source_files_unchanged=True, **before, output_directory=str(work), cases=results,
                  delay_model='One optimistic delivery delay per genuine serial transfer; not RTT. '
                              'custom starts at the first Custom screen; active excludes negotiation and Custom selection, starting once both battle timers run. '
                              'Cycle bookkeeping is local IPC. Each child executes only its own core; inherited peer cores are inert driver shadows.',
                  verdict='Raw serial transport feasibility measured; no production Arena implementation.')
    (work / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom', type=Path, required=True)
    parser.add_argument('--state', type=Path, required=True)
    parser.add_argument('--wall-ms', type=int, default=60000)
    parser.add_argument('--workers', type=int, choices=(1, 2, 3, 4), default=3)
    parser.add_argument('--window', choices=('custom', 'active', 'both'), default='both')
    args = parser.parse_args()
    if not 10000 <= args.wall_ms <= 3600000:
        parser.error('--wall-ms must be between 10000 and 3600000')
    try:
        return run(args.rom, args.state, args.wall_ms, args.workers, args.window)
    except RuntimeError as error:
        parser.exit(1, str(error) + '\n')


if __name__ == '__main__':
    raise SystemExit(main())
