# The PET

BN6's PET (START on the map) is Lan's own terminal, and so the most
in-world place for what the run knows: what lives in it reads as MegaMan's
and Lan's, not as a game menu (the diegesis matrix: docs/META.md, what
MegaMan knows). A run keeps BN6's entries and gives the ones BN6 greys out
a use of its own.

| Entry | In a run |
| --- | --- |
| ChipFolder, SubChip, Library, MegaMan | BN6's own |
| E-Mail | BN6's screen: Dad's mails, the dive and the records first, then the battle data on each guardian met |
| KeyItem | BN6's screen: the profile's own items beside the run's; A on the NetPass opens the pass |
| Comm | BN6's grey and buzz, as without a link cable |
| Save | Saves the run where MegaMan stands |
| PLACE | The area beside the layer ("JudgeTree 14") |

## Save

BN6's Save wrote its own save file; a run keeps its own checkpoints
(docs/EMULATION.md). Save closes the PET and takes a checkpoint where
MegaMan stands, as a layer's arrival does, and "Run saved" shows in the
corner; the quit prompt then says "Run saved where you saved it". In the
town, before the first layer, there is no run to save yet, and a note says
so.

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
  each code entered, a line each, the program as the NaviCust lists it and
  its ten buttons five and five ("Custom1  LBBRB AALRR"); not how to
  enter one, BN6's own, which the player has just done; NEW with each new
  code (docs/META.md).
- **Endless Net** (from NetBBS): netizens' threads about the net, more as
  the profile goes deeper, NEW with each new post (docs/META.md, Rumors).

They were once a screen of the engine's own behind Comm (the SciLab
link), drawn over the PET: it held the game still, its music with it, and
looked like none of BN6's screens. As mail they are BN6's own screen, its
music playing on, and Comm is BN6's again.

For every guardian MegaMan has battled, in any run, the PET holds a mail
from Dad, its subject the guardian: "Lan, I sorted out MegaMan's battle
data on HeatMan's copy. Here it is, as he logged it", then the guardian's
warning in MegaMan's words and face (the briefing's, docs/META.md). What
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
| NaviCode | once a guardian has been deleted twice (the Navi gates' code, docs/META.md) | whose codes MegaMan holds, two by name, and that each opens its gate |
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
net, one a run (docs/META.md): each lets a held NaviCust program of its
colour turn with L and R. The descriptions that count are made again on
each layer.

### The NetPass

The NetPass (`0x4A`, after NaviCode `0x47`, DarkPass `0x48` and LibCard
`0x49`) is the one key item that opens a screen: the pass of
[ONLINE.md](ONLINE.md#the-netpass), its four stamps (Square, Arena,
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
(scroll + row) * 2 + column ([ROM_DATA.md](ROM_DATA.md), the KeyItem
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
songs play on the engine's own MP2K, BN6's code (docs/EMULATION.md, The
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
  chapter's list read, docs/ROM_DATA.md, Real-world music): Central Town
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
  code). BN5's from its map-music lists (`0x080361C8`): ACDC
  Town 0x03, Indoors 0x04 (ACDC Town's houses), School 0x05 (group
  0x01), Crisis 0x0C (ACDC Town in the third list), Comp 0x10, The Net
  0x13, Undernet 0x14; its battles' 0x15 and 0x16; and the same calls
  as BN6's, at the same routines moved, for 0x01, 0x0F, 0x1A, 0x1B,
  0x1D and 0x1E.
- *Assumed from bn6f's names* (`constants/enums/SoundOffsets.inc`, the
  community disassembly's, read as leads, never copied as a list) where
  no map list or code reached them: The Expo 0x25, Pavilion 0x18,
  Pavilion Comp 0x20, Cybeasts 0x21, Before the Boss 0x1C (the boss
  prelude the run's guardians play), Showdown 0x22, Final Battle 0x17,
  The Chase 0x23, Villains 0x02, Sorrow 0x0D, Good Times 0x0E, Victory
  0x19, Fanfare 0x1F. BN5 numbers its songs as BN6 does wherever the
  two were checked (the eleven above), so its 0x02 (Nebula), 0x0B, 0x0D,
  0x0E, 0x17, 0x19, 0x1C and 0x1F carry BN6's names, assumed.
- *Not known yet:* BN5's own songs 0x06-0x09, 0x11, 0x12, 0x18 and
  0x20-0x24 (its places, its Liberation Missions'), which no map list or
  immediate reaches: they carry their number ("Song 06") in More until a
  source names them (docs/SOURCES.md).

## How the entries are taken over

The key items' names and descriptions and the mails' senders, subjects
and bodies are BN6's text archives rebuilt from the player's ROM on the
core's copy, our scripts in place of some, in the free space
(docs/EMULATION.md), and the literals the screens read them through
pointed at them (docs/ROM_DATA.md, the PET).

Flag `0x1706` (`EVENT_PET_COMM_SAVE_DISABLED`), which a run keeps set,
greys Comm and Save and makes A on them buzz. On the core's ROM copy
(`pet.c`, docs/ROM_DATA.md) a hook runs as the PET's input handler
begins: A on Save is taken from the game and written to the menu's spare
byte (`0x0200DF2F`), which the engine reads each frame; and the grey's
store to Save's colour is a no-op, so Save shows lit while Comm keeps its
grey and buzz. BN6's own Save never runs.
