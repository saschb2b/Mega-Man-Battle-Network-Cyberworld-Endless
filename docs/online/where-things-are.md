---
type: Code Map
title: "Where things are"
description: "Where online play lives: the game's side in `src/online/` with the gate in `director_gate.c`, the server in `server/`, the wire in `shared/online/`, each part's builds and tests; the server imports no game code."
tags: [online]
generated: { by: claude-code/unrecorded, at: 2026-10-21T14:18:42Z }
---

# Where things are

| Part | Sources | Builds and checks |
| --- | --- | --- |
| The game's side | `src/online/` (the NetPass's set and stamps in `netpass.c`), the gate in `src/director/director_gate.c` and `src/map/lanhp.c` | `build.py` and every target; `build.py test` (`tests/test_online.c`: the Square's plan and hash, `tests/test_arena.c`), `build.py lint` |
| The server | `server/`, its deployment in `deploy/coolify/` | `build.py server`, `build.py server-test`; its own Dockerfile and CI workflow |
| The wire | `shared/online/` ([protocol v11](../../shared/online/README.md)) | The server's tests hold its codecs, the C constants, the Square's generator version and hash and the ROM's identity to the client's |

The dependency runs client → wire ← server. The server imports no game
code, runs no emulator, reads no ROM and never sees a save: chips are
numbers to it, MegaMen bytes it bounds. Its Docker image copies only
`server/`, `shared/online/` and the licence.
