"""The zipapp is reproducible and contains only the explicit server allowlist."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
import zipfile

ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location("server_build", ROOT / "server/build.py")
BUILD = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(BUILD)


class PackagingTests(unittest.TestCase):
    def test_zipapp_reproducible_and_rom_free(self):
        with tempfile.TemporaryDirectory() as directory:
            first = BUILD.build(Path(directory) / "a/server.pyz")
            second = BUILD.build(Path(directory) / "b/server.pyz")
            self.assertEqual(first.read_bytes(), second.read_bytes())
            expected = {"__main__.py", "LICENSE", "cyberworld_server/protocol.json"}
            expected.update("cyberworld_server/" + file.name for file in (ROOT / "server/cyberworld_server").glob("*.py"))
            with zipfile.ZipFile(first) as archive:
                self.assertEqual(set(archive.namelist()), expected)
                self.assertEqual(json.loads(archive.read("cyberworld_server/protocol.json")),
                                 json.loads((ROOT / "shared/online/protocol.json").read_text()))
                self.assertTrue(all(entry.date_time == (1980, 1, 1, 0, 0, 0) for entry in archive.infolist()))


if __name__ == "__main__":
    unittest.main()
