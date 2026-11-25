---
type: Tool
title: "The Navis' recipe generator"
description: "`tools/xnavirecipe.py` derives the Navis' recipes as `tools/xrecipe.py` derives a family's, writing the control HeatMan's, the moved objects' and BN5's Navis' into `src/emu/xnavi_bn5.c`, with `--check` and a `--report`."
tags: [devtools]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-23T15:45:00Z }
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
decision, and so is a type it moves whose copy TYPES gives another track:
the Navis install first, so a type their code moves is theirs, which the
chips and the families then spawn); its sprites given one number and one
place in the shared art whichever Navi draws them, its effects numbers of its own, both in issue #82's
ranges for the Navis; a moved object's every compare of its parent's
CurAction with one of the Navi's own states made BN6's number; a block
whose code stores a byte of its own routines' addresses placed at its BN5
address's low byte (`LOW_BYTE_KEPT`: CloudMan's clouds, [Transplanted
Navis](../multirom/navis.md), CloudMan); an effect of a Navi's that draws
his own battle sprite given that sprite at its place (`NAVI_SPRITE_PLACES`:
GridMan's afterimage); an afterimage's spawn's word (the fourth kind's 0x28's
spawner's r4, loaded into r4 or ORed in from a literal) set to the
sprite's BN6 place (`afterimage_sprites`: LarkMan's), the afterimage's own
load the spawner's word, or its two low bytes made by two `movs`
(`AFTERIMAGE_MOVS`: DarkProto's); the chip its attack writes BN6's of its
name, by a `movs` or a pool word (DarkProto's AreaGrab); Nebula's two
(`NEBULA`) installed after Team ProtoMan's, in a room of their own, and
DarkProto after them and after BN5's chips' code (`AFTER_CHIPS`: the
types his code spawns whose copy `TYPES` gives the chips, his shield and
blade, or a Navi before him, LarkMan's afterimage, are that copy's,
`XV_TYPE_SHARED`, none copied again, so the chips' layout and numbers stay;
the recipe's `.after_chips`, installed by `xnavi_install_after_chips`;
every version of his named by his V2's entry, `SHOWN_AS`);
a sprite past the fourth room's end in the fifth, Team ProtoMan's sprites'
room's tail (`XN_SHARED_FIVE`); the image object's kinds it spawns each one
`tools/bn5_objects.py` fixes, and the moved objects' recipe itself
(`xnavi_objects_bn5`: the image object's module, its kinds' sprites by
that table, each twin checked by its bytes). A recipe with a decision
open is not written; `build.py test` holds every written one whole and
apart from the others and the families' (`test_xnavi_static`).
