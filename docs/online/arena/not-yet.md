---
type: Status
title: "Not yet"
description: "What the Arena has not yet done or tried: two a side's open points, teams of three, battle sound, NaviCust effects, rollback between devices, the stands on phones, Battle mode, the browser with a screen."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T17:15:21Z }
---

# Not yet

- Two MegaMen a side ([Two MegaMen a side](two-a-side/overview.md)): over
  the server ([Over the server](two-a-side/over-the-server.md)): a
  partner's picks bound by BN6's screen's
  ten slots, not by his Custom level's count (`0x0802A408`), and the
  code rule his messages are checked by assumed from the series, BN6's
  routine for it not traced ([His messages
  checked](two-a-side/messages-checked.md)); a partner's ColonelSoul's button pressed
  ([A partner's UNITE](two-a-side/unite.md)),
  the Dark Soul's chip lists his own folder's, the green flash's tint a
  MegaMan's own, a hit's mood gain the attacker's own; a chip of
  computed power in his hand (his hand takes a computed chip's power as
  0), WhiCapsl's and Uninstll's marks; the bytes past a Program Advance's
  end in his hand (BN6's built hand leaves the joined picks' last chip
  there, ours the hand's end: nothing reads past it, not checked by
  experiment); the screen copy's cost at the later turns (some 20 ms
  where the first is 0.14), and its later loads seen at up to 97 ms after
  a long wait; whether 30 seconds feels fair to the three who wait
  (`TEAM_TURN_LIMIT`, the owner's to lower); an attack of his
  seen asking his record by object (built, none asked yet).
- Team NetBattles ([Team NetBattles](teams.md)): teams of three
  (6 of a channel's 8); a heal on a carried HP, which the relay starts
  without (watch whether anchors sweep); partners' fixed phrases to each
  other during a bout; the referee's own count of team matches (he
  calls each bout, and each player's last bout as ever); the panel and
  the card on a handheld and in a browser, and a team match through the
  deployed server; whether four players gather in a channel (the
  online statistics can count it).

- The battle's sound: the scene plays the player's core only where the
  game's own core has made its sound ring (`arena_pair_sound`).
- The NaviCust's programs are written as the board; BN6 makes HP from
  the base HP and the board again in the menus (seen: the max HP alone
  was put back to 100), but the other programs' effects in a NetBattle
  are not yet checked one by one.
- After the battle the engine stops once both cores have left it; the
  game's own way back to the Comm menu is not shown (the result card
  rises from BN6's fade instead).
- Issue #167's third and fourth steps ([A rematch in a
  second](from-square/rematch-in-a-second.md), [The Square before a
  NetBattle](from-square/square-before.md)): seen through a local
  server on one machine; not yet over a real network or on a handheld.
  Which sound BN6's PET rings with for a call is not yet checked (the
  ring is BN6's effect `0x7E` for now); a question opened under another
  screen (the NetPass's stamp moment) is told seen as it opens. A
  spectator goes the whole way in for a rematch (his pair kept from the
  bout before would do, untried); a Team bout keeps no pair for the next
  (its MegaMen differ). The challenged player's game still waits for a
  chat box to close before the question (the research's "opens over
  reading" not done).
  The panel and the card on a handheld's screen and in the browser are
  drawn by the same code but seen headless only; the screens' sounds are
  BN6's own effects mixed over the held game's sound (`audio.c`), not yet
  heard on a device.
- Over a real network: measured on the deployed server (19 October 2026,
  [Online: the Net Square](../overview.md)) before the smoothed delay: delay 7, two full-rules
  battles to their end with every hash agreeing, the leading game waiting
  about one frame in eight (issue #128). The smoothed delay is measured
  through a local relay ([Smoothed delay](smoothed-delay.md)), not yet on
  the deployed server nor between two devices; a stall shows as the
  link's badge and, past a second, a note naming whose PET it waits for.
- Issue #167's quick wins ([Quicker on the
  wire](rollback/quicker-on-the-wire.md)): a capture of whether WinHTTP
  and iOS's WebSocket hold small sends back; the rollback's sound heard on
  a device; BN6's link battle's own three frames of input delay over a
  single battle's (measured; shortening them is the owner's call); the
  time sync's notes on lopsided links between devices.
- Rollback ([Rollback](rollback/overview.md)): on in every battle, measured through
  a local relay and on the Nova headless; not yet a NetBattle between two
  devices or through the deployed server (which runs an older protocol
  until it is redeployed with v9), nor on Android, iOS, Windows or macOS. A first NetBattle on
  a system asks with a window of 4 before its frame's cost is kept.
  Mispredictions are guessed as the last keys held:
  a press seen late shows as the other MegaMan's step snapping back.
- The Souls under full rules ([Full rules](from-square/full-rules.md), above): all of
  issue #130's seen on both GBAs. Not yet: a NetBattle in which a
  MegaMan grows worried by hits alone (the runs held his mood worried),
  ToadSoul's flag on BN5's sea and FloatShoe over a cracked panel (a
  NetBattle's field has neither; both flags were read set on both GBAs),
  and BN5's after-effect rule for a DarkChip where its battle's bit 0x40
  is set, if BN5's own NetBattles set it.
- BN5's own link battle as a reference: the differential runs one guest
  core and no cable, so MrkCan1 against a stepping MegaMan is compared
  against the Dark Soul (`chip_mrkcan1_darksoul`), not against BN5's
  NetBattle.
- The NetPass's rule in the Arena ([One rule](from-square/one-rule.md)):
  every proof above with a game without BN5's ROM in it predates v9;
  what v9's rule was seen doing between games: (to be filled).
- The stands ([The stands](stands.md)): watching on Android and iOS
  (built, untried), and a catch-up's pace in a window and on a handheld
  (measured headless, eight frames a tick); a spectator too slow to
  catch up (the refusal and the referee leaving that battle out are in
  the code, not seen); the report's chips past one a hand.
- Battle (not Practice) and its chip forfeit, the pack
  (it stays home), and Android and iOS, which build the Arena but are
  untried. The browser battles (seen against another browser and a native
  game through a local server); the New 3DS builds the Arena but cannot
  run its pair in time ([The New 3DS](rollback/new-3ds.md)): two CPU cores for
  the two GBAs is the way left, and needs a cable driver of our own.
- The browser: a NetBattle seen against another browser and against a
  native game through a local server, in headless Chrome; not yet in a
  browser with a screen, on a phone's browser, against the deployed
  server, or with a tab sent to the background mid-battle (its frames
  stop there, and the other game waits as for a phone sent to the
  background).
