# The PET

BN6's PET (START on the map) is Lan's own terminal, and so the most
in-world place for what the run knows: what lives in it reads as MegaMan's
and Lan's, not as a game menu (the diegesis matrix: docs/META.md, what
MegaMan knows). A run keeps BN6's entries and gives the ones BN6 greys out
a use of its own.

| Entry | In a run |
| --- | --- |
| ChipFolder, SubChip, Library, MegaMan | BN6's own |
| E-Mail | BN6's screen; mail from Dad and the net (planned) |
| KeyItem | BN6's screen; the run's and the profile's own items (planned) |
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

## How the entries are taken over

Flag `0x1706` (`EVENT_PET_COMM_SAVE_DISABLED`), which a run keeps set,
greys Comm and Save and makes A on them buzz. Two patches on the core's ROM
copy (`pet.c`, docs/ROM_DATA.md) turn the buzzer into a note for the
engine, the cursor's entry written to the menu's spare byte
(`0x0200DF2F`), and leave the two entries ungreyed; the engine reads the
byte each frame. BN6's own Comm and Save never run.
