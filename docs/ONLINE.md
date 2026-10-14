# Online development: monorepo and Phase 0

[Epic #120](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/120)
adds a social Net Square beside the run. This is its opt-in technical
spike. There is no public server, player-facing Network menu, trade, or
Arena in this phase.

## Repository boundaries

The repository builds three related components from one checkout:

| Component | Sources | Build and checks |
| --- | --- | --- |
| Game client | `src/`, including `src/online/`; existing platform directories | `python3 build.py linux`, the existing platform commands, `test`, `lint` |
| Presence server | `server/` | `python3 build.py server`, `server-test`; its own Dockerfile and CI job |
| Wire contract | `shared/online/` | Server tests check the C constants, protocol version, generator version/hash and ROM identity against the client |

The root `build.py`, `Makefile`, `tests/`, `tools/`, documentation and CI
coordinate the components. The existing client paths and output names
are stable across the supported platforms. `build.py all` builds the
server alongside the host and PortMaster clients; `build.py test` includes
the server suite. The server builds and tests without SDL, mGBA, Docker,
or a ROM. It is packaged as a Python zipapp requiring Python 3.11 or
newer; native standalone server executables are a Phase 1 packaging task.

The dependency direction is client → wire contract ← server. The server
does not import game code, run an emulator, receive a save state, or read
a ROM. Its Docker build copies only `server/`, `shared/online/` and the license.
ROMs, saves and local proof output never enter a release.

## Reproduce the two-game proof

Use the normal Linux build dependencies and your own verified BN6 Gregar
(USA) ROM. `CYBERWORLD_ROM_DIR` selects its folder, as with the other
developer tools. The proof has no dependency on a public endpoint.

```sh
python3 build.py server-test
python3 build.py online-link
python3 build.py online-proof
python3 tools/online_bench.py
```

`online-proof` builds the game and server, creates two ordinary game
checkpoints in new temporary data directories, then launches the built
server on an available loopback port and two **actual game processes**.
Each walks a different scripted route. It checks:

- Both clients accepted the server's seed and built the same map hash.
- Each client's local position and its remote peer's position changed.
- Remote positions reached the actual BN6 NPC object pool, matching the
  other game process's movement.
- A departing player disappeared from the remaining client.
- Stopping the server cleared the remaining client's connection/peers.
- Every checkpoint file retained exactly the same SHA-256 before and
  after the visits, including the forced disconnect.

The proof retains logs, numerical JSONL traces, full-frame screenshots
and `report.json` under a fresh `.build/online-proof/run-*/`. Failed runs
retain their evidence and exit unsuccessfully. Saves and emulator states
there are local only. `python3 tools/online_proof.py` repeats the test with
the existing binaries; `--seed` exercises another server world.

For two interactive windows, first build the client and server, then run
these in three terminals. Use different data folders:

```sh
python3 build.py server-run --config server/server.example.ini
```

```sh
build/linux/cyberworld --window --rom-dir "$CYBERWORLD_ROM_DIR" \
  --data-dir .build/online-alpha --online ws://127.0.0.1:8765/presence --online-name Alpha
```

```sh
build/linux/cyberworld --window --rom-dir "$CYBERWORLD_ROM_DIR" \
  --data-dir .build/online-beta --online ws://127.0.0.1:8765/presence --online-name Beta
```

The Phase 0 command requires an explicit `--data-dir`. It starts an
isolated Square without starting a run or invoking its save logic. The
ordinary direction buttons and B move MegaMan through BN6 itself. The
PET and jacking out are disabled in the technical room. Escape exits.
Headless online clients run at wall-clock speed so they can interact;
ordinary headless tests retain their existing fast execution.

For a browser/native pair, build with `python3 build.py web`, allow
`http://127.0.0.1:8088` in the server's `origins`, and serve the repository
with `python3 -m http.server 8088 --bind 127.0.0.1`. Open
`http://127.0.0.1:8088/tools/online_browser.html`, choose your local ROM,
and connect while a native client is present. This developer harness reads
the ROM into the tab's memory; it never uploads or persists it. It also
keeps numerical movement evidence at `/online-trace.jsonl` in the tab's
in-memory filesystem. It is not part of the published game site.

## Design and implementation

The dialectic is **the run's integrity against the community's
generosity**, from the epic and the repository's game-design skill.
This social loop sits beside the run and follows *variety, not power*.
The spike therefore has no healing, rewards, currency or encounters.
It tests presence and movement without choosing Phase 1's final entry
flow or Phase 2's trade policy.

The server sends only its seed and compatibility metadata. Each client
makes a small room with the existing ROM-backed map builder; a
seed-selected alcove and tile choices exercise deterministic construction.
This is a Square generator prototype, not the final collection of rooms.
It does not change `layer_generate`, `Run` or `RUN_MAGIC`.

The cross-platform proof exposed an existing tile-ordering difference:
equal-frequency candidates relied on `qsort` retaining their preceding
order, which glibc happened to do and WebAssembly's libc did not. The
comparator now resolves ties by tile pair and pad, with a ROM-free test
of all 720 permutations of six candidates. `LAYER_MAKE` is now 95 so a
checkpoint made with the earlier tile choices rebuilds safely. The
generation hash stays unchanged because `layer_generate` itself is unchanged.

Other players are noninteracting NPCs with MegaMan's sprite. Eight
reserved NPC scripts share one sprite entry, within the existing 16-NPC
and 12-sprite limits. Existing NPC bytecode sets position, facing and
standing/walking animation; no movement assistance is added to MegaMan.
The displayed peer name identifies the received avatar. Source
verification is in [SOURCES.md](SOURCES.md#net-squares-npc-movement-spike-issue-120).

The binary protocol rejects a mismatched protocol version, `LAYER_MAKE`,
generation hash, ROM code or ROM SHA-1. It limits names, positions, message
size, connected players, pending handshakes, per-client traffic and output
buffers. Phase 0 is one room of at most eight players. It relays position
snapshots at 20 Hz; smoothing, channels and moderation are Phase 1 work.

## Phase 0 evidence and gates

The numerical proof is repeatable with the commands above. The separate
[link and carried-MegaMan report](ONLINE_LINK.md) gives the serial test,
state size/timing and exact limits of that evidence.

Measured on the Linux development host, 15 October 2026:

- **Two native game processes:** seeds 120 and 121 passed. The latter's
  map hash was 1199273041; both clients' median round trip was 1 ms and
  maximum 2 ms. Actual NPC movement, peer departure, server shutdown and
  all 12 checkpoint files per client were checked.
- **Chrome/WebAssembly and native:** seed 120, map hash 4132376369 on
  both. Browser median/max round trip 3/7 ms; native 2/3 ms. Both traced
  moving NPC coordinates that matched the other process's local movement.
- **Eight ROM-free wire clients for ten seconds:** 197 complete room
  snapshots per client; median/max round trip 0/1 ms (millisecond clock).
  Aggregate traffic was 39,038 application bytes/s or 40,692 bytes/s
  including WebSocket framing, excluding TCP/IP and TLS. This measures
  the bounded presence workload, not maximum network throughput or
  eight running emulators. `tools/online_bench.py` repeats it.
- **TLS refusal:** a local server's untrusted self-signed certificate was
  rejected by the native game, with no plaintext fallback. System CA
  trust plus hostname verification is the selected model; a valid
  public certificate is needed for public hosting.

Evidence stays under `.build/online-proof/`, `.build/online-browser/`,
`.build/online-bench/` and `.build/online-tls/`. Loopback timings do not
predict internet or handheld performance.

| Spike | Implemented experiment | Gate before a player release |
| --- | --- | --- |
| Transport | Versioned WebSocket server, bounded native polling on a worker, ping/pong, TLS certificate/hostname verification | Measure every target on real hardware and finish the platform backends identified in the server README |
| Players as NPCs | Two game processes move remote MegaMan NPCs; telemetry reads actual NPC coordinates | Interpolation, identity labels over avatars, full room/channel load test |
| Link cable | Two mGBA cores exchange serial words using mGBA's lockstep API; memory state benchmark | Actual BN6 Practice NetBattle, separate-process serial scheduling and injected network delay |
| Carried MegaMan | Read-only folder/Pack/board/HP/powers record; save/load restores all EWRAM/IWRAM bytes; visit keeps checkpoint files unchanged | Apply the carried record to a visit and resume through the game's normal checkpoint flow; crash recovery across that whole flow |
| World from seed | Two game instances build identical geometry/tile hashes from server seed without sending assets | Final Square layout, generator-version migration and all platform determinism |

These gates are recorded answers from the spikes, not claims of a
completed Phase 1 or a working PvP battle. The official server address,
hosting budget and moderators remain owner decisions. No public endpoint
or new player-facing release is deployed by this work.
