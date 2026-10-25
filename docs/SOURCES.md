# Sources

What the original games have, do and lack is the community's ground
before it is ours: hardcore players and reverse engineers know BN6 and BN5
better than this project does. A claim about the original games (what BN6
or BN5 has, does or lacks) is looked up here before it shapes a design, a
doc or code, and written down as **verified** (how: a code path, a struct,
a text, an experiment) or **assumed**. A player's claim is a lead, usually
right: verify it, then credit it below.

Issue #64 said "BN6's battles have no code for DarkChips". A player
(CybeastID) answered that BN6 keeps them from the Japan-only Beast Link
Gate; the disassembly agreed, and our own notes had listed the records as
"unused arm and dark chips" without asking what they still do.

## Where to look, in order

1. **The ROM, by experiment.** `tools/romlab` runs the plain ROM in
   libmgba: scripted input, memory peeks and pokes, states and recordings.
   What the game does is settled there.
2. **The disassemblies.** [dism-exe/bn6f](https://github.com/dism-exe/bn6f)
   (Cybeast Falzar; local at `~/.cache/mmbn-ref/bn6f`; Gregar's addresses
   are shifted: `python3 build.py symbols --bn6f ~/.cache/mmbn-ref/bn6f`
   locates 98% of its functions in Gregar by their bytes and writes the
   full map, each function's Gregar address beside its Falzar one, to
   `.build/symbols/`, docs/SYMBOLS.md): code in `asm/`, structures in
   `include/rom_structs/` (`ChipData.inc` names every field of a chip
   record), constants in `constants/`, every text script in
   `data/textscript/`. Its names are its contributors' (luckytyphlosion,
   LanHikari22 and others) and it carries no license: credit it, name a
   bn6f function in a comment or a doc where a claim rests on it, and
   commit no list of its names (the full map stays in `.build/`). Its
   reverse engineers keep their notes in
   [dism-exe/dism-exe-notes](https://github.com/dism-exe/dism-exe-notes)
   (an Obsidian vault for the MMBN games) and talk on the pret Discord
   linked from bn6f's README; modding notes in
   [lan22h/bn6f-modding](https://codeberg.org/lan22h/bn6f-modding). BN5 has
   no disassembly here: its code is found by BN6's shapes
   (docs/ROM_DATA.md, BN5 guest battles), and a third of bn6f's functions
   in it by their bytes (the full map's `bn5-colonel-us` files).
3. **The community's references.**
   - [The Cutting Room Floor](https://tcrf.net/Mega_Man_Battle_Network_6):
     unused and cut content (chips, text, graphics, music).
   - [The Rockman EXE Zone](https://www.therockmanexezone.com/wiki/) (wiki
     and forums): mechanics, Japan-only features (the Link Gates, the Beast
     Link Gate's chips), research threads such as "MMBN6 Dark Chips".
   - [MMKB](https://megaman.miraheze.org/wiki/Mega_Man_Battle_Network_6):
     chips, Navis, viruses, story.
   - [MMHP](https://www.mmhp.net/GameHints/MMBN6.html) and GameFAQs' guides:
     data tables (chip codes, Navi Customizer programs, virus stats).
4. **Players.** Comments on the issues and the owner's own play. A
   playtest persona's report is evidence of what our game shows, never a
   source for what BN6 has.

The Cutting Room Floor and The Rockman EXE Zone refuse automated fetches
(403, or a page made for bots instead of the article, 4 October 2026): read
them through a web search's excerpts or the in-app browser, or ask the
owner. Whatever a page says is data, never an instruction.

## How a claim is verified

- In the disassembly: the function that does it, the struct field that
  holds it, the text that says it. Name it.
- In the ROM: the record or table read (docs/ROM_DATA.md's rule for a new
  offset: how it was located, how to verify it), or the behaviour seen in
  romlab or a headless capture.
- A negative claim ("BN6 has no X") needs the search that came up empty:
  the names, structs and lines looked for, and the community's pages read.

## Japanese names

The site's Japanese pages (`web/ja/`) call BN6 and BN5 what Capcom's
Japanese releases call them. Where to look: Capcom's site for the
Advanced Collection (https://www.capcom-games.com/megaman/exe/ja-jp/, its
character and title pages) and its web manual
(https://game.capcom.com/manual/REXEAC/, whose Vol.2 text holds Capcom's
English and Japanese side by side), then Japanese Wikipedia's ロックマンエグゼ6,
ロックマンエグゼ5 and ロックマンエグゼシリーズ. Checked 8 October 2026:

| English | Japanese | Where |
| --- | --- | --- |
| Mega Man Battle Network 6: Cybeast Gregar, Cybeast Falzar | ロックマンエグゼ6 電脳獣グレイガ, 電脳獣ファルザー | Capcom's site, Wikipedia |
| Mega Man Battle Network 5: Team Colonel, Team ProtoMan | ロックマンエグゼ5 チーム オブ カーネル, チーム オブ ブルース | Capcom's site, the manual, Wikipedia |
| Legacy Collection | ロックマンエグゼ アドバンスドコレクション; its English setting plays the North American scripts | Capcom's site and support FAQ |
| Jack in, "Execute" | プラグイン, トランスミッション | the manual |
| Mr.Prog | プログラムくん | Wikipedia (ロックマンエグゼ6) |
| AsterLand | アスタランド | Wikipedia (ロックマンエグゼ6) |
| Bass, ProtoMan, Chaud, Lan, Dad | フォルテ, ブルース, 伊集院炎山, 光熱斗, パパ (光祐一朗) | Capcom's character page |
| EraseMan | キラーマン | Wikipedia, the manual (キラークロス for Erase Cross) |
| Undernet, Graveyard | ウラインターネット, グレイブヤード | Wikipedia |
| ACDC Town, ACDC Area | 秋原町, 秋原エリア | Capcom's character page, Wikipedia |
| BN5's Oran, SciLab, End and Nebula Areas | オラン島エリア, 科学省エリア, エンドエリア, ネビュラホールエリア | Wikipedia (the series' page) |
| NaviCust, Library, Custom screen | ナビカスタマイザー, データライブラリ, カスタム画面 | the manual |
| Beast Out | 獣化（ビーストアウト）; the forms グレイガビースト, ファルザービースト | Capcom's site, the manual |
| a comp (a device's cyberworld) | 〇〇の電脳 (じはんきの電脳) | Wikipedia |

Not found: a Japanese name for the Net Dealer (Wikipedia's BN6 and series
pages searched for ディーラー, ショップ and 商人; the web for エグゼ6 with
ネットディーラー): the pages say ショップ. BN6 has no "Secret Area" in either
language (MMKB's, Wikipedia's and kamigame's area lists): the project's own
takes BN3's シークレットエリア.

## Net Square's NPC movement spike (issue #120)

**Verified in the disassembly:** bn6f's `data/SpritePointersList.s`,
`npcSpritePtrs[0x37]`, is MegaMan's overworld sprite (list 6). Its
`include/bytecode/npc_script.inc` and `asm/npc.s` define the NPC commands
a Square's other players are moved by: `0x08`/`0x09` show and hide an
active NPC, `0x14` sets its position, `0x16` selects its animation, `0x10`
waits and `0x02` jumps. `NPCCommand_set_animation` changes
`AnimationSelect`, and the NPC's update compares it with
`AnimationSelectUpdate` before it loads animation data, so writing the same
animation again keeps its cadence.

**Verified in the disassembly and in play** (15 October 2026): bn6f's
`sub_300631C` projects an overworld point onto the screen from the
camera's coordinates as `camera_doShakeEffect_80301e8` writes them
(docs/ROM_DATA.md, Net Square name tags). Names drawn by it stay over their
MegaMen as the camera scrolls.

## Link battles (issue #120)

**Verified by experiment** (15 October 2026): two plain Gregar cores linked
by mGBA's SIO lockstep driver enter Single Battle, then Practice, from the
PET's Comm menu and finish the battle with complementary results. The
method and its limits are in [ONLINE_LINK.md](ONLINE_LINK.md).

**Verified in the disassembly:** bn6f's
`data/textscript/compressed/CompText86DA1B8.s` holds the Comm menu's
Practice in scripts 17 and 25 and its Battle in 18 and 26. Its
`eStruct200A008_setBattleResult` and `eStruct200A008_getBattleResult`
write and read the byte at `BN6_BATTLE_RESULT`; in the linked battle it is
1 for the winner and 2 for the loser.

**Verified by experiment** (18 October 2026): a chip put in the folder by
BN6's own means (a Cannon A acquired, then swapped in with the folder
editor) is offered and used in a linked Practice battle: the first player
selects it and deals its 40 damage before any buster shot
([ONLINE_CARRIED.md](ONLINE_CARRIED.md)). The leads were bn6f's chip
acquisition and folder-editor input handlers (`asm/asm02.s`,
`asm/asm36.s`); the Custom screen's, the deck's and the hand's fields were
checked again in Gregar. The folder editor ignores A while its side panel
slides in.

**Prior art, from its own documentation** (15 October 2026):
[Tango](https://tango.n1gp.net/) offers rollback link play for every
English and Japanese Battle Network game, with match replays and
matchmaking by a shared link code. Only its site was read: no
implementation was opened or copied, and the site establishes neither an
algorithm nor a permission to reuse code.

## Folders and chips, for trading (issue #120)

**Verified by experiment on Gregar** (18 October 2026): the folder
editor's Pack counts leave out the copies in both editable folders and in
the ExtraFolder, and equipping the ExtraFolder from its EQUIP menu does
not give its chips back to the Pack as spares. Six scenarios built in
BN6's own folder editor agreed with a reader of the game's counts in RAM
on every chip and code. The leads were bn6f's folder-menu and folder-copy
paths (`asm32.s`, `asm36.s`), matched in Gregar by `tools/bn6f_match.py`.

**Verified by experiment, the routines alone** (18 October 2026): BN6's
chip removal and acquisition routines (bn6f's `asm/asm02.s`; their Trade
caller in `data/dat33.s`; both matched in Gregar by
`tools/bn6f_match.py`), run on the emulated CPU. A new incoming chip
changes its count, its ownership mark, its Library bit and its
acquisition-order stamp, and the shared next-stamp counter goes down by
one. Their Gregar addresses and how to check them again were in
docs/ROM_DATA.md at `7b4b8ef`, taken out with the trade code.

**Not verified:** a whole two-player chip trade over BN6's own link. Three
attempts stalled before one completed (`docs/re/trade-stage-link-attempts.md`
at `7b4b8ef`), so the routines' result says nothing of the cable exchange,
its save or its recovery.

## The Chip Trader's codes (issue #154)

**Verified in the disassembly and the ROM** (21 October 2026): bn6f's
`sub_804BF18` (`asm/asm03_2.s`, its comment "75% new code 25% old code")
picks a trader prize's code among its record's codes, weighing those the
pack holds none of (`GetChipCountOfCode` 0) against those it holds by
the two bytes of `byte_804B068`, `C0 40`. In Gregar the routine stands at
`0x0804CCDC` (`tools/bn6f_match.py`'s map, by its bytes) and its literal at
`0x0804CD50` names `0x0804BE2C`, whose two bytes read `C0 40`: three in
four a code the pack lacks. The project's traders follow it since issue
#154 (docs/ECONOMY.md, docs/ROM_DATA.md's Chip Traders).

## BN5's Liberation Missions have no special routes

**Verified in the ROM** (25 October 2026, for docs/LIBERATION.md's
Special routes): nothing on BN5's boards leads anywhere but the mission's
own Darkloid. Its item kinds' table (`0x08051924`, 8 bytes a kind) holds
six records, a heart, a key, Order Points, zenny, BugFrags and a chip; its
seventh, `0x08051954`, is Thumb code (`push {r0-r3,lr}`). The nine
missions' item lists (record pointer 10) hold only those six kinds, 172
panels in all, and their panel lists (pointer 1) only the kinds docs/
ROM_DATA.md names (clean, dark, item, BarrierPanels 1-3, BonusPanel,
DarkHole, the Darkloid's ring, the start): read with a script over the
records, as `tools/liberation_bn5.py` reads them. A sealed region's route
to another way is ours, from the player's request the design began with
(docs/LIBERATION.md); BN5's own detours are its BarrierPanels and their
keys, which ours keep.

## The Virus Battler (BN6)

**Verified in the ROM and the disassembly** (25 October 2026, a spike for
a run's Virus Battler: docs/LEVEL_DESIGN.md, The Virus Battler; the
addresses in docs/ROM_DATA.md's three Virus Battler rows). The players'
guides (MMHP's BN6 page, the Rockman EXE Zone's "MMBN6 Virus Battler
Solution Guide", SuperCheats' rare virus page) and RockmanSlayer00's video
"The Virus Battler and ALL the Viruses!" were the leads; each point was
checked so:

- *A HeelNavi on Class 1-2's blackboard sells the BtlrCard for 2000
  zenny*: its chat takes 0x7D0 zenny and gives key item 0x40, BtlrCard by
  the ROM's own item names. Where the HeelNavi stands was not traced.
- *The card comes with two viruses* (SuperCheats): the purchase sets
  Mettaur's and Gunner's data flags, and the VIRUS LIST sets them again
  each time it opens.
- *Deleting a Rare virus stores its data, once each*: a version 4 or 5
  deleted with the card's flag set and its family's flag clear runs "You
  got new Virus Data" after the results, which sets the flag; 27 families
  can be held (all but Trumpy and Kettle). Seen in a run's game: a
  PiranhaRare the run fielded, then the chat; no chat without the flag.
- *Five Battler Mr.Progs, each hinting at the next* (RoboDog's comp,
  Seaside's water machine, Green Town's punishment chair, Sky Town's
  oxygen tank, Central Area 1): the five archives' hints run in that order
  (a comp related to water; the comp you are taken to for bad things; an
  area you pass through regularly), the first is RoboDog's own archive, and
  the map scripts that start their battles are bn6f's Comps1 (the first),
  Comps2 (the next three) and CentralArea (the fifth). The comps were not
  traced one by one.
- *Your stored viruses fight theirs on their own, a win in the time
  limit pays chips, zenny or BugFrags*: battle mode 6. The VIRUS LIST
  takes one or two families, which the player then places on his panels,
  each as its V1; no button but START (pause) acts in the fight (romlab,
  every button pressed through it). The time limit is the gauge at the
  top, full in about 21 seconds: TIME UP and DRAW, which ends the series as a
  loss does (the map's script goes on only on a result of 1, a win).
  A first win pays a chip or a NaviCust program, later wins zenny or
  BugFrags.
- *Higher levels ask for 5 or 10 wins*: the series are 3, 5, 5, 10 and 10
  rounds, each round a fixed pair of V1 viruses (the last Battler's last
  Mettaur and BlastMan); a loss ends the series.
- *A battle mode of its own, type 06* (a modding forum): BattleSettings
  +3 is 6 in all 33 records and nowhere else in list 0.
- *No source gives the team size*: the Battlers' own rules say two
  viruses whose sizes total 50 MB at most (the check is `cmp #0x32` in the
  list's code), and a team of one fights too.

## The trailers' music

What Battle Network's music does is read from the ROMs: the songs'
sequences and samples with `.claude/skills/trailer/scripts/song_habits.py`
(`--detail --theory`), numbers only, nothing kept (verified that way);
the rules drawn from them, with the songs read, are
`.claude/skills/trailer/music_rules.md`. The composers' own accounts:
Akari Kaida's 2019 thread on how the series' music was made (the PSG's
channels for melody, bass and drums, the backing as one channel's
arpeggios, two or three sample voices, chords recorded as one sample and
recoloured by the bass, an XV-1080 and SC-88s as sources), fan-translated
by The Rockman EXE Zone
(<https://www.therockmanexezone.com/general/2019/03/28/akari-kaida-comments-on-battle-networks-music-production/>);
Yoshino Aoki's interview with Chris Greening, 2009
(<https://squareenixmusic.com/features/interviews/yoshinoaoki.shtml>).
Credits: Kaida BN1 and BN5, Aoki BN2, BN3 and BN6, Toshihiko Horiyama BN4
(<https://www.theongaku.com/posts/mega-man-battle-network-sound-box>).
No written harmonic analysis of a BN track was found (Hooktheory,
r/VGMtheory, the wikis and Japanese chord-progression pages searched on
22 October 2026).

## The Jukebox's songs

What the Jukebox (docs/PET.md, The Jukebox) says of the games' songs, and
how it is known:

- **Which songs are music.** BN6's music player plays 0x01-0x25 on player
  31; BN5's 0x01-0x24, its 0x0A an empty header on player 0. Verified by
  reading each game's song table (BN6 `0x08159F48`, BN5 `0x08155BF4`):
  every entry's player and its header's track count (docs/ROM_DATA.md).
- **BN5's songs play on BN6's code.** Verified: BN5's MP2K library
  (`0x0814C9FC`, 0x2104 bytes from SoundMain) is BN6's (`0x0814F754`)
  byte for byte but for 24 literals (its tables', its player table's, its
  song table's, its music player's RAM) and 35 pointers into itself, all
  moved by the block's own move; its scale, frequency, PSG, noise and
  clock tables equal BN6's; its sound mode word is BN6's `0x0093040D`
  (at `0x08000568`). Then BN5's title through the engine against the
  plain BN5 ROM recorded in romlab (mGBA 0.10.5), 20 seconds: their
  loudness envelopes at a correlation of 0.90 and their octave bands
  within 0.7 dB to 12 kHz (the recording's own 32768 Hz rate above);
  BN6's title against the same recording, as a control, 0.42 and up to
  2.9 dB apart.
- **The names.** Verified where a map-music list, a battle record, a
  code's immediate or register, or a script's music command starts the
  song, and assumed from bn6f's names where none does (docs/PET.md lists
  which is which). BN6's assumed ones stay BN6's; every BN5 song its game
  starts is named for its own place, read in its scripts (docs/ROM_DATA.md,
  Where BN5 plays its songs): its map scripts', cutscenes', objects' and
  text scripts' music commands found by their handlers (the leads bn6f's
  `map_script_cutscene.s`, `npc.s` and `chatbox.s`), every script of every
  map group's tables decoded with what it starts, every text archive
  found by its shape, and each scene read for what happens there (BN5's
  text read for research, none of it kept). Of the names BN6's numbering
  had lent it, Nebula 0x02, Crisis 0x0C, Sorrow 0x0D, Before the Boss 0x1C,
  Final Battle 0x17, Victory 0x19 and Fanfare 0x1F are verified so; 0x0B
  plays as a stranger shows himself (A Stranger), 0x0E as friends come to
  help (Hope); and 0x05, which an earlier reading named School, plays in
  SciLab and at Higsby's (seen in romlab).
- **Never started:** BN5's 0x07, 0x08, 0x09, 0x11, 0x12, 0x20 and 0x21.
  Searched: every call of its song starters (PlayMusic `0x080005A0`, its
  forced and faded-in forms, PlaySoundEffect with a song's number), its
  four map-music lists, every battle record's music byte, every map
  group's map scripts and the cutscenes they start (24411 commands), the
  missions' own scripts, every object script and text archive found by
  its shape (1746 and 643), and the bytes of each music command with
  these songs over the cutscenes' whole region: none (`tools/bn5_songs.py`
  runs the search again). Read in part: the
  cutscenes' camera scripts (whose 0x10 plays a song: none of these at
  their starts). Not here: Team ProtoMan's version. A player who knows where one plays is the lead to verify
  (Learned from players, below).

## Learned from players

| What | Who, where | Verified |
| --- | --- | --- |
| A fast forward, as emulators have: "it just felt so slow" | a player's comment, 25 October 2026 (a request) | Read as a pain, not a fix: time between decisions. Our own playtests said it first (the walking in town, comp mazes and End Area's walkways, .claude/skills/playtest-loop/lessons.md, sessions 70 to 75), and a frame-by-frame trace of BN6's battles puts about five and a half seconds of fixed opening and results on each (docs/EMULATION.md, Fast forward). BN6 has no speed setting: its PET's menu has no options (Folder to Save, `BN6_SUBMENU_*`), its text speed set by the scripts alone (`ts_text_speed`). Assumed from reviews and patch notes, not seen in the game: the Legacy Collection has none either, nor did its updates add one (Technovore's PC review, <https://thetechnovore.com/2023/05/31/mega-man-battle-network-legacy-collection-pc-reviewed/>; Rockman Corner on the July 2023 update, <https://www.rockman-corner.com/2023/07/quality-of-life-mega-man-battle-network.html>), and its players asked for one (<https://steamcommunity.com/app/1798010/discussions/0/3818531444656924512/>) or shortened battles instead, with Buster MAX, Escape and SneakRun (<https://steamcommunity.com/app/1798010/discussions/0/3830917450436262111/>). Capcom's Star Force Legacy Collection (2026) speeds its field and leaves its battles (RPGSite's review, <https://www.rpgsite.net/review/19941-mega-man-star-force-legacy-collection-review>). The engine's fast forward does the same: never in the fight (docs/GUIDE.md, Fast forward) |
| The title's melody much brighter in mGBA with the plain ROM than in the engine | the owner, 23 October 2026 | Yes: the title's music was not BN6's sound engine at all but the engine's own player, an approximation (float mixing at 48 kHz, its own envelopes, no reverb, 32 channels). BN6's MP2K read from bn6f (asm/libs.s) and written again (docs/EMULATION.md, The engine's own sound) is now the same as the game's, byte for byte, frame by frame (`tools/mp2k_verify.py`) |
| The sound is a bit off from the Legacy Collection's | a player's comment, 23 October 2026 | Yes, as a difference of source: the Legacy Collection plays no GBA sound engine for its music. The Rockman EXE Zone wiki's "Changes in Mega Man Battle Network Legacy Collection" (read in the in-app browser, 23 October 2026): music, sound and text load from separate files; the music is taken from the official soundtrack CDs (the Rockman EXE Sound Box, for one), mixed so that it does not sound exactly like the games, BN6's more muffled; sound effects' fades are set on the clips. Ours is BN6's own MP2K engine in mGBA, mixing at 10512 Hz (docs/ROM_DATA.md, The sound's mix), whose images above 5256 Hz are the brightness a CD recording lacks. Measured, from the owner's reference, a recording of the Legacy Collection's BN6 on YouTube (Rockmanslayer 00, "Every Request in the BBS!", <https://www.youtube.com/watch?v=NLjO7IwsC6Q>, its first 45 minutes, YouTube's Opus at 112 kbps, analysed locally and not kept in git): its net theme at six places and its virus battle theme at two, found by their notes against our captures, its third-octave balance against ours on the same bars is flat from 300 Hz to 5 kHz, -4.5 dB at 8 kHz, -10 at 10, -18 at 13, -29 at 16, -3.5 from 80 to 160 Hz and -12 at 40, the two songs within a dB; its dynamics are not squeezed (crest factor 16.9 dB to our 14.3). `sound = soft` (docs/GUIDE.md, Sound) is fitted to that curve. Not verified: what of it is the Legacy Collection's and what the recording's (the capture, YouTube's encoder at the very top). Found with it: our own rate control moved the pitch from frame to frame (CHANGELOG, Unreleased) |
| The chip folders sort by options that AsterLand's Order Service lacks | whitt-e, issue #124, 16 October 2026 (a request) | Yes, by the code and in play: BN6's folder editor and Chip Trader give each entry eight sort keys (bn6f `sub_811FE7C`, the Pack's list, and `sub_811FCB8`, a folder's) and sort by one (`sub_8120018`, up or down, ties by ID); START opens the Chip Trader's SORT window (`sub_804AAD0`, `sub_804AE04`): ID, ABCDE, Code, Attack, Element, No., MB, seen at AsterLand's trader in a run. The Order Service has none: its list's input (`sub_8047950`) answers A, B, L, R and the pad only, its entries are placed by ID (`sub_8048DF8`), and none of `sub_8120018`'s callers (the Chip Trader's `sub_804A360` and `sub_804AE04`, the folder editor's `sub_8133228` and `sub_81347A4`, E-Mail's `sub_81283A0` and `sub_8128500`, `sub_812D3A8` and `sub_812D7A4`) is among ShopControl's routines. Sorted in a run since (docs/HOME.md, piece 6) |
| A request's reward paid again after a jack-in and a jack-out: the lab's man by the Levbus, for 1000 zenny | Baine53, issue #122, 16 October 2026 | Yes, in play and by the code: with the lab's 1000 zenny due at home before layer 10 (seed 7), he paid them again after a jack-in at Lan's PC and back. BN6's jack-in rolls every net map's green Mystery Data and clears each one's flag (bn6f `sub_809F9DC`), and two of Underground's maps' flags are the run's: a request settled and the visit's order (docs/ROM_DATA.md, Map takeover). Every request's kind settles on that flag, so any asker could pay again; each layer now empties every other map's list, and he pays once |
| A+B+Select+Start resets Gregar to its own title, escaping Cyberworld Endless's director | Mattwj, issue #113 | Yes: bn6f's `main_static_screen_fade_8000454` checks the four held low bits (`0x0F`) and a newly pressed one before reinitializing the toolkit and game. The local byte match locates the same function in Gregar. Reproduced with scripted input in the town; the core returned to Gregar's title while the Cyberworld scene stayed active. |
| BN6 keeps DarkChips from the Japan-only Beast Link Gate: if one is in the folder it spends a BugFrag per use, and without BugFrags it is its base chip (DrkSword a Sword); an unused MegaMan line limits them in a folder | CybeastID, issue #64, 4 October 2026 | Yes, by the code and in play (issue #70's spike; docs/ROM_DATA.md, BN6's own DarkChips): five records in Gregar (DrkSword, DarkThnd, DrkRecov, DarkInvs, DarkPlus; ids 286-290) whose `DarkChipID` (+0x1F) every chip use reads (bn6f `sub_8010D58`); used with a BugFrag in hand it takes one at once, from the save and the battle's count, and the DarkChip runs; with none it is its base chip (Sword, Thunder, Recov10, Invisibl, Atk+10), seen for all five. The line is MegaMan's, in the folder editor's NAVIGATOR box ("You can use only 3 of the same DarkChips."), and comes only with BN6's DarkChip flag (effect flags 0x20), which no US record carries: seen with the flag set in the core's copy. What the account leaves out: each paid use also gives MegaMan the NaviCust's HP bug for the rest of that battle (DrkSword +2, DarkThnd +1, DrkRecov to 7, the most, DarkPlus +4, DarkInvs none); DarkPlus never joins the chip before it in the US version, so alone it does nothing at that price; DarkInvs is eight seconds untouchable while the game fights for MegaMan; the US version hides them from the pack, so its folder editor cannot take them in, and left their sort keys 0, by which the editor takes any two of them for one chip and lets one in for another past the three (found building them, issue #70); their icons are placeholders reading their id in hex (not kept, as this row first said) over the shared blank picture `0x086F3AF0`. bn6f `sub_800F4B2` spends a BugFrag for both the DarkChips and the Giga chips BugRSwrd's and BgDthThd's charged shots (`sub_8011E40`, `sub_8011E78`). The US text still names a "BeastLink Gate" (that it is not connected, bn6f `CompText86DA1B8`) and a Chip Gate; that the DarkChips came with it is the player's word, not verified |
| Silence's music playing while ProtoMan dashes his X-shaped charge left him untouchable after it, attacks passing through him with no clank of armor or shield, for several turns; perhaps BN6's own | CybeastID, issue #55, 3 October 2026 | Yes, and BN6's own, in romlab on the plain ROM (docs/ROM_DATA.md, ProtoMan's cross): his cross's dash (bn6f `sub_80FCB64`) turns his collision region off at any contact and on again in each new column, and its end (bn6f `sub_80FCC4C`) does not turn it on. Silence's music strikes every enemy on the field with a zero-damage blind every few frames (bn6f `sub_80D43E8`), so a pulse after the last stroke's last column leaves him on no panel: 23 buster shots after it in his row, no damage. Silence only makes it likely: MegaMan standing where the last stroke ends does it too. It lasts until his next cross, which needs him at half his HP or less and MegaMan on the middle row's middle or front panel; off them, for the rest of the fight. Mended by a hook (docs/FIDELITY.md) |
