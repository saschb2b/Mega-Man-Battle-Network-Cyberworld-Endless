# Prove the native-chip registry path before project staging

Status: independently reviewed acceptance, before implementation. This is a new
oracle qualification unit. It does not relax the pristine stage proof,
accept a project candidate, or enable trading. The full native cable Trade
remains INCONCLUSIVE.

## Reached precondition

The [project checkpoint goal](trade-project-stage-goal.md) required every
executed Take/Give instruction and required code record to remain pristine.
The first normal NEW GAME/CONTINUE research run reached both calls, but
that condition does not hold even with only the Gregar ROM installed:

| Observed path | Existing implementation | Consequence |
| --- | --- | --- |
| `BN6_PACK_OFFSET_TAIL` branches to `CHIPS_CODE` | `xchips_install.c`, native IDs below `BN6_PACK_IDS` use the original Pack; higher IDs use its extension | The transitive instruction stream differs from pristine BN6 |
| `BN6_CHIP_TABLE_REF` names `CHIPS_RECORDS` | The registry moves the chip records before boot, including ordinary native records | A code read uses a relocated address; selected record contents need independent comparison |
| Give reaches the hook at `BN6_SET_EVENT_FLAG` | `events.c` observes choice/job flags; `hook.c` replays the replaced instruction on `HOOK_CONTINUE` | Stored ROM words alone cannot establish the actual instruction executed by the hook |

The local exploratory trace returned from Take in 49 steps and Give in
104 steps to `BN6_OW_HOOK`. It observed the quantity branch and relocated
code reads. It also observed the flag-hook site twice, because the hook
replays an instruction. This is a reached failure of the earlier pristine
code precondition, not a claim that the registry is incorrect. The complete
trace, ROM images and states remain under ignored
`.build/trade-project-research/`; none is a distributed asset.

Two fresh normal CONTINUE controls from the first exploratory checkpoint
matched in every serialized byte and in the entire 16 MiB effective ROM at
both the return from `scene_emu_start` and free-map frame 120. The latter
boundary changed 4,140 bytes relative to the saved file through normal
reconstruction and execution; Pack, extension, ownership, Library, order
counter and equipped folder remained byte-identical. These observations
are characterization only. Every relevant normalization still needs its
responsible code and a fixed control; there is no 4,140-byte mask.

That same normal NEW GAME replay exposed a separate lifecycle defect:
abandoning the previous checkpoint discarded the new-run intent. The
save lifecycle now distinguishes abandonment from ending a run. Repeating
the UI sequence with fresh seeds 12001 and 12002 produced a `run.trade`
receipt in each first complete checkpoint. Only these newly bound sources
are eligible for the next unit; the earlier unbound fixture is not.

## Original oracle and copied inputs

Use two independent pristine Gregar cores from the already pinned boot
target, plus two independent normal reconstructions of each bound project
checkpoint. Keep the original ROM, mGBA library, every input file, observer
source and actual observer binary pinned. Record the full source receipt,
effective ROM digest and configuration. The project state CRC must still
come from its actual padded load context.

At a controlled map-call boundary, copy only the complete declared
inventory inputs from the immutable project source into a pristine
research fixture: native Pack, ownership marks and keys, Library flags,
shared acquisition counter and equipped folders. Validate the toolkit
pointers and lengths against the original layout first; refuse different
layouts. Publish every copied range and its source hash. The project core
reaches the same inventory through normal CONTINUE, with no inventory
patch. All declared input regions must match before either call.

This fixture copy supplies inputs only. Expected results come exclusively
from executing the original pristine Take/Give routines. Never derive
expected quantities, stamps, marks or Library flags from candidate code,
the project's replacement quantity routine, or a Python subtraction.

Use reciprocal ordinary native entries with distinct code slots. Include
one last spare, an incoming chip absent from Pack and Library, and a native
acquisition fixture at quantity 98. Include the shared counter boundary
already accepted in the pristine unit as a separately labeled copied-input
routine experiment. BN5, All *, Souls and IDs at or beyond the native Pack
bound remain outside this qualification.

## Observe the actual executed path

Log each instruction address, execution mode and fetched/prefetched word,
alongside the stored ROM word. Log every ROM data read used by the calls,
its width and value. A hook's BKPT and replay are separate observations.
Record register/stack/cycle effects, full memory before/after each call,
the bounded return path, and the complete hook event queue before/after.
Do not call a matching stored word proof that the replayed word matched.

The comparison may recognize only these individually established cases:

- The original Take/Give bodies and every other original instruction/data
  read are byte-identical to pristine at their original addresses.
- Each selected native code record at `CHIPS_RECORDS` equals its complete
  original record, with its canonical ID and each of four code slots
  checked. Relocation is allowed only for these explicitly matched reads.
- The exact installed quantity-tail branch, complete replacement routine
  and required literals match the reviewed installer bytes. Its returned
  address and complete inventory effects for native IDs match the original
  routine's observed result. This is behavior checked against the pristine
  oracle, not a declaration that any relocated code is acceptable.
- At the named flag hook, the fetched replacement is the original
  instruction, the handler leaves registers and inventory unchanged, and
  these chip Library flags enqueue no choice/job event. Any other hook or
  redirected instruction is unreviewed.

Compare complete inventory regions after Take and after Give. Enumerate
all noninventory memory effects separately, including CPU/stack and hook
bookkeeping. A difference in a live call does not authorize any additional
field in a staged file. The candidate file boundary remains exactly its
immutable source outside the individually original-observed inventory
fields; all other normal-reload comparisons retain complete fixed controls.

## Comparator first and failure closure

Commit and independently review this unit's comparator and immutable
context contract before extending `online_trade_stage`. Freeze the
original and unchanged-load control pins before candidate runs. Missing
return, missing/truncated trace, differing original controls, an unknown
instruction/data read, or an unclassifiable loader difference remains
INCONCLUSIVE and cannot count as a detected mutation.

Reached mutations must reject: wrong relocated record/code slot, wrong
quantity-tail destination, wrong native/extended boundary literal, a changed
original instruction, a flag hook replaying a different instruction or
queuing an event, and changes to an unlisted effective-ROM/configuration
component. Preserve the earlier candidate's twelve faults. Each fault
must execute the relevant path and leave complete observations before its
rejection can count.

Only a passed, reviewed qualification may supply the original-derived
inventory oracle for the existing project checkpoint acceptance. Normal
resume, original editor counts, profile Library merge, full directory
nonmutation and the isolated durable handoff remain required there.
