# The PET

BN6's PET (START on the map) is Lan's own terminal, and so the most
in-world place for what the run knows: what lives in it reads as MegaMan's
and Lan's, not as a game menu (the diegesis matrix: docs/META.md, what
MegaMan knows). A run keeps BN6's entries and gives the ones BN6 greys out
a use of its own.

| Entry | In a run |
| --- | --- |
| ChipFolder, SubChip, Library, MegaMan | BN6's own |
| E-Mail | BN6's screen: Dad's mail with the battle data on each guardian met |
| KeyItem | BN6's screen: the profile's own items beside the run's |
| Comm | The SciLab link: the lab's view of the dive, drawn by the engine |
| Save | Saves the run where MegaMan stands |
| PLACE | The area beside the layer ("JudgeTree 14") |

## Comm: the SciLab link

Dad's lab watches the dive over the PET's link. Choosing Comm opens it
over the PET, the game holding still: the first page is the dive (the
layer, of the short net's ten; the act and the area; the guardian ahead as
MegaMan knows him: his name from battle data, "Name?" as word on the net,
"???" before either; ScrtData; the folder, the Cross and its weakness, the
threat and the helpers brought); the next pages are the battle records,
every guardian met in any run with its wins and losses, eight a page, and
the best layer. LEFT and RIGHT turn the pages, A or B closes it (the key
kept from the game till it is let go, which would close the PET too).

## Save

BN6's Save wrote its own save file; a run keeps its own checkpoints
(docs/EMULATION.md). Save closes the PET and takes a checkpoint where
MegaMan stands, as a layer's arrival does, and "Run saved" shows in the
corner; the quit prompt then says "Run saved where you saved it". In the
town, before the first layer, there is no run to save yet, and a note says
so.

## E-Mail: Dad's battle data

For every guardian MegaMan has battled, in any run, the PET holds a mail
from Dad, its subject the guardian: "Lan, I sorted out MegaMan's battle
data on HeatMan's copy. Here it is, as he logged it", then the guardian's
warning in MegaMan's words and face (the briefing's, docs/META.md). What
the layer's briefing said once can be read again any time. A run's mail
lives in the game's memory: a new run is given every guardian's at its
first layer, without a word; a guardian battled for the first time mails
his on the next layer, and MegaMan says so ("Mail from Dad, Lan!").

## KeyItem: what the profile holds

BN6's list shows the run's items (ScrtData, with a description of our own:
three open the golden gate) and three of the profile's, in ids BN6 leaves
nameless:

| Item | Shown | Says |
| --- | --- | --- |
| NaviCode | once a guardian has been deleted twice (the Navi gates' code, docs/META.md) | whose codes MegaMan holds, two by name, and that each opens its gate |
| DarkPass | once the Secret Area has been cleared in any run | that the dark way to the Undernet is open |
| LibCard | always | how many chips the Library holds, and at how many a vault on this layer opens |

The Spin items stay, with BN6's own words: they are what lets a held
NaviCust program turn with L and R. The descriptions that count are made
again on each layer.

## How the entries are taken over

The key items' names and descriptions and the mails' senders, subjects
and bodies are BN6's text archives rebuilt from the player's ROM on the
core's copy, our scripts in place of some, in the free space
(docs/EMULATION.md), and the literals the screens read them through
pointed at them (docs/ROM_DATA.md, the PET).

Flag `0x1706` (`EVENT_PET_COMM_SAVE_DISABLED`), which a run keeps set,
greys Comm and Save and makes A on them buzz. Two patches on the core's ROM
copy (`pet.c`, docs/ROM_DATA.md) turn the buzzer into a note for the
engine, the cursor's entry written to the menu's spare byte
(`0x0200DF2F`), and leave the two entries ungreyed; the engine reads the
byte each frame. BN6's own Comm and Save never run.
