# Native chip exchange: bounded investigation

Verdict: **INCONCLUSIVE**. The three connection variants budgeted in
[trade-stage-goal.md](trade-stage-goal.md) reached the original Trade chip
chooser but did not complete an exchange. This harness unit is stopped;
its observations do not authorize a checkpoint patch or prove a chip
exchange succeeds.

## Target and method

The same pristine Gregar USA ROM and mGBA 0.10.5 library as the inventory
proof were used. ROM SHA-1 is
`89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6`; library SHA-256 is
`5b02195c77b74ffa6622952706025bc7e5a77bd5d14d9306e0bbc84ab789f0cd`.
The map-side boot state SHA-256 after the runs remains
`4b921c9cf48c22085605714797373114de206a32b8bba821606dc348f73f95b6`.
Both were opened read-only; each core used a private memory-backed flash
file. The exploratory probe SHA-256 is
`659eab15ca351bb8cd79b6986a2b20b77202a085a0016053683e824be2a222d6`.
It is an ignored local copy of `tools/online_link_bn6.c` with fixture and
observation commands; no accepted Practice script was changed.

The input followed PET Comm → Cable → the original save prompts → Trade
→ BattlChp → Begin connection. Both cores received the same menu inputs.
The existing harness anchors each input batch to each core's own current
frame. Full frames, logs, input scripts, RAM and state observations remain
under `.build/trade-stage-research/`, outside git.

## Attempts and first missing state

| Variant | Observation | Result |
| --- | --- | --- |
| Four-frame confirmation edges, controlled spare chips on both sides | Side 0 can choose its chip; side 1 shows the original wait message from its first chooser screen and does not finish selection | INCONCLUSIVE |
| One-frame final connection confirmation and selection edge | Same stalled wait; the shorter press does not release the second peer | INCONCLUSIVE |
| Pristine Pack control, no fixture writes, another 900 frames of waiting | Same stalled wait, with both peers offering None; this does not support blaming the spare-chip fixture | INCONCLUSIVE |

The controlled fixture gave side 0 three spare HiCannon L and side 1
three spare M-Cannon R. Each incoming chip was absent. All fixture writes
were to the private cores' Pack, ownership mark and Library bit; equipped
folders were left intact. No fixture wrote a trade result.

The last clean control observation was frame 2807/2835, main mode `0x28`
on both cores, submenu words `0x08302818` and `0x00302818`. The cable had
completed 12,236 transfers and both send words were zero. Serial activity
therefore continued while the original Trade handler stayed at substate
`0x30`. Reading the matching original wait-handler source is a locator
lead, not a verified diagnosis of the stall. A peer's frame lead alone
does not establish a scheduling defect.

The first required state never reached is **successful original exchange
completion on both peers**. No native before/after inventory result,
successful return to the map, or full-exchange mutation proof exists.
The original Trade completion path's take/give/save calls were located
in bn6f's `data/dat33.s`, but those calls were not observed executing in
this experiment. Their runtime effects remain unverified by this unit.

## Retained local evidence

| File under `.build/trade-stage-research/` | SHA-256 |
| --- | --- |
| `chip-list.txt` | `ce717a61478a0a380d3635ecd72a0b0074e86e4ed8a7d3fbb53c36215c3d39e6` |
| `chip-list.log` | `894d2ce1c52a3151a6d602691397241d61588fa3bbc71ab82c5476794009520c` |
| `short-select.txt` | `b5331a4d42f8d94018a98cc76e12f8a99f9c75320230f580c5768ed18ff34c33` |
| `short-select.log` | `c00abc624a0e70c7c3a15550d52cf549d3d73ab785f6223fe331a30338346255` |
| `plain-control.txt` | `73bebb6e4626e55e828a685067d93e4d606a94a71aa82ae8b5405e35efbc15ea` |
| `plain-control.log` | `cb5fc23bdc2e207043f38ffb575f0d6e70f3c7d6629741f065350c0838068b3f` |

The next proposed unit is the separately reviewed
[original-routine oracle](trade-stage-routine-goal.md). A pass there will
not turn this native peer-exchange result into PASS.
