---
type: Engine Mechanism
title: "What a rollback puts back"
description: "Everything a rollback restores, table by table: both cores' mGBA states, registers and SIO words outside them, the cable and its nodes, the pair's scheduling, carried Souls, BN5's hook slots, keys, the sound ring."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-21T13:53:33Z }
---

# What a rollback puts back

Everything outside the two cores' states that the pair's next frames
read:

| State | Where | Put back | How |
| --- | --- | --- | --- |
| Both cores: CPU, RAM, IO, video, timers, DMA | mGBA's state | yes | `arena_pair_save` / `arena_pair_load`: each core's raw state (no flash, no PNG); exact, measured |
| Each GBA's registers as they read, the SIO's control words (`rcnt`, `siocnt`), mGBA's idle loop detection, the open bus, a DMA under way, an interrupt sprung | `struct GBA`, outside its state | yes | in the snapshot (`GbaSnap`), put back after the state: a state loads each register through a write (`SIOMLT_SEND`, the SIO's busy bit came back otherwise) |
| The sound's PSG timing and rendered samples | mGBA's state | as a load leaves them | not byte-exact (above): the pair's hash leaves them out on every game since v8 |
| The flash saves | each core's save data | no | a Practice NetBattle writes none (assumed: not yet checked byte for byte); the hash reads them |
| The cable's lockstep: `mLockstep`'s attached, transfer phase and cycles; `GBASIOLockstep`'s attached counts and both receive buffers | `ArenaPair.cable` | yes | in the snapshot. A core's state loaded writes its SIO registers through the cable (SIOCNT's start bit can begin a transfer), so the cable is put back after both cores |
| Each node's next event, event difference, SO line, mode, finished flag, and its event's place in its core's timing | `ArenaPair.node` | yes | in the snapshot; a core's state loaded clears its timing (`mTimingClear`), and the node's event is scheduled again where it stood |
| The pair's scheduling: each side's cycle credit, who waits, who sleeps, who runs, the transfers counted | `ArenaPair` | yes | in the snapshot and the hash |
| The pair's frame clock: each core's frame at the anchor, the pair's frames since | `ArenaPair.start`, `relative` | yes | in the snapshot; each core's own frame counter is in its state |
| The Souls carried (full rules) | `ArenaPair.carried` | yes | in the snapshot |
| The Dark Soul's chip lists (full rules) | both cores' RAM | yes | written before the battle's first frame, in the cores' states |
| What BN5's hooks keep between two calls, one a core: `xsea.c`'s and `xpanels.c`'s way back through their stubs, `soulchaos.c`'s charge stepped (full rules) | static arrays by `hook_slot` | yes | each named as its hooks install (`hook_keep_slots`), a core's entries saved and loaded by its slot with its state (20 bytes a core); every other hook (the idle halt among them) is a function of the registers and its core's memory and what its install read from the ROMs. Measured: full rules' rollbacks exact for the game |
| UNITE's Souls offered (`soulunite_held_core`), BN5's installed bytes and hooks, the idle hook | each core's slot, ROM copy and hook table | not needed | made as the pair opens, the same through the battle |
| The keys (`ArenaInput`) | `arena_input.c` | not rolled back | the keys come are the truth; the other's not yet come are guessed (his last held) as a frame needs them, and a key that came otherwise rewinds `next` to the first frame that read it, the guesses made anew |
| What a frame left (its hash, the battle's end, the results) | `arena.c` | rewritten | noted as each frame runs, again where it runs again, taken up only once its keys are all known |
| The hashes pending, the counters, the link, the time sync | `arena.c`, `arena_link_net.c`, `arena_sync.c` | no | not the pair's state |
| Each core's sound out (blip buffers) and picture | `arena_pair.c`, `emu.c`'s ring | the sound's ring cut back | output: my GBA's sound into the ring a frame at a time, each frame's mark kept; a rollback cuts the ring back to the first frame run again, what is not yet heard replaced by the frames run again and what was heard skipped (`emu_audio_rewind`, issue #167; before, they were heard not at all); the last one's picture shown |

The stands' report reads the battle as it settles, never a guess: each
frame run notes both MegaMen as the first core has them (`note`, again
where a rollback runs it again), and the report takes the last frame
whose keys are all known (`arena_reading`). A battle of `--dev
arena,arenabusy` at delay 1 ended on frame 5221 with the same report
(HP 75/100 and 0/100, winner 1) and the same hashes with no guess (lag
0, window 0) as with 778 rollbacks (lag 3, window 6; lag 5, window 8).
