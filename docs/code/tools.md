---
type: Code Map
title: "Tools and skills"
description: "The research and test tools (romlab, uinput_keys.py, play.py, the cyberworld MCP server and cw.py, the voice catalog and dashboard) and the repository's skills (playtest-loop, game-design, trailer, course-check, agent-browser)."
tags: [code]
generated: { by: claude-code/claude-opus-5-5, at: 2026-10-26T17:30:00Z }
---

# Tools and skills

| Path | Contents |
| --- | --- |
| `tools/romlab/` | libmgba research harness (dev only, needs your ROM) |
| `tools/uinput_keys.py` | On-device input injection for testing |
| `tools/play.py` | Playtests: the Linux build headless (`--remote`), played a batch of input at a time, a picture and the state in words after each |
| `tools/battle_floor.py` | The battles per layer and act, played: seeds 1-12 headless side by side (the autopilot, `--dev god,onehit`), each layer's random battles from the run log against [the battle floor](../game/progression/structure.md)'s bands; `build.py pacing` runs it after its rolls ([Pacing report](../devtools/pacing-report.md)) |
| `tools/cw.py`, `tools/cw_mcp.py`, `.mcp.json` | The agents' tools ([Agents' tools: build, play, capture and compare in a call](../devtools/agents-tools.md)): the `cyberworld` MCP server's `shot` (a fresh game from a route or options, input, its state and picture; `ref` another commit's build beside it), `play` (`play.py`'s games, what changed and a picture a call) and `build` (a build or a check, its complaints alone); the routes to the screens in `cw_routes.py`, the pictures cut, sheeted and compared in `cw_pictures.py`, the benchmark `cw_bench.py`; the same from a shell as `tools/cw.py`. Stateless over `.build/play` and `.build/cw` |
| `tools/voice_catalog.py`, `tools/voice_dashboard/` | The voice review (issue #135, docs/voice/README.md): every line of the words files read as C into `docs/voice/catalog.json` (ids that survive rewording, boxes, speakers, placeholders, the chat box's pages, comments, [Voice](../voice/rules.md)'s flags, our numbers beside BN6's; `--check` for `build.py lint`), and the review page published as an artifact with the catalog beside it, which keeps the owner's verdicts and directions in its db |
| `docs/voice/` | The catalog (`catalog.json`, written by the tool, never by hand) and how the review runs: publishing the dashboard, what it stores, a voice pass (`README.md`) |
| `.claude/skills/playtest-loop/` | The playtest loop as a skill: a persona plays through `tools/play.py`, its reports are triaged and fixed, and the loop's log (`lessons.md`) grows with every session |
| `.claude/skills/game-design/` | The game-design skill (from saschb2b/skills): the lens, pattern catalog and frameworks to reason from before a design decision |
| `.claude/skills/trailer/` | The trailer skill: a release's trailer from the brief to the site, its music's habits read from the ROM (`scripts/song_habits.py`), checked without ears (`scripts/music_check.py`), its frames picked and reviewed on contact sheets (`scripts/sheets.py`), and the log of the trailers made (`lessons.md`) |
| `.claude/skills/course-check/` | The course check as a skill: the whole game's course reviewed (the players' statistics read first with `scripts/umami_summary.js`, a time-boxed team of six whose briefs are in `roles.md`, a devil's advocate, the owner's calls), written into [The core loop, reviewed](../game/core-loop.md), and the log of the checks run (`lessons.md`) |
| `.claude/skills/agent-browser/` | The agent-browser skill: a real Chrome driven from the shell (`npm i -g agent-browser`), to look over the site at desktop and phone widths |
