---
type: Feature
title: "Two games at once"
description: "Running two games in one Net Square for a test: a local server (`build.py server-run`), a `network.ini` or `--dev online=URL`, two `tools/play.py` sessions, and `tools/online_browser.html` for a browser beside a native game."
tags: [online]
generated: { by: claude-code/unrecorded, at: 2026-10-20T18:18:06Z }
---

# Two games at once

[The Net Square: two games at once](../devtools/net-square.md) says how:
a local server (`build.py server-run`), a `network.ini` per data folder or
`--dev online=URL,onlinename=NAME`, and two `tools/play.py` sessions
from `--scene home`; never the official server. A visit plays at the
clock's pace even headless, so the two meet. `tools/online_browser.html`
starts the browser build from home onto the gate, for a browser beside a
native game (the server's `origins` allowing the page's), with BN5's ROM
too where chosen (the full pass: a NetBattle, `--dev arenaboth` making a
game take a challenge and play it by itself). The Arena's own
proof is in [The two-client proof](arena/from-square/two-client-proof.md), the
cable pair's tool (`build.py online-pair`) in
[The link cable and the Arena](link.md).
