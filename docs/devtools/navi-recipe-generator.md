---
type: Tool
title: "The Navis' recipe generator"
description: "`tools/xnavirecipe.py` derives the Navis' recipes as `tools/xrecipe.py` derives a family's, writing the control HeatMan's, the moved objects' and BN5's Navis' into `src/emu/xnavi_bn5.c`, with `--check` and a `--report`."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-12T15:22:21Z }
---

# The Navis' recipe generator

```bash
python3 tools/xnavirecipe.py              # write src/emu/xnavi_bn5.c: the control's, the moved objects' and BN5's Navis' recipes
python3 tools/xnavirecipe.py --check      # write nothing: fail where the file differs from what it would write
python3 tools/xnavirecipe.py --report     # every Navi of Team Colonel's and the Dark Soul: .build/xnavirecipe/report.md
```

`tools/xnavirecipe.py` (issue #82; [Transplanted Navis](../multirom/navis.md))
derives a Navi's recipe as `tools/xrecipe.py` derives a family's, with
the families' walk, pairs and audits, from BN6's Navi tables instead of
the families' (`tools/bn5_navi_twins.py`: the Navis' own pairs, audits,
the routines copied with them, their stubs, the tables their code reads).
Two of the families' rules are its own: a number a Navi's code keeps is
kept without `bn5_twins.KEPT`'s audit (the Navis are proven frame by
frame instead: ten such words in their code, none audited yet), and a
sound is BN6's own only where BN6's song of its number plays on the same
player too (else BN5's is copied to its fixed slot).
In install order (`NAVIS`, after the control, BN6's own HeatMan derived
from BN6's ROM): its code walked from its states, hooks and companion
routines; each object type its code spawns decided as
`tools/bn5_twins.py`'s TYPES decides it (one decision a type, every
track's; a type the table decides otherwise, or not at all, is a
decision); its sprites given one number and one place in the shared art
whichever Navi draws them, its effects numbers of its own, both in issue
#82's ranges for the Navis; the image object's kinds it spawns each one
`tools/bn5_objects.py` fixes, and the moved objects' recipe itself
(`xnavi_objects_bn5`: the image object's module, its kinds' sprites by
that table, each twin checked by its bytes). A recipe with a decision
open is not written; `build.py test` holds every written one whole and
apart from the others and the families' (`test_xnavi_static`).
