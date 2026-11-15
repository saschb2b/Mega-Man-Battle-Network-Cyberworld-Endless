---
type: Code Map
title: "Others' code"
description: "What `third_party/` holds: libraries kept as their authors released them (Monocypher 4.0.2, for saves sent by a code), how each is built apart from the game's warnings and lint, and how one is checked and updated."
tags: [code]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-14T21:13:18Z }
---

# Others' code

`third_party/` holds code the project did not write, kept byte for byte
as its authors released it, so an update is a copy and a diff against
their release. The game's rules for its own code (the warnings, `build.py
lint`'s smells, layers, dead functions and analyzer) are not held to it:
the Makefile builds it from `THIRD_SRCS` with the game's flags but none
of its warnings, its headers as the system's (`-isystem`), and Android's
`CMakeLists.txt` as a library of its own; lint reads `src/` alone.

| Path | Contents |
| --- | --- |
| `third_party/monocypher/` | [Monocypher](https://monocypher.org) 4.0.2 (`monocypher.c`, `monocypher.h`, `LICENCE.md`: CC0-1.0 or BSD-2-Clause, the project's choice the CC0), a small audited cryptography library in one C file, no allocation of its own. Used by `src/online/transfer_box.c` alone: Argon2id and XChaCha20-Poly1305 for saves sent by a code ([Portable saves](../engine/saves.md), Sending by code). The release tarball's SHA-256, the same from GitHub's release and from monocypher.org: `38d07179738c0c90677dba3ceb7a7b8496bcfea758ba1a53e803fed30ae0879c` |

To update one: its release fetched from two of its own places and their
hashes compared, the files copied over unchanged, the row's version and
hash set, and the tests that pin what it makes run (for Monocypher,
`tests/test_transfer_code.c`'s pinned key: a change there breaks every
transfer between builds before it and after).
