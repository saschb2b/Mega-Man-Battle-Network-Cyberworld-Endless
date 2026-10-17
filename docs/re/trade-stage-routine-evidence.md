# Original chip routine controls

Verdict: **four original-routine controls PASS**, each repeated from two
fresh private cores. A staged checkpoint candidate is not implemented by
this commit. The [complete native peer exchange](trade-stage-link-attempts.md)
remains INCONCLUSIVE.

The [reviewed acceptance](trade-stage-routine-goal.md) and
`tools/online_trade_stage_compare.py` were committed before the observer.
The comparator SHA-256 is
`662e79911140f1f5b07ca1be77f522d2f9b11e448eff90c793c9a31c11defdd7`.
The actual observer binary SHA-256 is
`99bddd66933b97d5cbffb6296e08352931bbc0c77c6f48c7b59120a7e5d54402`.
Target ROM, boot state and library pins are the same as the reviewed goal.
`tools/online_trade_stage_proof.py` keeps fixed source hashes for the
observer, shared editor helper, comparator and inventory readers; a
candidate run must not refresh these pins.

Reproduce into a new ignored directory:

```sh
python3 tools/online_trade_stage_proof.py \
  --state .build/play/issue122-before/data/boot-3.state \
  --output .build/trade-routine-controls-new
```

The checked run is `.build/trade-routine-controls-final/controls.json`.
All raw state, RAM, call records and counts stay local. The probe opens
the ROM and boot state read-only and uses memory-backed flash. It first
reaches the original map update, installs a recorded fixture and requires
the accepted immutable inventory reader to approve the exact spare swap.

| Fixture | Original count result | Original shared order counter |
| --- | --- | --- |
| First native code slot | outgoing 3→2, incoming 0→1 | 32767→32766 |
| Second native code slot | outgoing 3→2, incoming 0→1 | 32767→32766 |
| Last spare, counter zero | outgoing 1→0, incoming 0→1 | 0→65535 |
| Recipient near capacity | outgoing 3→2, incoming 98→99 | 1→0 |

Every case calls the original removal with zero first as a no-op inventory
control, restores the private source, then calls removal/acquisition with
one. The observer records actual entry arguments, instruction count,
minimum stack pointer and every register before/after, and stops at the
verified original return PC. It saves complete serialized states after
each step. Two runs agree byte for byte in all four states and all call
observations, including the CPU/stack execution effects.

The original acquisition changes the exact incoming code count, its
16-bit order stamp and the shared 16-bit next-stamp counter. The new-chip
fixtures also change the ownership mark and Library flag. No other EWRAM
effect was observed. IWRAM changes lie within the observed native stack
extent; flash/extdata remains identical. These native execution effects
are visible in the local report's complete changed-byte list and do not
become allowed candidate differences.

After saving the original result, the helper restores only the interrupted
map caller's registers to resume the original menu. It does not restore
or synthesize inventory. The original editor is reached in all eight
runs; all 512×27 reported exact-code spare counts repeat. Candidate
acceptance will compare its fresh reload and editor against these results,
and must still pass all refusal and deliberate-fault checks.
