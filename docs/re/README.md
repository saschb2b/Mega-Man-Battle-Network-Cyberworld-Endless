---
type: Process
title: "Reverse-engineering work"
description: "How the reverse-engineering work is tracked: `ledger.json` written out by `tools/re_status.py` as STATUS.md and the dashboard, where ROM facts and claims live instead, the BN6 Gregar target's SHA-1, and the skill's provenance."
tags: [re]
generated: { by: claude-code/unrecorded, at: 2026-10-18T12:08:07Z }
---
# Reverse-engineering work

New investigations follow the project's
[reverse-engineering skill](../../.claude/skills/reverse-engineering/SKILL.md).
Where they stand is in [STATUS.md](STATUS.md), and the same as a page in
the [dashboard](dashboard.html): one row per feature, the decisions that
wait on someone, what changed since the last update, and one line per
experiment with what it means.

## Updating the status

[ledger.json](ledger.json) is the one source: edit it, then run

```sh
python3 tools/re_status.py
```

which checks it and writes [state.json](state.json) (what the dashboard
reads), STATUS.md, and the dashboard itself: the skill's page
(`.claude/skills/reverse-engineering/assets/dashboard.html`) with the
state embedded, so it shows when opened as a file too. `build.py test`
fails while any of them is out of step with the ledger. Dates are written
`YYYY-MM-DD`, or `null` where unknown; the page shows those as a dash.

## Where the facts live

| Record | Location |
| --- | --- |
| ROM offsets, their names and how to re-check them | [ROM data](../rom/overview.md), `RomLayout`, `src/emu/bn6*.h` |
| Claims about the games: verified, assumed, refuted, and their sources | [Sources](../research/sources.md) |
| What is original, generated or adapted | [Fidelity](../game/fidelity.md) |
| Free space and write claims | [Running the game's own logic](../engine/emulation/overview.md), `src/emu/free_space.h`, `claims.h` |
| The virus differential | `tests/xdiff/`, `tools/xdiff.py`, [Dev tools](../devtools/overview.md) |
| The link cable and the Arena | [The link cable and the Arena](../online/link.md) |
| The Net Square, its server and transports | [Online: the Net Square](../online/overview.md) |

docs/re/ holds the status, not a second address map: an offset or a claim
goes in the records above, and the ledger's lines link to them.

## The target

BN6 Gregar USA, SHA-1 `89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6`, on
mGBA 0.10.5 (`docker/mgba.sh` pins its source archive's SHA-256). The
player's ROM is only read; saves, states, traces and dumps stay under
`.build/`, and only whole frames of the game running become public
pictures. Network work runs against the project's own server and the
developer's own games. No Tango code is read or used.

## Skill provenance

The project copy is from
[`saschb2b/skills`, commit `31a6a136`](https://github.com/saschb2b/skills/tree/31a6a1364b73ff24f388f9e35c6e3ccd9baa7d89/skills/engineering/reverse-engineering),
installed on 18 October 2026 from the
[author's skill page](https://saschb2b.com/ai/skills/reverse-engineering),
with its MIT license beside `SKILL.md`. Since then the project's copy has
its own status page and schema (`assets/dashboard.html`,
`references/dashboard.md`) and words that allow network work against the
project's own servers.
