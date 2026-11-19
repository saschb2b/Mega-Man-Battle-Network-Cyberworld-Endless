---
type: Feature
title: "Discord's presence"
description: "How the desktop builds show a player's friends on Discord that they play and how far the run has come (Rich Presence): what friends see at each place of the game, the local pipe to the Discord app and the two commands sent over it, the pace and the retries Discord's limits ask for, the Options line and settings.ini's discord line, the Flatpak's permission, the application the owner registers, and what is left out."
tags: [engine, discord, platforms, online]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-18T16:00:00Z }
sources:
  - { id: bp, title: "Discord: Best Practices for Rich Presence", resource: "https://github.com/discord/discord-api-docs/blob/main/developers/rich-presence/best-practices.mdx" }
  - { id: assets, title: "Discord: How to get your game seen (asset keys and external image URLs)", resource: "https://github.com/discord/discord-api-docs/blob/main/developers/game-development/how-to-get-your-game-seen.mdx" }
  - { id: sdk, title: "Discord Social SDK: Setting Rich Presence (timestamps in Unix seconds, buttons)", resource: "https://github.com/discord/discord-api-docs/blob/main/developers/discord-social-sdk/development-guides/setting-rich-presence.mdx" }
---

# Discord's presence

A player's friends on Discord see that they play Cyberworld Endless and
how far the run has come, on the player's profile and in the member list:
the layer and its area, what MegaMan is doing there, the time played, the
player's best on the picture's tooltip, and a button to the game's page.
The desktop builds have it (Windows, Linux and macOS); the code is
`src/discord/` ([The code: Discord's presence](../code/discord.md)).

It runs on the Discord app the player already has open on the same
computer, over the app's local pipe, as Discord's own RPC libraries did:
no SDK, no library to ship, no account, no network request of the game's.
What it hands the app is shown as the player's Discord settings allow
(Discord's own Activity Privacy switch hides it there too). Where no
Discord runs, nothing shows and nothing waits.

## What friends see

Discord's best practice is snippets, not sentences, each on one line,
the large image the same for everyone and the tooltip for detail
([bp]). Where first, then what:

| Where | First line (details) | Second line (state) |
| --- | --- | --- |
| A layer | `Layer 14 · Sky Area` (the short net: `Layer 7 of 10 · Sky Area`) | `Exploring`, `Busting viruses`, `Fighting ElecMan`, `Dueling ProtoMan`; the threat after it where the run has one: `Fighting ElecMan · Threat 3` |
| Home between acts | `Home · layer 4 next` | the town's name or `Lan's HP`; `Battle Lab` in a test battle |
| The Net Square | `In the Net Square` | `Online`, or `NetBattling` |
| The title | `On the title screen` | |
| The run's setup | `Setting up a run` | |
| A run's summary | `Run over on layer 14`, or `Won the run on layer 10` | `New best!` where it was |
| The ROMs, SAVES, the intro | `In the menus` | |

The picture is the game's icon from its site (Discord takes an address
for the art as it takes a key uploaded to the portal, [assets]), its
tooltip `Best: layer 22 · 12 runs` once a run has ended, the version
before. The time counts from the game's start, one clock for the session
that no screen resets. The button, **Get Cyberworld Endless**, opens the
game's page; Discord shows it to others, not to the player.

The layer is the one MegaMan stands on (on a trip back, the older act's);
home names the run's own next. A side layer names its own area (the
Undernet, the Secret Area). The words are composed from the run's facts
in `discord_text.c`, apart from what reads them (`discord_game.c`), as
every line of the game is ([The voice rules](../voice/rules.md)); the
voice catalog lists them as a screen's words.

### Why it shows this

The design pass (the game-design skill, before the numbers): the presence
restates the game's own score, how deep the run is, on the social layer
between sessions; it is an ambient signal a friend reads, never a
contingency the game rewards (no "share for a prize", nothing of the
friend list read), so it passes the skill's ethics test. It is on by
default, as Discord's players expect of a game, and one line switches it
off.

## Options and settings.ini

**Discord** on Options' Game page, after Updates, switches it off and on
(`discord = off` in `settings.ini`, written with its note when it is
first turned); off clears what Discord shows at once and lets the pipe
go. The line shows where the build can reach a Discord app and a player
started the game (never headless, a test's or a developer's start), and
the build has an application to name. START's defaults on the Game page
leave it as it is, as they leave Updates and Statistics.
`discord_app = ID` in `settings.ini` names another application (a fork's
own; the game's own otherwise). `--dev discord` lets a developer's start
show it too, a headless one included: an autopilot run on the friend
list, Options' line in a capture
([the dev menu's switches](../devtools/dev-menu.md)).

## The wire

The Discord app listens on a pipe per client, the first free of ten:
Windows' named pipe `\\.\pipe\discord-ipc-0` to `-9`, elsewhere a Unix
socket `discord-ipc-0` to `-9` in `$XDG_RUNTIME_DIR`, `$TMPDIR` (macOS's
per user), `$TMP`, `$TEMP` or `/tmp`, and in each of those the Flatpak's
Discord's (`app/com.discordapp.Discord/`), the Snap's (`snap.discord/`)
and Vesktop's own folder. `discord_ipc.c` opens the first that answers;
it never waits on a read (a peek on Windows, a non-blocking socket
elsewhere).

A frame is an opcode and a length, each four bytes little-endian, and
that many bytes of JSON (`discord_activity.c`): 0 the handshake, 1 a
command or its answer, 2 a close, 3 a ping (answered with 4, its pong,
the same body). The game sends two things:

- the handshake, `{"v":1,"client_id":"<the application>"}`, answered by
  a `READY` event, or by a close (code 4000: an application Discord does
  not know, which the session then leaves alone);
- `SET_ACTIVITY` with the game's process id and the activity
  (`details`, `state`, `timestamps.start` in Unix seconds [sdk],
  `assets.large_image` and `large_text`, one button, `instance: false`),
  or with no activity, which clears it. A line under two bytes is left
  out, as Discord refuses one; quotes, backslashes and control
  characters are escaped, the rest kept as UTF-8.

Checked on 19 November 2026 against the Discord app's Snap on Ubuntu
(its socket in `snap.discord/`): the handshake's `READY` names the
signed-in user, and Discord's answer to `SET_ACTIVITY` gives the activity
back as friends get it, named "Cyberworld Endless" by the application,
the picture's address taken as an external image (`mp:external/...`, Discord's
proxy), the button's label and address kept and the start in
milliseconds.

## The pace

Discord takes five activity updates in 20 seconds. `discord_game.c`
reads the game twice a second; `discord.c` sends only what differs from
what Discord shows, at most once in five seconds, the newest after the
wait (a battle's start and end a second apart show as the end). Discord
not found is looked for again every 20 seconds; a handshake unanswered
in ten is dropped and tried again later; a pipe that breaks (Discord
closed) goes back to looking. At the game's end the activity is cleared
and the pipe closed; Discord clears a closed pipe's activity itself too.

The game is only read: `frame_watch` (set by `start_net.c`, called by
`main.c` after a frame's update and before its drawing) reads the scene,
the run, the director's battle and home, the Net Square's visit and the
profile's best, and changes none of them.

## The Flatpak

The Flatpak's sandbox reaches the Discord app's socket through two
permissions in its manifest, as Flathub's games have them:
`--filesystem=xdg-run/discord-ipc-0` (a native or AppImage Discord's) and
`--filesystem=xdg-run/app/com.discordapp.Discord:create` (the Flatpak
Discord's).

## The application

What a friend reads as the game's name ("Playing Cyberworld Endless") is
the name of an application on Discord's developer portal, whose id the
handshake names (`DISCORD_APP_ID` in `discord.h`): the owner's
**Cyberworld Endless**, `1572683382142410772`, registered on 18 November
2026 with the game's icon (`web/assets/icons/512.png`) as its app icon.
Its name and icon are changed there, never in the game. A build with the
id empty shows nothing, Options' line included. The id is public, no
secret: every game's presence names its own.

## Left out

- The handhelds, the consoles, the phones and the browser: no Discord app
  runs beside them (a PortMaster handheld's build has no pipe at all).
- Join and spectate (Discord's party, secrets and invites): the Net
  Square's visit and the shared dive could offer a seat to a friend
  later ([Online play](../online/overview.md), [the shared dive](../online/shared-dive.md)); nothing here reads a
  friend list or sends a request.
- Pictures per area or per guardian: Discord's art for each state is a
  best practice ([bp]), and they would be original art on the site; the
  game's icon stands for every state until then.
