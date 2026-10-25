---
type: Design
title: "Measuring it"
description: "How progression is measured: `build.py pacing` rolls every area's battles per act and 500 runs' guardians against their bands, `tests/test_core.c` checks the rules, `runlog.txt` records battles, and `build.py economy` measures handouts."
tags: [game, progression]
generated: { by: claude-code/unrecorded, at: 2026-10-20T22:52:50Z }
---

# Measuring it

- `python3 build.py pacing` rolls every area's battles in every act and
  draws 500 runs' guardians, and marks anything past its band
  ([Dev tools](../../devtools/overview.md)). It reports 0 now. Median battle HP by act: about
  140, 200, 270, 340, 400 and 500, and 510 in the Nest. The super bosses'
  forms are listed after the guardians, unflagged: their battles are
  BN6's own.
- `tests/test_core.c` checks the acts, bands, versions, area tiers, guardian
  choice and heals without a ROM.
- `runlog.txt` in the data folder records every battle (area, foes, their
  HP, MegaMan's HP before and after, the zenny on hand, a win's busting
  level and reward) and where each run ended.
- `python3 build.py economy` measures what a run hands out, act by act:
  chips, good chips, codes, zenny against what there is to buy, Mystery
  Data ([Economy](../economy/overview.md), [Dev tools](../../devtools/overview.md)), the flat plan (before issue
  #154) beside the opening one in play, each against its bands on the
  endless net and the short one. It reports 0 of the plan in play's
  outside them; a change to drops, prices or Mystery Data runs it as a
  change to battles runs the pacing report.
