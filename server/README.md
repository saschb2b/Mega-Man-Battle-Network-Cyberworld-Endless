# Cyberworld Net Square server

A separate, ROM-free component of the Cyberworld Endless monorepo.
Python 3.11 or newer is its only runtime dependency. It relays presence,
quick phrases and emotes, and public Inspect cards; commits Trade
Central's one-for-one chip trades; and pairs two players for the Arena
and relays their keys between them. Eight players fit in each channel;
the default total cap of 64 creates eight channels. There are no
accounts, e-mail addresses, arbitrary chat or currency, and it never
runs a game, holds a ROM or sees a save.

## The official server

`wss://cyberworld.saschb2b.com/presence` is the Net Square the game's
gate offers first. This is its policy. Anyone can run a server of their own
(the rest of this page); its host sets its rules.

### Who runs it

Saschb2b, who makes Cyberworld Endless, runs it on his own server. It
is a hobby server with no guarantees: no company behind it, and no
promise that it is up, that it is fast, that what it keeps survives, or
that it is still there tomorrow. It can go down for an update, a fault
or for good without notice. Your run never depends on it: nothing in the
Square changes your run but a chip you traded.

### Moderators

Saschb2b is its moderator. A moderator is whoever is given the server's
command line (its container's terminal in Coolify); anyone given it
later is named here first, by their GitHub name, with the date.

### The rules

- **Fixed phrases only.** Players speak in the game's quick phrases and
  emotes, which the server passes on and never stores. There is no free
  text, and the server refuses to start with it on. A burst of three,
  then one a second; an emote aimed at a player past two he has not
  answered in 30 seconds reaches its sender alone (protocol v14,
  `social.py`).
- **Names.** You choose the name others see: up to 16 letters, digits,
  spaces, `-` and `_`. Any player can block another from his card, which
  hides that MegaMan and his name from the blocker's Square; a moderator
  takes a name off the week's Board (`hide`) and removes a player whose
  name breaks these rules.
- **What gets a player muted, kicked or banned:** a name meant to
  offend, or to pass for Saschb2b or a moderator; phrases or emotes sent
  to flood a channel or to follow and pester someone; a modified game
  (a dive on the Board it could not have made, chips traded that it does
  not have, NetBattles broken on purpose); coming back with a new key
  after a ban. A moderator picks the least that stops it: a mute (no
  phrases or emotes; walking and cards stay), a kick (out for 10 minutes
  to 7 days), a Board entry hidden, and last a ban (out until lifted).

### Blocking, and no reports

The Square has no free text, so there is little to report: the one
thing a player writes himself is his name. To stop seeing someone, block
him from his card; that needs no moderator and works at once. The
moderator acts on what he sees himself (a name on the Board or in the
roster, a dive the net could not have given) and keeps no log of who came
under which name.

### What is kept, and for how long

The server has no accounts. Your game makes a random key on your device
(`network.ini`'s `identity`) and sends the server a token made from it
for this server's address, never the key itself. The server turns that
token into a public identity with a secret of its own: a number that
means nothing on another server and cannot be turned back into your key.
It sees your connection's IP address, as every server must, and writes
it nowhere.

| What | What it holds | Why | Kept |
| --- | --- | --- | --- |
| The server's secret, `identity.key` | 32 random bytes, nothing of yours | your public identity and the week's net stay the same across restarts | the server's life |
| The roster, `players.json` | public identity, name and channel of each player connected | moderators see who is on | while you are connected |
| The players' log, `logs/` | when you came, under which name and public identity, in which channel | a host who takes reports can match a name with its identity | not kept on the official server (`log_days = 0`) |
| The trades' log, `trades.jsonl` | each trade: when, both public identities, the chip each gave | what a moderator checks when a game trades chips it should not have | 14 days |
| The Board, `weeks.jsonl` | your best dive of a week: public identity, name, layer, act, minutes | the Board shows this week and the last | 2 weeks |
| Moderation, `moderation.json` | public identity, when, and the moderator's note, for each ban, mute, kick and hidden Board entry | the server enforces them | a ban or a mute until lifted, a kick until it ends, a hidden entry while its week is kept |
| The server's console log | when the server started, session numbers coming and going, trades' and battles' numbers, refusals: no names, public identities or IPs | running the server | 3 files of 5 MB, the oldest written over |
| The daily totals, `stats.jsonl` | a line a UTC day: joins, how many different players (a count: the day's public identities are held in memory and dropped at its end), the most online at once, trades, NetBattles started and finished, battles watched, dives posted, joins by game version and by NetPass; no names, public identities or IPs. Anyone can read today's and the last 30 days' at `/stats` | how many play and when, so the server and the game are sized and tuned for them | 365 days |
| Backups | the server's secret, moderation and the Board | to bring the server back after a fault | 14 days |

Everything else stays in memory and goes when you leave or the server
restarts: where you walk, your card, the phrases you send, your trade
offers, trades in progress, and your NetBattles' inputs (the stands keep
a battle's for a minute after it ends). That you came today is counted
for the day's totals, then forgotten at the day's end. The server prunes each record
itself, at its start and every hour. These are the official server's
settings ([What the state keeps](#what-the-state-keeps)); another server
may keep more or less.

### Removing your data

The server knows you only by your public identity, so it cannot look
you up by anything else. Two ways:

- **Yourself:** stop visiting, or delete `network.ini` from your data
  folder: the gate asks your name again and you have a new key, so a new
  identity. What the old one left goes by itself within the times above,
  and the backups 14 days after that. Blocks other players made of the
  old identity, and a ban, stay with it.
- **On request:** open an issue on the project's
  [GitHub page](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/new)
  titled "Net Square: remove my data", with the name your dives show on
  the Board (never your `network.ini` or its `identity` line). The
  moderator finds your public identity there and runs `forget`
  ([Identity and moderation](#identity-and-moderation)), which takes it
  out of the trades' log (your side of each trade; the other player's
  stays), the Board and any kick or hidden Board entry, and restarts the
  server. A ban or a mute stays while it lasts. The backups are not
  edited: they age out within 14 days. Without a dive on the Board the
  server has no name to find you by, and what you left goes by itself
  within the times above.

## Build and host locally

From the repository root:

```sh
python3 build.py server-test
python3 build.py server
python3 build.py server-run --config server/server.example.ini
```

Or independently of the game's build tools:

```sh
python3 server/build.py
python3 build/server/cyberworld-server.pyz --config server/server.example.ini
```

The zipapp can be copied to another computer with Python installed. The
configuration is supplied explicitly; without one the server binds
`127.0.0.1:8765`, seed 120, room capacity 8, total cap 64, snapshots at 20 Hz. Ctrl+C stops it
and closes its clients. `--host`, `--port` and `--seed` override the INI.
Port 0 chooses a free port and reports it in the `listening` log event.

Copy `server.example.ini` to `server.ini` for local settings (the latter
is ignored by Git). The client URL is `ws://HOST:PORT/presence`.
The subprotocol is `cyberworld.presence.v15`; incompatible clients receive
an explicit refusal rather than being admitted to a different world. A
game offering another version's `cyberworld.presence.vN` is let through
the upgrade so its HELLO can be answered with which side is behind (ERROR
2, the game older, or 17, the server older), which its gate says; before
v10 it was refused at the upgrade and could only say "no answer".

## Container

```sh
docker build -f server/Dockerfile -t cyberworld-server .
docker run --rm -p 127.0.0.1:8765:8765 -v cyberworld-state:/data cyberworld-server
```

The Dockerfile-specific ignore file restricts the build context to server
code and the shared protocol. The final image runs as a non-root user,
contains the zipapp and Python, and has no game or ROM dependency.
Mount a config read-only and pass `--config /path/server.ini` to customize
it; container traffic needs `host = 0.0.0.0` inside the container.

`GET /health` returns JSON with the configured name, message of the day,
protocol, occupancy, limits and rules. It contains no player identities.
`GET /stats` returns the daily totals: today so far and the last 30 days
([The daily totals](#the-daily-totals)).
`trading` and `arena` (both on unless set `false`) open or close Trade
Central and the Arena; `free_text` must stay false (quick phrases and
emotes only), and setting it refuses startup.

## Trade Central and the Arena

The Net Square is open to every game: walking, phrases and emotes,
Inspect cards, the trade board's offers, the Arena's stands and the
week's Board. Trading, posting an offer, NetBattles, watching one and
posting a week's dive need the full set: every game the build supports
for online play, BN6 and BN5 Team Colonel today, as each HELLO's ROM
identities say (`roms_full` in `shared/online/protocol.json`). The
server refuses those to a game without the full set, and a challenge or
a trade aimed at one, with NOTICE 28 before anything changes; the game
tells its player through its NetPass (docs/ONLINE.md, The NetPass).
Between two games of the full set there are no per-pair rules: every
chip trades and every battle is by full rules.

A trade is one chip for one chip between two players of a channel (ids 1
to 511, codes A to Z and \*, BN5's among them); one proposes with a
chip, the other answers with one, and the server commits only once both
have accepted the very pair they were shown. It writes each commit as a
line of `trades.jsonl` in the state directory (both public identities
and the chips each gave), kept `trade_days` days ([What is kept, and for
how long](#what-is-kept-and-for-how-long)), and tells both; each game applies its side when
its player comes home. A step that does not match what the server holds,
a player leaving or switching channel, or five minutes without progress
ends a trade for both, and neither swaps. Games are trusted for what
they hold; the log is what a host has to go on. A session trades once:
its game keeps one swap until its player is home.

The trade board: each player may keep one offer up in his channel, a
chip he has and a number for the kind he wants (the games' fixed list;
the server never reads a chip), at most one post a second. It lasts
while he is in the channel and until he trades; the server tells the
channel's players its offers as they change, leaving a BN5 chip's offer
(past id 319, or on BN6's blank records) out for a game without BN5's
ROM, which could not name it, and keeps no offer on disk. Taking an
offer starts a trade with both chips named, the poster's as posted.

The Arena never sees a battle: each game runs both players' linked BN6
cores, by full rules (BN5's chips and Souls among them), and the server
keeps no rules per battle. The server pairs a challenge with its answer,
gives each player its side of the cable and an input delay fitted to
both games' latencies (2 to 30 frames: the lockstep's frame, a tick, the
longer latency, each a round trip with its jitter less what its rollback
window covers; docs/ONLINE_ARENA.md, Smoothed delay and Rollback), then
relays their loadouts, keys and state hashes to each other alone, in a
rate bucket of their own (120 a second). A player leaving ends the
battle for the other. Since protocol v13 (epic #167) a pair of players'
next battle keeps their last one's sides, whoever challenges, so both
games go into a rematch from the pair they kept (the server remembers,
in memory alone, the cable's first of each pair's last started battle,
and forgets a player's pairs as he leaves); a challenge's 30 seconds run
from its question shown on the other's screen (ARENA's `seen`), 60 while
it is not; and each SNAPSHOT says what each player is busy with (a
battle, watching, a trade) and his link's bars from the round trip his
PING says, never the number itself.

Team NetBattles (`teams.py`, protocol v12, epic #167): two players of a
channel team up (one asks, the other answers; a team lasts until one
splits it or leaves), and a challenge between players of two teams
begins a relay instead of a battle. Each team's captain names its lead,
blind (30 seconds, then he leads himself); each bout is an Arena battle
whose two fighters both say they are ready before it starts; the
winner (the cable's first's report; an OVER from another seat is ignored) stays, the loser's partner comes in,
and the team with no MegaMan standing loses. A leave or a drop breaks the
match off for both teams. The server keeps teams and matches in memory
only and writes `team`, `team_match`, `team_bout` and `team_over` events
to the log; nothing is kept for a player. Since protocol v15 (issue
#181) a team challenge names its format, the relay or **side by side**:
side by side no order is asked (each captain leads, his partner beside
him) and the match is one Arena battle of four seats (the two captains
the cable's two GBAs, seats 0 and 1, their partners seats 2 and 3), all
four ready before its START, which tells each its seat and the four
sessions; each seat's messages go to the three others as ARENA_SEAT and
to the stands, whose spectators get all four seats' keys. Its winner's
team wins; a leave or a drop of any of the four breaks it off for both
teams. The server relays a partner's sixteen-bit stream as it relays a
lead's keys and reads none of it: his chips, Cross and Program Advance
ride in it, and each game's linked GBAs take them from their own link
packets (docs/ONLINE_ARENA.md, The partners' words in BN6's packet).

The stands (`stands.py`, issue #128): every started battle's loadouts,
ready, the cable's first's hashes and both players' keys from the first
frame are kept in memory while it runs (20 minutes of keys at most,
about 290 KB a battle) and a minute after it, for its spectators. A
player of the channel with the full set who asks to watch is sent all of
it, at most 2 KB a tick and only while his output is under 4 KB, so a
spectator who came late catches up without the players' relay ever
waiting on him; six watch a battle at most. Each channel, every game in
it, is told its battles (STANDS) as they begin and end and at most once
a second as they change: who fights whom, how many watch and cheer, and
the cable's first game's report (each MegaMan's HP, the last chip each
used, the winner). A phrase or emote of the channel's during a battle
counts as a cheer and is held from its two players; nothing a spectator
sends reaches the battle.

## Coolify deployment

The development server is deployed in Coolify at
`wss://cyberworld.saschb2b.com/presence`, with
[HTTPS health status](https://cyberworld.saschb2b.com/health). Its certificate,
browser WSS connection from `https://saschb2b.github.io`, and moderator
mute/unmute have been verified. The same browser identity survived a server
redeploy with the persistent state volume retained. Two actual native game
instances also passed the public WSS proof: 211 matching NPC observations, shared seed 120 and
map hash 4132376369, median RTTs of 33/39 ms and a maximum of 49 ms. All 24
save files stayed unchanged, and leaving restored both depth-3 loadouts.
The proof used a process-local DNS mapping because the test machine retained
a negative DNS cache entry; TLS still verified the server hostname through
the normal trust store. Network remains a development-branch feature and
has not been included in a published game release.

The deployment's files are in [`deploy/coolify/`](../deploy/coolify):
`compose.yaml` and the server's `server.ini`. Use the Docker Compose build
strategy in a Coolify Git resource with **Base Directory** `/` and
**Docker Compose Location** `/deploy/coolify/compose.yaml`. Enable
**Preserve repository during deployment** so the read-only configuration
file remains available on the deployment host. Leave raw deployment and
custom build/start commands off. Coolify passes the Base Directory as
Compose's project directory; the build context `.` and bind source
`./deploy/coolify/server.ini` are therefore relative to the repository
root, even though the Compose file is in `deploy/coolify/`.
Give
the `cyberworld` service the domain `https://cyberworld.saschb2b.com` and
container port **8765**. Coolify's proxy terminates TLS and forwards
WebSocket upgrades; no certificate is stored in the app container. Point
the domain's DNS record to the Coolify host before enabling its certificate.
Do not publish container port 8765 directly to the internet.

The compose file has two services from the same image, each running as
UID 10001 on a read-only root filesystem with every capability dropped:

- `cyberworld`, the server: its private persistent `cyberworld-state`
  volume at `/data`, and the backups read-only at `/backups` for a
  restore from its terminal. Docker restarts it when it exits, and after
  a reboot of the host, unless it was stopped (`unless-stopped`); its
  health check asks `/health` every 30 seconds and calls it unhealthy
  after three misses. Docker does not restart an unhealthy container:
  Coolify shows it, and its Restart does. Its log (Coolify's Logs) keeps
  three files of 5 MB, the oldest written over, and names no player
  ([What is kept](#what-is-kept-and-for-how-long)).
- `backup`, the state's daily archive ([Backups and
  restores](#backups-and-restores)): the state read-only at `/data`, its
  own volume `cyberworld-backups` at `/backups`, no port and no domain.
  It restarts as the server does, its health check is unhealthy once
  the newest archive is more than 26 hours old, and its log keeps two
  files of 1 MB.

The read-only `deploy/coolify/server.ini` is mounted into both. It
allows the published game's browser origin `https://saschb2b.github.io`
(add another exact origin only when serving the game there) and sets
the official server's retention. The server binds `0.0.0.0` inside its
container.

Local validation uses:

```sh
docker compose --project-directory . -f deploy/coolify/compose.yaml config --quiet
docker compose --project-directory . -f deploy/coolify/compose.yaml build
```

Run these commands from the repository root. Omitting `--project-directory`
changes relative-path resolution to the Compose file's `deploy/coolify/` directory.
Coolify's [Compose guide](https://coolify.io/docs/applications/builds/docker-compose)
documents the Base Directory settings and repository preservation needed
for file bind mounts. After loading the Compose definition, check that the
configuration mount resolves to a file ending in `/deploy/coolify/server.ini`,
the server's only writable persistent mount is `/data` and the backup
service's `/backups`, and the domain forwards to the `cyberworld`
service's internal port 8765. Do not add a host port mapping.

After deployment, verify HTTPS `/health`, a native client and a browser
client through `/presence`, then test a moderator action and a restart
with the same identity ([At a release](#at-a-release-the-official-server-follows-main)
lists the steps). The local and device checks in the platform table
remain necessary; a healthy HTTP endpoint alone proves no gameplay.

## At a release: the official server follows main

Until a release ships online play, the official server is a test
instance that follows `dev`. From the release that ships it (0.13.0) on,
it follows `main`, which moves only at releases, so the server always
speaks the protocol of the newest release's games. The owner does these
steps in Coolify (project Gaming, environment production, application
cyberworld-net-square); nothing here changes the live server by itself.

Before the tag:

1. Deploy `dev` once with this compose file, so the `backup` service and
   its `cyberworld-backups` volume exist; it is healthy within two
   minutes, its first archive made.
2. **Servers ›** the Coolify host **› Proxy**: check the proxy's
   configuration has no access log (Traefik's `--accesslog`). If it has
   one, it keeps every visitor's IP address: turn it off, or add it to
   [What is kept](#what-is-kept-and-for-how-long) with its retention.

On release day, once `vX.Y.Z` is tagged on `main` and the release is
published:

1. An archive to come back to: **Terminal**, container `backup`:
   `python3 /app/cyberworld-server.pyz --config /app/server.ini --state-dir /data --backup /backups --once`.
2. **Configuration › Source** (the Git source): Branch `dev` → `main`,
   Save. Check Base Directory `/`, Docker Compose Location
   `/deploy/coolify/compose.yaml` and **Preserve repository during
   deployment** on.
3. **Reload Compose File**, then check the services: `cyberworld` with
   the domain `https://cyberworld.saschb2b.com` and port 8765, `backup`
   with none; the storages `cyberworld-state` at `/data` (read-only in
   `backup`) and `cyberworld-backups` at `/backups` (read-only in
   `cyberworld`).
4. **Redeploy**, and wait until both containers are healthy.
5. `curl -s https://cyberworld.saschb2b.com/health`: `"status":"ok"`,
   `"protocol"` the release's (`version` in
   [`shared/online/protocol.json`](../shared/online/protocol.json): 10
   for 0.13.0), `"trading":true` and `"arena":true`.
6. A game of the release through the gate into the Square; `/health`
   counts it in `"players"`.
7. A moderator action: **Terminal**, container `cyberworld`:
   `python3 /app/cyberworld-server.pyz --state-dir /data --admin list`
   shows that game's name and public identity; `--admin mute --identity
   ID --reason "Release check"`, and the game's next phrase is refused;
   `--admin unmute --identity ID`.
8. A restart: **Restart**; `/health` is ok again, the game goes back in
   through the gate, and `--admin list` shows the same public identity
   as in step 7 (the server's key kept); the week's Board is as it was.
9. The backup: **Terminal**, container `backup`: `ls -l /backups` has
   today's archive. The `cyberworld` container's **Logs** show the join
   and the leave without a name or a public identity, and `/data/logs`
   holds no players' log (`log_days = 0`).
10. Write the result down: docs/ONLINE.md's line on the official server
    (it follows `main`, the protocol it speaks) and this section's dates.

Where a step fails, **Deployments** has the last good deployment to
redeploy (or set the branch back to `dev`), and a `--restore` of step
1's archive puts the state back ([Backups and restores](#backups-and-restores)).

## Identity and moderation

Clients keep a random private guest key on the device and derive a separate
token for each server address; the original key is never sent. The server
derives a public identity from that token, so the identity does not become
a cross-server tracking identifier. Its `identity.key` salt and moderation
lists live in the private state directory (`server-data/` by default;
`--state-dir` overrides it). The salt is generated once. Keep that
directory across upgrades: losing the salt changes public identities and
invalidates bans and players' local block lists. Do not publish it.

There is no remote administration endpoint. Use the CLI with filesystem
access to the state directory. The local roster supplies public IDs of
the players connected now, the players' log those of the last `log_days`
days; private player keys are never retained. Commands take effect within a
second. `ban`, `unban`, `mute`, `unmute`, `kick`, `unkick`, `hide`,
`unhide` and `forget` are supported:

```sh
python3 build/server/cyberworld-server.pyz --state-dir server-data --admin list
grep -h '"name":"NAME"' server-data/logs/players-*.jsonl
python3 build/server/cyberworld-server.pyz --state-dir server-data --admin mute --identity PUBLIC_ID --reason "Repeated disruption"
python3 build/server/cyberworld-server.pyz --state-dir server-data --admin kick --identity PUBLIC_ID --minutes 30
python3 build/server/cyberworld-server.pyz --state-dir server-data --admin hide --identity PUBLIC_ID --week 2963
python3 build/server/cyberworld-server.pyz --state-dir server-data --admin forget --identity PUBLIC_ID
python3 server/admin.py unmute PUBLIC_ID --state-dir server-data
```

In Coolify's container terminal, substitute `/data` and
`/app/cyberworld-server.pyz`. A kick disconnects the profile's sessions
and refuses it for ten minutes, or `--minutes` (1 to 10080); then it ends
by itself, `unkick` lifts it early, and the server's retention pass
prunes the kicks that ended. A ban refuses the profile until `unban`. Mutes persist and suppress phrases and
emotes, while movement and Inspect remain available. An optional
`whitelist` is a space-separated list of public IDs allowed to connect.
Profiles remain anonymous: resetting a private key can evade a ban.
An allowlist is available where a host needs stricter admission.
`hide` takes a profile's entry off the week's Board (docs/ONLINE.md, The
week's net) for the week `--week` names, this week where none, and
refuses that profile's posts for that week; `unhide` puts it back.
`forget` answers a player's request to be removed: it takes the public
identity out of the trades' log (its side of each trade blanked, the
other player's kept), the Board's log, the players' log and its kick or
hidden entry. A ban or a mute stays while it lasts. The running server
reads the Board only as it starts: restart it after a `forget`.

The week's net: every Monday 00:00 UTC the server begins a week (its
number counts weeks from Monday 5 January 1970) whose seed is derived
from `identity.key` and the week's number, so each server has its own
week, the same across restarts. Its Board keeps one entry per profile and
week, the best a game with the full set reported, appended to `weeks.jsonl` in the state
directory (public identities, names, layers, minutes; no keys) and read
back at the start, pruned to `board_weeks` weeks. It is in the backups,
or a restored server's Board would start the week empty.

Moderation updates use an atomic replace. Every write of the state's
files (moderation, the trades' and the Board's logs, the players' log, a
retention pass, a restore) holds the state directory's lock,
`moderation.lock`, so local CLI edits and the running server never meet
in a half-written file. There is no game save or ROM on the server.

The server reads `moderation.json` again only when the file changes. If a
hand edit leaves it unreadable, the running server keeps the last rules it
read, logs the error once and goes on admitting players; the CLI refuses
to edit a file it cannot read, the retention pass leaves it as it is, and
a server does not start with one. The
roster `players.json` is rewritten, outside the network loop, only when a
player joins, leaves or changes channel.

### What the state keeps

The retention's numbers are keys of the INI's `[server]` section, at
these defaults, and a pass of the server prunes each thing to them at its
start and every hour (`retention.py`):

| Key | Default | What it bounds |
| --- | --- | --- |
| `trade_days` | 14 | `trades.jsonl`, the trades committed; 0 writes none |
| `board_weeks` | 2 | `weeks.jsonl`, the Board's weeks: at least 2, this one and the last, which the Board shows; with them go the entries `hide` hid |
| `log_days` | 14 | `logs/players-YYYY-MM-DD.jsonl`, the players' log, a file a UTC day; 0 writes none |
| `backup_days` | 14 | the archives `--backup` keeps |
| `stats_days` | 365 | `stats.jsonl`, the daily totals, a line a UTC day ([The daily totals](#the-daily-totals)); 0 writes none |

The players' log holds each event that names a player (a join, a
session replaced) with its time. The console's log (stdout, a
container's log) has the same events without the name and the public
identity: session numbers, channels, counts, trades' and battles'
numbers and refusals, never a name, a public identity or an IP.
`identity.key` and the bans and mutes stay until the host or a moderator
removes them; `players.json` holds the players connected now and is
emptied when the server stops. Everything else is in memory alone.

### The daily totals

Each UTC day the server counts, with no personal data (`stats.py`):

| Field | What it counts |
| --- | --- |
| `joins` | sessions that joined |
| `identities` | different public identities among them: a number only. The identities are a set in memory for the day, dropped at its end and never written or served; a restart the same day starts the set afresh, so a player seen before and after it counts twice |
| `peak` | the most players online at once |
| `trades` | trades committed |
| `battles_started`, `battles_finished` | NetBattles begun, and those whose end the cable's first game told (a battle left or broken is begun and not finished) |
| `watched` | battles watched from the stands |
| `dives` | the week's dives posted to the Board |
| `versions` | HELLOs by the protocol version they carry, those of another version refused (16 versions apart, the rest as `other`) |
| `passes` | joins by the NetPass: `full` (BN6's and BN5's ROMs) or `square` (BN6's alone) |

HELLO names no system, so the server counts none by system; the game's
own statistics do, where the player said yes ([GUIDE.md, Anonymous
statistics](../docs/GUIDE.md#anonymous-statistics)). A day is written once,
as a line of `stats.jsonl` in the state directory, when the next begins or
as the server stops; a restart the same day takes that line up again and
writes it over. The retention's pass keeps `stats_days` days. `GET /stats`
returns `{"today": {...}, "days": [...]}`, today so far and up to 30 days
before it, oldest first, the same fields with `day` (`YYYY-MM-DD`).

## Backups and restores

`--backup DIR` archives the state directory into `DIR` in the first hour
of each UTC day that has no archive yet, as
`cyberworld-state-YYYYMMDD-HHMMSS.tar.gz` (UTC), deletes the archives
older than `backup_days`, and looks again every hour; a failure is
logged and tried again a minute later. `--once` makes one archive now
and exits. An archive holds `identity.key`, `moderation.json` and
`weeks.jsonl`: what the server needs to come back as it was, its
identities, bans and Board. The trades' and the players' logs and the
roster are left out: they are kept for moderation alone and go with their
days. It reads the state under the state's lock, shared, so it never holds
half a moderator's edit, and works from a read-only mount of the state.

```sh
python3 build/server/cyberworld-server.pyz --state-dir server-data --backup server-backups --once
python3 build/server/cyberworld-server.pyz --state-dir server-data --restore server-backups/cyberworld-state-20261020-001500.tar.gz
```

`--restore ARCHIVE` checks the archive (regular files, a server's 32-byte
`identity.key`; anything else in it is never extracted), writes its files
into the state directory whole, and removes those of the three it lacks,
which were not there when it was made. The trades' and the players' logs
stay as they are. The server reads the key and the Board only as it
starts: restart it after a restore.

In Coolify the `backup` service of `deploy/coolify/compose.yaml` runs
`--backup /backups` beside the server, so the official server's backups
need nothing set in Coolify. Both volumes live on the Coolify host: the
backups cover a moderator's mistake, a bad deploy or a lost file, not the
loss of the host. A copy off the host is the owner's to make (`docker cp`
from the backup container on the host), and is deleted after
`backup_days` like the others.

To restore the official server:

1. In Coolify, open the application's **Terminal** and choose the
   `cyberworld` container (the server, not `backup`).
2. `ls -l /backups` lists the archives; pick the one to go back to.
3. Run `python3 /app/cyberworld-server.pyz --state-dir /data --restore /backups/cyberworld-state-YYYYMMDD-HHMMSS.tar.gz`.
   It prints the files it restored.
4. Press **Restart** for the application.
5. Check `https://cyberworld.saschb2b.com/health` says `"status":"ok"`,
   and `python3 /app/cyberworld-server.pyz --state-dir /data --admin list`
   shows the moderation lists the archive held.

Restored this way locally with Docker Compose (20 October 2026): the
stack up, an archive made, the state's key, moderation and Board changed
and removed, restored from the server's terminal and the server
restarted, every file the same bytes as before and the server healthy.

## Connections

Only joined players count against `player_cap`. At most 16 connections
may be mid-handshake at once, each given five seconds for its HTTP Upgrade
and HELLO together; further sockets are closed at once, so idle sockets
cannot lock players out. A new connection with a profile that is already
connected (a reconnect the server has not yet seen drop) replaces the old
session: the old one receives ERROR 12 and a close frame saying why, and
the new one takes its place and, unless it asks for another, its channel.

### A dropped link: the place held

A player whose link drops without a goodbye (the game says BYE when it
leaves, at the pad or as it closes) keeps his place for **40 seconds**
(`holds.py`, protocol v10): his slot in his channel, which counts
against the room and the cap, his session ID, and his MegaMan standing
where he stood in the other players' Square. The server notices a drop
at once where the socket closes, or after 15 seconds of silence. What he
had under way ends at once, as on any leave: a trade not committed is
off for both, a NetBattle ends for the other, watching stops, his board
offer comes down. His game tries again for 30 seconds from the moment
its link went quiet or dropped, with a resume's HELLO: the same place,
session ID and channel, the other players told nothing, and the trade's
last word (done or off) said again in case the link lost it. Forty
seconds cover the game's 30, its last try's handshake, and a drop the
server notices before the game does. The log says `hold`, then `resume`
or `leave`; a resume is the same visit, neither a `join` nor one of the
day's joins. A refusal, a moderator's removal or a goodbye frees the
place at once; so does a new visit of the same profile from the gate,
which arrives anew. Positions have a token bucket of their own (200 deep,
25 a second) beside the 40 of the rest, so the backlog a stalled link
lets through at once as it clears is not taken for a flood.

### Maintenance

`maintenance` in `server.ini` closes the Square: set it to a line of up
to 40 printable characters (`maintenance = until 14:00 UTC`) and restart,
and every game that comes is turned away with ERROR 16 and that line,
which its gate's keeper reads out ("SORRY! THE SQUARE'S UNDER
MAINTENANCE!", "THE SIGN ON IT SAYS, UNTIL 14:00 UTC") rather than "no
answer"; `/health` carries it as `"maintenance"` (and the hold as
`"hold_seconds"`). Players in the Square when the server goes down for it
come home with the same word as their tries meet it. Empty or absent:
open. A game newer than the server (its protocol or its Square
generator) is answered ERROR 17, and its gate says the server is behind;
an older game is answered ERROR 2 or 3 and told a newer version is out.

## TLS and privacy

Public hosting needs `wss://`. Supply `tls_certificate` and `tls_key` in
the INI, or terminate TLS at a reverse proxy that forwards WebSocket
Upgrade requests to `/presence`. The native Linux/macOS transport uses
libcurl's system CA trust and verifies both certificate and hostname.
The browser uses its own trust store. There is no trust-on-first-use,
certificate-warning bypass, or automatic fallback to plaintext.

Browser clients must have their exact origin listed, separated by spaces,
in `origins`. For example, `origins = http://localhost:8080`. Empty means
native clients only. An HTTPS page requires `wss://`, as enforced by the
browser. The origin check is not player authentication.

The host sees connecting IPs at the network layer, display names and
positions and the numeric public card (HP, buster stats, Souls, Crosses,
depth, best depth, title marks and colored NaviCust board). The console's
log has joins, channels, leaves and refusals by session number; public
IDs and names go to the players' log alone, kept `log_days` days
([What the state keeps](#what-the-state-keeps)); neither has IPs or
private player files. `--event-log FILE` optionally writes JSONL
evidence, names and public IDs included, which the server does not
prune: choose your own rotation and retention, and say so in your
policy. Sampled positions are logged only with that explicit evidence
option; the official server does not use it.
The world is reconstructed from its seed.

The official server's hosting policy and moderators are [The official
server](#the-official-server); a host of another server writes his own.
The server can validate ranges and compatibility, but a
modified client can claim another HP or board; Inspect is informational.

## Client platform status

| Target | Backend | Verification |
| --- | --- | --- |
| Linux desktop | SDL worker, runtime libcurl byte stream, bounded WebSocket framing with HTTP/1.1 upgrade | Two actual games over public WSS: 211 matching NPC observations, median RTTs 33/39 ms, saves unchanged and both loadouts restored on leaving; local peer departure/server-stop checks also pass |
| PortMaster/Linux ARM64 | Same libcurl backend | Physical Pocket Nova on ROCKNIX: public WSS and reciprocal desktop movement pass (189 actual NPC samples, median/max RTT 40/47 ms); depth-29 checkpoint restored identically, all 13 installed save files unchanged; untrusted TLS rejected |
| macOS | Same backend with system `libcurl.dylib` | Actual transport runs on macOS 15.7.9 ARM64: public WSS, protocol exchange, reconnect, cancellation, disconnect, hostile frames and untrusted TLS pass; full game/checkpoint testing on a Mac remains |
| Browser | Emscripten WebSocket callbacks on the main thread | Chrome/WebAssembly and native: same seeded map, reciprocal NPC movement, median 4 ms loopback RTT; public WSS connection from the allowed site origin verified |
| Windows | Asynchronous WinHTTP WebSocket callbacks, decoded on the SDL worker; Windows certificate and hostname verification | Actual transport runs on Windows Server 2022 x64: public WSS, protocol exchange, reconnect, cancellation, disconnect, hostile frames and untrusted TLS pass; full game/checkpoint testing on Windows remains |
| New 3DS | Linked libcurl/mbedTLS, shared SOC/SSL services, packaged Let's Encrypt roots, PS random bytes | Full devkitARM build and CIA/3DSX link with warnings as errors; hardware Wi-Fi, trust and memory-budget tests required |
| Android | Java Socket/SSLSocket through JNI, shared bounded WebSocket framing, system certificate and hostname verification | Actual APK on Android 14 emulator passed public WSS, movement, 104 positioned NPC samples, unchanged checkpoints and certificate refusal; physical phone remains untested |
| iOS | NSURLSessionWebSocketTask, bounded send/open waits and queued receive, system trust | Device and Simulator builds plus an actual Simulator launch pass CI; live device connection and TLS refusal tests remain |

The current build/package evidence is
[CI run 38654155398](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/actions/runs/38654155398).
Server executable smoke tests below are separate from game-client connection tests.

The native transport probe compiles the game's actual SDL worker, protocol
decoder and platform socket backend, with compatibility metadata in place
of a ROM. It checks movement against an independent socket client, Inspect,
emotes and channel changes, then hostile frames, interrupted connections,
reopening and refusal of an untrusted TLS server before any application
data is sent. Build it with `make online-transport` in the matching build
environment (`TARGET=windows` or `TARGET=macos` there), then run
`python3 tests/test_online_transport.py --binary PATH --public` on that OS.
The CI workflow's optional `public_wss` input enables its deployed-server
check; ordinary CI needs only local test servers. Reports are retained as
the `online-transport-*` workflow artifacts. These are networking runtime
checks, not ROM-backed gameplay or proof of every physical device.

All existing offline builds remain supported. This is not a claim that
the epic's full cross-platform transport spike is complete. Production
platform tests must cover the remaining targets. The 3DS's bundled roots
support the documented Let's Encrypt deployment; other certificate
authorities require an explicit root-bundle update, never a validation bypass.

See [online play](../docs/ONLINE.md) and
[the wire format](../shared/online/README.md).

## Release packaging

The release workflow builds six ROM-free server archives beside the game:

| Archive | Runs with |
| --- | --- |
| `cyberworld-server-python.zip` | Python 3.11+ on any supported server OS |
| `cyberworld-server-linux-x86_64.zip` | x86-64 Linux, Ubuntu 22.04/glibc 2.35 or newer |
| `cyberworld-server-linux-arm64.zip` | ARM64 Linux, Ubuntu 22.04/glibc 2.35 or newer |
| `cyberworld-server-windows-x64.zip` | 64-bit Windows 10/11 or Server 2022 |
| `cyberworld-server-macos-x86_64.zip` | Intel Mac, macOS 15 or newer |
| `cyberworld-server-macos-arm64.zip` | Apple Silicon Mac, macOS 15 or newer |

Each archive includes the executable or zipapp, example INI, this guide,
MIT license and `BUILD.json`. Native archives need no installed Python and
carry the notices of what was frozen into them in `licenses/`, written at
build time from the build host's own files (`server/notices.py`; none are
committed): CPython's from the building interpreter's `LICENSE`, each
native library frozen from outside that interpreter's install from its
Debian or Ubuntu package's copyright file or the licence file beside it,
and PyInstaller's from its wheel; `licenses/README.md` lists which file
covers which library. A frozen library with no licence file on the build
host fails the build. The Docker recipe remains available for
containers. No new release has been published by this change. All five
native server targets passed their packaging jobs in
[CI run 38652295286](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/actions/runs/38652295286):
Linux x86-64/ARM64, Windows x64 and macOS Intel/ARM64. Each passed all 21
server tests, freezing, isolated help/health/admin smoke checks and artifact
upload. The Python zipapp bundle also passed its build and process smoke.

Build the zipapp archive with `python3 server/release.py`. For a native
archive, create a Python virtual environment, install the build tools with
`python -m pip install --require-hashes --only-binary=:all: -r server/requirements-build.txt`,
then run `python server/release.py --native` on the target OS and architecture.
The workflow pins Python 3.13.16 and PyInstaller 6.16.0, with all packaging
dependencies and wheel hashes fixed. It uses the documented
[GitHub runner labels](https://docs.github.com/en/actions/reference/runners/github-hosted-runners)
and [PyInstaller's native build and data-file options](https://pyinstaller.org/en/v6.16.0/usage.html).

Every package must pass `--help`, an isolated server process serving
`/health` with the current protocol, and a local administration command.
The smoke runs outside the checkout, so a missing bundled `protocol.json`
fails before publication. Protocol/moderation tests run on every matrix OS.

The five native builds are `.github/workflows/server-native.yml`, which a
release runs on its tag and the Server workflow runs by hand (Run
workflow). A push or pull request that touches `server/`, `shared/online/`
or `deploy/` runs the Server workflow's Linux job instead: the tests on
Python 3.11, the zipapp's process smoke and the Coolify image's build. CI's
unit tests run the server suite on every change.
Source zipapp entries have fixed ordering, times and permissions; native
builds also fix the hash seed and build timestamp. Hosted runner images
and system libraries can change, so native outputs are not promised to be
byte-identical across runner updates. Release checksums cover every archive.

Keep the shared protocol in the same release as the game; a game of
one protocol version cannot enter a server of another (v9 games and a v10
server refuse each other). Preserve the private state directory when upgrading.
