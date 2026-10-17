# Staged trade: unchanged-source reload control

Status: reviewed harness correction; comparator review precedes candidate
acceptance. The candidate is
not accepted. The original native peer Trade remains INCONCLUSIVE.

The first candidate preserves every serialized source byte outside the
six reviewed inventory fields and matches the original routine result.
Its fresh reload differs from its file on ten audio bytes. This same
change occurs without linking or running the candidate: a separate
`tools/online_trade_reload.c` loads the original source fixture, verifies
its PC is already at the map boundary, then saves immediately. No CPU
instruction or frame runs after the load. Two independent fresh cores
repeat exactly for each of the four pinned source fixtures.

## Observed control, before any comparator change

Local evidence: `.build/trade-reload-independent/observations.json` and
its eight complete reload states. Original source states are unchanged
from `trade-stage-routine-evidence.md`. The control executable's SHA-256
is `7bf8c483defbdaa78a1fd267acd53bc6fb6ba9d7cb2ad62ca73e8021495b0717`;
its source is `ed0b8ba29a4ef71bc1cbc19a75b930564b98ded427c313204edcbf28b2ab4377`.
The original runtime helper and mGBA library retain the existing pins.

All four unchanged-source controls produce the exact same raw delta:

| Serialized byte | Before | Reloaded | Named mGBA field |
| --- | --- | --- | --- |
| 0x132 | 0x41 | 0x61 | audio.psg.ch1.envelope (duty index) |
| 0x141 | 0x38 | 0xb8 | audio.psg.ch1.lastUpdate |
| 0x146 | 0xc0 | 0xe0 | audio.psg.ch2.envelope (duty index) |
| 0x151 | 0x41 | 0xc1 | audio.psg.ch2.lastUpdate |
| 0x17c | 0xd4 | 0x4c | audio.psg.ch4.lfsr |
| 0x17d | 0x4b | 0x6c | audio.psg.ch4.lfsr |
| 0x184 | 0xe9 | 0xc9 | audio.psg.ch4.lastEvent |
| 0x185 | 0x6a | 0x6f | audio.psg.ch4.lastEvent |
| 0x188 | 0x09 | 0xe9 | audio.psg.ch4.nextEvent |
| 0x189 | 0x6b | 0x6f | audio.psg.ch4.nextEvent |

| Fixture | Complete reload SHA-256, identical for both controls |
| --- | --- |
| first-code | `64389d911cc93b04d64c95b47506e6a8cbe932007bc8cfaac60531b0a6f6afb2` |
| second-code | `a734435a8976bb17fe0e8b1daa23405383fba82c9ac7c6fe54a9784aeda36cf8` |
| counter-wrap | `0e32bac01d8ce6fe6fe7ab2fa9c19acd6beec15215c9d4693a35f7bf5cdbf183` |
| incoming-98 | `ba7d6186aa59edd3fe88094295a45ebe4aaa0bf352d1672067cea17144d94b61` |

The field classification comes from the installed pinned mGBA
`GBASerializedState` layout. Its [PSG serializer and deserializer](https://github.com/mgba-emu/mgba/blob/0.10.5/src/gb/audio.c)
write duty indices, timing and the noise generator's state; the
[GBA load/save path](https://github.com/mgba-emu/mgba/blob/0.10.5/src/gba/serialize.c)
restores IO and audio separately. In `GBAAudioDeserialize`, the PSG is
restored before the function writes sound registers through `GBAIOWrite`;
those writes call `GBAudioRun`, advancing channel bookkeeping to the same
current time. The independent Arena restore investigation found the same
path ([replay notes](../ONLINE_REPLAY.md)). This establishes which fields changed,
not a general authorization to ignore audio or assume every future
normalization is harmless. The complete reproducible control is the
proposed comparison authority.

## Reviewed acceptance correction

Keep the original candidate-file check unchanged: exactly the source
checkpoint outside the six observed inventory fields; complete Pack,
ownership, Library and order counter match the original Take/Give result.
Do not alter the candidate to copy audio state from the oracle.

For reload, require two independent unmodified-source controls to match
byte for byte and match the four fixed complete hashes above. Record the
actual control binary/source pins in the manifest. Missing controls,
unexpected changes, changed pins or nonrepetition are INCONCLUSIVE.
Compare every byte of the fresh candidate reload to this independent
source reload, allowing only the same six inventory fields; those fields
and all complete inventory regions must still equal the original routine
result and the candidate file. This adds no audio mask or other ignored
byte, and cannot accept a candidate-authored expected state.

Keep the original editor comparison, source/live nonmutation checks and
all deliberate faults. Add synthetic faults for missing/differing source
reload controls, changed pinned control hash, candidate audio corruption,
and protected reload corruption even when that byte belongs to audio.
The revised comparator must be separately reviewed before another
candidate acceptance run. BN5, All *, patched project states and journal
installation remain outside this unit.
