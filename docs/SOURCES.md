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

## Learned from players

| What | Who, where | Verified |
| --- | --- | --- |
| A+B+Select+Start resets Gregar to its own title, escaping Cyberworld Endless's director | Mattwj, issue #113 | Yes: bn6f's `main_static_screen_fade_8000454` checks the four held low bits (`0x0F`) and a newly pressed one before reinitializing the toolkit and game. The local byte match locates the same function in Gregar. Reproduced with scripted input in the town; the core returned to Gregar's title while the Cyberworld scene stayed active. |
| BN6 keeps DarkChips from the Japan-only Beast Link Gate: if one is in the folder it spends a BugFrag per use, and without BugFrags it is its base chip (DrkSword a Sword); an unused MegaMan line limits them in a folder | CybeastID, issue #64, 4 October 2026 | Yes, by the code and in play (issue #70's spike; docs/ROM_DATA.md, BN6's own DarkChips): five records in Gregar (DrkSword, DarkThnd, DrkRecov, DarkInvs, DarkPlus; ids 286-290) whose `DarkChipID` (+0x1F) every chip use reads (bn6f `sub_8010D58`); used with a BugFrag in hand it takes one at once, from the save and the battle's count, and the DarkChip runs; with none it is its base chip (Sword, Thunder, Recov10, Invisibl, Atk+10), seen for all five. The line is MegaMan's, in the folder editor's NAVIGATOR box ("You can use only 3 of the same DarkChips."), and comes only with BN6's DarkChip flag (effect flags 0x20), which no US record carries: seen with the flag set in the core's copy. What the account leaves out: each paid use also gives MegaMan the NaviCust's HP bug for the rest of that battle (DrkSword +2, DarkThnd +1, DrkRecov to 7, the most, DarkPlus +4, DarkInvs none); DarkPlus never joins the chip before it in the US version, so alone it does nothing at that price; DarkInvs is eight seconds untouchable while the game fights for MegaMan; the US version hides them from the pack, so its folder editor cannot take them in, and left their sort keys 0, by which the editor takes any two of them for one chip and lets one in for another past the three (found building them, issue #70); their icons are placeholders reading their id in hex (not kept, as this row first said) over the shared blank picture `0x086F3AF0`. bn6f `sub_800F4B2` spends a BugFrag for both the DarkChips and the Giga chips BugRSwrd's and BgDthThd's charged shots (`sub_8011E40`, `sub_8011E78`). The US text still names a "BeastLink Gate" (that it is not connected, bn6f `CompText86DA1B8`) and a Chip Gate; that the DarkChips came with it is the player's word, not verified |
| Silence's music playing while ProtoMan dashes his X-shaped charge left him untouchable after it, attacks passing through him with no clank of armor or shield, for several turns; perhaps BN6's own | CybeastID, issue #55, 3 October 2026 | Yes, and BN6's own, in romlab on the plain ROM (docs/ROM_DATA.md, ProtoMan's cross): his cross's dash (bn6f `sub_80FCB64`) turns his collision region off at any contact and on again in each new column, and its end (bn6f `sub_80FCC4C`) does not turn it on. Silence's music strikes every enemy on the field with a zero-damage blind every few frames (bn6f `sub_80D43E8`), so a pulse after the last stroke's last column leaves him on no panel: 23 buster shots after it in his row, no damage. Silence only makes it likely: MegaMan standing where the last stroke ends does it too. It lasts until his next cross, which needs him at half his HP or less and MegaMan on the middle row's middle or front panel; off them, for the rest of the fight. Mended by a hook (docs/FIDELITY.md) |
