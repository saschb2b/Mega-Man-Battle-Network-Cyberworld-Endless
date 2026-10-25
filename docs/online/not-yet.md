---
type: Status
title: "Not yet"
description: "What online play has not yet done or tried: emotes, the strip and palette, a dropped link, the week's net, the NetPass, Trade Central's screens, the Arena on the New 3DS, rollback, the stands, hardware."
tags: [online]
generated: { by: claude-code/unrecorded, at: 2026-10-22T12:28:49Z }
---

# Not yet

- **The emotes, what is left** (issue #176): the mapping is proposed for
  the owner to adjust ([Emotes](square/emotes.md)), and its sounds were picked by
  their names and BN6's uses, unheard: a listen on a device. Seen
  headless on the host build only (the Windows and browser builds made,
  not played); touch's L held with the D-pad, Android, iOS, the 3DS and
  the handhelds untried.
  The leap (0x43) waits on BN6's hop (`22 11 60 DC 21`) run on a puppet,
  whose loop sets its place every frame. A game whose chat box is open
  at its own emote's beat skips the pose there (the others show it, and
  he may walk meanwhile). Emotes earned from a
  run's milestones and chip stickers (Have, Want, Fave) are the issue's
  Later.

- **The strip and the palette, what is left** (issue #178): played
  headless on the Linux build, three games in one Square on a local
  server (`.build/strip/harness.py`: the strip's pages, a favourite kept,
  a phrase and an aimed Wave sent from it, L's chord sending the kept
  one, a phone's touch flow at 2340 x 1080, the palette tapped at an
  Android display's 413 x 360 with `stap`, each send seen on the other
  two games). The 3DS's touch panel and Android's `nativeTap` are built
  (the 3DS's CIA, the browser's and Windows' builds made), not touched
  on a device; the stylus's tap is read where it was let go. The
  release pictures of the Square (`build.py screenshots online-square`,
  the 0.13.0 cut's `meet` scene) now send through the strip, not yet
  captured again.

- **A dropped link, what is left** (issue #153): played on the host
  build and in the browser, through a local relay; Android, iOS, Windows
  and the 3DS are built and untried, and so is a real network's hiccup (a Wi-Fi handover, a phone
  sent to the background past 15 seconds). A trade committed as the link
  dropped is held by the server's tests, not played between two games. A
  game that gives up (past 30 seconds, or B) cannot say goodbye: the
  others see its MegaMan stand until the server's hold ends (up to 55
  seconds after a stall). A NetBattle's own stall handling is unchanged: a
  link silent past 8 seconds now ends one (15 before). The planned
  restart's countdown and drain (the issue's ideas 1 and 2) are later.

- **The week's net, what is left:** a dive's summary hash is a format
  check, not evidence. A dive won (the Nest) has not been played to its
  end here; the 3DS's and the browser's dives are untried.
  A diver's game calls off an offer made to it, but the other game
  hears only "The trade's off": the snapshot does not say who is diving.
- **The NetPass, what is left:** seen headless on the host and Linux
  builds only (the KeyItem's A in a run, START in the Square, the gate's
  strip, the second screen's panel, the stamp moment, the referee, the
  clerk, the greeter); on the 3DS, Android, iOS, the browser and the
  handhelds it is built, untried. A pass's stamp moment plays on the
  next visit after BN5's ROM is added, but a game that loses BN5's ROM
  and gets it back does not see it again (`netpass.ini` keeps the most
  stamps seen). Touch has no button of its own on the pass (the touch
  controls' A and B close it). A trade's settling at home and a
  WEEK_POST refused for the stamp were not played again here (the
  server's tests hold the refusal).
- **Trade Central's screens, what is left** (issue #145): seen headless
  on the host and Linux builds, keys from `tools/play.py`, and on a
  phone's screen headless the touch controls' D-pad and A picking a chip,
  A on No deal and B declining (`--taps`); on the 3DS, Android, iOS, the
  browser and the handhelds they are built, untried. The spares list has
  no touch of its own on its rows. A trade called off by
  the other player while the list is up for an answer is found only when
  the answer is sent (the server's NOTICE, and MegaMan's "The trade's
  off"). The second screen shows the NETPASS panel meanwhile, not the
  chips.
- **The Arena on the New 3DS:** measured, not offered
  ([The New 3DS](arena/rollback/new-3ds.md)): a pair frame 25.8 ms
  with its picture, 20.7 without, two GBAs on one ARM11 core. What is
  left is the two cores on two CPU cores at once, the third core taking
  one, which needs a cable driver of our own whose scheduling is the same
  however the threads interleave (a new pair hash on every system, a
  protocol version), and even then a frame is at the edge of a tick:
  10.4 ms a core and 5 for the picture. The stands there are the report
  alone. A NetBattle in the browser was seen against another browser and
  a native game through a local server; against the deployed server and
  a phone's or a handheld's game, untried.
- **Rollback, what is left** (issue #128, [Rollback](arena/rollback/overview.md)): a
  NetBattle with rollback between two devices and through the deployed
  server (protocol v9 there once it is redeployed), on Android, iOS,
  Windows and macOS (built, untried; the Nova measured headless); a first
  NetBattle on a system asks with a window of 4 until a battle has kept
  its frame's cost; the guess is the other's last keys held, nothing
  smarter.
- **The stands, what is left:** watching on Android, iOS and the
  handhelds (built, untried), a catch-up's pace in a window and on a
  handheld (measured headless only), a machine too slow to catch up (in
  the code, not seen), and the report's chips past one a hand
  ([Not yet](arena/not-yet.md)).
- **On hardware:** the gate, a visit and a trade on the Nova, the Flip 2,
  a New 3DS, an Android device (the emulator's visit, trade and
  NetBattle above) and iOS, macOS and Windows itself (Wine's above); the
  Arena's smoothed delay and rollback over a real network (issue #128:
  measured through a local relay, not yet on the deployed server); a phone's app sent to the background
  mid-battle, which the lead's cap should now wait out ([What runs](arena/what-runs.md#lockstep),
  Lockstep), untried.
- **In the browser with the published server:** the official server
  allows the site's origin alone, so the browser's visit was checked
  against a local one; a trade and the trade board there are untried.
- **The official server:** its policy, moderator and retention are
  published and its pruning and backups are in the server, tried locally
  (a Compose stack's archive restored byte for byte, 20 October 2026);
  on the Coolify service itself the backup service, a restore, the
  proxy's access log and the switch from `dev` to `main` at the release
  are the owner's ([server/README.md, At a release](../../server/README.md#at-a-release-the-official-server-follows-main)),
  untried; a trade and a NetBattle through it.
- **The Souls under full rules, what is left:** a MegaMan worried by
  hits alone in a NetBattle, ToadSoul's flag and FloatShoe on panels a
  NetBattle's field lacks, and BN5's own NetBattle as a reference for
  MrkCan1 and for a DarkChip's after-effect
  ([Not yet](arena/not-yet.md)).
- **A pending trade across systems:** `trade.pending` stays beside the
  saves, outside the `.cwsave` carrier; a save moved between a trade and
  its settling leaves the swap on the system the trade was made on.
