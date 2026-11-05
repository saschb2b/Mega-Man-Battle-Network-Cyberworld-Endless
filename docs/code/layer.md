---
type: Code Map
title: "What stands on a layer"
description: "What `src/layer/` holds: the objects on a layer (NPCs, services, shops, choices, guardians), the chat parts in `scripts.c` and the words files the lines live in."
tags: [code]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-05T17:00:00Z }
---

# What stands on a layer

| Path | Contents |
| --- | --- |
| `src/layer/` | What stands on a layer: NPC and text scripts, services, shops, choices, guardians. `layer_objs.c` installs each kind of object, `scripts.c` holds the parts every chat is built of; the words are files of their own ([Voice](../voice/rules.md#where-the-lines-live), Where the lines live): `layer_words.c` (the Net Dealer, the NaviCust vendor, ProtoMan's terms, a ScrtData), the chats in `service_words.c`, `reward_words.c` and `lock_words.c`, MegaMan's read of the layer behind a fork's way in `fork_words.c` ([a fork's read](../world/level-design/fork-read.md)), the guardians' `guardian_lines.c`, the NaviCust's `navicust_words.c`, the bystanders' `npc_lines.c` and `rumor_lines.c`; a talker whose sprite and mosaic the director changes through its idle loop (`npc_talker_looks` in `npc.c`, the Glitch warp's looks); `glitch_words.c` the Glitch warp's and the copy's talk and the stage words, the copy's pick in `reward_words.c` ([the Glitch layer](../game/glitch-layer.md)); `bracket.c` the tournament's bracket as a layer holds it: whether one stands (BN4's screen and its faces), its pool of the games' moved Navis, its rounds' Navi chips (their scripts beside the guardian's, `guardian_objs.c`; `ta_round_prize` in `reward_words.c`) and its eight entrants as BN4's screen takes them ([the tournament](../game/tournament.md)) |
