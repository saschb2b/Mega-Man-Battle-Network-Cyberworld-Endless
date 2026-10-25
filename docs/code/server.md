---
type: Code Map
title: "The server, its deployment and the wire"
description: "What `server/` (the ROM-free Python server: presence, Trade Central, the week, the Arena, teams, stands, retention, backups), `deploy/` (Coolify) and `shared/online/` (the versioned wire contract) hold, file by file."
tags: [code]
generated: { by: claude-code/unrecorded, at: 2026-10-24T07:19:34Z }
---

# The server, its deployment and the wire

| Path | Contents |
| --- | --- |
| `server/` | The ROM-free Python server anyone can host (issue #120): presence and channels (`app.py`), Trade Central's commits and its board's offers (`trade.py`), the week's net's seed and Board (`week.py`), the Arena's pairing and relay (`arena.py`), the phrases' and emotes' burst, aims, answers and beat (`social.py`), Team NetBattles' teams and relay matches (`teams.py`), a dropped link's place held and taken back (`holds.py`), the daily totals with no personal data (`stats.py`, `/stats`), the stands: a battle kept for its spectators and the channel's battles told (`stands.py`), moderation, what its state keeps and for how long, pruned by itself, and the players' log (`retention.py`), the state's daily archive and its restore (`backup.py`), a start with nothing set (`hosting.py`: the INI found beside it, the environment's `CYBERWORLD_SERVER_*`, the addresses players type said on the console), its tests, zipapp, native and Docker packaging; never import game code or package ROMs/saves here |
| `deploy/` | The official server's deployment (`deploy/coolify/`: its Compose file, the server and its backup service, and configuration, the retention's numbers among it); its policy is server/README.md's The official server. `deploy/selfhost/`: anyone's own Square with HTTPS, one Compose file (the release's image from ghcr.io, `.github/workflows/server-image.yml`; its backup; Caddy with Let's Encrypt), server/README.md's Host your own Net Square |
| `shared/online/` | The versioned wire contract (v15): JSON spec and C constants, held together with the Square's generator version (`SQUARE_MAKE`, `square_plan.h`) and ROM identity by the server's tests |
