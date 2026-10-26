---
type: Code Map
title: "The anonymous statistics"
description: "What `src/analytics/` holds: the anonymous statistics' answer, events and Umami payloads, the question, and each system's way to send them off the frame loop."
tags: [code]
generated: { by: claude-code/unrecorded, at: 2026-10-23T00:30:46Z }
---

# The anonymous statistics

| Path | Contents |
| --- | --- |
| `src/analytics/` | The anonymous statistics (issue #104, [The player's guide](../GUIDE.md#anonymous-statistics)'s Anonymous statistics): `analytics.c` the player's answer (`settings.ini`'s `statistics`), the events and their Umami payloads and the systems' queue (the game's own website, its host and Umami's endpoint named once in `analytics.h`), `analytics_online.c` online play's events (a visit, a NetBattle, a trade, a watch, a DeepNet post; issue #156) through the payload helpers of `analytics_payload.h`, `analytics_ask.c` the question at the first start and on Options' Statistics line (shown in Options' place, `src/core/options.c`), `analytics_net.c` each system's way to send, off the frame loop (a thread with `analytics_http.c`: libcurl loaded on Linux and the handhelds, linked with mbedTLS and `analytics_roots.c`'s Let's Encrypt roots on the 3DS, WinHTTP loaded on Windows; `analytics_apple.m`'s NSURLSession; Android's `GameActivity`; the browser's fetch), its words in `analytics_text.c`; the run log (`src/director/runlog.c`) hands it the guardians' battles and the runs' ends, and counts the run's economy (`runlog_tally_*`, the Mystery Data opened from `director_jobs.c`: chips, good chips, Mystery Data opened, zenny earned, sent at the run's end) and what the run chose (`runlog_choice`, `runlog_walked`, `runlog_fought`, `runlog_folder`, `runlog_layer_entered`: requests, orders, draft skips, dark and Undernet layers, duels and netbattles, folder edits, Virus Battlers' series, fight-ons and pots taken (`director_battler.c`), map and battle seconds, sent as run-choices beside run-end; [The core loop, reviewed](../game/core-loop.md), Phase 0) |
