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
| Options (BN6's Comm) | The game's Options over the PET |
| Save | Saves the run where MegaMan stands |
| PLACE | The area beside the layer ("JudgeTree 14") |

## Save

BN6's Save wrote its own save file; a run keeps its own checkpoints
(docs/EMULATION.md). Save closes the PET and takes a checkpoint where
MegaMan stands, as a layer's arrival does, and "Run saved" shows in the
corner; the quit prompt then says "Run saved where you saved it". In the
town, before the first layer, there is no run to save yet, and a note says
so.

## Options

Options is reachable wherever the game is (docs/GUIDE.md, Options): the
title's menu has its line, and in a run the PET's menu does, in Comm's
place. BN6's Comm opens the link cable's screens, which a run never had a
use for: it was greyed and buzzed in every run. Its row now reads
**Options**, lit, and A on it opens the game's Options over the PET,
which waits under it, its music held with the game; B (or Done) closes
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
([ROM_DATA.md](ROM_DATA.md), the PET menu's labels).

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
three open the golden gate) and four of the profile's, in the ids
`0x47`-`0x4F` BN6 leaves nameless:

| Item | Shown | Says |
| --- | --- | --- |
| NaviCode | once a guardian has been deleted twice (the Navi gates' code, docs/META.md) | whose codes MegaMan holds, two by name, and that each opens its gate |
| DarkPass | once the Secret Area has been cleared in any run | that the dark way to the Undernet is open |
| LibCard | always | how many chips the Library holds, and at how many a vault on this layer opens |
| NetPass | always | what the player's games open online, as stamps; A opens the pass (in the Net Square START opens it, the PET staying shut there) |

The profile's four are given on each layer as it begins, and at home
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

## How the entries are taken over

The key items' names and descriptions and the mails' senders, subjects
and bodies are BN6's text archives rebuilt from the player's ROM on the
core's copy, our scripts in place of some, in the free space
(docs/EMULATION.md), and the literals the screens read them through
pointed at them (docs/ROM_DATA.md, the PET).

Flag `0x1706` (`EVENT_PET_COMM_SAVE_DISABLED`), which a run keeps set,
greys Comm and Save and makes A on them buzz. On the core's ROM copy
(`pet.c`, docs/ROM_DATA.md) a hook runs as the PET's input handler
begins: A on Save or on Options (Comm's row) is taken from the game and
written to the menu's spare byte (`0x0200DF2F`), which the engine reads
each frame; and the grey's stores to Save's and Comm's colours are
no-ops, so both show lit. BN6's own Save and Comm never run. Where the
player's ROM does not hold the labels as read, Comm stays as BN6 has it,
greyed, and A on it buzzes.
