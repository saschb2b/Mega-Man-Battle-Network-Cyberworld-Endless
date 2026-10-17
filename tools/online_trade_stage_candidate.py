#!/usr/bin/env python3
"""Prove isolated pristine-state staging against the fixed original observer."""
import argparse
import json
from pathlib import Path
import shutil
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import online_trade_stage_compare as compare
from tools import online_trade_stage_proof as original

ROOT = original.ROOT
STAGE = 'src/online/online_trade_stage.c'
PROBE = 'tools/online_trade_stage_probe.c'


def build(binary, stage=STAGE, probe=PROBE):
    relative = str(binary.relative_to(ROOT))
    original.docker(['make', 'WERROR=1', f'TRADE_STAGE_PROBE={relative}',
                     f'TRADE_STAGE_SOURCE={stage}', f'TRADE_STAGE_PROBE_SOURCE={probe}', relative])
    return relative


def run(relative, rom, directory, name):
    case = compare.CASES[name]
    source = directory / 'original-a/before.state'
    before = original.sha(source)
    give = case['give'] | case['codes'][0] << 9
    receive = case['receive'] | case['codes'][1] << 9
    original.docker([relative, '/input/game.gba', '/input/boot.state', str(directory.relative_to(ROOT)),
                     str(give), str(receive)], rom, source)
    shutil.move(directory / 'editor.counts', directory / 'candidate-editor.counts')
    compare.require(original.sha(source) == before, 'Source fixture file changed')


def replace_once(source, before, after):
    compare.require(source.count(before) == 1, f'Mutation anchor changed: {before}')
    return source.replace(before, after)


def mutations(output, rom, manifest):
    stage, probe = (ROOT / STAGE).read_text(), (ROOT / PROBE).read_text()
    outgoing = '--out[location(&state, state.pack + give_id * BN6_PACK_ENTRY + give_slot)];'
    incoming = '++out[location(&state, state.pack + receive_id * BN6_PACK_ENTRY + receive_slot)];'
    mark = 'out[location(&state, state.marks + receive_id)] = *ram(&state, BN6_CHIP_KEYS + receive_id, 1) ^ BN6_CHIP_KEY_XOR;'
    library = 'out[location(&state, BN6_EVENT_FLAGS + flag / 8)] |= (uint8_t)(0x80u >> (flag & 7));'
    order = 'put16(out + location(&state, state.pack + receive_id * BN6_PACK_ENTRY + BN6_PACK_ORDER + 2 * receive_slot), order);'
    counter = 'put16(out + location(&state, BN6_CHIP_ORDER_COUNTER), (uint16_t)(order - 1u));'
    faults = {
        'wrong-outgoing-code': (outgoing, outgoing.replace('+ give_slot', '+ (give_slot + 1) % ONLINE_TRADE_CODES')),
        'wrong-incoming-code': (incoming, incoming.replace('+ receive_slot', '+ (receive_slot + 1) % ONLINE_TRADE_CODES')),
        'remove-two': (outgoing, outgoing + '\n\t' + outgoing),
        'create-without-give': (outgoing, '/* deliberate removal omission */'),
        'omit-ownership': (mark, '/* deliberate ownership omission */'),
        'omit-library': (library, '(void)flag; /* deliberate Library omission */'),
        'omit-code-order': (order, '/* deliberate code-order omission */'),
        'omit-order-counter': (counter, '/* deliberate shared-counter omission */'),
        'equipped-folder': ('*written = size;', 'out[location(&state, state.folders)] ^= 1;\n\t*written = size;'),
        'protected-hp': ('*written = size;', 'out[location(&state, BN6_NAVI_HP)] ^= 1;\n\t*written = size;'),
        'stale-buffer': ('*written = size;', 'memcpy(out, source, size);\n\t*written = size;'),
        'live-core': None,
    }
    results = []
    for name, change in faults.items():
        directory = output / 'mutations' / name
        directory.mkdir(parents=True)
        for item in ('original-a', 'original-b'):
            shutil.copytree(output / 'first-code' / item, directory / item)
        for item in ('reload-a.state', 'reload-b.state'):
            shutil.copyfile(output / 'first-code' / item, directory / item)
        mutated_stage = stage if change is None else replace_once(stage, *change)
        mutated_probe = probe
        if change is None:
            mutated_probe = replace_once(probe, '\tbool ok = staged &&',
                '\ttrade_probe_core->rawWrite8(trade_probe_core, BN6_NAVI_HP, -1, 77);\n\tbool ok = staged &&')
        source, harness = directory / 'stage.c', directory / 'probe.c'
        source.write_text(mutated_stage); harness.write_text(mutated_probe)
        binary = directory / 'candidate-probe'
        relative = build(binary, str(source.relative_to(ROOT)), str(harness.relative_to(ROOT)))
        run(relative, rom, directory, 'first-code')
        changed_manifest = dict(manifest, binaries=dict(manifest['binaries'], candidate=original.sha(binary)))
        try:
            compare.accept_case(directory, 'first-code', changed_manifest, original.REVIEWED)
        except compare.Unreached:
            raise  # Missing observations never count as detecting a reached fault.
        except compare.Mismatch as error:
            results.append(dict(name=name, verdict='FAIL', reason=str(error), binary_sha256=original.sha(binary),
                                stage_sha256=original.sha(source), probe_sha256=original.sha(harness)))
            print(f'{name}: deliberate fault detected', flush=True)
        else:
            raise compare.Mismatch(f'Deliberate fault accepted: {name}')
    return results


def candidate(rom, state, output):
    controls = original.controls(rom, state, output)
    binary = output / 'candidate-probe'
    relative = build(binary)
    reload_binary = output / 'reload-probe'
    reload_relative = str(reload_binary.relative_to(ROOT))
    original.docker(['make', 'WERROR=1', f'TRADE_RELOAD_PROBE={reload_relative}', reload_relative])
    compare.require(original.sha(reload_binary) == compare.RELOAD_BINARY_SHA256, 'Reload observer binary changed')
    manifest = dict(before=controls['target_before'], after=controls['target_after'],
                    layout=compare.LAYOUT, sources=original.REVIEWED,
                    binaries=dict(oracle=controls['binary_sha256'], candidate=original.sha(binary), reload=original.sha(reload_binary)),
                    fixtures={name: case['source_sha256'] for name, case in controls['cases'].items()})
    results = []
    for name in compare.CASES:
        directory = output / name
        source = directory / 'original-a/before.state'
        for repeat in ('a', 'b'):
            lane = directory / f'reload-{repeat}'
            lane.mkdir()
            original.docker([reload_relative, '/input/game.gba', '/input/boot.state', str(lane.relative_to(ROOT))], rom, source)
            shutil.move(lane / 'reload-control.state', directory / f'reload-{repeat}.state')
        run(relative, rom, directory, name)
        result = compare.accept_case(directory, name, manifest, original.REVIEWED)
        result['candidate_sha256'] = original.sha(directory / 'candidate.state')
        results.append(result)
        print(f'{name}: routine-level candidate PASS', flush=True)
    faults = mutations(output, rom, manifest)
    original.verify_sources()
    manifest['after'] = dict(rom_sha256=original.sha(rom), state_sha256=original.sha(state),
                            library_sha256=original.docker(['sha256sum', original.LIBRARY]).stdout.split()[0])
    compare.verify_manifest(manifest, original.REVIEWED)
    (output / 'candidate.json').write_text(json.dumps(dict(manifest=manifest, cases=results, mutations=faults,
        candidate_sources={name: original.sha(ROOT / name) for name in (STAGE, PROBE)},
        native_peer_exchange='INCONCLUSIVE'), indent=2) + '\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom', type=Path, default=Path.home() / '.cache/mmbn-ref/roms/bn6-gregar-us.gba')
    parser.add_argument('--state', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    if not output.is_relative_to(ROOT / '.build'):
        parser.error('all evidence must remain under .build')
    try:
        candidate(args.rom.resolve(), args.state.resolve(), output)
    except (OSError, subprocess.CalledProcessError, compare.Mismatch) as error:
        print(f'FAIL / INCONCLUSIVE: {error}', file=sys.stderr)
        if isinstance(error, subprocess.CalledProcessError):
            print(error.stdout, error.stderr, file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
