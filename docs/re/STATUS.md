# Research status: Cyberworld Endless: online Net Squares (epic #120)

Updated 18 Oct 2026. Written by `tools/re_status.py` from [ledger.json](ledger.json); the [dashboard](dashboard.html) shows the same.

| Feature | Status | Now | What is left | Updated |
| --- | --- | --- | --- | --- |
| [Net Square](../ONLINE.md) | Works, being reworked | Presence works: the server, the transports on every system, and a prototype Square where other players walk as NPCs. | Move the way in to a gate in Lan's HP, rebuild the Square from BN6's own pieces, and make the people in it talkable. | 18 Oct 2026 |
| Trade | Designed, not built | Designed: counters at Trade Central in the Square, the server commits a trade, and the swap is applied after the visit returns, with BN6's own give and take chips. | All of it: the counters, the server's commit, the swap in the live game, and a pending file that holds CONTINUE until the trade settles. | 18 Oct 2026 |
| [Arena](../ONLINE_LINK.md) | Proven in research, not built | Two linked cores per client with only inputs exchanged complete BN6's Practice at 0 to 150 ms; a complete snapshot of the pair replays exactly. | The production pair in the game, a loadout installer for BN6's own content, and the server's challenge, accept, input and hash messages. Rollback after that. | 18 Oct 2026 |

## Since the last update (17 Oct 2026)

- 18 Oct 2026: The trade machinery and the save gate it put on every run came out of the game; Trade starts again from its design.
- 18 Oct 2026: The Arena research was trimmed to what the production Arena builds on: the linked pair, the input wire, the Practice script and the pair's snapshot. `build.py online-pair` runs them.
- 18 Oct 2026: The link cable's findings are one page, docs/ONLINE_LINK.md.
- 18 Oct 2026: This status is written by tools/re_status.py from docs/re/ledger.json.

## Experiments

One line each, newest first; each stands on its own.

| Date | Feature | Experiment | Result | What it means |
| --- | --- | --- | --- | --- |
| 18 Oct 2026 | Arena | [Inputs only, two cores per client](../ONLINE_LINK.md) | Completes at 0, 50, 100 and 150 ms one-way delay, every battle timer as the original; a flipped input and a killed peer both stop the fight. | The Arena's design: each client runs both games, only inputs travel. |
| 18 Oct 2026 | Arena | [Snapshot of the pair](../ONLINE_LINK.md) | Restored at six points and replayed 1, 4 and 8 frames, twice each: every replay exact; 863,016 bytes, 1.2 ms a restore on the desktop. | Rollback is possible, but only with more than mGBA's own state save. |
| 18 Oct 2026 | Arena | [A carried chip in the pair](../ONLINE_LINK.md) | Cannon A, given by BN6's own routine, chosen and hitting for 40 in Practice. | A run's chip fights natively; the installer for a whole loadout is still to build. |
| 15 Oct 2026 | Net Square | [Two games through the hosted server](../ONLINE.md) | The same room built from seed 120 on both; 211 peer positions matched; round trips 33 to 49 ms; both MegaMen back unchanged, all 24 checkpoint files unchanged. | Presence over the public internet works, and a visit leaves the save alone. |
| 15 Oct 2026 | Net Square | [Sixteen games, two full channels](../ONLINE.md) | 10,248 peer positions matched; every game's loadout back after a disconnect; 192 checkpoint files unchanged. | A full room holds on one machine; not a measure of handhelds or a busy network. |
| 15 Oct 2026 | Net Square | [Browser and native game together](../ONLINE.md) | Both built the same room; 578 browser and 592 native peer positions matched. | The browser build can share a Square with the native ones. |
| 15 Oct 2026 | Net Square | [Retroid Pocket Nova on the public server](../ONLINE.md) | Same room as a desktop peer; 189 handheld positions matched; round trips 40 to 47 ms; a 1,000 HP checkpoint back unchanged. | The PortMaster build visits as the desktop does; the only physical handheld tried. |
| 15 Oct 2026 | Arena | [ROM-free cable on every desktop and the Nova](../ONLINE_LINK.md) | 256 transfers and all 1,024 received words right on Linux, Windows x64, macOS ARM64 and the Nova. | mGBA's lockstep cable links two cores on every system tried. |
| 15 Oct 2026 | Arena | [Practice on one pair of cores](../ONLINE_LINK.md) | BN6's Practice unpatched: 18,855 serial transfers, 100 to 0, both back at the menu, all 1,433 battle frames agreeing. | BN6's own NetBattle runs on two linked cores without help. |
| 15 Oct 2026 | Arena | [Serial words sent over a socket](../ONLINE_LINK.md) | Completes at no delay; at 50, 100 and 150 ms only 95, 48 and 35 battle frames in a minute. | The cable cannot be split across the network. |
| — | Trade | Spare chips read from the Pack | Six folder scenarios through BN6's own folder editor agreed with the reader; six planted faults caught. | Which copies are spare (the Pack less every folder's copies) can be read exactly; the rule is kept at commit 7b4b8ef. |

<details><summary>Method</summary>

- Target: BN6 Gregar USA, SHA-1 89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6, on mGBA 0.10.5 (docker/mgba.sh pins its source)
- Compared against: The unpatched game on a fresh pair of cores from the same boot-3.state, run again for each experiment
- The player's ROM is only read; saves, states and traces stay under .build/.
- Network work runs against the project's own server and the developer's own games, never anyone else's service or account.
- No masks: a difference between the original and ours is explained or it fails.
- No Tango code is read or used.

| Record | Where |
| --- | --- |
| ROM offsets and how to re-check them | docs/ROM_DATA.md, src/emu/bn6*.h |
| Claims about the games and their sources | docs/SOURCES.md |
| What is original, generated or adapted | docs/FIDELITY.md |
| The link cable and the Arena | docs/ONLINE_LINK.md |
| The Net Square, its server and transports | docs/ONLINE.md, server/README.md |

</details>

## Glossary

- **Net Square**: The online place players visit between acts; other players walk in it as NPCs.
- **Lan's HP**: BN6's own homepage map, where MegaMan arrives after every act; the Square's gate is to stand there.
- **Trade Central**: The Square's counters where two players are to swap chips.
- **Arena**: NetBattles between players, BN6's own fights over the network.
- **Pair**: Two mGBA cores joined by an emulated link cable, both games running on one client.
- **Practice**: BN6's NetBattle choice that costs no chip; the fight every experiment scripts.
- **Delay-based**: Each frame waits until both players' inputs for it have arrived.
- **Rollback**: Run ahead on a guessed input, then rewind to a snapshot and replay when the real one arrives.
- **Snapshot**: Everything needed to resume the pair exactly: both cores, both flash saves, the cable.
- **Oracle**: The unchanged original, run the same way, that ours is compared against.
- **Planted fault**: A deliberate break, to show the comparison catches it.
- **boot-3.state**: The game's own state after boot, before a run changes anything; made on each machine, never committed.
