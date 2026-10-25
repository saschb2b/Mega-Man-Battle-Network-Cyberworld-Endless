---
type: Feature
title: "The server and the wire"
description: "Protocol v15, binary over a WebSocket: what each version since v7 added, what the server refuses and bounds (NOTICE 28 without the full set), and the official server's moderation and how long it keeps what."
tags: [online]
generated: { by: claude-code/unrecorded, at: 2026-10-24T07:19:34Z }
---

# The server and the wire

Protocol v15 ([shared/online/README.md](../../shared/online/README.md))
is binary over a WebSocket (`cyberworld.presence.v15`; v7 added the
stands, v8 the latency each game asks for with rollback on, v9 the
NetPass's rule, above, v10 a dropped link's place held and taken back,
BYE, maintenance and which side is behind: A dropped link, above; v11
the Arena's lag notes, by which the time sync splits the waits evenly:
[Quicker on the wire](arena/rollback/quicker-on-the-wire.md); v12 Team NetBattles; v13 the
bout's seed, a rematch's sides kept, a challenge seen, and the busy marks
and link bars in SNAPSHOT, its round trip in PING; v14 the emotes: an
emote aimed, its answer flagged, the beat it starts on, twelve emotes and
a burst of three then one a second: [Emotes](square/emotes.md); v15 a team
challenge's format and two MegaMen a side, four seats in one battle:
[Team NetBattles](arena-in-brief.md#team-netbattles)). The
server refuses
another protocol version, another Square generator or hash, another ROM
code or SHA-1 (BN6's, and BN5's where one is named), a name out of
bounds and a full Square; it refuses what needs the full set from a
session without it (NOTICE 28: a challenge, an accept, a trade's steps,
a board post, a WATCH, a WEEK_POST, and a challenge or a trade aimed at
such a session); it bounds every
message, the players, pending handshakes, each client's traffic and its
output, and relays the Arena's keys under a bucket of its own. Trading
and the Arena each have their switch in its configuration
(`trading`, `arena`). A player's identity is a key of the PET's own,
derived per server from a local secret; moderators ban, mute, kick and
allowlist through the server's command line, and take a week's Board
entry down (`hide`). Hosting, TLS, backups and
the platforms' transports are [server/README.md](../../server/README.md)'s:
a Square of one's own started with nothing set on a computer of the
network, or behind HTTPS with `deploy/selfhost/` (Caddy and Let's Encrypt
in front of the release's image), its [Host your own Net
Square](../../server/README.md#host-your-own-net-square).

## The official server

`wss://cyberworld.saschb2b.com/presence`, run by Saschb2b, who is its
moderator, as a hobby server with no guarantees; its policy is
[server/README.md, The official server](../../server/README.md#the-official-server).
Fixed phrases and emotes only; a player blocks another from his card,
and a moderator mutes, kicks, hides a Board entry or bans on what he sees
himself; with no free text there are no reports. The server has no
accounts and writes no IP address: a public identity derived
from the game's per-server token, in the trades' log for 14 days (no
players' log on the official server: `log_days = 0`), the Board for 2 weeks, bans
and mutes until lifted, and daily backups of its key, moderation and
Board for 14 days, and its daily totals, with no names, for a year at
`/stats` (`server.ini`'s `trade_days`, `board_weeks`,
`log_days`, `backup_days`, `stats_days`), each pruned by the server itself
(`retention.py`) and by its backup service (`backup.py`, a second
service of `deploy/coolify/compose.yaml`). The console's log names no
player. `--admin forget` answers a request to be removed. It follows
`dev` until the release that ships online play, then `main`
([server/README.md, At a release](../../server/README.md#at-a-release-the-official-server-follows-main)).
