"""A Square hosted with nothing set (hosting.py, cli.py): the INI found
beside the server or in its folder, the environment's settings over it,
every interface listened on, and the console's word to the host naming the
addresses players type at the gate; no ROM needed."""

import argparse
import io
import json
import os
from pathlib import Path
import socket
import subprocess
import sys
import tempfile
import threading
import time
import unittest
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "server"))

from cyberworld_server import cli, hosting


def arguments(**given):
    return argparse.Namespace(**dict(dict(config=None, state_dir=None), **given))


def free_port():
    with socket.socket() as probe:
        probe.bind(("127.0.0.1", 0))
        return probe.getsockname()[1]


class ConfigTests(unittest.TestCase):
    def test_config_found_in_the_folder_first_then_beside_the_program(self):
        with tempfile.TemporaryDirectory() as directory:
            here, beside = Path(directory, "here"), Path(directory, "beside")
            here.mkdir()
            beside.mkdir()
            self.assertIsNone(hosting.find_config((here, beside)))
            (beside / "server.ini").write_text("[server]\n")
            self.assertEqual(hosting.find_config((here, beside)), beside / "server.ini")
            (here / "server.ini").write_text("[server]\n")
            self.assertEqual(hosting.find_config((here, beside)), here / "server.ini")

    def test_environment_over_the_ini_and_arguments_over_both(self):
        with tempfile.TemporaryDirectory() as directory:
            ini = Path(directory, "server.ini")
            ini.write_text("[server]\nname = From the INI\nseed = 5\nstate_dir = ini-state\n")
            environ = {"CYBERWORLD_SERVER_NAME": "100% Square", "CYBERWORLD_SERVER_STATE_DIR": "env-state",
                       "CYBERWORLD_SERVER_": "nothing", "CYBERWORLD_ROM_DIR": "not the server's"}
            settings, state = cli.configured(arguments(config=str(ini)), environ)
            self.assertEqual(settings.get("name"), "100% Square")
            self.assertEqual(settings.getint("seed"), 5)
            self.assertEqual(state, "env-state")
            self.assertNotIn("rom_dir", settings)
            _, state = cli.configured(arguments(config=str(ini), state_dir="argument-state"), environ)
            self.assertEqual(state, "argument-state")

    def test_a_named_config_that_is_missing_is_refused(self):
        with self.assertRaisesRegex(ValueError, "Could not read"):
            cli.configured(arguments(config="/nonexistent/server.ini"), {})


class BannerTests(unittest.TestCase):
    def test_every_address_players_type(self):
        lines = hosting.banner("My Square", None, 8765, False, ["192.168.1.20", "10.0.0.5"], "server-data")
        text = "\n".join(lines)
        self.assertIn('"My Square" is open on port 8765', text)
        self.assertIn("  ws://192.168.1.20:8765", lines)
        self.assertIn("  ws://10.0.0.5:8765", lines)
        self.assertIn("ws://localhost:8765", text)
        self.assertIn("forward TCP port 8765", text)
        self.assertIn(str(Path("server-data").resolve()), text)
        self.assertNotIn("firewall", text)
        self.assertIn("firewall", "\n".join(hosting.banner("S", None, 8765, False, [], "d", windows=True)))

    def test_one_address_listened_on_is_the_one_named(self):
        lines = hosting.banner("S", "192.168.1.20", 9000, False, ["192.168.1.20", "10.0.0.5"], "d")
        self.assertIn("  ws://192.168.1.20:9000", lines)
        self.assertNotIn("  ws://10.0.0.5:9000", lines)

    def test_this_computer_alone_says_so(self):
        text = "\n".join(hosting.banner("S", "127.0.0.1", 8765, False, ["192.168.1.20"], "d"))
        self.assertIn("Only games on this computer", text)
        self.assertNotIn("192.168.1.20", text)

    def test_a_container_and_a_certificate_name_no_address_of_their_own(self):
        text = "\n".join(hosting.banner("S", "0.0.0.0", 8765, False, ["172.17.0.2"], "/data", container=True))
        self.assertNotIn("172.17.0.2", text)
        self.assertIn("port published", text)
        self.assertIn("Stopping the container", text)
        text = "\n".join(hosting.banner("S", None, 443, True, ["192.168.1.20"], "d", config=Path("server.ini")))
        self.assertIn("wss://YOUR-DOMAIN:443", text)
        self.assertNotIn("192.168.1.20", text)
        self.assertIn("Settings: server.ini", text)

    def test_only_reachable_addresses(self):
        self.assertTrue(hosting.usable("192.168.1.20"))
        self.assertTrue(hosting.usable("100.101.102.103"))
        for address in ("127.0.0.1", "169.254.1.1", "0.0.0.0", "224.0.0.1", "not an address"):
            self.assertFalse(hosting.usable(address), address)
        self.assertTrue(all(hosting.usable(address) for address in hosting.local_addresses()))


class ProcessTests(unittest.TestCase):
    def test_started_with_nothing_but_a_server_ini_in_its_folder(self):
        """The server started bare in a folder holding a server.ini: it reads
        it, takes the environment's name over it, listens on every
        interface and tells the host where players come in."""
        port = free_port()
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            (work / "server.ini").write_text(f"[server]\nname = From the INI\nport = {port}\nstate_dir = state\n")
            environ = dict(os.environ, CYBERWORLD_SERVER_NAME="Lan's Square", PYTHONPATH=str(ROOT / "server"))
            process = subprocess.Popen([sys.executable, str(ROOT / "server/main.py")], cwd=work, env=environ,
                                       stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
            lines, done = [], threading.Event()

            def read():
                for line in process.stdout:
                    lines.append(line.rstrip("\n"))
                    if line.startswith("Ctrl+C"):
                        done.set()
            threading.Thread(target=read, daemon=True).start()
            try:
                opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))
                deadline, health = time.monotonic() + 15, None
                while health is None and time.monotonic() < deadline:
                    try:
                        with opener.open(f"http://127.0.0.1:{port}/health", timeout=2) as response:
                            health = json.load(response)
                    except OSError:
                        time.sleep(0.1)
                self.assertIsNotNone(health, "the server never answered")
                self.assertEqual(health["name"], "Lan's Square")
                self.assertTrue(done.wait(15), lines)
            finally:
                process.terminate()
                process.wait(timeout=10)
                process.stdout.close()
            text = "\n".join(lines)
            self.assertIn(f"\"Lan's Square\" is open on port {port}", text)
            if not hosting.contained():
                self.assertIn(f"ws://localhost:{port}", text)
            self.assertIn("Settings: " + str(work / "server.ini"), text)
            self.assertTrue((work / "state" / "identity.key").is_file())


if __name__ == "__main__":
    unittest.main()
