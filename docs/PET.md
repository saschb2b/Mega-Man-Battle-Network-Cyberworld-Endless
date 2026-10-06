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
| KeyItem | BN6's screen: the profile's own items beside the run's |
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
  its ten buttons five and five ("Custom1  LBBRB AALRR"), after how to
  enter one; NEW with each new code (docs/META.md).
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
three open the golden gate) and three of the profile's, in ids BN6 leaves
nameless:

| Item | Shown | Says |
| --- | --- | --- |
| NaviCode | once a guardian has been deleted twice (the Navi gates' code, docs/META.md) | whose codes MegaMan holds, two by name, and that each opens its gate |
| DarkPass | once the Secret Area has been cleared in any run | that the dark way to the Undernet is open |
| LibCard | always | how many chips the Library holds, and at how many a vault on this layer opens |

The Spins show with BN6's own words, those the profile has found in the
net, one a run (docs/META.md): each lets a held NaviCust program of its
colour turn with L and R. The descriptions that count are made again on
each layer.

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
