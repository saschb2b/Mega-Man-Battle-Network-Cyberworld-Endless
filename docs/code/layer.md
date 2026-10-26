---
type: Code Map
title: "What stands on a layer"
description: "What `src/layer/` holds: the objects on a layer (NPCs, services, shops, choices, guardians), the chat parts in `scripts.c` and the words files the lines live in."
tags: [code]
generated: { by: claude-code/claude-opus-5-526-10-26T19:56:00Z }
---

# What stands on a layer

| Path | Contents |
| --- | --- |
| `src/layer/` | What stands on a layer: NPC and text scripts, services, shops, choices, guardians. `layer_objs.c` installs each kind of object, `scripts.c` holds the parts every chat is built of; the words are files of their own ([Voice](../voice/rules.md#where-the-lines-live), Where the lines live): `layer_words.c` (the Net Dealer, the NaviCust vendor, ProtoMan's terms, a ScrtData), the chats in `service_words.c`, `reward_words.c` and `lock_words.c`, MegaMan's read of the layer behind a fork's way in `fork_words.c` ([a fork's read](../world/level-design/fork-read.md)), the guardians' `guardian_lines.c`, the NaviCust's `navicust_words.c`, the bystanders' `npc_lines.c` and `rumor_lines.c` |
