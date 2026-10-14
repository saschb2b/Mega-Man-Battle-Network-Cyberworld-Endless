# The online cable and carried MegaMan

## Phase 1: the checkpoint visit

The Network screen and `--online` now borrow the actual saved run. Entry
waits for outstanding save writes, reads the checkpoint, and goes through
the same map reconstruction and loadout restoration as CONTINUE before
building the Square. The director is stopped for the visit. The local
`OnlineLoadout` includes the folder, Pack, NaviCust, upgrades and Souls;
only the bounded public Navi card is sent to the server.

Leaving the Network menu, the reset chord, or a server disconnect stops
the online session and rebuilds the checkpoint's ROM map before loading
its state again. The saved Run, Profile, random-generator state and last
battle memory are restored. The held Leave buttons are released before
offline input resumes. Until fresh input resumes play, closing the game
also keeps the original checkpoint; the normal suspend writer cannot
replace it with arrival frames. Closing during a visit never saves Square
RAM. A visit without a saved run returns to the title.

An existing run whose state is missing or unreadable is refused. Online
entry does not use CONTINUE's fresh-layer fallback in that case.

`python3 build.py online-proof` runs two native game processes through the
built server. Each first creates its own depth-three checkpoint, then
walks in the shared Square. One uses Start → Leave, and the other returns
after the server stops. The proof checks the actual game NPC coordinates
against their interpolated peer positions, matching world hashes, the
entire local carried-loadout record on return, and SHA-256 hashes of every
checkpoint file before and after process shutdown. It also checks an
empty save folder and missing or unreadable state files. Its files stay
under `.build/online-proof/`; none are release inputs.

The Linux run on 15 October 2026 carried both depth-three MegaMen and
restored both loadouts identically. All 12 checkpoint files per client
were unchanged after the visit and shutdown. The two processes agreed on
the server's seed and generated map, and all measured NPC coordinates
matched the rendered peer coordinates. These are presence and checkpoint
results; the separate linked-battle measurements follow below.

## Phase 0: the local cable experiment

The runnable experiment is `python3 build.py online-link`. It uses a
256-byte ROM of its own instructions, two copies of the pinned mGBA 0.10.5
core, and mGBA's multiplayer SIO driver. No game ROM is needed. The test
exchanges 256 pairs of distinct serial words, checks all 1,024 received
words and the assigned player IDs, and fails on a timeout, busy bit or
serial error. Its scheduler advances the cores together in one process.
It exercises the serial API; it does not execute BN6's NetBattle code.

The same command checks the carried-loadout reader against synthetic
folder, Pack, NaviCust and upgrade values, including pointers that would
read past EWRAM. It overwrites every byte of EWRAM and IWRAM, restores an
in-memory state, and compares all 294,912 bytes. A complete state includes
the emulated flash save too; when one exists, the probe overwrites and
restores it and compares every byte.

Give it a player's own ROM and existing state to repeat the restoration
test with a real MegaMan:

```sh
python3 build.py online-link --rom /path/to/bn6g.gba --state /path/to/run.state
```

The ROM and checkpoint are opened read-only. No states, ROM bytes, chip
art or extracted text are written by the probe. A state loaded this way
is inspected without running its CPU: a run's ROM map additions must be
rebuilt before that state could safely execute.

### Measured here

Measured on the Linux x86-64 host in the project's build container on
15 October 2026. Times are means of 200 operations and vary with host
load; these are feasibility measurements, not performance guarantees.

| Measurement | Result |
| --- | --- |
| Serial transfers, same process | 256 passed, 1,024 received words checked |
| Raw core state | 397,312 bytes |
| Eight raw rollback states | 3,178,496 bytes |
| Raw state save / restore | about 0.010 / 0.014 ms |
| Full checkpoint used in the experiment | 430,144 bytes, including 32,768 bytes of emulated savedata |
| Full checkpoint restore | about 0.18 ms |
| EWRAM and IWRAM restoration | all 294,912 bytes identical |
| Savedata restoration | all 32,768 bytes identical |
| Carried MegaMan reader | 6,664 bytes in this C build, identical before and after restore |

`OnlineLoadout` is a local record, not a wire format. It reads the current
folder; all 512 Pack entries including the extension used by other games'
chips; the NaviCust grid and its 49 placement slots; current, base and
maximum HP; the buster's levels; Regular and TagChip settings; folder
limits; Cross and Beast flags; and the Souls mask supplied by the caller.
Every address is an existing `bn6.h` name. Souls and the run's other side
records remain separate from the emulator state. The standalone probe
supplies zero Souls; its synthetic test checks a nonzero caller mask.

The same Release/NDEBUG probe also ran on the physical Retroid Pocket
Nova through PortMaster on 15 October 2026, with the installed ROM and a
temporary copy of its real run state (1,000 HP and a 30-chip folder).
The raw state is 397,312 bytes, or 3,178,496 bytes for eight slots; the
complete state is 430,144 bytes. Three runs measured raw capture at
0.0144–0.0153 ms and restore at 0.0199–0.0210 ms. Complete-state restore
took 0.1361–0.1404 ms. All 294,912 RAM bytes, 32,768 savedata bytes and
the 6,664-byte carried record restored exactly. The ROM-free cable's 256
transfers took 11.00–13.56 ms. Hashes of all 13 original files remained
unchanged. Local evidence: `.build/online-platform-nova/state-report.json`
and `state-benchmark.txt` beside it. These capture and restore timings do
not measure rollback replay or a full game's replay cost.

## Actual Practice NetBattle

`python3 build.py online-link --practice --rom /path/to/bn6g.gba` now
finishes BN6's own Single Battle → Practice on two cores. It creates the
project's ordinary pre-director boot state in a new ignored evidence
directory, then loads that state into two **plain ROM** cores. An existing
`--state /path/to/boot-3.state` skips boot preparation. A run checkpoint
is not an acceptable substitute: its patched map would need reconstruction.

From that boot, the script opens the PET, selects Comm, Link Cable,
NetBattle, Single Battle and Practice, and confirms the game's save and
connection questions. Both flash-save devices exist only in memory. Each
game chooses OK without chips; side 0 charges and releases its buster
while side 1 stands still. No battle function is injected and neither
core's ROM is patched. This is one controlled battle with the starter
MegaMen, not coverage of every folder, form or battle setting.

The command fails unless the game's result bytes identify the winner and
loser, both cores return to Comm, and their battle timer, global HP and
positions agree. A CSV also records both cores at every video frame. The
Python verifier aligns those observations by BN6's battle timer and
requires complete active-frame coverage with identical HP and positions.
SHA-256 checks confirm that the supplied ROM and boot state are unchanged.
The report, CSV and complete-frame PPM screenshots stay under
`.build/online-battle/`; they are not release inputs.

Measured on the pinned Linux host core on 15 October 2026:

| Observation | Result |
| --- | --- |
| Actual multiplayer serial transfers, including menu negotiation | 18,855 |
| Active battle frames compared | 1,433, with no HP or position disagreement |
| Damage | Ten charge shots of 10 HP, on the same battle timers in both cores |
| Final global HP | Side 0: 100; side 1: 0 |
| Game results | Side 0: 1 (won); side 1: 2 (lost) |
| Final battle timer | 1,433 in both cores |
| Return | Both at the original Practice menu |

`tools/online_link_script.c` holds the exact controller sequence and the
logical result checks. `tools/online_link_bn6.c` runs the cores with the
pinned SIO driver; it also accepts bounded `run`, `shot`, `status`,
`ipcfull` and `ipcactive` research commands.

### Two processes and delayed serial delivery

```sh
python3 tools/online_link_ipc.py --rom /path/to/bn6g.gba \
  --state /path/to/boot-3.state --wall-ms 60000 --window both
```

Each worker executes its own core in a separate process, starting from
the fresh boot state. The parent coordinates numeric cable fields, cycle
credits and transfer phases over bounded local socket messages. Inherited
peer cores are inert driver shadows; no ROM or RAM is sent. The pinned
mGBA scheduler's sleeping-slave and posted-credit rules are preserved.
The report checks separate worker PIDs, serial receive transcripts and
both games' HP and panel positions at each active battle timer.

The complete zero-delay run passed with 18,828 serial transfers per
worker, identical serial hashes, all 1,433 active frames agreeing, and
the same 1/2 results and 100/0 HP as the local experiment. Under the
concurrent matrix's host load it took 37.658 seconds; this developer
coordinator's wall time is not a production performance target.

The delay model adds one optimistic delivery hold per real serial
transfer, when the master's words become available to the slave. This
is **not an RTT** and not a WebSocket transport. Cycle bookkeeping stays
local. `--window custom` begins at the first Custom screen. The additional
`--window active` excludes negotiation, Custom selection and the intro;
it starts only after both battle timers run, then keeps applying delay.

| Added delivery delay | Active frames compared at the 60-second bound | Applied delay | Full battle completed |
| --- | --- | --- | --- |
| 50 ms | 95, all matching | 43.727 s | No |
| 100 ms | 48, all matching | 44.657 s | No |
| 150 ms | 35, all matching | 49.133 s | No |

The active-only cases began at relative frame 1,149 and serial transfer
4,579. All three kept identical serial transcripts until the deadline.
The wider Custom-screen cases also reached their bound without completing
the fight. A forced disconnect and a deliberately corrupted delivered
word each stopped the coordinator after 20 transfers, rather than hanging
or reporting a successful battle. The corruption check is the harness's
serial consistency check, not a claim about BN6's own desync detection.
Malformed framing, EOF and stalled input also have ROM-free checks.

Local evidence is `.build/online-battle/ipc-mqvtfpaj/report.json` for the
active-only matrix and `ipc-mc8btuem/report.json` for the wider window.
Both reports retain unchanged source-file hashes. No delayed case is
claimed as a completed fight: these bounded results reject raw serial
delivery as a real-time Internet battle design, even under the optimistic
delay model. The production presence server carries no serial messages.

### What this answers, and what remains

The pinned core can link two local instances without adding another
emulator or importing Tango code. Local state capture and restore have
ample time on this desktop, and a complete local snapshot can protect
RAM and flash. This does **not** establish a network Arena or rollback.

Actual Practice battles now pass both in one process and in two separate
processes at zero added delay. The 50/100/150 ms experiments establish a
negative real-time feasibility result for raw serial delivery. A network
Arena still needs a frame-level input design, replay validation, recovery
and per-platform replay budgets. It remains disabled.

mGBA's local lockstep object shares cycle budgets, transfer phases and
receive buffers between the two cores. Sending the words alone over a
WebSocket is not a substitute for that scheduler. Its external driver
state also needs to be saved or reconstructed alongside a core state for
rollback. A raw core state does not contain the ROM patches, the run's
side records or the network session. These are explicit design gates.

The browser, Android, Apple systems, Windows and the New 3DS
have not been timed by this state experiment. In particular, the desktop's
state times cannot establish the 3DS's replay budget. The experiment's
binary can be built for each target, but a target's runtime measurement
and a real linked battle are required before enabling its Arena.

BN6's Practice choice itself is **verified in the local bn6f text
scripts**: `CompText86DA1B8` scripts 17/25 describe and label Practice,
and 18/26 describe and label Battle, whose loss forfeits a Pack chip.
The actual starter-loadout Practice experiment above additionally verifies
its result and return path. It does not establish the chip-forfeit path
for the separate Battle choice. The existing experiment recorded in
[RIVAL.md](RIVAL.md) establishes that the link-battle option waits for a
partner. No new ROM offset was introduced here.

The emulator interfaces were checked against the pinned upstream
[SIO lockstep implementation](https://github.com/mgba-emu/mgba/blob/0.10.5/src/gba/sio/lockstep.c)
and [core interfaces](https://github.com/mgba-emu/mgba/blob/0.10.5/src/gba/core.c).
The probe must be compiled with `NDEBUG`, matching the Release library:
the upstream lockstep structures have conditional debug fields. No
Tango implementation code was consulted or copied.

As documentation-only prior art, [Tango's official site](https://tango.n1gp.net/)
advertises rollback, support for every English and Japanese Battle Network
game, automatic accurate replays and matchmaking by a shared link code
(checked 15 October 2026). Those product claims establish existing
behavior, not an algorithm or code that can be reused. The frame-level
design direction above follows independently from this probe's measured
serial stalls.
