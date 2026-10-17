# Proposed cable snapshot observation boundary

Unit 2's first observer control matched both independent full traces
(2,823 records each) and all 1,433 active timers of a fresh original.
All 2,823 completed pair-frame boundaries had `TRANSFER_IDLE`. The
guarded comparator returned **INCONCLUSIVE: nonidle-cable**. Native
flash changes were observed at frames 296, 297 and 299, among others.
Evidence is local in `.build/online-replay/observer-control/`.

This proposes an observer/comparator extension, not an allowed state
difference. The whole-frame input-delay worker and its accepted comparison
stay unchanged. No restore implementation may be accepted under this
extension until it is separately reviewed and pinned.

## Boundary and control

Add an instruction scheduler for the replay experiment only. One scheduler
round gives core 0 its existing opportunity to step (unless waiting), then
core 1 its opportunity. It uses the same per-core frame-indexed inputs,
entry-step targets and cable callbacks as the existing pair. Stop/observe
after both opportunities, **before checking whether the pair-frame target
was reached**. No observer changes target state.

The exact executor cursor is explicit: stage `check-targets`, completed
pair-frame number, scheduler-round count within that frame, both frame
targets, relative script frame and battle/entry mode. The stage implies
that input setup already ran and that resumption first checks the two
targets; if incomplete, its next instruction opportunity is core 0.
Snapshot restoration must restore this cursor as well as core/cable data.
It must not repeat initial input setup or skip a pending target check.

Before using it for restore, require the new executor's uninterrupted
whole-frame trace to equal the already controlled original pair's full
trace exactly. Also repeat the new executor twice. Both runs must select
the same first active, non-idle, scheduled cable transfer at this boundary
and agree in its immediate full state and exact cursor. No state mask or
alignment shift is allowed. Failure here is an observer defect, not a
snapshot result.

## Evidence and comparator change

Whole-frame trace records retain their existing schema. A separate
`original-cable.json` records the exact cursor and the existing complete
state record at the selected instruction boundary. Its frame number names
the last completed pair frame, not a claim that the partially executed
frame is complete. The comparator validates active battle samples,
non-idle phase, a scheduled cable event, nonzero bounded round count,
targets consistent with the original frame clock and identical input
lookahead. Missing or idle evidence remains INCONCLUSIVE.

For cable replay horizons 1, 4 and 8, repeated twice, compare the immediate
post-restore cursor/state against `original-cable.json`. Then resume that
exact boundary and compare the next 1, 4 or 8 completed pair frames with
the uninterrupted original trace. This retains the full-frame horizon;
an instruction count is not substituted for a video frame. The five
other snapshot points keep their existing whole-frame comparisons.

The cable/credit/receive/event-delay mutations must change their named
component immediately after restore at this non-idle boundary. Their
subsequent traces still contain the full replay horizon and unchanged
controller script. The omitted-flash mutation remains at the independently
observed native save-memory transition. Comparator fixtures must reject
wrong cursor stage, round, targets, missing immediate record, idle or
unscheduled cable evidence, repeated/missing full frames and a subject
that resumes the frame from its beginning.

The observer and comparator extension land in separately reviewed commits.
The original whole-frame control's hashes, the extended observer's hashes,
both selected cursor records and their equality are recorded before any
restore implementation is written.
