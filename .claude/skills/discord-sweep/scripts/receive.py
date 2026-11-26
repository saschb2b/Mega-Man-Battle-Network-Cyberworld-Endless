#!/usr/bin/env python3
"""Takes pictures the Discord page sends and saves them, for a while.

The discord-sweep skill's way round a tab whose screenshots time out
(.claude/skills/discord-sweep/SKILL.md, Pictures): the page fetches each
attachment and POSTs it here, discord_page.js's cw.savePictures.

    python3 receive.py OUT_DIR [--port 8765] [--seconds 600]

Listens on 127.0.0.1 alone, answers discord.com's pages alone, keeps a
file's name to letters, digits, dot, dash and underscore, and ends by
itself.
"""
import argparse
import http.server
import os
import re
import threading

LIMIT = 30 * 1024 * 1024
ORIGIN = "https://discord.com"


def handler(out_dir):
    class Handler(http.server.BaseHTTPRequestHandler):
        def _cors(self):
            self.send_header("Access-Control-Allow-Origin", ORIGIN)
            self.send_header("Access-Control-Allow-Methods", "POST, OPTIONS")
            self.send_header("Access-Control-Allow-Headers", "*")
            self.send_header("Access-Control-Allow-Private-Network", "true")

        def _answer(self, code, text=b""):
            self.send_response(code)
            self._cors()
            self.end_headers()
            self.wfile.write(text)

        def do_OPTIONS(self):
            self._answer(204)

        def do_POST(self):
            name = re.sub(r"[^A-Za-z0-9._-]", "", self.path.strip("/"))[:80].lstrip(".")
            size = int(self.headers.get("Content-Length", 0))
            if not name or size <= 0 or size > LIMIT:
                self._answer(400, b"refused")
                return
            data = self.rfile.read(size)
            with open(os.path.join(out_dir, name), "wb") as f:
                f.write(data)
            print(f"{name} {len(data)}", flush=True)
            self._answer(200, b"ok")

        def log_message(self, *args):
            pass

    return Handler


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("out_dir")
    ap.add_argument("--port", type=int, default=8765)
    ap.add_argument("--seconds", type=int, default=600)
    a = ap.parse_args()
    os.makedirs(a.out_dir, exist_ok=True)
    server = http.server.HTTPServer(("127.0.0.1", a.port), handler(a.out_dir))
    threading.Timer(a.seconds, server.shutdown).start()
    print(f"saving into {a.out_dir} on 127.0.0.1:{a.port} for {a.seconds} s", flush=True)
    server.serve_forever()


if __name__ == "__main__":
    main()
