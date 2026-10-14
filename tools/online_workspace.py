"""The monorepo's independent server commands, called by build.py."""
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def build_server():
    return subprocess.call([sys.executable, str(ROOT / 'server/build.py')], cwd=ROOT)


def dispatch(action, rest):
    if action == 'server-test':
        return subprocess.call([sys.executable, '-m', 'unittest', 'discover', '-s', 'server/tests', *rest], cwd=ROOT)
    code = build_server()
    if code or action == 'server':
        return code
    if action == 'server-run':
        return subprocess.call([sys.executable, str(ROOT / 'build/server/cyberworld-server.pyz'), *rest], cwd=ROOT)
    raise ValueError(action)
