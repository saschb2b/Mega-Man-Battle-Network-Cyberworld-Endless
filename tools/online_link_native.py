"""Run the ROM-free serial/state probe on the current native platform."""
import argparse
import json
import math
from pathlib import Path
import platform
import subprocess


def fields(output):
    sections = {}
    for line in output.splitlines():
        words = line.split()
        if words and words[0] in ('serial', 'loadout', 'state', 'complete_state'):
            sections[words[0]] = dict(word.split('=', 1) for word in words[1:] if '=' in word)
    return sections


def verify(sections):
    serial, loadout, state, complete = (sections[key] for key in ('serial', 'loadout', 'state', 'complete_state'))
    assert serial['passed'] == 'yes' and int(serial['cores']) == 2
    assert int(serial['transfers']) == 256 and int(serial['words_checked']) == 1024
    assert loadout['synthetic_fields_and_bounds'] == 'yes'
    assert state['restored'] == 'yes' and int(state['ram_bytes']) == 0x48000
    assert int(state['bytes']) > 0 and int(state['ring_8_bytes']) == int(state['bytes']) * 8
    assert complete['savedata_restored'] == 'yes' and int(complete['bytes']) >= int(state['bytes'])
    for section, key in ((serial, 'elapsed_ms'), (state, 'save_ms'), (state, 'load_ms'), (complete, 'load_ms')):
        value = float(section[key])
        assert math.isfinite(value) and value >= 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--output', type=Path, default=Path('.build/online-link-native'))
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    report = dict(platform=platform.system(), architecture=platform.machine(),
                  synthetic=True, commercial_rom=False, real_battle_replay_measured=False,
                  benchmark_iterations=200, passed=False)
    try:
        process = subprocess.run([str(args.binary.resolve())], text=True, capture_output=True,
                                 encoding='utf-8', errors='replace', timeout=120)
        output = process.stdout + process.stderr
        (args.output / 'probe.log').write_text(output, encoding='utf-8')
        print(output, end='')
        report.update(exit_code=process.returncode, measurements=fields(process.stdout))
        if process.returncode:
            raise RuntimeError(f'Probe exited with {process.returncode}')
        verify(report['measurements'])
        report['passed'] = True
    except (OSError, subprocess.TimeoutExpired, RuntimeError, KeyError, ValueError, AssertionError) as error:
        report['error'] = str(error) or type(error).__name__
    (args.output / 'report.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(report, indent=2))
    return 0 if report['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
