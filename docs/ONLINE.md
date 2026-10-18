# Online development: monorepo and Net Square

[Epic #120](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/120)
adds a social Net Square beside the run. Phase 1 provides the opt-in
Network screen, checkpoint-safe visits, channels, public Navi cards,
fixed quick phrases and emotes, and moderation. Trading and the Arena
remain disabled. The selected official endpoint is
`wss://cyberworld.saschb2b.com/presence`; hosting and operational checks
are described in [the server README](../server/README.md).
It is deployed in Coolify as a development service, from the Compose file
and configuration in `deploy/coolify/`; this branch has not been released
to players.

## Repository boundaries

The repository builds three related components from one checkout:

| Component | Sources | Build and checks |
| --- | --- | --- |
| Game client | `src/`, including `src/online/`; existing platform directories | `python3 build.py linux`, the existing platform commands, `test`, `lint` |
| Presence server | `server/`; its Coolify deployment in `deploy/coolify/` | `python3 build.py server`, `server-test`; its own Dockerfile and CI workflow |
| Wire contract | `shared/online/` | Server tests check the C constants, protocol version, generator version/hash and ROM identity against the client |

The root `build.py`, `Makefile`, `tests/`, `tools/`, documentation and CI
coordinate the components. The existing client paths and output names
are stable across the supported platforms. `build.py all` builds the
server alongside the host and PortMaster clients; `build.py test` includes
the server suite. The server builds and tests without SDL, mGBA, Docker,
or a ROM. It is packaged as a Python zipapp requiring Python 3.11 or
newer. Release CI packages native standalone server executables for Linux,
Windows and macOS alongside the portable Python archive.

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
python3 tools/online_load_proof.py --clients 16
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
- Start → Leave and a server disconnect both restored the carried
  checkpoint through map reconstruction and the regular CONTINUE path.
- Each restored folder, Pack, NaviCust, HP and powers record was identical
  to the one borrowed at entry.
- Every checkpoint file retained exactly the same SHA-256 before and
  after the visits, including the forced disconnect.
- Empty save folders returned to the title; missing and unreadable state
  files were refused without changing the remaining checkpoint files.

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

The developer command requires an explicit `--data-dir`; the Network
screen uses the player's existing folder. Both borrow the saved run's
MegaMan when a readable checkpoint exists. A player without one visits
with the boot loadout. Ordinary direction buttons and B move MegaMan
through BN6 itself. A opens the player list; Start opens Network options,
including Leave. PET access and jacking out are disabled in the Square.
Escape closes the game while preserving the checkpoint.
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
The Square therefore has no healing, rewards, currency or encounters.
The player chooses a name and server, gives server-specific consent,
then borrows the checkpoint for a visit. The save remains authoritative;
online RAM is never checkpointed. [ONLINE_LINK.md](ONLINE_LINK.md) describes
the return and shutdown guards. Phase 2's trade policy remains undecided.

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
Movement renders from a bounded 100 ms interpolation history, while
collision and local movement remain BN6's. Peer names and fixed social
messages appear over their avatars. Source
verification is in [SOURCES.md](SOURCES.md#net-squares-npc-movement-spike-issue-120).

The binary protocol rejects a mismatched protocol version, `LAYER_MAKE`,
generation hash, ROM code or ROM SHA-1. It limits names, positions, message
size, connected players, pending handshakes, per-client traffic and output
buffers. Each channel holds at most eight players; the default total cap
of 64 provides eight channels. It relays position snapshots at 20 Hz.
The private local guest secret derives a separate key for each server.
Local mute/block choices use the server's stable public identity; hosts
can ban, mute, kick or allowlist through a local administrative CLI.

Inspect sends a bounded informational card: HP, buster levels, Souls,
Crosses, depth/act, best depth, title marks and 49 NaviCust cell colors.
Folder and Pack contents, saves and emulator states stay on the device.
The server validates ranges but cannot prove a modified client's claims.

## Evidence and remaining gates

The numerical proof is repeatable with the commands above. The separate
[link and carried-MegaMan report](ONLINE_LINK.md) gives the serial test,
state size/timing and exact limits of that evidence.

Measured on the Linux development host, 15 October 2026:

- **Hosted WSS, two native games:** the Coolify server at
  `cyberworld.saschb2b.com` passed with a trusted HTTPS certificate.
  Both games built seed 120/hash 4132376369, moved reciprocally, and
  produced 211 matching interpolated NPC samples. Median round trips
  were 33/39 ms, maximum 49 ms. Both restored identical depth-three
  loadouts through Leave; all 24 checkpoint files remained unchanged.
  The test exposed and fixed curl negotiating HTTP/2 for a raw HTTP/1.1
  WebSocket Upgrade: desktop and 3DS curl streams now request HTTP/1.1.
- **Hosted browser and moderation:** a browser at the published
  `https://saschb2b.github.io` origin negotiated protocol v2 and received
  snapshots. Muting its temporary test identity through the container's
  administrative CLI suppressed its phrase; unmuting restored it.
  The same browser identity survived redeployment with the server's
  persistent state volume retained; the temporary mute was removed.
  This checks the deployed browser transport; the full browser game
  proof below checks the emulator and generated world.
- **Phase 1, two native games:** both borrowed real depth-three
  checkpoints. Start → Leave and a server disconnect restored identical
  carried records; all 12 checkpoint files per game were unchanged after
  shutdown. All 191 measured peer NPC samples matched the interpolated
  game coordinates. Empty, missing and corrupt checkpoint cases passed.
- **Phase 1, sixteen native games:** two full eight-player channels,
  at least 90 full-channel samples per game, 10,248 matching actual NPC
  samples, and 192 unchanged checkpoint files. All sixteen restored their
  carried loadouts after disconnect. Median local round trips were 1–2 ms.
- **Phase 1, Chrome/WebAssembly and native:** the v2 subprotocol accepted
  a maximum-length 16-character browser name. Both worlds had seed 120
  and hash 4132376369. All 578 browser and 592 native measured NPC samples
  matched interpolation; each traced movement that matched the other
  process. Median local round trips were 4 ms browser and 2 ms native.
- **Phase 1, social controls:** actual game instances exercised Inspect,
  local block, a received quick phrase and emote, and a channel switch.
  Both returned with their save files unchanged. A separate held-reset
  regression kept the restored game active through 20 held chord frames;
  a released and freshly pressed chord then reached the title normally.
- **Phase 0, two native game processes:** seeds 120 and 121 passed. The latter's
  map hash was 1199273041; both clients' median round trip was 1 ms and
  maximum 2 ms. Actual NPC movement, peer departure, server shutdown and
  all 12 checkpoint files per client were checked.
- **Phase 0, Chrome/WebAssembly and native:** seed 120, map hash 4132376369 on
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

Additional platform checks on 15 October 2026:

- **Physical Pocket Nova / ROCKNIX:** the aarch64 game and a desktop peer
  visited the public WSS server with identical seed 120/hash 4132376369.
  All 189 handheld and 197 desktop NPC samples matched interpolation.
  Handheld median/max RTT was 40/47 ms. The actual depth-29, 1,000-HP
  checkpoint was borrowed into a temporary test directory, restored
  identically after leaving, and all 13 installed files stayed unchanged.
  An untrusted TLS fixture received a TLS handshake but no application
  bytes; the game reported certificate failure and restored the checkpoint.
  `tools/online_platform_rocknix.py` repeats the test through an existing
  authenticated SSH control socket and removes its temporary launcher.
- **Windows and macOS native networking:** the game's actual transport
  sources passed on Windows Server 2022 x64 and macOS 15.7.9 ARM64 in
  [CI run 38654155398](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/actions/runs/38654155398).
  Each exercised real protocol exchange, reconnect, Inspect, emotes,
  channels, disconnect, malformed server frames, cancellation, untrusted
  TLS rejection and the public WSS endpoint. Public RTTs were 128–129 ms
  from Windows and 126 ms from macOS. The probe reads no ROM, so full
  game/checkpoint and device-specific behavior remain separate checks.
- **Android 14 emulator, actual release APK:** joined the public WSS
  server, moved through six observed positions, and produced 104 positioned
  peer-NPC samples matching interpolation in the desktop game. All 12
  Android checkpoint files and the desktop checkpoint stayed unchanged.
  A self-signed local TLS server observed a ClientHello and a certificate
  rejection alert, with no application bytes. This exposed and fixed an
  Android partial-read mismatch during WebSocket handshakes, and a held
  touch leaking from the title into the Network address editor.
  `tools/online_platform_android.py` repeats this on a disposable emulator;
  screenshots verify the touch menu, peer display and checkpoint return.

The Pocket Nova was the only physical test device available. No physical
iPhone/iPad, Android phone or New 3DS pass is implied by a build, native
desktop runtime check or emulator result.

Evidence stays under `.build/online-proof/`, `.build/online-load-proof/`,
`.build/online-browser/`, `.build/online-browser-v2/`, `.build/online-social/`,
`.build/online-reset-proof/`, `.build/online-bench/` and `.build/online-tls/`.
Hosted proof records are in `.build/online-public/` and
`.build/online-public-browser/`. Physical handheld results are in
`.build/online-platform-rocknix/` and `.build/online-platform-nova/`;
downloaded native transport reports are in `.build/online-transport-ci/`.
Android APK evidence is in `.build/online-platform-android/`; actual
Practice-battle and delayed-worker evidence is in `.build/online-battle/`.
The public A record was independently
verified as `204.168.144.168`; while the development machine's resolver
retained its earlier NXDOMAIN, only the proof processes mapped that one
hostname to the published address. The original URL, TLS SNI, certificate
verification and browser origin stayed intact. Loopback timings do not
predict internet or handheld performance.

| Spike | Implemented experiment | Gate before a player release |
| --- | --- | --- |
| Transport | Versioned WebSocket server, native platform backends and browser WebSocket, bounded polling, ping/pong, TLS certificate/hostname verification | Measure and test the implemented backends on every target's real hardware; see the server README's verification matrix |
| Players as NPCs | Sixteen native games, two full channels, interpolated NPC coordinates and identity labels | Hardware performance, adverse internet conditions and longer soak tests |
| Link cable | Original Practice repeated; delayed frame inputs complete at 0/50/100/150 ms with 2,822 full replica frames and 1,433 original active timers agreeing; corruption and peer loss detected ([limits](ONLINE_FRAME.md)) | Two cores per client and three buffered frames; production integration, carried loadouts, rollback and hardware performance remain unproven |
| Carried MegaMan | Actual checkpoint loadout borrowed and restored through map reconstruction; leave, disconnect and process shutdown preserve checkpoint files | Broader device and power-loss testing; no Arena result or trade mutation is enabled |
| World from seed | Two game instances build identical geometry/tile hashes from server seed without sending assets | Final Square layout, generator-version migration and all platform determinism |

These results establish the tested presence and checkpoint paths and a
local Practice battle experiment. They do not establish an internet Arena.
A public launch also needs the host's
retention policy, named moderators, backup/restart checks and the remaining
target checks in the server README.

## Epic continuation

`codex/online-net-square` continues toward epic #120 from the monorepo,
deployed Net Square development service, checkpoint-safe visits and
repeatable technical proofs.

The official server now stays first beside four custom entries, with
selection and consent migrated by URL. `tools/online_network_proof.py`
checks its controller and SDL touch navigation. Rapid
Inspect reopen requests receive a fresh response instead of waiting
indefinitely behind the former card-specific throttle.

Between acts, R at Lan's PC opens Jack in / Network / Cancel. A visit
waits for a fresh checkpoint; an observed write failure stays at the PC
with a retry. `tools/online_pc_proof.py` passes 12 checks covering failure,
retry, cancel, local-server visit, unchanged checkpoint return and ordinary
jack-in.

Without a checkpoint, the visitor uses the title's remembered, unlocked
folder, Cross and helper choices. This serves the same run-integrity / social
expression tension: the social loop shows a starting loadout, using
loadout-as-budget and variety rather than permanent power. It never creates
a run, rolls a seed or grants the run's starting gift. The title and visit
share one validator. `tools/online_fresh_proof.py` checks six real-game cases:
Standard, Blade with Cross, Storm with All *, unlocked BeastOut, locked
choices and invalid remembered values. All connect and return, keep fresh
HP, leave the profile unchanged and create no checkpoint. The optional
`CYBERWORLD_LOADOUT_TRACE` contains local numeric observations only.

Trade Central and the production Arena are not enabled. The
[delayed Practice experiment](ONLINE_FRAME.md) completes its four-delay
matrix, with explicit two-core and observation limits.
[Reviewed research goals](re/README.md) now gate native chip exchange,
private checkpoint mutation and complete cable snapshot/replay. Full trade
integration, carried-loadout battles, rollback and remaining physical-device
checks are ongoing work. This draft does not yet close epic #120 or
publish a game release.
