"""The monorepo's server and online spike commands, called by build.py."""
import argparse
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def build_server():
    return subprocess.call([sys.executable, str(ROOT / 'server/build.py')], cwd=ROOT)


def link_probe(rest):
    import build
    parser = argparse.ArgumentParser(prog='build.py online-link')
    parser.add_argument('--rom', type=Path)
    parser.add_argument('--state', type=Path)
    args = parser.parse_args(rest)
    if bool(args.rom) != bool(args.state):
        parser.error('--rom and --state are supplied together')
    build.ensure_image()
    code = build.docker('make', 'online-link', 'WERROR=1')
    if code:
        return code
    extra, mounts = [], []
    if args.rom:
        for path, dest in ((args.rom, '/probe/rom.gba'), (args.state, '/probe/checkpoint.state')):
            if not path.is_file():
                parser.error(f'not a file: {path}')
            mounts.append((str(path.resolve()), dest + ':ro'))
            extra.append(dest)
    return build.docker('build/host/online-link-probe', *extra, mounts=mounts)


def dispatch(action, rest):
    if action == 'server-test':
        return subprocess.call([sys.executable, '-m', 'unittest', 'discover', '-s', 'server/tests', *rest], cwd=ROOT)
    if action == 'online-link':
        return link_probe(rest)
    code = build_server()
    if code or action == 'server':
        return code
    if action == 'server-run':
        return subprocess.call([sys.executable, str(ROOT / 'build/server/cyberworld-server.pyz'), *rest], cwd=ROOT)
    if action == 'online-proof':
        import build
        build.build('linux')
        return subprocess.call([sys.executable, str(ROOT / 'tools/online_proof.py'), *rest], cwd=ROOT)
    raise ValueError(action)
