# Cyberworld presence server: Phase 0

A separate, ROM-free component of the Cyberworld Endless monorepo.
Python 3.11 or newer is its only runtime dependency. It holds one temporary
Net Square, eight players at most, and relays their positions. It has no
account database, trades, chat, battle relay or persistent player state.

## Build and host locally

From the repository root:

```sh
python3 build.py server-test
python3 build.py server
python3 build.py server-run --config server/server.example.ini
```

Or independently of the game's build tools:

```sh
python3 server/build.py
python3 build/server/cyberworld-server.pyz --config server/server.example.ini
```

The zipapp can be copied to another computer with Python installed. The
configuration is supplied explicitly; without one the server binds
`127.0.0.1:8765`, seed 120, capacity 8, snapshots at 20 Hz. Ctrl+C stops it
and closes its clients. `--host`, `--port` and `--seed` override the INI.
Port 0 chooses a free port and reports it in the `listening` log event.

Copy `server.example.ini` to `server.ini` for local settings (the latter
is ignored by Git). The client URL is `ws://HOST:PORT/presence`.
The subprotocol is `cyberworld.presence.v1`; incompatible clients receive
an explicit refusal rather than being admitted to a different world.

## Container

```sh
docker build -f server/Dockerfile -t cyberworld-server .
docker run --rm -p 127.0.0.1:8765:8765 cyberworld-server
```

The Dockerfile-specific ignore file restricts the build context to server
code and the shared protocol. The final image runs as a non-root user,
contains the zipapp and Python, and has no game or ROM dependency.
Mount a config read-only and pass `--config /path/server.ini` to customize
it; container traffic needs `host = 0.0.0.0` inside the container.

## TLS and privacy

Public hosting needs `wss://`. Supply `tls_certificate` and `tls_key` in
the INI, or terminate TLS at a reverse proxy that forwards WebSocket
Upgrade requests to `/presence`. The native Linux/macOS transport uses
libcurl's system CA trust and verifies both certificate and hostname.
The browser uses its own trust store. There is no trust-on-first-use,
certificate-warning bypass, or automatic fallback to plaintext.

Browser clients must have their exact origin listed, separated by spaces,
in `origins`. For example, `origins = http://localhost:8080`. Empty means
native clients only. An HTTPS page requires `wss://`, as enforced by the
browser. The origin check is not player authentication.

The host sees connecting IPs at the network layer, display names and
positions. Default logs contain joins, leaves, refusals and sampled
positions, not IP addresses or private player files. `--event-log FILE`
optionally writes JSONL evidence; choose your own rotation/retention.
There is no database to back up in Phase 0: retain only the config and
TLS material if needed. The world is reconstructed from its seed.

No official public server is launched by this spike. Accounts, moderation,
channels and the official hosting policy remain later work.

## Client platform status

| Target | Phase 0 backend | Verification |
| --- | --- | --- |
| Linux desktop | SDL worker, runtime libcurl byte stream, bounded WebSocket framing | Two real game processes, movement, peer departure and server shutdown |
| PortMaster/Linux ARM64 | Same libcurl backend | Cross-build and lint; device network test still required |
| macOS | Same backend with system `libcurl.dylib` | Code path supplied; needs a Mac build and live test |
| Browser | Emscripten WebSocket callbacks on the main thread | Chrome/WebAssembly and native: same seeded map, reciprocal NPC movement, median 3 ms loopback RTT |
| Windows, Android, iOS, New 3DS | Explicit unavailable result | Backend implementation and device measurements remain a gate |

All existing offline builds remain supported. This is not a claim that
the epic's full cross-platform transport spike is complete. A production
portable TLS backend must be selected and packaged for the remaining
targets, including the 3DS's trust roots and socket initialization.

See [the Phase 0 proof](../docs/ONLINE.md) and
[the wire format](../shared/online/README.md).
