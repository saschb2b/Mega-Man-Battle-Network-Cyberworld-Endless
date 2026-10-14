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
results; the linked-battle gates below remain open.

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

### What this answers, and what remains

The pinned core can link two local instances without adding another
emulator or importing Tango code. Local state capture and restore have
ample time on this desktop, and a complete local snapshot can protect
RAM and flash. This does **not** establish a network Arena or rollback.

The next cable experiment must start BN6's actual Practice NetBattle,
finish a battle with both sides agreeing, then put the serial boundary
between two processes with 0, 50, 100 and 150 ms of injected delay. That
gate remains open. The presence server has no serial-relay message in
Phase 0; its successful two-client connection is evidence for presence,
not evidence for a NetBattle.

mGBA's local lockstep object shares cycle budgets, transfer phases and
receive buffers between the two cores. Sending the words alone over a
WebSocket is not a substitute for that scheduler. Its external driver
state also needs to be saved or reconstructed alongside a core state for
rollback. A raw core state does not contain the ROM patches, the run's
side records or the network session. These are explicit design gates.

The browser, PortMaster, Android, Apple systems, Windows and the New 3DS
have not been timed by this experiment. In particular, the desktop's
state times cannot establish the 3DS's replay budget. The experiment's
binary can be built for each target, but a target's runtime measurement
and a real linked battle are required before enabling its Arena.

BN6's Practice choice itself is **verified in the local bn6f text
scripts**: `CompText86DA1B8` scripts 17/25 describe and label Practice,
and 18/26 describe and label Battle, whose loss forfeits a Pack chip.
That verifies the menu distinction, not the game's entire reward or
result path. The existing experiment recorded in [RIVAL.md](RIVAL.md)
establishes that the link-battle option waits for a partner. No new ROM
offset was introduced here.

The emulator interfaces were checked against the pinned upstream
[SIO lockstep implementation](https://github.com/mgba-emu/mgba/blob/0.10.5/src/gba/sio/lockstep.c)
and [core interfaces](https://github.com/mgba-emu/mgba/blob/0.10.5/src/gba/core.c).
The probe must be compiled with `NDEBUG`, matching the Release library:
the upstream lockstep structures have conditional debug fields. No
Tango implementation code was consulted or copied.
