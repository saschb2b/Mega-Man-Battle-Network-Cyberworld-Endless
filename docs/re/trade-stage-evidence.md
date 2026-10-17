# Isolated pristine checkpoint staging

Verdict: **four routine-level cases PASS; all twelve deliberate faults
rejected**. No ignored bytes. Native peer Trade remains
[INCONCLUSIVE](trade-stage-link-attempts.md). Actual project checkpoint
staging, BN5, All *, profile Library persistence and journal integration
retain separate acceptance requirements.

The [original-routine acceptance](trade-stage-routine-goal.md) and its
comparator were reviewed before implementation. A first run exposed
mGBA's audio normalization on unchanged-source reload; the separate
[control experiment and correction](trade-stage-reload.md) were reviewed
before the final candidate run. Candidate files still preserve every
source byte outside the six individually observed inventory fields.
Fresh reloads compare every protected byte with two pinned independent
unchanged-source reloads, and all inventory with the original routines.
No audio mask or copied candidate expectation was introduced.

## Reproduction and pins

```sh
python3 tools/online_trade_stage_candidate.py \
  --state .build/play/issue122-before/data/boot-3.state \
  --output .build/trade-stage-reproduce
```

The output must be a new directory. The checked report is
`.build/trade-stage-candidate-full/candidate.json`; raw source, staged,
reloaded and original states remain local. ROM, source state and actual
mGBA library hashes are recorded before and after and remain unchanged.
Fixed reviewed observer/comparator source pins live in
`tools/online_trade_stage_proof.py`; candidate runs verify those constants.

| Artifact | SHA-256 |
| --- | --- |
| Reviewed comparator | `a5333ea68048981275d9e5c61c64388886488f7ff33d196a33bcda7e6e314ad4` |
| Original routine observer binary | `99bddd66933b97d5cbffb6296e08352931bbc0c77c6f48c7b59120a7e5d54402` |
| Independent reload observer binary | `7bf8c483defbdaa78a1fd267acd53bc6fb6ba9d7cb2ad62ca73e8021495b0717` |
| Candidate observer binary | `cfae765691812806b8d3b52c44a0b7805640dcf1347365af5e0bd4f1fe1f8b68` |
| Candidate implementation | `4ce3757aa2fa431dab88cffc2c45b1d31387ce9edca69edb8c7daa6294b7eac2` |
| Candidate observer source | `6334f05f87f3caa36e104931cd34e489e3d28e0426f05cbf363782e7047ee146` |

| Case | Candidate bytes changed | Result |
| --- | --- | --- |
| First native code slot, incoming absent | 7 | PASS |
| Second native code slot, incoming absent | 7 | PASS |
| Last spare with shared order counter at zero | 8 | PASS |
| Incoming count 98, increment to 99 | 5 | PASS |

Every case matches the original full Pack, ownership table, Library flags
and shared ordering counter; all other serialized bytes, including CPU,
folders, HP, board/grid, powers, position, RNG and SRAM/RTC extdata, remain
unchanged in the returned buffer. A fresh private core reaches the
original folder editor; all 512×27 exact-code spare counts match both
original controls. The separate live-core sentinel and source buffer
remain byte-identical.

The proof compiles a changed implementation for each fault: wrong outgoing
slot, wrong incoming slot, remove two, receive without giving, omitted
ownership, omitted Library flag, omitted incoming order stamp, omitted
shared order counter, changed equipped entry, changed protected HP, and
stale returned buffer. The final fault changes the private live sentinel
in the observer only. All twelve reach complete observations and fail
comparison; missing files or an unreached editor cannot count as a
caught fault. Each mutant's actual source and binary hashes are retained.

## Runtime boundary and refusal checks

`online_trade_stage` takes immutable checkpoint bytes, copied prepared
inventory, validated terms and participant side. It returns a separately
allocated buffer, or refusal with zero output length. It never receives
a live core or filesystem handle. The stage is not a confirmation or an
install; the journal must separately certify the full source directory,
run ID, revision, terms and persistence.

The first envelope is deliberately narrow: pristine Gregar, mGBA 0.10.5
raw state, exactly the observed SRAM/RTC extdata layout. Named
`GBASerializedState` members locate RAM. Compile-time assertions reject
an incompatible core layout. Prepared folders, Pack and protected
loadout fields must match the source; eligibility is checked again.
Patched project states, BN5, Souls and All * refuse in this unit.

ROM-free `tests/test_online_trade_stage.c` passes WERROR with ASan/UBSan.
It checks owned output, both participant indices, immutable inputs, stale
Pack/folder/HP/NaviCust/powers, equipped-only quantity, unavailable or
ambiguous code, missing recipient capability, canonical-ID mismatch,
full recipient, identical pair, rule mismatch, unsupported scope,
malformed pointers/folder state/envelope and invalid arguments. These
synthetic checks demonstrate failure closure, not original game behavior.
The eleven comparator tests also cover missing/pinned/nonrepeating reload
controls and audio/protected-byte corruption. Both join normal CI tests;
the isolated stage target also participates in reachability lint.
