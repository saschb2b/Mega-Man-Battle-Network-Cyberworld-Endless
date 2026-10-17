# Local cable snapshot experiment

Developer research for issue #120, after the
[delayed Practice proof](ONLINE_FRAME.md). This is a complete local linked-pair
restore experiment. Production Arena remains disabled.

**The bounded matrix passes.** Fresh original controls agree across 2,823
complete records; all 36 restored continuations match with no masks, and all
five deliberate failures are detected. The numerical result, source and
binary pins, effective build commands and trace hashes are in
[arena-replay-evidence.json](re/arena-replay-evidence.json).

The reviewed acceptance is [Arena unit 2](re/arena-next-goal.md), with separate
[instruction-boundary](re/arena-cable-boundary.md) and
[bounded cable-omission](re/arena-cable-omission.md) extensions. The normal
comparator has no masks: both serialized core states, both complete flash
devices, numeric cable state, script bookkeeping and logical game samples
must match the uninterrupted original immediately and after every replayed
pair frame.

| Original snapshot point | Pair frame | Accepted continuations |
| --- | ---: | --- |
| Before a native flash change | 295 | 1, 4 and 8 frames, each twice |
| Both players in Custom | 813 | 1, 4 and 8 frames, each twice |
| Active nonidle cable, scheduler round 7333 | 1150 | 1, 4 and 8 frames, each twice |
| Held charge | 1151 | 1, 4 and 8 frames, each twice |
| Before charge release | 1252 | 1, 4 and 8 frames, each twice |
| After damage | 2527 | 1, 4 and 8 frames, each twice |

Each complete snapshot uses 863,016 bytes on this host. The 46 restores in
the proof took 56.63 ms in aggregate, about 1.23 ms each. These are desktop
measurements, including the fault setup restores, not a handheld budget.
The subject's runtime is recorded separately from copying and replay costs.

## Reproduce locally

```sh
python3 tools/online_replay_proof.py \
  --rom /path/to/bn6-gregar-us.gba \
  --state /path/to/boot-3.state
```

The command builds the original, both independent observing executors and the
restore subject with the repository warning flags and `WERROR=1`. It starts
fresh original controls before selecting snapshot positions. Reviewed source
hashes, the original complete trace hash, the ROM, boot state and linked mGBA
library are pinned; a mismatch requires separate review. The input files are
mounted read-only. Every core's save backing is a memory file. Reports and
traces go under `.build/online-replay/`; no ROM, raw emulator state or flash
dump is published.

The fixture is deliberately narrow: the same Gregar USA starter Practice
script as the delay proof. It is not an arbitrary player checkpoint. The
command refuses a different boot-state hash instead of silently claiming
that another run has the same coverage.

## What the snapshot includes

`online_replay_snapshot.c` owns the memory copy. The snapshot holds both raw
core states and flash images; the pair's frame/script counters; cable phase,
credits, wait mask, transfer counters and receive words; both serial nodes'
numeric state, relative event due times and scheduledness; and the suspended
executor position. Live pointer links and event contexts are reconstructed
against the current pair. Callback addresses are never saved as data.

The original micro observer reaches an actual active transfer at pair frame
1150, after scheduler round 7333 and before the pending frame-target check.
Both cable events are already due: their delays are negative. Restoration
preserves those values and resumes that pending check before setting new
inputs or stepping either core. The normal whole-frame worker is unchanged.

The implementation also captures numeric state that mGBA's ordinary state
loader does not round-trip exactly in this scenario:

| Live state | Experiment and source |
| --- | --- |
| Controller latch and CPU event countdown | The raw serializer records `keysLast`, while input uses `keysActive`; queue reconstruction interrupts the countdown. Restore the captured latch without invoking keypad IRQ testing and restore the countdown after rebuilding events. [mGBA serialization](https://github.com/mgba-emu/mgba/blob/0.10.5/src/gba/serialize.c) |
| Four PSG channel value structs | The first implementation differed in channel timing immediately after load. Restoring these pointer-free channel values removed all immediate differences. The loader deserializes PSG state and then writes channel control registers, which can run the channels again. [mGBA audio loader](https://github.com/mgba-emu/mgba/blob/0.10.5/src/gba/audio.c) |
| Live IO latches | Immediate raw state then matched, but the first following transfer diverged. The serializer omits some live registers, including `SIOMLT_SEND`; the cable reads that latch for its next transfer. Capturing the numeric IO array made all 36 normal replays match. [IO serializer](https://github.com/mgba-emu/mgba/blob/0.10.5/src/gba/io.c), [lockstep driver](https://github.com/mgba-emu/mgba/blob/0.10.5/src/gba/sio/lockstep.c) |

These are observations of the pinned mGBA build, not new BN6 ROM offsets.
Re-verify them by running the complete command; private diagnostic byte dumps
were used to identify fields and are not repository artifacts. The refuted
assumption is that mGBA's raw state plus external cable fields alone suffices
for exact pair replay.

## Coverage and limits

The independent comparator selects Custom, a held charge, its release,
post-damage play, a native flash transition and an active scheduled cable
transfer from original observations. Each is restored twice for horizons
of one, four and eight complete pair frames. Snapshot-taking alone must
preserve the full original trace. Deliberate omissions or changes to flash,
cable, credit, a received word and an event delay must be detected.

Whole-frame observations alone never saw a nonidle transfer. That first
observer was correctly recorded as INCONCLUSIVE. The separately controlled
micro observer retains the exact same full original trace and exposes the
actual scheduled transfer without changing it. Cable omission is reported
separately when it reaches the existing executor bound; incomplete logs or
arbitrary failures cannot stand in for that evidence.

In the accepted run, omitting flash first differs at frame 295; corrupting a
credit, receive word or scheduled event first differs at frame 1150. Omitting
the entire cable differs only in cable state immediately at frame 1150,
then produces four complete frames and reaches the observed four-frame lead
bound at frame 1154. The wrapper observes the dedicated bounded exit. This
negative result is separate from the 36 fully matching normal replays.

This does not establish prediction, remote input correction, a rollback
queue, audio or presentation-buffer continuity, another loadout, selected chips, other ROMs,
another mGBA build or handheld performance. It does not enable online battles.
The next accepted unit is the carried native-chip scenario in the same
[unit plan](re/arena-next-goal.md).
