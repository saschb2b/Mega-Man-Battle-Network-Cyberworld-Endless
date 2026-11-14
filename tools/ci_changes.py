#!/usr/bin/env python3
"""ci_changes.py [FILE ...]: which of CI's platform jobs a pull request
needs (.github/workflows/ci.yml's first job; docs/code/platforms.md, CI on
a pull request).

Every pull request runs the checks that find most of what CI finds: the
scripts' lint, the code checks, the unit tests, the Linux desktop,
PortMaster and Windows builds, and the 3DS's as the 32-bit console. The
other nine (Switch, Vita, PSP, PS4, PS5, Android, macOS, iOS, the browser)
build the same C with another toolchain and their own packaging: in 400
pull request runs one of them failed alone in 8, while iOS's alone is the
slowest job there is. So on a pull request each of those runs only where
the change could reach it:

  its own folder (ios/, vita/, web/ ...)             that job
  what every platform builds with (COMMON: the       all nine
    Makefile, build.py, the build images, the
    workflows, the wire)
  a source file that holds a platform's own code     all nine
    (PLATFORM: an #if on a system's macro in it,
    the PSP's core's CW_GPSP among them, or
    Objective-C), or one that is gone

A push (dev, main) and a run started by hand run all of them, so dev's
own CI on each merge is still the check of every target. To run them all
on a branch: gh workflow run CI --ref BRANCH.

Prints NAME=true|false a line for $GITHUB_OUTPUT. With files named, as if
a pull request changed those (to try a rule).
Standard library only.
"""
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# A job's own files: a change there runs that job.
OWN = {
    'switch': ('switch/',),
    'vita': ('vita/',),
    'psp': ('psp/',),
    'ps4': ('ps4/',),
    'ps5': ('ps5/', 'ps4/'),          # (the PS5's app is built with the PS4's pieces beside its own)
    'android': ('android/',),
    'macos': ('macos/',),
    'ios': ('ios/', 'macos/'),        # (iOS shares the Mac's Objective-C)
    'web': ('web/',),
}
# What every platform's build reads beyond the game's C: a change there runs them all.
COMMON = ('Makefile', 'build.py', 'docker/', '.github/', 'tools/ci_changes.py', 'shared/')
# A system's own macro: a source file that tests one holds code only that system's toolchain compiles.
PLATFORM = re.compile(r'__3DS__|__SWITCH__|__vita__|__PSP__|CW_GPSP|__PS4__|__ORBIS__|__PROSPERO__|__APPLE__|'
                      r'__ANDROID__|__EMSCRIPTEN__|_WIN32|TARGET_OS_')


def shared(path):
    """Whether a change to this file can reach every platform's build."""
    if path.startswith(COMMON):
        return True
    if not path.startswith('src/') or not path.endswith(('.c', '.h', '.m')):
        return False
    if path.endswith('.m'):
        return True
    try:
        with open(os.path.join(ROOT, path), encoding='utf-8', errors='replace') as f:
            return bool(PLATFORM.search(f.read()))
    except OSError:
        return True   # (removed or moved: what it held is not here to read)


def changed():
    """A pull request's files (its merge against dev's tip under it); None where everything runs."""
    if len(sys.argv) > 1:
        return sys.argv[1:]
    if os.environ.get('GITHUB_EVENT_NAME') != 'pull_request':
        return None
    r = subprocess.run(['git', 'diff', '--name-only', 'HEAD^1', 'HEAD'], capture_output=True, text=True)
    return r.stdout.split('\n') if r.returncode == 0 else None   # (no parent to compare with: run everything)


def main():
    files = changed()
    files = [f for f in files if f] if files is not None else None
    every = files is None or any(shared(f) for f in files)
    picked = {job: every or any(f.startswith(own) for f in files) for job, own in OWN.items()}
    for job, run in picked.items():
        print(f'{job}={"true" if run else "false"}')
    why = ('not a pull request' if files is None else
           'a file every platform builds from changed: ' + next(f for f in files if shared(f)) if every else
           'by their own folders')
    ran = [job for job, run in picked.items() if run]
    print(f'platform jobs: {", ".join(ran) or "none"} ({why})', file=sys.stderr)
    summary = os.environ.get('GITHUB_STEP_SUMMARY')
    if summary:
        with open(summary, 'a', encoding='utf-8') as f:
            f.write(f'Platform jobs beyond the always-run set: **{", ".join(ran) or "none"}** ({why}). '
                    'All of them: `gh workflow run CI --ref BRANCH`.\n')
    return 0


if __name__ == '__main__':
    sys.exit(main())
