"""The zipapp is reproducible and contains only the explicit server allowlist;
a native archive's notices come from the build host, never from git."""
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest
import zipfile

ROOT = Path(__file__).resolve().parents[2]


def script(name):
    spec = importlib.util.spec_from_file_location("server_" + name, ROOT / "server" / (name + ".py"))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


BUILD = script("build")
NOTICES = script("notices")


def has_licence():
    try:
        NOTICES.interpreter_licence()
        return True
    except RuntimeError:
        return False


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

    def test_no_third_party_licence_text_is_committed(self):
        self.assertFalse((ROOT / "server/licenses").exists())

    @unittest.skipUnless(has_licence(), "this Python carries no LICENSE file")
    def test_notices_cover_every_frozen_library_or_fail(self):
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            keg = work / "keg"
            (keg / "lib").mkdir(parents=True)
            (keg / "lib/libkeg.so.1").write_bytes(b"")
            (keg / "LICENSE.txt").write_text("keg licence\n")
            bare = work / "a/b/c/d"
            bare.mkdir(parents=True)
            (bare / "libbare.so.1").write_bytes(b"")
            inside = str(Path(sys.base_prefix) / "lib/libinside.so.1")
            toc = work / "work/cyberworld-server/Analysis-00.toc"
            toc.parent.mkdir(parents=True)
            toc.write_text(repr((["main.py"], [("libinside.so.1", inside, "BINARY"),
                                               ("libkeg.so.1", str(keg / "lib/libkeg.so.1"), "BINARY"),
                                               ("_ssl.so", "/x/_ssl.so", "EXTENSION")])))
            notices, covered = NOTICES.collect(work)
            names = [name for name, _, _ in notices]
            self.assertEqual(names, ["Python.txt", "keg.txt"])
            self.assertEqual(covered, {"Python.txt": ["libinside.so.1"], "keg.txt": ["libkeg.so.1"]})
            self.assertIn("`keg.txt`", NOTICES.readme(notices, covered, "linux-x86_64"))
            toc.write_text(repr([("libbare.so.1", str(bare / "libbare.so.1"), "BINARY")]))
            with self.assertRaisesRegex(RuntimeError, "libbare.so.1"):
                NOTICES.collect(work)


if __name__ == "__main__":
    unittest.main()
