# Arena unit 1: delayed frame inputs

Status: acceptance draft, implementation not accepted. The production Arena
stays disabled. This unit uses the reverse-engineering skill installed for
this epic; the original running game is the oracle.

For the **starter-loadout Practice fight**, our **frame-input replication
experiment** must match **the pristine ROM's existing local cable harness**
on **both MegaMen's HP and panel positions at every active battle timer,
the complete timer range, results and return menu**, for **the pinned entry
and charge-shot script at 0, 50, 100 and 150 ms of added one-way controller
delivery delay**, with **no allowed differences in those observations**.
In addition, the two independently executing client replicas must have
identical complete raw core-state and in-memory savedata hashes at every
simulation frame. The declared hash is `fnv1a64-core-savedata-cable-v1`,
a 16-digit lowercase hexadecimal FNV-1a digest of both zero-initialized
raw core-state buffers, both complete in-memory flash contents, and the
numeric cable/scheduler state. It is a repeatability checksum, not a
cryptographic authenticity check; collision resistance is not established.
This is a bounded interoperability experiment on the
user's own ROM, with our own local relay and no third-party service.

## Target and oracle pins

- Cartridge: BN6 Cybeast Gregar, USA; SHA-1
  `89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6`.
- Core: mGBA 0.10.5, Release/NDEBUG; upstream source archive SHA-256
  `91d6fbd32abcbdf030d58d3f562de25ebbc9d56040d513ff8e5c19bee9dacf14`
  (`docker/mgba.sh`). Record the actual linked library and executables'
  SHA-256 in every report; all arms use the same library.
- Input: the pre-director `boot-3.state`, opened read-only. Record its
  SHA-256 before and after; reject a run checkpoint. ROMs, states, flash,
  dumps and extracted assets never enter git or the relay.
- Live oracle: `tools/online_link_bn6.c` SHA-256
  `3cf188845d4d7885d8aa1dbb6abf0ec6fc53f9e59cf3bef31b43431f3c52eceb`,
  run with `--practice` on two plain ROM cores.
- Shared controller script: `tools/online_link_script.c` SHA-256
  `bbca6b2079acc64f1fdd5b1f78fbca328f222b9df0fe9e358529d638c359eaf8`.
  Its entry has 958 frames and its battle script 1,864; the complete
  subject stream has 2,822 frame inputs per side (tool-counted).

The oracle runs twice from the same state before a subject is accepted.
It must agree with itself. The subject runs an independent local cable
pair in each worker; the network carries controller words, frame numbers,
and numeric verification reports. The parent samples input at the GBA's
video cadence, queues peer delivery, and records actual delivery delay.
The subject waits for both inputs, so input latency is paid once per
frame stream rather than once per serial transaction. No prediction or
rollback is included.

## Acceptance and mutations

| Spec | Required result |
| --- | --- |
| original-control | Both original runs cover every active timer, agree in HP/positions, finish with result 1/2, and return to the same menu |
| frame-0, frame-50, frame-100, frame-150 | Both replicas complete; their full hashes match at every simulation frame; every active-timer observation and final result/menu matches the live oracle |
| input-corruption | Flip a delivered controller word during active play; replica hash comparison must abort at the first different frame, never report completion |
| peer-loss | Close a worker's socket during active play; the session must abort within its bounded deadline and stop the other worker |
| source-preservation | ROM and boot-state hashes stay unchanged after every normal and fault case; all flash files are memory-only |

The independent comparator's target-free tests must reject an altered HP,
panel position, result, final menu, missing active timer, truncated replica
trace, differing raw-state hash, a missing delay measurement, and an
uncompleted battle. The unaltered fixture is its negative control.
Wire tests separately reject stale, duplicate, invalid and over-capacity
inputs, malformed magic/type, EOF and a stalled partial message.

Alignment uses each game's `BN6_BATTLE_TIMER`, with the last observed
active sample at each timer (the existing original harness can sample a
timer more than once). Active samples are the existing `0x0c04` phase
predicate in `online_link_script.c`. Host time is only a transport
measurement. Full replica hashes compare corresponding emulated cores
and frame numbers; side-local core 0 is never compared to core 1.
The last-observation rule can miss a transient mismatch in an earlier
observation of the same timer. This proves the sampled timer sequence,
not every intra-timer state or every original video frame.
No masks are authorized. Missing setup or nondeterministic original
traces are INCONCLUSIVE, not PASS.

The comparator, this specification and its mutations land in a separate
reviewed commit before the worker implementation; their content hashes
are recorded in run manifests. Changing them requires separate review.
The first implementation has an attempt budget of eight non-improving
oracle runs; record the first divergence and stop this unit if exhausted.

## Not reached by this unit

Carried run loadouts, selected chips, forms, Souls, custom ROM patches,
round rematches, RNG-heavy attacks, jitter/reordering/loss recovery,
Internet transport, prediction, rollback, physical-device timing and
production Arena integration are not reached. The dual-core-per-client
cost needs separate device measurements; this experiment cannot qualify
the New 3DS. Full replica equality establishes repeatability of this
specific execution, not correctness of unobserved game memory against
the original harness, whose existing trace exposes only the listed
logical observations. The next unit must broaden the original oracle's
memory trace and battle scenarios before any production decision.

Existing BN6 addresses remain documented once in `docs/ROM_DATA.md` and
named in `src/emu/bn6.h`; this unit introduces no ROM location. Original
Practice/menu behavior is credited in `docs/SOURCES.md`. No Tango
implementation source is used.
