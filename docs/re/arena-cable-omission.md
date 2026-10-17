# Cable omission: bounded negative termination

Proposed negative-only acceptance extension. Normal controls, all 36 complete
replays and the other four complete mutation traces keep their existing rules.

The complete restore now passes the fixed comparator at all six exercised
points. Deliberately omitting the external cable restore leaves both cable
events unscheduled after mGBA clears its timing queues. The original nonidle
boundary's core, flash, input and executor state still match immediately; only
cable state differs. The native program stops making linked progress. Four
complete pair frames are observed, then the executor's existing four-frame
input-lookahead guard refuses to step a core. This is a meaningful failure of
the mutation, but it cannot satisfy the old negative fixture's eight-frame
continuation requirement. The current verdict remains INCONCLUSIVE for this
mutation, not a pass inferred from a crash.

Accept this specific stopped mutation only when the independent comparator:

1. Rechecks the pristine old logical control, both micro controls against the
   controlled original full trace, snapshot noop and all 36 normal replays.
2. Verifies the unchanged ROM/state/library manifest and reviewed observer,
   comparator, snapshot and subject source pins. The wrapper observes the
   executed subject's binary hash, return code and elapsed time externally.
3. Requires the original active, nonidle, scheduled instruction boundary.
   The mutated immediate cursor and every non-cable component must equal it;
   cable state must differ and both previously scheduled cable events must
   be absent. This directly identifies the omitted component.
4. Requires at least one and fewer than eight contiguous completed frames,
   beginning at the original boundary plus one. Every record is validated,
   with original input lookahead and original script/start-frame bookkeeping.
5. Reads a separate full numeric stop observation at the attempted next pair
   frame. Its executor cursor must identify the next side before stepping,
   with the original frame targets, script position and input lookahead.
   Its observed core frame counter must independently establish exactly the
   four-frame lead that the unchanged executor rejects. The last completed
   prefix frame must equal the stop's unfinished pair frame.
6. Requires the subject to return the dedicated bounded-stop code 3 within
   the wrapper's 180-second process limit. Generic nonzero exits, signals,
   timeouts, absent or truncated logs, zero completed steps, missing actual
   frame lead, wrong pins and altered inputs cannot pass.

The subject observes the already-existing bound after execution returns;
it does not change that bound or the pinned original executor. No missing
record is fabricated, repeated or compared under a mask. If these conditions
are not all established, report the stopped mutation as INCONCLUSIVE. This
exception is confined to omission of the entire cable at this boundary.
