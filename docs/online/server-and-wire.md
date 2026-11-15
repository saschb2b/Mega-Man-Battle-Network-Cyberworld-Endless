---
type: Feature
title: "The server and the wire"
description: "Protocol v19, binary over a WebSocket: what each version since v7 added, what the server refuses and bounds (NOTICE 28 without the full set), and the official server's moderation and how long it keeps what."
tags: [online]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-14T22:45:00Z }
---

# The server and the wire

Protocol v19 ([shared/online/README.md](../../shared/online/README.md))
is binary over a WebSocket (`cyberworld.presence.v19`; v7 added the
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
[Team NetBattles](arena-in-brief.md#team-netbattles); v16 chip ids to 1023, for
[ten-bit chip ids](../multirom/bn4-chips.md); v17 BN4 Red Sun in the
NetPass's full set: [A game joins the set](netpass.md#a-game-joins-the-set); v18
a system of one extra ROM, its full pass and a trade's common ground:
[A system with room for one game](netpass.md#a-system-with-room-for-one-game); v19
the shared dive's server and wire, built ahead of the game's side: rooms
by layer beside the Square, the week's traces on each layer, the feed
and DIVERS, crews, the guardians' doors' queues and party battles over
the Arena's seats with linked fighters, and the Board's party marks and
fastest busts: [The shared dive](shared-dive.md), the wire's Protocol
v19). The
server refuses
another protocol version, another Square generator or hash, another ROM
code or SHA-1 (BN6's, and BN5's and BN4's where one is named), a name out of
bounds and a full Square; it refuses what needs the full set from a
session without it (NOTICE 28: a challenge, an accept, a trade's steps,
a board post, a WATCH, a WEEK_POST, and a challenge or a trade aimed at
such a session; a trade's steps and a board post ask a full pass, which
a system of one extra ROM holds with one game) and a trade's chip
outside the pair's common ground (NOTICE 19); it bounds every
message, the players, pending handshakes, each client's traffic and its
output, and relays the Arena's keys under a bucket of its own. Trading
and the Arena each have their switch in its configuration
(`trading`, `arena`). A player's identity is a key of the PET's own,
derived per server from a local secret; moderators ban, mute, kick and
allowlist through the server's command line, and take a week's Board
entry down (`hide`, which since v19 takes that week's traces of the dive
by that identity off every layer too). Hosting, TLS, backups and
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
player. Saves sent by a code ([Portable saves](../engine/saves.md),
Sending by code) it holds sealed, unreadable to it, in memory alone for
15 minutes or until taken, never in a file, a log or a backup, and
counts only how many were held and taken a day. Saves kept in step it
holds on disk, sealed and unreadable to it too, a slot each with the
hash of its auth, until the game deletes it or 180 days pass unused; out
of its backups.
`--admin forget` answers a request to be removed. It follows
`dev` until the release that ships online play, then `main`
([server/README.md, At a release](../../server/README.md#at-a-release-the-official-server-follows-main)).
