"""Reproduce original-only carried-chip controls before building an installer.

The player's ROM and boot state stay read-only. Derived native map states and
all detailed traces remain local. PASS covers these controls, not Arena.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from tools.online_carried_compare import (Mismatch, Unreached, check_seed, chip_evidence,
    compare_controls, compare_loadout, read_trace, require)  # noqa: E402
from tools.online_frame_compare import compare_game  # noqa: E402
from tools.online_replay_compare import TARGET  # noqa: E402

PINS = {
    "docs/re/arena-carried-boundary.md": "cf8e472359f9b6db722982ef26512db2b9e034659ad58e11bd41f1ebd2f89ecb",
    "docs/re/arena-next-goal.md": "f1ba9d8b3664b617f095c9fce8daa929b1ebeb1ad92197edbc9e3f848af95636",
    "src/core/sha256.c": "a8501a17522241e931d01dcec699170373db50339a51c701c09abbf5a8b94b24",
    "src/core/sha256.h": "d82bef532dd873452a24414e098a036c0d43b3661d68fea9a9f01dd7da44ad73",
    "src/emu/bn6.h": "8681082989ce5520e8da66478acee46ea0dec403b68179b300ef3fd1a2f02036",
    "src/emu/bn6_attacks.h": "d0c3c53ac4ea169632cc4da0e376c82bef24e41af391864a9fff32230fe390d8",
    "src/emu/bn6_chips.h": "cb98fd2d2f2859c4ed5cbd0af3dd2fa9838bfb8b9b2c1504635ddb874f26e4ba",
    "src/emu/bn6_families.h": "c2390d4c1ca0301bb7a3a364ce17474674cf506e434811f9ccafe622540a508e",
    "src/emu/bn6_fields.h": "d39de71a4d99e398589bfad2d2fe78388aa5bcb4d4d9bc3d33ad371ee3db2ee5",
    "src/emu/bn6_forms.h": "ef8f5a2691f7e3c664801ddb4cab5bd90cf255708db64403a6d8bb4bd77f6a5a",
    "src/emu/bn6_navis.h": "0562b8f1fa04c08ea4ea5c3da2a9ba45daeb3df6f323cb1817bfdf3f92ff9c50",
    "src/emu/bn6_panels.h": "a94707aeed415b27d54d6c35690bf4a1e8b5404cb68db4072651d2fa125bb9a7",
    "src/emu/bn6_pet.h": "b3606cfe7fbc8e131befd04369f5ab9980333894f06692386443740fad3d65f1",
    "src/emu/emu.h": "7b995e6acdf81f15558a50d8ae45e67a267fcdcf07c59afedb09f3530fa09d2a",
    "src/emu/free_space.h": "76df929b16812126ab01717b04e819e3bd8c4f3fd02669f845509f1b694700d4",
    "src/emu/hook.h": "7ba6e14a60363508c36cf417351d5c049ecbf1bae5aa8aa37e51dea6ebe21d63",
    "src/online/online_snapshot.c": "9a17dab9dc5bfac9b754c087200c0f60c171ffd0251f8a5048c43212492bb44a",
    "src/online/online_snapshot.h": "77e47a3d9b35147b650346273c6717e7110cfe3c8cfc1c314fbc8ec9b24ae29c",
    "tests/test_online_carried_compare.py": "eb876a0cb7b81f9b5dfc85bab53dd65668a4a0f896795d3470bd9f3fc3a09aca",
    "tools/online_carried_compare.py": "f5fe742f154f9ebfe880ec8f38a0c8f436c6119c5402de4e7fadab3fa41e1d0c",
    "tools/online_carried_native.c": "3dc50a1a8f6b71fc6f1ff90cb24a44fd9d8a7afa7677ec91c5a7b678f3092482",
    "tools/online_carried_observe.c": "cae3b819e50b20edcf8a82a49f3f5a8d33aea04d20be03dd5d31b507e78c0bd3",
    "tools/online_carried_observe.h": "779860a3108bc900d00f5ec48df25fff3fc8e486af3efdf0bf9fa9de8079832a",
    "tools/online_carried_observer.c": "700f2025963829401e1c90eda05e357e81bde2f7ddb7800fd1b74241b0745901",
    "tools/online_carried_read.c": "640fb287acee5abfae0f608815052010d8306a378efc0ddfa490cbcdc6803e23",
    "tools/online_carried_read.h": "ca82abfe0a5c50fa370da84185d660ede599f91dfbd6e554bb5ace687c790aa0",
    "tools/online_carried_script.c": "93b35a1d8c440b67c88c9569d8e9af36d2f3d2a15b09bfe8429681187155a60b",
    "tools/online_carried_script.h": "343974b06a26536e36df501d3203367de3d205a6130e9cc08ed79edcd565268a",
    "tools/online_frame_compare.py": "155c12abbb3e7de93f724b0572aec22a59fda1d31a2aa5676c9cdc5a724e292b",
    "tools/online_frame_pair.c": "eb483a3a0dbe5d78a4717dbbcfa730b0e5438c0ce2fcf136f9cdce8bc0ec9ed4",
    "tools/online_frame_pair.h": "fa5fe070e4043ff130eba017ab53e4d74af3474588f2676f79b711c3108c6a71",
    "tools/online_frame_pair_internal.h": "eb146b21b9b62b5bc5f6045c8ddd922dc57a8ff8359671ae6c9bf39f67750432",
    "tools/online_frame_wire.c": "c29d8d1ca06ccaa586e10939432710e77b8ce02d05e3dd0313cb8612b04097f2",
    "tools/online_frame_wire.h": "910d89a73e57320ef57e8790b58520054ec9ccda348aaa14ac6e70c90ad711ab",
    "tools/online_link_bn6.c": "3cf188845d4d7885d8aa1dbb6abf0ec6fc53f9e59cf3bef31b43431f3c52eceb",
    "tools/online_link_ipc.c": "bc5c01f2dcb1ce86d6ff88a7077d5a81b3eb841d47cb56e7802a6011116e98d4",
    "tools/online_link_ipc.h": "111d7de1654e747ee445481ad717a2232d7641ae98ebe01fd7e402351fbfbd40",
    "tools/online_link_ipc_wire.c": "0904f5a34b192387b3ae2f973b5cfeb563610559c120eb87e1f1542862cb3440",
    "tools/online_link_ipc_wire.h": "8798863f9486a4b52635581252b8542843ef7a75c5c57f013623a2418cdf1175",
    "tools/online_link_script.c": "bbca6b2079acc64f1fdd5b1f78fbca328f222b9df0fe9e358529d638c359eaf8",
    "tools/online_link_script.h": "5fdb06cc4b9d661e2b212ec97b06d870bd818b6f840b0072e4c2ad2b9f57dc5b",
    "tools/online_replay_compare.py": "12e1146aa806617fe4425de78fd4c86e3e419d3efb181adcf538547da8ca3dd3",
    "tools/online_replay_observe.c": "5c4dce38d643a230340c0fb6f57e56fad182cf36aecca84ca39b0afbe6add8a0",
    "tools/online_replay_observe.h": "0ce82f9e625ca9102344f3421eccfb69ce8913130c2c77430a8fc7d22b6ae9fe"
}
RECIPE_SHA256 = '0572a9c8f8018fb7c6479e01041bd23a451ad1717d2f05f18bf962b2637307c4'
FIXTURE_SHA256 = '4975e5f78e1fe3375ed8aad42c31ffd91eb58e0d6f86dcaad857d78751c7c5fe'
FULL_SHA256 = '906d2192be4d949411e6a49e5bfc8f82c5fed6705b8c54d4d487ba0604d8b864'
TRACE_SHA256 = 'b72353090476bc9664161cdff0b3c058e9746c149a7dfd9a855a5b8998e41f3a'
BINARIES = ('online-carried-native', 'online-carried-observer', 'online-carried-original')


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def sources():
    result = {name: digest(ROOT / name) for name in PINS}
    require(PINS and result == PINS, 'Reviewed carried-chip control changed; separate review required')
    return result


def target(rom, state):
    result = dict(rom_sha256=digest(rom), boot_sha256=digest(state),
                  linked_library_sha256=digest('/opt/mgba/host/lib/libmgba.a'))
    require(result == TARGET, 'Wrong ROM, source checkpoint or mGBA library')
    return result


def recipe():
    command = ['make', '--no-print-directory', '-nB', *['build/host/' + n for n in BINARIES], 'WERROR=1']
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, check=True, timeout=30)
    text = '\n'.join(' '.join(line.split()) for line in result.stdout.splitlines() if line.strip()) + '\n'
    require(hashlib.sha256(text.encode()).hexdigest() == RECIPE_SHA256, 'Research build recipe changed')
    return text


def execute(command, work, name):
    with (work / f'{name}.log').open('wb') as log:
        result = subprocess.run(command, stdout=log, stderr=log, timeout=180, check=False)
    if result.returncode:
        raise Unreached(f'Original control {name} stopped with exit {result.returncode}')


def native_fixtures(rom, state, work):
    def prepare(name):
        execute(['build/host/online-carried-native', str(rom), str(state), str(work / name)], work, name)
    with ThreadPoolExecutor(max_workers=2) as pool:
        list(pool.map(prepare, ('native-a', 'native-b')))
    for name in ('native-a', 'native-b'):
        for side in (0, 1):
            require(digest(work / f'{name}-{side}.state') == FIXTURE_SHA256,
                    'Native acquisition/editor state differs from discovered original')
        compare_loadout(work / 'native-a-native.json', work / f'{name}-native.json', work / f'{name}-starter.json')
        for profile in ('starter', 'native'):
            record = json.loads((work / f'{name}-{profile}.json').read_text())['record']
            binary = work / f'{name}-{profile}.loadout'
            require(binary.stat().st_size == record['bytes'] and digest(binary) == record['sha256'],
                    'Local captured loadout differs from complete production-reader record')
    return work / 'native-a-0.state'


def native_battles(rom, fixture, work):
    jobs = [('observer-a', None), ('observer-b', None), ('no-seed', '--no-seed'), ('no-chip', '--no-chip')]
    def observe(job):
        name, option = job
        execute(['build/host/online-carried-observer', str(rom), str(fixture), str(work / name)]
                + ([option] if option else []), work, name)
    with ThreadPoolExecutor(max_workers=3) as pool:
        list(pool.map(observe, jobs))
    execute(['build/host/online-carried-original', '--practice', str(rom), str(fixture),
             str(work / 'original')], work, 'original')
    report = json.loads((work / 'original-report.json').read_text())
    require(report.get('passed') is True, 'Original input-only Practice did not complete')
    evidence = compare_controls(work / 'observer-a.jsonl', work / 'observer-b.jsonl', work / 'no-seed.jsonl')
    for name in ('observer-a', 'observer-b', 'no-seed'):
        require(digest(work / f'{name}.jsonl') == TRACE_SHA256 and digest(work / f'{name}-full.jsonl') == FULL_SHA256,
                'Pinned uninterrupted native trace changed')
        check_seed(json.loads((work / f'{name}-seed.json').read_text()), no_write=name == 'no-seed')
        compare_loadout(work / 'native-a-native.json', work / f'{name}-loadout.json', work / 'native-a-starter.json')
    active = compare_game(work / 'original-trace.csv', work / 'observer-a.csv')
    try:
        chip_evidence(read_trace(work / 'no-chip.jsonl'))
    except Unreached as error:
        require(str(error) == 'Cannon A slot zero was not selected', 'No-chip control failed for an unrelated reason')
        negative = str(error)
    else:
        raise Mismatch('No-chip original incorrectly passed chip-use acceptance')
    return dict(selected_chip=evidence, original_active_timers=active, no_chip_negative=negative)


def inside(rom, state, work):
    before, pins, commands = target(rom, state), sources(), recipe()
    driver = digest(__file__)
    binaries = {name: digest(ROOT / 'build/host' / name) for name in BINARIES}
    fixture = native_fixtures(rom, state, work)
    evidence = native_battles(rom, fixture, work)
    require(before == target(rom, state) and pins == sources() and driver == digest(__file__), 'Read-only target/source changed during controls')
    require(binaries == {name: digest(ROOT / 'build/host' / name) for name in BINARIES}, 'Control binary changed')
    require(digest(fixture) == FIXTURE_SHA256, 'Native map fixture changed during battle')
    result = dict(verdict='PASS', scope='original native fixture and selected-chip controls only',
                  unit3_verdict='INCONCLUSIVE', source_files_unchanged=True, production_arena_enabled=False,
                  unreached=['carried installer', 'nondefault stats/programs', 'delayed carried fights',
                             'corruption/disconnect preservation', 'production visit', 'device qualification'],
                  target=before, oracle_pins=pins, driver_sha256=driver, binary_sha256=binaries, fixture_sha256=FIXTURE_SHA256,
                  core_frame_trace_sha256=TRACE_SHA256, uninterrupted_full_trace_sha256=FULL_SHA256,
                  build_recipe_sha256=RECIPE_SHA256, build_recipe=commands,
                  core_source_sha256='91d6fbd32abcbdf030d58d3f562de25ebbc9d56040d513ff8e5c19bee9dacf14',
                  **evidence)
    (work / 'report.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({key: result[key] for key in ('verdict', 'scope', 'unit3_verdict', 'selected_chip', 'no_chip_negative')}, indent=2))
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom', type=Path, required=True)
    parser.add_argument('--state', type=Path, required=True)
    parser.add_argument('--inside', action='store_true', help=argparse.SUPPRESS)
    parser.add_argument('--work', type=Path, help=argparse.SUPPRESS)
    args = parser.parse_args()
    rom, state = args.rom.resolve(), args.state.resolve()
    try:
        sources()
        if args.inside:
            return inside(rom, state, args.work)
        require(digest(rom) == TARGET['rom_sha256'] and digest(state) == TARGET['boot_sha256'], 'Wrong original target')
        import build
        build.ensure_image()
        if build.docker('make', *['build/host/' + n for n in BINARIES], 'WERROR=1', '-j3'):
            return 1
        parent = ROOT / '.build/online-carried'
        parent.mkdir(exist_ok=True)
        work = Path(tempfile.mkdtemp(prefix='proof-', dir=parent))
        command = ['python3', 'tools/online_carried_control.py', '--inside', '--rom', '/probe/bn6.gba',
                   '--state', '/probe/boot-3.state', '--work', '/src/' + str(work.relative_to(ROOT))]
        status = build.docker(*command, mounts=[(str(rom), '/probe/bn6.gba:ro'), (str(state), '/probe/boot-3.state:ro')])
        require(digest(rom) == TARGET['rom_sha256'] and digest(state) == TARGET['boot_sha256'], 'Read-only target changed')
        print(f'Local original control evidence: {work}')
        return status
    except Unreached as error:
        print(f'INCONCLUSIVE: {error}', file=sys.stderr)
        return 2
    except (Mismatch, OSError, ValueError, subprocess.SubprocessError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
