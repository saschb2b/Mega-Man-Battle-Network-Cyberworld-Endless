---
type: Status
title: "What has been shown"
description: "The dated record of what online play has been shown doing: visits, trades, the trade board, NetBattles across networks and systems, full rules and Souls, rematches, the stands, the NetPass, dropped links, emotes, crowds."
tags: [online]
generated: { by: claude-code/unrecorded, at: 2026-10-23T14:25:12Z }
---

# What has been shown

On this machine, 18 October 2026, the player's Gregar ROM, a local v3
server:

- **The gate and a visit.** The first step's questions (the name field,
  what the server learns, MegaMan's question), the save at the gate, the
  connection, the Square built and entered; the keeper's channel switch
  (the server logged it), the L menu, a card read, R home and the pad
  home, each back on the gate with MegaMan's word. The Square drawn in
  Central, Sky and Green Area.
- **A trade between two games.** Two `tools/play.py` sessions with spare
  chips: an offer picked on the order screen, answered, both Deals, the
  line in `trades.jsonl`, `trade.pending` on both; one game settled on
  its way home, the other, closed in the Square, at its CONTINUE, each
  folder and pack as the trade said.
- **The trade board** (19 October 2026, protocol v5, a local server, two
  host builds each with BN5's ROM beside BN6's, `tools/play.py` from
  `--scene home --run-depth 7 --seed 3 --dev quiet`; spares from the
  `chip` dev step: Alpha Cannon A, Beta FireBrn1 F and Cannon B). Alpha
  alone in the channel at the clerk: Offer a chip, Trade board, How it
  works; "THE TRADE BOARD! NO OFFERS UP YET!", Post an offer, Cannon A on
  the order screen, Fire chip from the wants, "IT'S UP! Cannon A FOR A
  FIRE CHIP!". Beta at the Board: "Here now: Alpha", "Alpha has Cannon
  A,wants a Fire chip."; at the clerk "1 OFFER UP!", "Alpha HAS Cannon A!
  WANTS A FIRE CHIP!", Answer it, and the order screen listing FireBrn1 F
  alone (Cannon B, his other spare, left off). Alpha heard "Lan! Beta took
  our offer! FireBrn1 F for our Cannon A!", both said Deal, the server's
  `trades.jsonl` held `[20,5]` for `[1,0]`, the clerk then said "YOU
  TRADED ALREADY! ONE TRADE A VISIT!", and each game settled at home:
  Alpha's "the Pack's 4 to 3 of the one, 0 to 1 of the other", "Lan!
  FireBrn1 F is in our Pack now!", Beta's Cannon A. Then a BN5 chip's
  offer, which a game without BN5's ROM is not sent (so since v9 too):
  Alpha back with MrkCan1 M (411, the pack's extension)
  posted it from the clerk's chat-box list ("IT'S UP! MrkCan1 M FOR
  ANYTHING!"), and Gamma, a third game with BN6's ROM alone (the
  server's join line `"roms":0`), read the Board beside him: "Here now:
  Alpha", "Trade board: no offers up.". Found on the way:
  alone in a channel, the clerk's words were never written (his first
  state's key was 0, the empty archive's), so the desk said nothing
  until a second player came; fixed. The server's tests hold the rest:
  a BN5 chip's offer kept from a game without BN5 and refused from one,
  an offer gone with its poster, a take of a chip not the one posted,
  the post's second, a session's second trade.
- **A NetBattle between two games.** The challenge, the way in, the
  battle to its end on both with every hash agreeing, a desync and a
  player leaving each stopping both, a declined challenge
  ([The two-client proof](arena/from-square/two-client-proof.md)).
- **The Square's second plan and its life** (the same day, later). The
  Square through real visits in all three areas (seeds 120, 121, 122:
  Central with the statue, Sky with its stage and ring raised and
  MegaMan walked up both stairs, Green with the giant cybertree); in two
  games at once: the greeter's "ALPHA LOGGED IN!" over his head as the
  second arrived, the Board's posts naming the channel, who was there and
  the log-in, the referee's words, a walker spoken to (she stopped and
  turned), both games' walkers at the same positions at the same clock
  seconds, both games cheering at once when their players said Hello
  round the statue (the "!" over the walker nearer it), Trade Central's
  desk answering from its wing (to "no spare chips", the run having
  none), and a Practice NetBattle asked for on the Arena's spectators'
  floor, both cores agreeing frame by frame; and a run timed to a
  parade, the two walkers marching round the statue past MegaMan, two
  panels apart, at the positions the clock gives.
- **The official server.** Redeployed from `deploy/coolify/compose.yaml`
  on the same state volume: `/health` says protocol 3, trading and the
  Arena on, and a `cyberworld.presence.v3` upgrade is answered (101). A
  headless game went through the gate to it over TLS, built the Square
  from its seed (120, Central Area) and came home by R to "We're
  home,Lan!" (it is a test instance until a release ships online play).
  The transport's checks below were against its v2.
- **A NetBattle across a real network** (19 October 2026, issue #129).
  Two host builds, each with BN5's ROM beside BN6's, met on the deployed
  server (protocol v5) and fought two Practice NetBattles by full rules
  through it: the server gave a delay of 7 frames from their latencies,
  both pairs made the same ready hash, and each battle went to its end on
  both games with every hash agreeing (17 and 25). The game whose ticks
  ran ahead waited on the other's keys for about one frame in eight (137
  of 1,080 frames, then 127 of 1,540), where one machine's runs waited on
  none: what issue #128's smoothed delay is for ([Smoothed delay](arena/smoothed-delay.md), Smoothed
  delay: through a local relay at that delay, 16 to 24 waits in 100 frames
  before, none after the first five seconds since).
- **The browser** (the same day, issue #129). `tools/online_browser.html`
  served from the repository, a local v5 server allowing its origin and a
  native game in the Square: the browser build went through the gate,
  saw the native game's MegaMan and his name, and R took it home to
  "We're home,Lan!". It could not before: its HELLO's buffer was still
  v3's 52 bytes, and v4's HELLO is 76, so it never opened its WebSocket
  (`online_browser.c`, now sized by `ONLINE_HELLO_BYTES`).
- **Builds and checks.** Host, aarch64, Windows, 3DS and browser builds
  with `-Werror`; `build.py test` (the Square's plan and hash, the
  Arena's wire and links) and the server's 36 tests; `build.py lint` with
  nothing new.
- **The pair's common ground, before v9** (the same day, protocol v4, a
  local server; two host builds, one with BN5's ROM beside BN6's or
  both). Two games with BN5: one offered MrkCan1 M (411, the pack's
  extension) from the clerk's chat-box list, the other answered with
  WideSht1 L (204, a blank record) from the order screen, both said Deal,
  `trades.jsonl` held `[411,12]` for `[204,11]`, and each game's CONTINUE
  settled it: "the Pack's 2 to 1 of the one, 0 to 1 of the other" (BN6's
  TakeChips and GiveChips on both kinds of id), MegaMan's "Lan! MrkCan1 M
  is in our Pack now!". Mixed pairs, which v9 no longer allows (issue
  #131, The NetPass above): the clerk's "THAT NAVI TRADES BN6 CHIPS!"; a
  NetBattle by BN6 rules with Folder2 carried (26 hashes agreeing), and
  one with Folder1's two MrkCan1 M stood in for by Pack chips (25). v9
  removed BN6 rules and the stand-ins with them.
- **A NetBattle by full rules** (the same day, a local server, two host
  builds each with BN5's ROM beside BN6's). "Full rules! Let's see if
  Beta takes us on!" and "Full rules: Souls and BN5 chips too!"; both
  games made their pairs with the same installs' record (1,913,709
  bytes and 132 hooks) and the same ready hash; Alpha's MrkCan1 M took
  Beta from 100 to 30 on both screens and the battle went to its end with
  every hash agreeing. Alpha holding every Soul and a folder of
  JustcOne: UNITE on the Custom screen lit KnightSoul, and on both
  screens KnightMan's summon, Alpha in KnightSoul with his three turns and
  his wrecking ball's hits, the battle to its end with every hash
  agreeing. A desync stopped both; a game whose installs differed (one
  core made without BN5's families) stopped both before the first frame
  ([Full rules](arena/from-square/full-rules.md)).
- **Every Soul in a NetBattle** (19 October 2026, issue #130, the same
  harness). Each of the six Souls united by the challenger, then two at
  once (ShadowSoul and TomahawkSoul, NumberSoul and ToadSoul, KnightSoul
  and ColonelSoul, the second player's at the second Custom screen): the
  Soul's Navi summoned and MegaMan in the Soul on both screens, its
  charged shot (ScreenDivide, the slash, the dice bomb, the swing over a
  field of grass, the wrecking ball, the paralysing notes) hitting on
  both, every hash agreeing. Chaos Unison (ColonelSoul's DrkSonic,
  NumberSoul's) and ColonelSoul's button (its Cannon fired as his
  charged shot) too, once two things only one GBA had written were
  carried or marked on both: the Soul's end and the button's chip.
  The ready apart because BN5's installs differ now says so on both
  games ("Huh? Our PETs don't match on BN5...").
- **The rest of the Souls in a NetBattle** (19 October 2026, issue #130,
  the same harness and a full-rules `--dev arena`). A failed Chaos
  Unison: the Dark Soul at the back of the other player's area firing at
  the one who failed, on both screens, once each side's chip list for it
  is made on its own player's GBA and given to both as the battle begins
  (before, the other GBA made it from the wrong folder and the two GBAs'
  battles went apart, both clients' hashes agreeing). Chaos Unison's
  attack for the other four Souls (DarkInvs, DrkLance, DarkDril,
  DarkWide); ShadowSoul's FloatShoe and ToadSoul's flag set on both
  GBAs as each unites; NumberSoul's hand of ten, a CrakShot from its
  second row dealt with its +10; KnightSoul's chip charge (JustcOne
  doubled to 440); a worried MegaMan offered DarkDril at the hand's end,
  picked and used, dark on both; MrkCan1 locking on a MegaMan who steps.
  Every hash agreeing, and the two GBAs of each pair the same, frame by
  frame ([Full rules](arena/from-square/full-rules.md)).
- **A rematch** (19 October 2026, issue #128, a local server, two host
  binaries). After a battle both MegaMen asked "Again with NAME?"; both
  Yeses started the second battle over the same challenge (the second
  game's question held it until its Yes), with the first battle's pair
  hash, to its end, all 36 hashes agreeing; the referees called each
  bout and then 1 to 1. A No (B) turned the other's Yes down at once
  ([The two-client proof](arena/from-square/two-client-proof.md)).

- **The stands** (20 October 2026, issue #128, protocol v7, a local
  server, three host builds: two players and a spectator). A Practice
  NetBattle watched from the referee's "WANNA WATCH?" to its end on the
  third game: its pair built there agreeing with the players' ready, 41
  hashes checked against the players' 41, the end at the same frame
  (2,506) on all three; a spectator who came 28 seconds late caught up
  from 1,686 frames behind and watched it live to its end, 41 hashes
  checked; a battle by full rules watched to its end by a game with
  BN5's ROM, its pair made with the players' installs' record, 51 hashes
  checked; a spectator whose copy of the keys was changed stopped by
  itself, saying so, while the players fought on. Three cheers from the
  stands, held from the players, on the Board and in the referee's word
  to the winner after. A battle by full rules called by the referee on a
  game without BN5's ROM, the report alone; the browser build, which has
  no Arena, given the same report live (HP, MiniBomb used)
  ([The three-game proof](arena/stands.md#the-three-game-proof)).

- **Android and Windows** (19 October 2026, protocol v5, a local server
  with seed 120, then the deployed test server once each; issue #129).
  *Android*: the release APK (`build.py android`) in the emulator
  (Android 14, x86-64), BN6's ROM given through the ROMs screen's folder
  picker; NEW GAME, the jack-in at Lan's PC and the touch D-pad to the
  gate's keeper: Another server, `ws://10.0.2.2:8929/presence` typed
  with Android's keyboard, "DONE! IT LEADS TO 10.0.2.2:8929!". On the
  gate "WAIT,WAIT! FIRST, YOUR NAME,PLEASE!", the name field, the
  keeper's question, Yes, and the Square, a host build's MegaMan
  ("Linux") standing in it and each game showing the other's name; the
  D-pad walking there, his card ("HP 100 of 100. Act 3,layer 7!", "His
  PET holds BN5 chips too!"), a Wave ("o/" over MegaMan's head), R and
  Yes home to "We're home,Lan!". Then the keeper's Official Square, his
  questions again for that server, the deployed server over TLS (its
  `/health` counting the one player) and home again. Found on the way:
  the name and address fields were cut off at both sides on a phone held
  upright (the menu canvas is 160 across there and the field 240), and
  Android's keyboard covered their lower half. Fixed: the field asks for
  a canvas 240 across (`platform_own_wide`) and stands in the upper part
  of an upright screen, whole above the keyboard, its keys answering
  taps ("GOT IT! HELLO, Droid!").
  *Windows*: the .exe (`build.py windows`) under Wine 10 headless, its
  data folder's `network.ini` naming the local server: the gate, the
  Square built from seed 120 with the host build's hash (3354384409), R
  and Yes home to "We're home,Lan!", through WinHTTP's WebSocket.
  Then the deployed server over TLS (WinHTTP's own): the same Square and
  hash, and home again. Its first try failed, "Windows WebSocket
  connection or TLS verification failed": Wine's root store was empty
  because the test's Wine prefix had never been written (its container
  ended before wineserver saved the registry, so no crypto provider was
  registered). A prefix made with `wineboot -u` and saved with
  `wineserver -w` fixed it; the game's code was not at fault.
- **A trade and a NetBattle on Android and Windows** (19 October 2026,
  protocol v5, a local server with seed 120, issue #129), each against a
  host build ("Linux", BN6's and BN5's ROMs) in `tools/play.py`; each
  game with BN6's ROM alone, so a mixed pair (before v9, which leaves
  such a game the Square and no trade or NetBattle), its run made by a host
  build with spares from the `chip` dev step (FireBrn1 F, Cannon B) and
  saved at the gate, then copied into its data folder (Android's with
  adb as root). The Square's places were read from a third host build's
  view of its peers.
  *Android*: the release APK in the emulator (Android 14, x86-64, KVM,
  its GPU in software). The trade board: Linux posted Cannon A for a Fire
  chip; Droid walked to Trade Central by the touch D-pad, "THE TRADE
  BOARD! 1 OFFER UP!", "Linux HAS Cannon A! WANTS A FIRE CHIP!", Answer
  it, the order screen listing FireBrn1 F alone (Cannon B left off),
  "Lan! Droid took our offer!" on the host, both Deals, `[20,5]` for
  `[1,0]` in `trades.jsonl`, then the clerk's "YOU TRADED ALREADY!".
  The app, closed in the Square (reinstalled), settled at its CONTINUE
  ("Lan! Cannon A is in our Pack now!", the Pack's 2 to 1 of the one, 2
  to 3 of the other), the host build on its way home (4 to 3, 0 to 1,
  "Lan! FireBrn1 F is in our Pack now!"). A Practice NetBattle by BN6
  rules ("His PET holds BN6 chips only."), delay 6: the same ready hash
  on both (`919efa43fb0d0db9`), the battle to its end at frame 2,809 with
  all 46 hashes the same, Droid the winner. Through the battle the
  emulator's game played 59-61 frames a second, both pair cores in each
  (a frame's update 3.8 ms, 4.5 at most, its longest frame 11.3 ms); the
  emulator's software GPU showed 51-59 of them (its present 14 ms), a
  phone's shows them all. Droid, ahead, waited on the host's keys 219
  ticks of 2,809 (one in 13); the host none.
  *Windows*: the .exe under Wine 10 in a window (Xvfb, the keyboard by
  xdotool). Linux offered Cannon A from the clerk's Offer a chip; Win
  heard "Lan! Linux offers us Cannon A!", walked to the desk, Offer one
  back, FireBrn1 F, both Deals (`[1,0]` for `[20,5]`), and each settled
  on its way home (Win's "Lan! Cannon A is in our Pack now!", 2 to 1 and
  2 to 3; the host's 3 to 2 and 1 to 2). The NetBattle by BN6 rules,
  delay 6: the same ready hash, the battle to its end at frame 2,781 with
  all 46 hashes the same, Linux the winner; 60 frames shown and played a
  second through it (update 4.5 ms, 5.5 at most, the longest frame 11.9
  ms), the way in's five seconds down to 29 shown a second while the pair
  cores boot (its longest frame 110 ms). Win waited 717 ticks, 600 of
  them the ten seconds the host build was held still mid-battle on
  purpose; the challenger gone before the battle began left Win's
  "Huh? They're gone...".
  Found and fixed on the way: Android threw the game's own output away,
  so none of the online, Arena or frame lines could be read there; it
  now goes to logcat (`startup_output`, android/README.md). And a game
  held still for four seconds mid-battle (the host build between
  `tools/play.py` batches; a phone's app sent to the background) stopped
  the battle on both, twice on Android: "my keys could not be sent" and
  "the other player's keys came out of order"; the waiting game now
  stops sampling keys 128 frames ahead and waits
  ([Lockstep](arena/what-runs.md#lockstep)), which the Windows
  battle above survived. And on the host build's second visit in one
  process its Offer a chip's order screen said BN6's own "I bought a new
  chip! "Cannon A"!!" in Lan's box where the desk's "We'll offer" belongs
  (a word only: the trade settled 3 to 2). The Order Service's script 7,
  MegaMan's word after the Yes, begins with BN6's `EF 00` on flag `0x171D`
  (`BN6_FLAG_NAVI_IN_PET`) and goes to script 20, Lan's, where it is
  clear; `trade_order.c` now says script 20 as its 7. Seen with the flag
  cleared by the `flags` dev step at the desk: "We'll offer "FireBrn1
  F"!" in MegaMan's box. Why the flag was clear on that visit is not
  known yet.
- **The week's net** (19 October 2026, protocol v6, a local server, the
  Linux build in two `tools/play.py` sessions, Alpha from `--scene home
  --run-depth 7 --seed 3`, Beta from `--run-depth 4 --seed 4`, both with
  `--dev quiet,online=...`). Alpha at the greeter: his welcome, "THE NET
  SHIFTED! THIS WEEK'S DEEP NET IS OPEN!", "OCT 19-OCT 25. EVERY NAVI
  DIVES THE SAME NET!", "DIVE IN NOW?" with the cursor on No; Yes, "OFF
  YOU GO! HEAD HOME AND JACK IN!", the visit's end (12) and the log's
  "a dive of week 2963 begun (seed f521fe36)", MegaMan in Lan's room with
  Dad's call; the dev menu's "Depth 1 seed 4112645686", the server's
  week's seed. The normal run's twelve files, hashed after the gate's
  save, unchanged while the dive was played to layer 4 (the dev menu's
  Next layer) and the game was closed there. Opened again: the title's
  CONTINUE "Home", the normal run at the gate; at the greeter "YOUR DIVE
  INTO THE DEEP NET ISN'T OVER!" and Keep diving, Start over, Not now;
  Keep diving took the dive up on layer 4 (RoboDog Comp). On layer 5,
  MegaMan deleted in a battle; the log's "the dive of week 2963 ended on
  layer 5", the title's summary, the title's CONTINUE again the normal
  run's ("Home"), its files hashed before the dive taken up unchanged.
  The next visit: "the week's dive to layer 5 posted", `weeks.jsonl`'s
  line (week 2963, layer 5, act 2, two minutes), and the greeter's "YOUR
  DIVE TO LAYER 5 IS UP ON THE BOARD!". Beta, a second game, at the
  Board: after its channel's posts, the week's heading with "Oct 19-Oct
  25" and "1.Alpha:5". A second dive from the greeter's new offer ended on
  layer 3, the normal run's files unchanged again; on the final build two
  more, to layer 3 and then layer 2, the second's summary "Reached Layer
  2" and "This week Layer 3", the next visit's post of the first ("the
  week's dive to layer 3 posted", the Board's best for Alpha still 5,
  and the greeter's word on it rightly left out). The saves
  of that data folder, both slots, packed by the game's own carrier code
  (`backup_pack`, 22 files) and installed into a fresh folder
  (`backup_restore`, status OK): every file the same but `profile.sav`,
  whose volumes are the receiving device's, as designed. Found on the
  way: the greeter's first flag was Trade Central's (`0x1494`, the clerk
  answered); the dive's flag is the Square's one left free, `0x14AF`. The
  transport took no WEEK_POST until it was named among its own messages.
  An autopilot run (15000 frames, seed 1) ended as runs do.
- **The NetPass** (20 October 2026, issue #131, protocol v9, a local
  server, the Linux build in `tools/play.py`: Alpha and Gamma with BN5's
  ROM beside BN6's, Beta with BN6's alone, `--scene home --run-depth 7`).
  The server's join lines named the ROMs (`"roms":1`, `"roms":0`). Beta
  in the Square: START opened the Square pass ("1 of 4", the BN5 slot
  open, "Team Colonel's ROM stamps Arena, Trade and DeepNet"); the
  referee said "NO ARENA STAMP ON YOUR NETPASS!", the clerk "HMM... NO
  TRADE STAMP ON YOUR NETPASS!", the greeter "THIS WEEK'S DEEP NET IS
  OPEN!" and "BUT YOUR NETPASS HAS NO DEEPNET STAMP! SORRY!" with no
  question. Alpha's first visit: the greeter's "WHOA! YOUR NETPASS HAS
  ROOM FOR EVERY STAMP NOW!", the pass with Arena, Trade and DeepNet
  landing one by one, then "★ Full pass", and `netpass.ini` saying
  `stamps=4`; Alpha's and Gamma's next visits had no moment. Alpha
  reading Beta's card: "His NetPass has just the Square stamp." and Wave,
  Mute, Block, no Battle; Gamma reading Alpha's: Battle first. Gamma's
  challenge, Alpha's Yes, and a NetBattle by full rules (`arenahp=12`)
  to its end at frame 4,944 on both: the same installs' record
  (1,913,709 bytes, 132 hooks) and ready hash, all 82 hashes agreeing,
  rollback on (window 9, delay 2; 469 and 515 rollbacks), Gamma the
  winner. Then a trade between the two: the clerk listing Gamma alone,
  Alpha's spares Cannon A and MrkCan1 M (411, BN5's, in the clerk's
  list), MrkCan1 M offered, Gamma's Cannon A back from the order screen,
  both Deals, `trades.jsonl`'s `[411,12]` for `[1,0]` and
  `trade.pending` on both. A ROM-free client with BN6's identity alone,
  at the same server, was answered NOTICE 28 ("That needs the full
  NetPass") to an ARENA challenge, a TRADE proposal, an OFFER post and a
  WATCH. In a run (seed 7, `--run-depth 2`): the PET's KeyItem listed
  NetPass with its description, A opened the pass, B closed it back to
  the KeyItem screen. The gate's strip over MegaMan's question in both
  states, the second screen's NETPASS panel in the Square, and the pass
  at 240x160, at 640x480 and on a phone held upright (1080x1920, the
  touch controls under it) captured headless.
- **Trade Central's two screens** (20 October 2026, issue #145, a local
  v9 server, the Linux build in `tools/play.py`, Alpha and Beta each with
  BN5's ROM beside BN6's, `--scene home --run-depth 7`, spares from the
  `chip` dev step). Alpha at the clerk, Offer a chip, Beta: the spares
  list "Offer a chip", "To Beta", Cannon A and FireBrn1 F with their
  counts and Cannon's card; FireBrn1 F picked, "SENT!". Beta heard the
  offer, answered at the desk with Offer one back: the list "Offer one
  back", its note "ALPHA OFFERS FIREBRN1 F", MrkCan1 M (411, BN5's)
  picked, and the chip trade at once, You MrkCan1 M, Them FireBrn1 F, the
  cursor on No deal. Alpha heard "Lan! Beta answered us!", then the same
  pair the other way round; Alpha's Deal! ("Waiting on Beta...", B to call
  it off), Beta's Deal!, the cards crossed on both and landed,
  "Traded!"; `trades.jsonl` `[20,5]` for `[411,12]`, `trade.pending` on
  both the moment it was done, then "Deal! FireBrn1 F is ours once we're
  home!" and each game settled on its way home (the Pack's 2 to 1 and 0
  to 1 on each). The board, the next visits: Beta posted MrkCan1 M from
  the list ("Post an offer", "Trade board") for a Fire chip; Alpha, with
  FireBrn2 S given, read "Beta HAS MrkCan1 M! WANTS A FIRE CHIP!",
  Answer it: the list "Answer an offer", "SPARES THAT FIT: A FIRE CHIP",
  FireBrn1 F and FireBrn2 S alone (Cannon A and MrkCan1 M left off);
  FireBrn2 S taken, Alpha's chip trade at once, Beta's after "Lan! Alpha
  took our offer!", Beta's Deal! first this time, the swap on both,
  `[21,18]` for `[411,12]`, both settled at home. The screens captured
  headless with the `trade` dev step ([Dev tools](../devtools/overview.md)) at 240x160,
  640x480 and a phone held upright, BN6's FireBrn1 F and BN5's MrkCan1 M
  on the cards, every sort and the swap's frames. And the game font's
  narrow glyphs: BN6's l, i, j, r, I, 1, !, :, ; and the brackets stand in
  their 8-pixel cells with blank columns on their left, which the
  engine's letters, drawn at their advance, read as a gap before them
  ("Team Co lone l's ROM" on the NetPass); each now starts one column in,
  as the round letters do (`gfx.c`), seen on the pass with BN6's ROM
  alone.

- **A dropped link** (21 October 2026, issue #153, protocol v10, a
  local server, the host build in two `tools/play.py` sessions, Alpha
  through `tools/link_relay.py` (a TCP relay that holds every byte for N
  seconds, `cut N`, or closes every link and refuses new ones, `close N`),
  Beta straight to the server with `CYBERWORLD_ONLINE_TRACE`; both ROMs).
  A five-second **cut**: Alpha's wait from 2.5 seconds, then "Back
  online!" on the same session; the server saw no drop, and Beta's trace
  had Alpha in all of its 120 frames over those 14 seconds. A five-second
  **close**: the server held Alpha's place ("hold"), his tries were
  refused for five seconds, then "resume" with the same session ID and
  channel, and Beta's trace had Alpha in every one of its 150 frames. A
  forty-second cut: the wait's bar ran down to 0 and Alpha came home to
  "Whoa! The link to the Square broke!"; the server held the place from
  its 15 seconds of silence and let it go 40 seconds later, Beta seeing
  Alpha stand there 55 seconds in all, then leave. B on the wait: home at
  once, the same word. The server restarted (twice) under two games in
  the Square: both back in the same Square (seed 120) on new sessions.
  The server restarted with `maintenance = until 14:00 UTC`: Alpha's
  tries refused, home to the keeper's "SORRY! THE SQUARE'S UNDER
  MAINTENANCE!". A NetBattle (Beta's game taking it and playing by itself,
  `arenaboth`), Alpha's link closed 14 seconds in: "The NetBattle
  stopped: the connection to the Square was lost" on Alpha, the wait,
  the resume; "the other player left the NetBattle" and "Huh? They're
  gone..." on Beta, both back in the Square, fine after. A trade offered
  to Alpha by a ROM-free client (Gamma), Alpha's link closed with the offer
  open: Gamma told the trade was off at once and never saw Alpha leave;
  Alpha's resume brought the server's word on it, "Aw... The trade's
  off." The browser build (`tools/online_browser.html` in headless
  Chrome, its gate at the relay, the server allowing the page's origin):
  a five-second close showed the wait and took the held place back (the
  same session, "resume"); a 45-second one came home after its 30
  seconds. Found on the way and fixed: a stall's backlog of positions, let
  through at once, tripped the server's rate limit (now a bucket of their
  own, and fewer sent while nothing is heard); the new bucket refilled only
  between two positions, so a NetBattle's keys drained it (refilled by
  any message now); and a trade offered on the new session was called off
  by the resume when its mail was read first (the resume is taken up
  before the new session's mail, and the last session's unread mail set
  aside). The wait captured at 240x160 and on a phone held upright
  (1080x1920, the touch controls under it) with the `reconnect` dev step
  ([Dev tools](../devtools/overview.md)). The server's tests (`server/tests/test_holds.py`)
  hold the rest: the place held and taken back, its end, a goodbye, a
  resume with nothing held, a new visit while held, a trade off and said
  again, a trade committed as the link dropped coming home, one trade a
  visit across it, a NetBattle ended for the other, a challenge to a place
  held refused, a moderator freeing one, a stall's backlog, maintenance.

**NetBattles in the browser** (20 October 2026, a local v9 server,
headless Chrome, the harness in `.build/`): two browser tabs fought a
full-rules NetBattle to its end, 28 hashes agreeing, one tab rolling
back 39 times; a tab against the host build, the same; and a tab
watched two host builds' battle from the stands, catching up from its
first frame and checking all 60 of their hashes
([In the browser](arena/rollback/browser.md)). The New 3DS ran
`--dev rollbench` and a battle playing itself over 3dslink: its pair
frame is past a 60th of a second, so it offers none
([The New 3DS](arena/rollback/new-3ds.md)).

**The emotes** (22 October 2026, issue #176, protocol v14, the player's
Gregar ROM and Team Colonel's, a local server with seed 120's Square,
three to five host builds headless at once from `--scene home
--run-depth 7 --seed 3`, scripted by the wall clock as
`tools/trailer_online.py` runs a trailer's games, the dev step `emote K`
beside the player's keys; the harness and its pictures kept in
`.build/`):

- **Every emote on every screen.** Beta played the twelve, three seconds
  apart: Alpha's and Gamma's games each began all twelve on Beta's
  puppet and played its sound (their logs name each emote and its BN6
  sound id), and Beta's own MegaMan played each through `F8 01` with no
  chat box, his own facing and animation back after it (Spin's last step
  his own facing: `F8 04` would have turned him). Contact sheets of each
  emote's frames, on a peer and on MegaMan himself, are the issue's.
- **In step.** Each emote began within 15 ms of the wall clock on every
  game that showed it (four games, nine emotes); five MegaMen pressing
  Cheer over 200 ms began on three beats a quarter second apart.
- **Wave and wave back.** Alpha's Wave at Beta, faced: on Beta's screen
  the hand over Alpha's name; Beta's L and up answered it two seconds
  later, and all four games played the answer's "!!" between the two and
  BN6's sparkle. Alpha's next Wave and Bow at Beta, unanswered, reached
  everyone; his third reached Alpha alone (the cap).
- **The burst and the quiet.** Gamma's five favourites in 1.3 seconds
  sent four (three at once, one a second after), all seen; Delta's L,
  More, Quiet please hid everyone's emotes from then on (none begun on
  his game, none sounded).
- **The Square's answer.** Two MegaMen round the landmark: the walker's
  "!!"; then five within a second: the Square cheered for 2, 3, 4 and 5
  in turn (the walker's cheer and BN6's pops from three, the greeter's
  call from four).

On this machine, 23 October 2026, a local v15 server and `SQUARE_MAKE` 3
(`tools/square_tour.py`; seeds 120, 121 and 122, the Square in Central,
Sky and Green Area, three games each):

- **The hub and its parts.** Mira placed in the Arena's stands and Dex at
  Trade Central's desk; Kei arrived in the welcome plaza and read, from
  the arrival, "TRADE CENTRAL: Dex" at the screen's upper left, "ARENA:
  Mira" at its upper right and "STAGE" at its lower left, then was run by
  the pad to each warp's gem, beamed across with the camera riding along,
  landed on the gem's rim facing the part's own (the desk and its
  clerks, the ring's stair, the stage, the GirlNavi on its floor) and
  stepped back onto the gem to land in the plaza again, the timings in
  [The layout](square/layout.md). At the Arena's braziers MegaMan said "Lan,look! The
  Arena's over there!", "Navis NetBattle in its ring!" and "Mira's over
  there!"; L's Look around named each warp's way and who was where.
- **A warp that bounced.** The first build landed MegaMan on the gem's
  middle: BN6's step off it ended inside its trigger, and the pair beamed
  him to and fro every 150 frames until he moved. Landing on the rim
  ended it ([The net's maps and their pieces](../rom/net-maps.md), Teleports); the unit tests hold every
  landing on a screen diagonal.

The transport itself was measured on 15 October 2026, before the gate
replaced the first Network screen (those proof tools went with it; they
are in the history before 433865a8):

- **Hosted WSS:** two native games on the Coolify server with a trusted
  certificate, median round trips 33 and 39 ms; a browser at the
  published site's origin; moderation through the container's command
  line; an identity kept across a redeploy.
- **Many players:** sixteen native games in two full channels, 10,248
  remote positions matching the interpolation; eight ROM-free wire
  clients at about 40 KB/s together.
- **Systems:** a Retroid Pocket Nova on ROCKNIX against the public server
  (median 40 ms); Windows Server 2022 and macOS 15 running the transport's
  sources in CI against the public server; the Android APK in an
  emulator; an untrusted certificate refused on each, with no plaintext
  fallback.
