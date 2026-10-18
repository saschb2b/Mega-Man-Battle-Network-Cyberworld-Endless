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
| ROM offsets, their names and how to re-check them | [ROM_DATA.md](../ROM_DATA.md), `RomLayout`, `src/emu/bn6*.h` |
| Claims about the games: verified, assumed, refuted, and their sources | [SOURCES.md](../SOURCES.md) |
| What is original, generated or adapted | [FIDELITY.md](../FIDELITY.md) |
| Free space and write claims | [EMULATION.md](../EMULATION.md), `src/emu/free_space.h`, `claims.h` |
| The virus differential | `tests/xdiff/`, `tools/xdiff.py`, [DEVTOOLS.md](../DEVTOOLS.md) |
| The link cable and the Arena | [ONLINE_LINK.md](../ONLINE_LINK.md) |
| The Net Square, its server and transports | [ONLINE.md](../ONLINE.md) |

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
