# Reverse-engineering work

Use the project's [reverse-engineering skill](../../.claude/skills/reverse-engineering/SKILL.md)
for new investigations. The [dashboard](dashboard.html) is an offline,
dependency-free view of the current research, not evidence by itself.

## Sources of truth

| Record | Location |
| --- | --- |
| ROM identity, offsets, symbolic names and how to verify them | [ROM_DATA.md](../ROM_DATA.md), `RomLayout`, `src/emu/bn6*.h` |
| Verified, derived, assumed, open and refuted claims | [SOURCES.md](../SOURCES.md), the linked experiment report |
| Original/generated/adapted behavior | [FIDELITY.md](../FIDELITY.md) |
| Free space and write claims | [EMULATION.md](../EMULATION.md), `src/emu/free_space.h`, `claims.h` |
| Existing virus differential | `tests/xdiff/`, `tools/xdiff.py`, [DEVTOOLS.md](../DEVTOOLS.md) |
| Link and snapshot experiments | [ONLINE_LINK.md](../ONLINE_LINK.md) |
| Accepted Arena frame-input experiment | [Arena frame goal](arena-frame-goal.md), [results and limits](../ONLINE_FRAME.md), `tools/online_frame_compare.py` |
| Accepted local cable snapshot/replay | [Replay report](../ONLINE_REPLAY.md), [numeric evidence](arena-replay-evidence.json) |
| Current carried-chip unit | [Next Arena goals](arena-next-goal.md) |
| Accepted trade inventory and recovery design | [Trade inventory goal](trade-inventory-goal.md), [inventory evidence](trade-inventory-evidence.md), [ONLINE_TRADE.md](../ONLINE_TRADE.md) |
| Current checkpoint swap unit | [Native Trade and staged swap goal](trade-stage-goal.md) |

New work starts with a sentence naming the unit, observable, input/state
coverage, original-game oracle and allowed differences. Pin its ROM hash
and emulator build. Prove the harness repeatable before changing the
subject. Commit specs and comparator separately, have another reviewer
check them, then implement against that fixed contract. Record the first
divergence; change the harness only in a separate explained oracle commit.
Mutation tests must demonstrate that plausible faults fail the comparison.

The repository's existing research is not retroactively marked as meeting
this process. Its reports retain their original evidence and limits. New
iterations improve the relevant records as they touch them. No aggregate
count of verified ROM facts is claimed without a machine-readable audit.

## Current target and boundaries

The Arena experiment uses the player's BN6 Gregar USA image, SHA-1
`89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6`, running on mGBA 0.10.5.
`docker/mgba.sh` pins the upstream archive's SHA-256 to
`91d6fbd32abcbdf030d58d3f562de25ebbc9d56040d513ff8e5c19bee9dacf14`.
Every target-bound runner must verify the image before using a checkpoint.
The input file stays read-only and all saves, traces and dumps stay local
under `.build/`. Only whole gameplay frames may become public media.

The owner's requested link work uses controlled local peers. It neither
contacts an original game's online service nor touches other players'
accounts. No Tango implementation is an input to this MIT codebase.

Raw serial delivery with injected internet-scale delay is a recorded
negative result, not a completed delayed fight. The accepted frame-input experiment keeps
the cable inside each local pair of cores and sends frame-numbered inputs.
It completes at 0/50/100/150 ms of one-way delay; see its report for the
three-frame buffer and observation limits. This does not prove
carried-loadout exchange, production matchmaking, rollback, replay
integrity or handheld performance. Those remain separate units with
their own acceptance criteria.

## Skill provenance

The project copy is from
[`saschb2b/skills`, commit `31a6a136`](https://github.com/saschb2b/skills/tree/31a6a1364b73ff24f388f9e35c6e3ccd9baa7d89/skills/engineering/reverse-engineering),
installed on 18 October 2026 from the
[author's skill page](https://saschb2b.com/ai/skills/reverse-engineering).
Its MIT license is included beside `SKILL.md`. The installed Codex copy
and project copy match; the project instructions make the workflow
available to collaborators without relying on one machine's settings.
