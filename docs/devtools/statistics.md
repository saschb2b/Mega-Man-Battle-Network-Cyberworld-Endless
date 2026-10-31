---
type: Tool
title: "Statistics: asked, sent and checked"
description: "Only a plain player's start asks about and sends the anonymous statistics; `--dev statistics=ask|on|off` and `statsurl=URL` test the question, Options' line and a local Umami stand-in, and `statscheck` sends one event to Umami itself."
tags: [devtools]
generated: { by: claude-code/claude-opus-5-5, at: 2026-10-31T01:06:27Z }
---

# Statistics: asked, sent and checked

A plain start (no `--scene`, `--seed`, `--dev` or a capture's option, not
headless) is a player's: it asks once and sends where the answer is yes.
Any other start neither asks nor sends, but for these switches:

```bash
# the question, as a first start asks it (a fresh --data-dir: the answer is written there)
python3 build.py shot --data-dir /src/.build/ask --dev statistics=ask --frames 120 --shot 100:/src/.build/ask.bmp
# Options' Statistics line (SELECT on the title opens Options too), the answer held
python3 build.py shot --dev statistics=on --input "30:,4:START,70:,4:SELECT,60:" --shot 150:/src/.build/row.bmp
```

`statistics=ask|on|off` holds the session's answer; such a session sends
nothing but to `statsurl=URL`, a local stand-in for Umami's `/api/send`
(any server that logs what it is posted). `statscheck` sends one event,
`game-dev-check` at `/dev-check` (its system as its data), to Umami itself
whatever the answer, and nothing else: on the systems that send from a
thread of the game's (Linux, the handhelds, Windows, the 3DS, the Switch, the Vita, the PS5) the output
says `statistics: taken (2xx) by ...` or `not taken by ...`. It is the
one way a test reaches the real Umami; keep it to a check of a system's
way there.
