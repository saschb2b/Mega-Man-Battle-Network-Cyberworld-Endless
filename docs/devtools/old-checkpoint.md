---
type: Tool
title: "An old checkpoint: the last release's run continued"
description: "`tools/old_checkpoint.py` saves a run with the last releases' own builds, continues it in this build and plays it on by the autopilot, failing where the game stands still, and lists what the old state's RAM still holds that this build's own run does not."
tags: [devtools, saves]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-13T23:20:00Z }
---

# An old checkpoint: the last release's run continued

```bash
python3 tools/old_checkpoint.py                    # the newest two releases' runs, continued by build/host/cyberworld
python3 tools/old_checkpoint.py --from v0.20.0     # one release's (a tag or a commit)
python3 tools/old_checkpoint.py --from v0.20.0 --new v0.21.0   # continued by another commit's build
python3 tools/old_checkpoint.py --pointers         # every word the state carried, not three a region
```

A checkpoint is an emulator state: the game's whole RAM as the build
that saved it left it, the engine's writes in it. The next release loads
it into a ROM copy it has laid out anew. Issue #482 was that: 0.20.0's
checkpoint held the engine's patch of BN6's hit gate in IWRAM with
0.20.0's address in the free space, 0.21.0 had moved the code there by
0x40 bytes, and a run continued across the update stood still at the
first hit of any attack ([Hooks](../engine/emulation/hooks.md), RAM kept
patched). No test had ever loaded another build's state. This one does,
before a tag: it is the Release row's check in `AGENTS.md`. It needs the
ROMs and Docker (the releases' builds), so it is local, not CI's.

## What it does

For each release (the newest two tags of `v*` before HEAD, as players
skip one; `--from` names others) and one seed:

1. **The release's build**, its host binary made once in a worktree of
   its own under `.build/cw/ref/` ([Agents' tools](agents-tools.md),
   Comparing; `tools/cw.py prune` removes them).
2. **Its run**: a new game, headless, the autopilot walking to layer 2
   (`--depth`), whose arrival writes a checkpoint. The data folder is
   copied as the checkpoint left it.
3. **The control**: the same with this build, its own checkpoint
   continued and played as below. A seed whose autopilot loses there
   tells nothing, and the next is tried (1 to 6; `--seed` names them).
4. **CONTINUE**: this build started on the release's folder at the
   title, START and A pressed. The run must stand on the layer saved;
   a build that shows no CONTINUE (a new `RUN_MAGIC` with no reader of
   the old) fails, unless `--allow-refused` says it is meant.
5. **Played on** by the autopilot until a battle is won and the next
   layer reached, a sample every 60 frames. It fails where the game's
   own frame count stands for 300 frames (the state's `gameframe` line,
   `BN6_FRAME_COUNT`: the main loop's, which stops when the CPU is lost
   in a loop while the engine's frames go on), where a battle's picture
   stands for 600 (a build from before the `gameframe` line is watched
   by its pictures alone), where the game leaves the run or stops, or
   the layer is not left in `--limit` frames (20000). MegaMan deleted is
   the autopilot's loss: the seed tells nothing.
6. **NEW GAME on the release's boot state**: `boot-3.state` in a data
   folder is written by the first build started there and loaded by
   every later one at each new run (`src/emu/boot.c`). A new run by this
   build from the release's folder is played the same way to layer 2.
7. **What the state carried**: the release's run continued once more
   and left at once, so its quit's state is this build's RAM after the
   CONTINUE; this build's own checkpoint of the same layer, continued
   and left the same way, beside it.

Its games stay in `.build/old-checkpoint/RELEASE-sSEED/` (`saved`,
`continued`, `new-game`, `mine`, `left`, each `tools/play.py`'s folder
with its `log.txt`), a failure's last picture among them.

## What the state carried

Two lists, to read, not a verdict:

- **IWRAM's routines** (`0x03005B00`-`0x030079D0`, the image the game
  copies in as it powers on; a state's bytes there are the ROM's but for
  the engine's patches, measured): each byte that differs from this
  build's own run. A patch of another build's left in place shows here,
  and so would a hook's BKPT this build does not put.
- **Words that point into the free space** (`0x08800000` on), of IWRAM
  and EWRAM: those this build's own run has too, those this build wrote
  since the CONTINUE, and those still the release's own where this
  build's run has another address or none, each with the region of
  `src/emu/free_space.h` it lies in (the release's name where the region
  has moved). Data that reads like an address is among them (tiles,
  palettes): the comparison with this build's own run is what sorts
  them, and a word listed is a place to look, no more.

On 13 November 2026, 0.20.0's and 0.21.0's runs:

| Continued by | 0.20.0's run | 0.21.0's run |
| --- | --- | --- |
| 0.21.0 (`--new v0.21.0`) | FAIL: stands still in layer 2's first battle; `0x03007254` and `0x03007298` still 0.20.0's (`0x08D90071`, `0x08D900A1` for `0x08D900B1`, `0x08D900E1`) | |
| `dev` before the fix | FAIL, the same | ok |
| `dev` with the fix (#512) | ok; IWRAM's routines the same bytes | ok; the same bytes, every word as this build's own |
| 0.21.1's candidate (14 November) | ok; IWRAM's routines the same bytes; `SOULS_FACE_PALS`' two words still 0.20.0's (`0x08C82D00` for this build's `0x08C82D60`) | ok; the same bytes, every word as this build's own |

0.21.2's candidate (14 November) continued the last two releases' runs, 0.21.0's and 0.21.1's: both ok (a battle won and layer 3 reached, and two), IWRAM's routines the same bytes and every word into the free space as this build's own.

## What a state holds of the engine's

What the engine writes into RAM a state carries to the next build, and
what makes it this build's again:

| RAM | Written by | After another build's state |
| --- | --- | --- |
| IWRAM's routines patched in place (`emu_keep`): the hit gate's two calls into the sea's code (`BN6_HIT_HIDDEN`, `BN6_HIT_HIDDEN2`, `xsea.c`), the panels' bonus and hit tests and the panel flags' literal (`xpanels.c`) | as the core starts | made the engine's after every load whatever the state holds, and a checkpoint saved with the game's own bytes (issue #482's fix). Proved here: fails on 0.21.0's build, the same bytes now |
| A hook's BKPT in RAM | `hook_add` on a RAM address | `hook_reapply` puts this build's again. A BKPT a state holds where this build has no hook is a stray: its instruction is skipped (`hook_strays`). No checkpoint of 0.20.0's or 0.21.0's holds one (their IWRAM and EWRAM searched for the BKPT's words); a release that takes a RAM hook away would leave one, and the first list shows it |
| The boot state, `boot-3.state` | the first build started in a data folder, `emu_save_state` (the game's own bytes under the kept patches) | loaded at each new run by every later build until its number is bumped. Step 6 plays it: ok from both releases |
| The layer's maps, objects, warp list, label and texts, pointed at from EWRAM (`FREE_LAYER`, `FREE_WARP_LIST`, `FREE_LABEL`, `FREE_TALK`) | as a layer is entered | the layer is rebuilt before the state loads and entered again after it (`layer_reenter`); where another build made it, from its start, its flags and Mystery Data picks forgotten (`director_resume`, the `!same` branch) |
| The song the music player plays, a copy in `FREE_SONGS` | a map's or the director's song | stopped where another build made the layer; the map plays its own. Seen on seed 3: the player then still names the song of the checkpoint's moment where this build's own run has gone on to the next; both are this build's addresses. Not listened to |
| A battle's record, families' and Navis' rooms, chips' code (`FREE_RECORDS`, `FREE_FAMILIES`, `FREE_VIRUS_ROOM`, `FREE_NAVI_CODE`, `FREE_CHIP_*`) | as a battle starts | no checkpoint is written in a battle; what a state holds of the last one is written anew by the next. Step 5's battle is the check |
| The emotion window's palettes (`SOULS_FACE_PALS` in `FREE_SOULS`, two words at `0x0200B820` and `0x0200B8E8`) | the Souls' faces (`soul_room.h`) | carried as they are: right while the table stays where it is (0.20.0's at `0x08C82D00`); a release that moves it draws a face's colours from the old place until the window is set again. 0.21.1 lays it 0x60 further on (`0x08C82D60`), so a run continued from 0.20.0 carries the old words; what the window then shows is not looked at |
| The engine's own RAM (`RAM_PACK_EXT`, `RAM_PLAYER_AI` and the battle's, `src/emu/free_space.h`) and the cart's save RAM in the state | the installers and the run | read as this build lays it out; a format that changes needs its reader (`xchips_pack_ready` for the pack's entries) |

A row whose fix is not proved by the tool stays a row here: what to
suspect when a later release's check fails.
