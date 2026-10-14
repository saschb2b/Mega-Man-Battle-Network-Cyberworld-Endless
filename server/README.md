# Cyberworld Net Square server v1

A separate, ROM-free component of the Cyberworld Endless monorepo.
Python 3.11 or newer is its only runtime dependency. It relays presence,
quick phrases and emotes, and public Inspect cards. Eight players fit in
each channel; the default total cap of 64 creates eight channels. There
are no accounts, e-mail addresses, trades, arbitrary chat or battle relay.

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
`127.0.0.1:8765`, seed 120, room capacity 8, total cap 64, snapshots at 20 Hz. Ctrl+C stops it
and closes its clients. `--host`, `--port` and `--seed` override the INI.
Port 0 chooses a free port and reports it in the `listening` log event.

Copy `server.example.ini` to `server.ini` for local settings (the latter
is ignored by Git). The client URL is `ws://HOST:PORT/presence`.
The subprotocol is `cyberworld.presence.v2`; incompatible clients receive
an explicit refusal rather than being admitted to a different world.

## Container

```sh
docker build -f server/Dockerfile -t cyberworld-server .
docker run --rm -p 127.0.0.1:8765:8765 -v cyberworld-state:/data cyberworld-server
```

The Dockerfile-specific ignore file restricts the build context to server
code and the shared protocol. The final image runs as a non-root user,
contains the zipapp and Python, and has no game or ROM dependency.
Mount a config read-only and pass `--config /path/server.ini` to customize
it; container traffic needs `host = 0.0.0.0` inside the container.

`GET /health` returns JSON with the configured name, message of the day,
protocol, occupancy and limits. It contains no player identities. The
`free_text`, `trading` and `arena` flags must remain false in server v1;
attempting to enable an unimplemented feature refuses startup.

## Coolify deployment

The development server is deployed in Coolify at
`wss://cyberworld.saschb2b.com/presence`, with
[HTTPS health status](https://cyberworld.saschb2b.com/health). Its certificate,
browser WSS connection from `https://saschb2b.github.io`, and moderator
mute/unmute have been verified. Two actual native game instances also passed
the public WSS proof: 211 matching NPC observations, shared seed 120 and
map hash 4132376369, median RTTs of 33/39 ms and a maximum of 49 ms. All 24
save files stayed unchanged, and leaving restored both depth-3 loadouts.
The proof used a process-local DNS mapping because the test machine retained
a negative DNS cache entry; TLS still verified the server hostname through
the normal trust store. Network remains a development-branch feature and
has not been included in a published game release.

Use the Docker Compose build strategy in a Coolify Git resource with
**Base Directory** `/` and **Docker Compose Location**
`/server/compose.coolify.yaml`. Enable **Preserve repository during
deployment** so the read-only configuration file remains available on the
deployment host. Leave raw deployment and custom build/start commands off.
Coolify passes the Base Directory as Compose's project directory; the
build context `.` and bind source `./server/server.coolify.ini` are therefore
relative to the repository root, even though the Compose file is in `server/`.
Give
the `cyberworld` service the domain `https://cyberworld.saschb2b.com` and
container port **8765**. Coolify's proxy terminates TLS and forwards
WebSocket upgrades; no certificate is stored in the app container. Point
the domain's DNS record to the Coolify host before enabling its certificate.
Do not publish container port 8765 directly to the internet.

The compose file includes a health check, restart policy, read-only root
filesystem, a private persistent `cyberworld-state` volume at `/data`, and
bounded Docker logs. It runs as UID 10001. The read-only
`server.coolify.ini` allows the published game's browser origin
`https://saschb2b.github.io`; add another exact origin only when serving
the game there. The process binds `0.0.0.0` inside the container.

Local validation uses:

```sh
docker compose --project-directory . -f server/compose.coolify.yaml config --quiet
docker compose --project-directory . -f server/compose.coolify.yaml build
```

Run these commands from the repository root. Omitting `--project-directory`
changes relative-path resolution to the Compose file's `server/` directory.
Coolify's [Compose guide](https://coolify.io/docs/applications/builds/docker-compose)
documents the Base Directory settings and repository preservation needed
for file bind mounts. After loading the Compose definition, check that the
configuration mount resolves to a file ending in `/server/server.coolify.ini`,
the only writable persistent mount is `/data`, and the domain forwards to
the service's internal port 8765. Do not add a host port mapping.

After deployment, verify HTTPS `/health`, a native client and a browser
client through `/presence`, then test a moderator action and a restart
with the same identity. The local and device checks in the platform table
remain necessary; a healthy HTTP endpoint alone proves no gameplay.

## Identity, moderation and backups

Clients keep a random private guest key on the device and derive a separate
token for each server address; the original key is never sent. The server
derives a public identity from that token, so the identity does not become
a cross-server tracking identifier. Its `identity.key` salt and moderation
lists live in the private state directory (`server-data/` by default;
`--state-dir` overrides it). The salt is generated once. Keep that
directory across upgrades: losing the salt changes public identities and
invalidates bans and players' local block lists. Do not publish it.

There is no remote administration endpoint. Use the CLI with filesystem
access to the state directory. The local roster supplies public IDs;
private player keys are never retained. Commands take effect within a
second. `ban`, `unban`, `mute`, `unmute` and `kick` are supported:

```sh
python3 build/server/cyberworld-server.pyz --state-dir server-data --admin list
python3 build/server/cyberworld-server.pyz --state-dir server-data --admin mute --identity PUBLIC_ID --reason "Repeated disruption"
python3 server/admin.py unmute PUBLIC_ID --state-dir server-data
```

In Coolify's container terminal, substitute `/data` and
`/app/cyberworld-server.pyz`. A kick disconnects current sessions; a ban
also refuses subsequent visits. Mutes persist and suppress phrases and
emotes, while movement and Inspect remain available. An optional
`whitelist` is a space-separated list of public IDs allowed to connect.
Profiles remain anonymous: resetting a private key can evade a ban.
An allowlist is available where a host needs stricter admission.

Back up the config and private state volume together, with access limited
to moderators. Moderation updates use an atomic replace; concurrent local
CLI edits are locked. There is no game save or ROM to back up on the
server. Restore the same `identity.key` when restoring moderation lists.

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
positions and the numeric public card (HP, buster stats, Souls, Crosses,
depth, best depth, title marks and colored NaviCust board). Default logs
contain joins, public IDs, names, channels, leaves and refusals, not IPs
or private player files. `--event-log FILE`
optionally writes JSONL evidence; choose your own rotation/retention.
Sampled positions are logged only with that explicit evidence option.
The world is reconstructed from its seed.

The official hosting policy and named moderators must be published before
a general-availability release. The server can validate ranges and compatibility, but a
modified client can claim another HP or board; Inspect is informational.

## Client platform status

| Target | Backend | Verification |
| --- | --- | --- |
| Linux desktop | SDL worker, runtime libcurl byte stream, bounded WebSocket framing with HTTP/1.1 upgrade | Two actual games over public WSS: 211 matching NPC observations, median RTTs 33/39 ms, saves unchanged and both loadouts restored on leaving; local peer departure/server-stop checks also pass |
| PortMaster/Linux ARM64 | Same libcurl backend | Cross-build and lint; device network test still required |
| macOS | Same backend with system `libcurl.dylib` | Full app build passes CI; live game connection and TLS refusal tests remain |
| Browser | Emscripten WebSocket callbacks on the main thread | Chrome/WebAssembly and native: same seeded map, reciprocal NPC movement, median 4 ms loopback RTT; public WSS connection from the allowed site origin verified |
| Windows | Asynchronous WinHTTP WebSocket callbacks, decoded on the SDL worker; Windows certificate and hostname verification | Full build passes with warnings as errors and in CI; live Windows connection and TLS refusal test required |
| New 3DS | Linked libcurl/mbedTLS, shared SOC/SSL services, packaged Let's Encrypt roots, PS random bytes | Full devkitARM build and CIA/3DSX link with warnings as errors; hardware Wi-Fi, trust and memory-budget tests required |
| Android | Java Socket/SSLSocket through JNI, shared bounded WebSocket framing, system certificate and hostname verification | Three-ABI APK build passes; socket/TLS boundary tests pass locally after correcting the test container's runner UID; live device connection remains |
| iOS | NSURLSessionWebSocketTask, bounded send/open waits and queued receive, system trust | Device and Simulator builds plus an actual Simulator launch pass CI; live device connection and TLS refusal tests remain |

The current build/package evidence is
[CI run 38650948269](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/actions/runs/38650948269).
Server executable smoke tests below are separate from game-client connection tests.

All existing offline builds remain supported. This is not a claim that
the epic's full cross-platform transport spike is complete. Production
platform tests must cover the remaining targets. The 3DS's bundled roots
support the documented Let's Encrypt deployment; other certificate
authorities require an explicit root-bundle update, never a validation bypass.

See [the Phase 0 proof](../docs/ONLINE.md) and
[the wire format](../shared/online/README.md).

## Release packaging

The release workflow builds six ROM-free server archives beside the game:

| Archive | Runs with |
| --- | --- |
| `cyberworld-server-python.zip` | Python 3.11+ on any supported server OS |
| `cyberworld-server-linux-x86_64.zip` | x86-64 Linux, Ubuntu 22.04/glibc 2.35 or newer |
| `cyberworld-server-linux-arm64.zip` | ARM64 Linux, Ubuntu 22.04/glibc 2.35 or newer |
| `cyberworld-server-windows-x64.zip` | 64-bit Windows 10/11 or Server 2022 |
| `cyberworld-server-macos-x86_64.zip` | Intel Mac, macOS 15 or newer |
| `cyberworld-server-macos-arm64.zip` | Apple Silicon Mac, macOS 15 or newer |

Each archive includes the executable or zipapp, example INI, this guide,
MIT license and `BUILD.json`. Native archives include their runtime notices
and need no installed Python. The Docker recipe remains available for
containers. No new release has been published by this change. All five
native server targets passed their packaging jobs in
[CI run 38650948269](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/actions/runs/38650948269):
Linux x86-64/ARM64, Windows x64 and macOS Intel/ARM64. Each passed all 21
server tests, freezing, isolated help/health/admin smoke checks and artifact
upload. The Python zipapp bundle also passed its build and process smoke.

Build the zipapp archive with `python3 server/release.py`. For a native
archive, create a Python virtual environment, install the build tools with
`python -m pip install --require-hashes --only-binary=:all: -r server/requirements-build.txt`,
then run `python server/release.py --native` on the target OS and architecture.
The workflow pins Python 3.13.16 and PyInstaller 6.16.0, with all packaging
dependencies and wheel hashes fixed. It uses the documented
[GitHub runner labels](https://docs.github.com/en/actions/reference/runners/github-hosted-runners)
and [PyInstaller's native build and data-file options](https://pyinstaller.org/en/v6.16.0/usage.html).

Every package must pass `--help`, an isolated server process serving
`/health` with the current protocol, and a local administration command.
The smoke runs outside the checkout, so a missing bundled `protocol.json`
fails before publication. Protocol/moderation tests run on every matrix OS.
Source zipapp entries have fixed ordering, times and permissions; native
builds also fix the hash seed and build timestamp. Hosted runner images
and system libraries can change, so native outputs are not promised to be
byte-identical across runner updates. Release checksums cover every archive.

Keep the shared protocol in the same release as the game; v1 wire clients
cannot enter a v2 server. Preserve the private state directory when upgrading.
