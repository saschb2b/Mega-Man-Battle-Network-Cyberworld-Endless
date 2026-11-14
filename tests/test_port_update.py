#!/usr/bin/env python3
"""port/update.py, the PortMaster port's update as its launcher runs it,
on a made-up ports folder: the new version in place with the player's
files kept, a zip that holds what it may not refused with the old version
kept, and an update cut short undone (docs/engine/updates.md)."""
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
import zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = os.path.join(ROOT, 'port', 'update.py')
sys.path.insert(0, os.path.join(ROOT, 'port'))
import update  # noqa: E402

LAUNCHER = 'Cyberworld Endless.sh'


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w') as f:
        f.write(text)


def read(path):
    with open(path) as f:
        return f.read()


class PortUpdate(unittest.TestCase):
    def setUp(self):
        self.ports = tempfile.mkdtemp(prefix='cw-ports-')
        self.game = os.path.join(self.ports, 'cyberworld')
        self.launcher = os.path.join(self.ports, LAUNCHER)
        self.work = os.path.join(self.ports, '.cyberworld-update')
        write(os.path.join(self.game, 'cyberworld.aarch64'), 'old game')
        write(os.path.join(self.game, 'update.py'), 'old updater')
        write(os.path.join(self.game, 'settings.ini'), 'music = 7\n')
        write(os.path.join(self.game, 'run.sav'), 'the run')
        write(os.path.join(self.game, 'rom', 'bn6.gba'), 'the ROM')
        write(self.launcher, 'old launcher')
        os.makedirs(self.work)

    def tearDown(self):
        shutil.rmtree(self.ports, ignore_errors=True)

    def ready(self, files, version='0.22.0'):
        with zipfile.ZipFile(os.path.join(self.work, 'update.zip'), 'w') as z:
            for name, text in files.items():
                z.writestr(name, text)
        write(os.path.join(self.work, 'ready.txt'), version + '\n')

    def run_update(self, command='apply'):
        return subprocess.run([sys.executable, SCRIPT, command, '--game-dir', self.game, '--launcher', self.launcher],
                              capture_output=True, text=True).returncode

    def test_apply(self):
        self.ready({LAUNCHER: 'new launcher', 'cyberworld/cyberworld.aarch64': 'new game',
                    'cyberworld/update.py': 'new updater', 'cyberworld/licenses/LICENSE.mgba.txt': 'mGBA',
                    'cyberworld/rom/PUT_YOUR_ROM_HERE.txt': 'put it here'})
        self.assertEqual(self.run_update(), update.DONE)
        self.assertEqual(read(os.path.join(self.game, 'cyberworld.aarch64')), 'new game')
        self.assertEqual(read(os.path.join(self.game, 'licenses', 'LICENSE.mgba.txt')), 'mGBA')
        self.assertEqual(read(self.launcher), 'new launcher')
        # (the player's files as they were)
        self.assertEqual(read(os.path.join(self.game, 'settings.ini')), 'music = 7\n')
        self.assertEqual(read(os.path.join(self.game, 'run.sav')), 'the run')
        self.assertEqual(read(os.path.join(self.game, 'rom', 'bn6.gba')), 'the ROM')
        self.assertTrue(os.access(os.path.join(self.game, 'cyberworld.aarch64'), os.X_OK))
        self.assertFalse(os.path.exists(self.work))
        self.assertEqual(sorted(os.listdir(self.ports)), sorted(['cyberworld', LAUNCHER]))

    def test_nothing_ready(self):
        self.assertEqual(self.run_update(), 0)
        self.assertEqual(read(os.path.join(self.game, 'cyberworld.aarch64')), 'old game')

    def test_refused(self):
        for files in ({LAUNCHER: 'x', 'cyberworld/cyberworld.aarch64': 'x', '../escape.txt': 'x'},
                      {LAUNCHER: 'x', 'cyberworld/cyberworld.aarch64': 'x', 'elsewhere/file': 'x'},
                      {LAUNCHER: 'x'},
                      {'cyberworld/cyberworld.aarch64': 'x'}):
            with self.subTest(files=sorted(files)):
                self.ready(files)
                self.assertEqual(self.run_update(), 1)
                self.assertEqual(read(os.path.join(self.game, 'cyberworld.aarch64')), 'old game')
                self.assertEqual(read(self.launcher), 'old launcher')
                self.assertEqual(read(os.path.join(self.game, 'run.sav')), 'the run')
                self.assertFalse(os.path.exists(os.path.join(self.ports, 'escape.txt')))
                self.assertFalse(os.path.exists(os.path.join(self.work, 'update.zip')))
                self.assertFalse(os.path.exists(os.path.join(self.work, 'ready.txt')))

    def test_damaged(self):
        write(os.path.join(self.work, 'update.zip'), 'not a zip')
        write(os.path.join(self.work, 'ready.txt'), '0.22.0\n')
        self.assertEqual(self.run_update(), 1)
        self.assertEqual(read(os.path.join(self.game, 'cyberworld.aarch64')), 'old game')

    def test_recover(self):
        """Switched off between the old folder stepping aside and the new one taking its place."""
        previous = os.path.join(self.work, 'previous')
        shutil.copy2(self.launcher, os.path.join(self.work, 'previous-launcher.sh'))
        update.write_json(os.path.join(self.work, 'transaction.json'), {'game': self.game})
        os.replace(self.game, previous)
        write(os.path.join(self.game, 'cyberworld.aarch64'), 'half of the new game')
        write(self.launcher, 'new launcher')
        self.assertEqual(self.run_update('recover'), 0)
        self.assertEqual(read(os.path.join(self.game, 'cyberworld.aarch64')), 'old game')
        self.assertEqual(read(os.path.join(self.game, 'run.sav')), 'the run')
        self.assertEqual(read(self.launcher), 'old launcher')
        self.assertFalse(os.path.exists(os.path.join(self.work, 'transaction.json')))


if __name__ == '__main__':
    unittest.main()
