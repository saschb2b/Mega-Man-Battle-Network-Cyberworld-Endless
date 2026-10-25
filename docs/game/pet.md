---
type: Feature
title: "The PET"
description: "The PET in a run (`src/director/pet.c`): BN6's screens, Save taking a checkpoint, Options in Comm's place, E-Mail as Dad's lab, KeyItem's profile items, the NetPass and the Jukebox, and the hooks that take them over."
tags: [game, pet]
generated: { by: claude-code/unrecorded, at: 2026-10-23T20:35:40Z }
---

# The PET

BN6's PET (START on the map) is Lan's own terminal, and so the most
in-world place for what the run knows: what lives in it reads as MegaMan's
and Lan's, not as a game menu (the diegesis matrix: [What MegaMan knows](meta/what-megaman-knows.md), what
MegaMan knows). A run keeps BN6's entries and gives the ones BN6 greys out
a use of its own.

| Entry | In a run |
| --- | --- |
| ChipFolder, SubChip, Library, MegaMan | BN6's own |
| E-Mail | BN6's screen: Dad's mails, the dive and the records first, then the battle data on each guardian met |
| KeyItem | BN6's screen: the profile's own items beside the run's; A on the NetPass opens the pass |
| Options (BN6's Comm) | The game's Options over the PET |
| Save | Saves the run where MegaMan stands |
| PLACE | The area beside the layer ("JudgeTree 14") |

## Save

BN6's Save wrote its own save file; a run keeps its own checkpoints
([Running the game's own logic](../engine/emulation/overview.md)). Save closes the PET and takes a checkpoint where
MegaMan stands, as a layer's arrival does, and "Run saved" shows in the
corner; the quit prompt then says "Run saved where you saved it". In the
town, before the first layer, there is no run to save yet, and a note says
so.

## Options

Options is reachable wherever the game is ([The player's guide](../GUIDE.md), Options): the
title's menu has its line, and in a run the PET's menu does, in Comm's
place. BN6's Comm opens the link cable's screens, which a run never had a
use for: it was greyed and buzzed in every run. Its row now reads
**Options**, lit, and A on it opens the game's Options over the PET,
which waits under it, its music held with the game; B closes
Options and the PET is there as it was, its cursor on Options, and the
button that closed it is kept from the game until it is let go, so that
B does not close the PET too.

BN6 has no Options to drive (bn6f searched for config, options and
stereo), and its labels are pictures, so the word is set from the
labels' own letters as the core starts (`src/director/pet_options.c`,
`src/gfx/letters.c`): the p and the i of ChipFolder, the t and the s of
Status (the label BN6 shows in MegaMan's row for another Navi), Comm's o,
MegaMan's n, and Comm's C closed into the O by its mirror image, shaded
as the font shades. They are read from the player's ROM: no picture of
Capcom's is in the source. The word is a column longer than Comm, so the
labels' tiles are copied into the free space with Comm's made Options and
two tiles more after them, and the PET's load list points at the copy
([ROM data](../rom/overview.md), the PET menu's labels).

## E-Mail: Dad's lab

Dad's lab watches the dive, and two mails from him head the list, made
again on each layer, read and never marked NEW:

- **Dive report**: the layer (of the short net's ten), the act and the
  area; the guardian ahead as MegaMan knows him (his name from battle
  data, word on the net, or "a strong Navi. We don't know who yet");
  ScrtData; the folder, the Cross and its weakness, the threat and the
  helpers brought.
- **Records**: every guardian met in any run with MegaMan's wins and
  losses, three a page, "code" beside those whose NaviCode he holds; how
  many of the seventeen he has met, the best layer and the Nest's wins.
- **Compression**, from the first compression code entered in any run:
  Dad compresses these programs before every dive ("Lan,I compress these
  for MegaMan before every dive."), then each code entered, a line each,
  the program as the NaviCust lists it and its ten buttons five and five
  ("Custom1  LBBRB AALRR"); not how to enter one, BN6's own, which the
  player has just done, nor that entered again it undoes itself; NEW with
  each new code ([The meta layer](meta/overview.md)).
- **Endless Net** (from NetBBS): netizens' threads about the net, more as
  the profile goes deeper, NEW with each new post ([Rumors](meta/rumors.md)).

They were once a screen of the engine's own behind Comm (the SciLab
link), drawn over the PET: it held the game still, its music with it, and
looked like none of BN6's screens. As mail they are BN6's own screen, its
music playing on, and Comm is BN6's again.

For every guardian MegaMan has battled, in any run, the PET holds a mail
from Dad, its subject the guardian: "Lan, I sorted out MegaMan's battle
data on HeatMan's copy. Here it is, as he logged it", then the guardian's
warning in MegaMan's words and face (the briefing's, [The meta layer](meta/overview.md)). What
the layer's briefing said once can be read again any time. A run's mail
lives in the game's memory: a new run is given every guardian's at its
first layer, without a word; a guardian battled for the first time mails
his on the next layer, and MegaMan says so ("Mail from Dad, Lan!").

On a second screen (the 3DS's bottom one, an Android handheld's second
display) the mail under the cursor, or open, shows by its sender as BN5
DS reads a mail: Dad's face and name, the BBS by its name alone, with
the subject and how many of the list's mails are new (issue #83).

## KeyItem: what the profile holds

BN6's list shows the run's items (ScrtData, with a description of our own:
three open the golden gate) and five of the profile's, in the ids
`0x47`-`0x4F` BN6 leaves nameless:

| Item | Shown | Says |
| --- | --- | --- |
| NaviCode | once a guardian has been deleted twice (the Navi gates' code, [The meta layer](meta/overview.md)) | whose codes MegaMan holds, two by name, and that each opens its gate |
| DarkPass | once the Secret Area has been cleared in any run | that the dark way to the Undernet is open |
| LibCard | always | how many chips the Library holds, and at how many a vault on this layer opens |
| NetPass | always | what the player's games open online, as stamps; A opens the pass (in the Net Square START opens it, the PET staying shut there) |
| Jukebox | always | every song of the town and the Net; A opens the Jukebox |

The profile's five are given on each layer as it begins, and at home
wherever the game's state lacks them (`pet_text_items`, from `pet.c`'s
frame): a NEW GAME's PET, and the week's dive's, held only the PET in
Lan's room and Lan's HP until the first layer, so the NetPass the
greeter had just stamped was nowhere in KeyItem (session 74).

The Spins show with BN6's own words, those the profile has found in the
net, one a run ([The meta layer](meta/overview.md)): each lets a held NaviCust program of its
colour turn with L and R. The descriptions that count are made again on
each layer.

### The NetPass

The NetPass (`0x4A`, after NaviCode `0x47`, DarkPass `0x48` and LibCard
`0x49`) is the one key item that opens a screen: the pass of
[The NetPass](../online/netpass.md), its four stamps (Square, Arena,
Trade, DeepNet), the cartridge slots of the games that earn them and a
line of PET text, drawn by the engine in the project's own pixel art
over the KeyItem screen. A on it opens the pass; A or B closes it. The
game's keys are held from the core while it is up, so BN6's list does
not move under it.

BN6 itself opens a screen from A on a key item (bn6f's `sub_8128BF0`,
for key item `0x40`), so a key item that opens one is BN6's own way. The
engine reads the KeyItem screen's cursor to know which item A was
pressed on (`pet_key_item` in `pet.c`, no hook): the PET screen's state
at `0x02009A30` while it takes input (+0x01 and +0x02 both 4), +0x0D the
column, +0x20 the row, +0x24 the scroll, and the list of held items at
`0x0201BF00`, two bytes an entry, the entry under the cursor at
(scroll + row) * 2 + column ([ROM data](../rom/overview.md), the KeyItem
screen's cursor). A on any item but `0x40` does nothing in BN6, so the A
that opens the pass is also kept from the game. In the Net Square BN6's
PET stays shut (its screens would change the Square's RAM, thrown away
on the way home); START opens the same pass there.

### The Jukebox

The Jukebox (`0x4B`) is a music player: every song of the music player
of the games in use, BN6's 37 and, with BN5's ROM beside it, BN5's 35,
played as the cartridge plays them. A on it opens a PET screen over the
KeyItem screen in the look of the NetPass and Trade Central (our own
drawing: BN6 has no screen of the kind). Under it the game holds still,
its core's frames not run (`pet_holds` in `scene_emu.c`), so its music
stops with it and goes on where it was once the Jukebox closes; the
songs play on the engine's own MP2K, BN6's code ([The engine's own sound](../engine/emulation/engine-sound.md), The
engine's own sound).

The songs are in sections by game, then by where they play: Towns, The
Net, Battles, Story, Screens, and BN5's More. On the left the song
playing: its game's label, its name, its number in its game's song table,
how long it has played, a light a track for its eight tracks, lit by
their loudness, and how often it has gone round. On the right the
section's songs; under them the keys.

| Key | Does |
| --- | --- |
| Up, Down | the cursor |
| L, R (Left, Right) | the section before or after, the cursor on the song playing where it is in it |
| A | plays the song under the cursor |
| START (SELECT) | the play mode: **Repeat** (the song over and over), **In order** (the section in its order), **Shuffle** (the section in any order) |
| B | closes it, the song stopped |

A tap does each: a song's row plays it, the section's arrows turn it, the
mode's slot and the keys' caps do as their buttons. In order and
Shuffle, a song that loops plays its loop twice, then fades over two
and a half seconds; a song that ends (the jingles) ends, BN6's holding a
minute of rests before their end, which three seconds of silence after
it has sounded counts as its end. Then the next, after three quarters of
a second. The last song and the play mode are kept in the data folder's
`jukebox.ini` (never in the run's save), so it opens there again; it
opens quiet. Left out on purpose: favourites, a song as the title's
music, and music over play (the owner's design call for this first
version: the game's own music is the run's).

**The names** are ours, for where each song plays (`jukebox_text.c`):

- *Verified in the ROM:* a song a map plays from BN6's map-music lists
  (`0x080360E4`, read by `playCertainMapMusicBasedOnEventByte`; every
  chapter's list read, [The real world and home](../rom/world.md), Real-world music): Central Town
  0x03 (group 0x01's first map), Indoors 0x04 (Lan's house, Aster Land),
  Cyber Academy 0x05, Seaside Town 0x06 (group 0x03), Green Town 0x08
  (group 0x04), Sky Town 0x07 (group 0x05's first map), ACDC Town 0x24
  (group 0x00's first map), The Net 0x13 (every area's maps), Robot
  Control 0x10 (group 0x80), Aquarium Comp 0x11 (0x81), Judge Tree Comp
  0x12 (0x82), Mr.Weather Comp 0x0A (0x83), Graveyard 0x09 (0x96),
  Undernet 0x14 (0x95); the battles' songs from their BattleSettings
  (Virus Busting 0x15, Boss Battle 0x16); songs started by code with
  their number as an immediate (`movs r0,#n` before a `bl` to
  PlayMusic, `0x080005D4`): the title 0x01, Game Over 0x1B (the game
  over screen's), Credits 0x1D (the staff roll's), NaviCust 0x1E.
  Verified where they play, their names bn6f's: Something's Up 0x0B and
  Crisis 0x0C (the towns' maps in some chapters), Transmission 0x0F (one
  call, an ending song) and Defeat 0x1A (four calls in the battle's
  code). BN5's ([BN5's maps, songs, text and objects](../rom/bn5-world.md), Where BN5 plays its songs), each
  where its game starts it: its map-music lists (`0x080361C8`) for ACDC
  Town 0x03, Indoors 0x04 (ACDC Town's houses), SciLab 0x05 (group
  0x01's five maps, and Higsby's, 0x00:7: its lab, lobby and
  control room, seen in romlab), Comp 0x10, The Net 0x13, Undernet 0x14;
  its battles' records for 0x15, 0x16 and Final Battle 0x17 (its last
  story records, its final boss's); the same calls as BN6's, at the same
  routines moved, for 0x01, 0x0F, 0x1A, 0x1B, 0x1D and 0x1E, and its
  battle's win for Victory 0x19 and Fanfare 0x1F (by the battle's
  options, as BN6's). Its scripts' music commands, every script of every
  map walked (its cutscenes', its objects' and its text archives'), for
  the rest: Nebula 0x02 (Regal's and Nebula's scenes), A Stranger 0x0B
  (a fast Navi who slipped past MagnetMan shows himself), Crisis 0x0C
  (ACDC Town in the third map list; an attack, a cave-in, Nebula on the
  move), Sorrow 0x0D (MegaMan
  taken), Hope 0x0E (the friends' Navis come to help, ProtoMan and
  Colonel hear MegaMan's voice, the last rally), Beach Day 0x06 (the
  swimming race and the fishing at the beach), Challenge 0x23 (the
  fishing contest's big catch, a battle against a hundred foes, and the
  link's name entry and battle menu), Before the Boss 0x1C (ProtoMan or
  Colonel showing himself, the scenario comps' last maps, a mission's
  opening). Its Liberation Missions' (heard in romlab on mission 1, the
  BGM's player, 31 at `0x02011470`, its song header read against the
  song table `0x08155BF4`): Liberation 0x22, the board's own through
  both phases (its clock running on from one to the next); Area
  Liberated 0x18 as the mission is won and the Colonel tallies its
  phases (the board's routine 6); One-Turn Win 0x24, where a
  liberation's battle is won in its first turn, in place of the win's
  usual song.
- *Assumed from bn6f's names* (`constants/enums/SoundOffsets.inc`, the
  community disassembly's, read as leads, never copied as a list) where
  no map list or code reached them: The Expo 0x25, Pavilion 0x18,
  Pavilion Comp 0x20, Cybeasts 0x21, Before the Boss 0x1C (the boss
  prelude the run's guardians play), Showdown 0x22, Final Battle 0x17,
  The Chase 0x23, Villains 0x02, Sorrow 0x0D, Good Times 0x0E, Victory
  0x19, Fanfare 0x1F: BN6's alone now, BN5's each named for its own
  place above.
- *Never started:* BN5's 0x07, 0x08, 0x09, 0x11, 0x12, 0x20 and 0x21,
  which nothing in Team Colonel starts (the search, [Sources](../research/sources.md#the-jukeboxs-songs), The
  Jukebox's songs): they carry their number ("Song 07") in More.

**In a run**, three of BN5's songs play where BN5 plays them
([BN5's music in a run](../multirom/music.md)): Before the Boss as a guardian
logs in on BN5's areas, Area Liberated as a dark layer is won, One-Turn
Win at a liberation's battle won in its first turn.

## How the entries are taken over

The key items' names and descriptions and the mails' senders, subjects
and bodies are BN6's text archives rebuilt from the player's ROM on the
core's copy, our scripts in place of some, in the free space
([Running the game's own logic](../engine/emulation/overview.md)), and the literals the screens read them through
pointed at them ([The PET and the second screen](../rom/pet-and-second-screen.md), the PET).

Flag `0x1706` (`EVENT_PET_COMM_SAVE_DISABLED`), which a run keeps set,
greys Comm and Save and makes A on them buzz. On the core's ROM copy
(`pet.c`, [ROM data](../rom/overview.md)) a hook runs as the PET's input handler
begins: A on Save or on Options (Comm's row) is taken from the game and
written to the menu's spare byte (`0x0200DF2F`), which the engine reads
each frame; and the grey's stores to Save's and Comm's colours are
no-ops, so both show lit. BN6's own Save and Comm never run. Where the
player's ROM does not hold the labels as read, Comm stays as BN6 has it,
greyed, and A on it buzzes.
