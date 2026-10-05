# Symbols

What this project has mapped of Mega Man Battle Network 6: Cybeast Gregar
(USA) and Battle Network 5: Team Colonel (USA), written out for anyone to
use: per ROM, a symbol file that mGBA's and no$gba's debuggers load, and
the same symbols as JSON and CSV. Each symbol is an address with a name, a
kind, what it is, how it was verified and the line of this repository it
comes from.

We do not decompile. The engine runs the player's own ROM and directs it,
so it has to know where things are: RAM it reads and writes, routines it
calls or hooks, tables it reads. Each was located and verified, in the ROM
by experiment (`tools/romlab`, headless captures, autopilot runs) and
against the [bn6f disassembly](https://github.com/dism-exe/bn6f), and is
kept in the engine's headers and docs/ROM_DATA.md. These files are those
sources, written out by `tools/symbols.py`, so they never drift from what
the engine uses.

Beside them, `tools/bn6f_match.py` locates the functions of bn6f, which
was made from Cybeast Falzar, in Gregar and in BN5: 13,386 of its 13,646
functions in Gregar (98.1%), whose addresses differ from Falzar's for most
of the ROM, and 4,329 (31.7%) in BN5, which has no public disassembly.
bn6f carries no license, so its names are not redistributed here: the
files in this repository hold only this project's own symbols, and the
full map, with bn6f's names, is made on your machine from your own bn6f
checkout ([below](#making-the-full-map)).

## The files

| File | What |
| --- | --- |
| [`symbols/bn6-gregar-us.sym`](symbols/bn6-gregar-us.sym) | BN6 Cybeast Gregar (USA): the symbol file for no$gba and mGBA |
| [`symbols/bn6-gregar-us.json`](symbols/bn6-gregar-us.json) | ... every symbol, a structure's fields under it, the record types, event flags and values |
| [`symbols/bn6-gregar-us.csv`](symbols/bn6-gregar-us.csv) | ... the same, a row a symbol |
| [`symbols/bn5-colonel-us.sym`](symbols/bn5-colonel-us.sym), [`.json`](symbols/bn5-colonel-us.json), [`.csv`](symbols/bn5-colonel-us.csv) | BN5 Team Colonel (USA), likewise |

They are for these ROMs only, told by their SHA-1 (the ones `src/core/rom.c`
checks):

| ROM | Header code | SHA-1 |
| --- | --- | --- |
| Mega Man Battle Network 6: Cybeast Gregar (USA) | BR5E | `89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6` |
| Mega Man Battle Network 5: Team Colonel (USA) | BRKE | `5f472f78d8de2df01d5039e045c043cb40969a39` |

A table's columns: `game`, `rom_sha1`, `name`, `kind`, `address` (on the
GBA's bus: `0x08` ROM, `0x02` EWRAM, `0x03` IWRAM, `0x04` I/O), `base` and
`offset` (a field's structure and its place in it), `size` and `count`
where known, `value` (a flag's number, a constant), `description`,
`verified` (the rows of docs/ROM_DATA.md that give the address or name it,
by line), `how` (the first row's own words on how to verify it, or how it
was found) and `source` (the file and line it comes from).

The kinds:

| Kind | What |
| --- | --- |
| `function` | a routine's first instruction (in the full map: where a located function starts) |
| `code` | a place in code: a routine the engine calls or hooks, an instruction it patches or reads |
| `data` | ROM data: a table, text, a literal pool's word |
| `rom` | a ROM address the sources do not say is code or data |
| `ram` | a variable or a structure in EWRAM or IWRAM |
| `io` | an I/O register |
| `field` | a field of a structure: `base` and `offset`, and its own address where the structure has one place |
| `flag` | an event flag (the bit the game's flag routines take) |
| `value` | a value a field takes (`of` names the field) |
| `constant`, `size`, `count` | a number: an id, a limit; a structure's size in bytes, how many there are (`of` names it) |

The JSON holds the same symbols, one a line: a structure's fields stand
under it (`fields`), a pointer field's in turn under it (the toolkit's
`S2001c04_Ptr`), and a structure found in many places (BN5's
`BattleSettings`) is a `type` with its fields and no address. A symbol's
`verified` gives the lines of the docs/ROM_DATA.md rows that verify it,
and the file's `rom_data_rows` says once what each of those rows is and
how to verify it.

## What is mapped

<!-- tools/symbols.py writes this part: holdings -->

| Symbols | BN6 Gregar | BN5 Team Colonel |
| --- | ---: | ---: |
| Code (routines, hooks, instructions) | 29 | 2 |
| ROM data (tables, text, literals) | 85 | 21 |
| ROM, code or data not told | 72 | 19 |
| RAM (variables, structures) | 53 | 21 |
| I/O registers | 3 | 0 |
| Fields of structures | 105 | 26 |
| Event flags | 22 | 3 |
| Values of fields | 40 | 12 |
| Constants, sizes and counts | 64 | 22 |
| All | 473 | 126 |

| Part of the game | BN6 Gregar | BN5 Team Colonel |
| --- | ---: | ---: |
| PET, mail and key items | 40 | 1 |
| Shops and traders | 24 | 0 |
| NaviCust | 17 | 0 |
| Chips and folders | 68 | 12 |
| Battle | 101 | 86 |
| Text and fonts | 26 | 1 |
| Sound | 8 | 2 |
| Maps and the overworld | 106 | 15 |
| Events and progress | 25 | 2 |
| Engine core | 58 | 7 |

<!-- end of what tools/symbols.py writes -->

## bn6f's functions in Gregar and BN5

What `tools/bn6f_match.py` located, as counts (the full map names each
one; [making it](#making-the-full-map)):

<!-- tools/symbols.py writes this part: located -->

Made from bn6f at commit `d57c196`.

| bn6f's functions | BN6 Gregar | BN5 Team Colonel (partial) |
| --- | ---: | ---: |
| All (thumb_func_start, thumb_local_start, arm_func_start, arm_local_start) | 13,386 of 13,646 (98.1%) | 4,329 of 13,646 (31.7%) |
| ... global (thumb_func_start, arm_func_start) | 2,688 of 2,751 (97.7%) | 1,304 of 2,751 (47.4%) |
| ... named by bn6f (not sub_ and an address) | 2,306 of 2,323 (99.3%) | 1,290 of 2,323 (55.5%) |
| ... of them with no address in the name | 1,062 of 1,072 (99.1%) | 776 of 1,072 (72.4%) |
| Located by `bytes` | 9,837 | 3,009 |
| Located by `calls+bytes` | 1,905 | 570 |
| Located by `neighbours` | 1,525 | 0 |
| Located by `calls` | 108 | 750 |
| Located by `shift` | 11 | 0 |
| Data labels the located functions name | 2,734 | 215 |
| RAM labels their literals hold at Falzar's address | 895 | 10 |
| RAM labels their literals hold elsewhere | 0 | 341 |

| Part of the game (bn6f's files) | BN6 Gregar | BN5 Team Colonel |
| --- | ---: | ---: |
| Battle | 9,096 of 9,210 (98.8%) | 2,486 of 9,210 (27.0%) |
| Menus and screens | 1,581 of 1,581 (100.0%) | 432 of 1,581 (27.3%) |
| Maps and the overworld | 1,009 of 1,061 (95.1%) | 348 of 1,061 (32.8%) |
| Sound and libraries (m4a, link cable, wireless) | 407 of 407 (100.0%) | 373 of 407 (91.6%) |
| Shops and traders | 296 of 296 (100.0%) | 171 of 296 (57.8%) |
| Engine core | 278 of 286 (97.2%) | 161 of 286 (56.3%) |
| NaviCust | 204 of 204 (100.0%) | 111 of 204 (54.4%) |
| Text and fonts | 202 of 202 (100.0%) | 127 of 202 (62.9%) |
| Minigames and their maps | 66 of 152 (43.4%) | 3 of 152 (2.0%) |
| Other | 146 of 146 (100.0%) | 26 of 146 (17.8%) |
| Graphics | 83 of 83 (100.0%) | 74 of 83 (89.2%) |
| Chips and folders | 18 of 18 (100.0%) | 17 of 18 (94.4%) |

- BN6 Gregar: 39 times an address of ours names a bn6f function or label the matcher located: 30 at the same place, 7 inside the function or structure, 2 a field of the structure it is, 0 elsewhere.
- BN6 Gregar: of the 895 RAM labels the located functions' literals hold, 895 read bn6f's (Falzar's) address.
- BN6 Gregar: 13,162 located functions laid out as in Falzar, 0 lying over another; 0 that a located reference names elsewhere.
- BN5 Team Colonel: 8 times an address of ours names a bn6f function or label the matcher located: 3 at the same place, 5 inside the function or structure, 0 a field of the structure it is, 0 elsewhere.
- BN5 Team Colonel: 3,574 located functions laid out as in Falzar, 0 lying over another; 40 that a located reference names elsewhere.

<!-- end of what tools/symbols.py writes -->

A function counts as located once one of the ways below places it.
"Named" is a name bn6f's people gave it, not one IDA made of its address
(`sub_` and the address). BN5 is partial: it is another game, and only
what its code shares with BN6's, byte for byte where nothing depends on
where things are, is found; what it does differently is not looked for.
The checks: each address of this project's own that names a bn6f function
or label (its comment's "bn6f NAME", the name it opens with, or its
docs/ROM_DATA.md row), against where the matcher put that one, and the
match tables' own consistency.

### How they were located

`tools/bn6f_match.py` reads bn6f's source (agbasm, GNU as with
luckytyphlosion's extensions) and lays each function out as its
assembler would at its Falzar address: every Thumb and ARM instruction
made into its bytes (divided syntax, as GNU as makes it: a `mov` between
low registers is `adds rd, rs, #0`, an `ldr rd, =x` that fits a byte a
`mov`), its literal pools, its alignments, the structures' offsets added
up from their fields' types. What depends on where things are is masked:
a `bl`'s target, a branch out of the function, a load from a label outside
it, a literal word that names a label. A function's Falzar address comes
from the addresses in its labels' IDA names, where its layout puts them;
where a function has none, it follows the one before. bn6f's `loc=`
comments on its structures are not always right, so a field's offset is
the one its type adds up to, and masked where that and the comment
disagree. Then:

| Confidence | How |
| --- | --- |
| `bytes` | its masked bytes found exactly once in the 8 MB ROM (at least 48 known bits, a run of 4 whole bytes) |
| `calls+bytes` | its bytes found more than once (bn6f has many alike), and the calls, branches and pointers of located functions all name this one |
| `neighbours` | its bytes are at the shift from Falzar that the located functions before and after it in its file share |
| `calls` | named by the calls, branches and pointers of located functions, all agreeing; its own bytes differ from Falzar's (it changed between the versions) |
| `shift NN%` | estimated: at its neighbours' shift, where NN% of its known bits are Falzar's |

Only a function whose bytes were found (`bytes`, `calls+bytes`,
`neighbours`) is laid out as in Falzar, so only its references are read:
each `bl`, outside branch, literal and jump table entry is read from the
ROM where it was found, and names where its target is. Where bn6f's bytes
were found at one place for two functions (bn6f has some twice, Gregar
once), the one whose neighbours moved alike keeps it; a function found once
but where both of its located neighbours, moved alike, say it is not, is
another function's bytes and not taken; two that would lie over each other
are untangled the same way. The data between functions (jump tables,
pools) is placed where the references put its labels, its known bytes
there, and its own pointers name more. The IWRAM code is found by its copy
in the ROM, which is copied to IWRAM in one piece: each function runs at
its copy's place less the piece's shift (Gregar's copy is 4 bytes shorter
from `0x03006F20` on, so those run 4 bytes before Falzar's). In BN5,
another game laid out otherwise, only the bytes and the references count:
no neighbours, no IWRAM code, and a function found by its bytes where more
located references name another place than this one is dropped.

What is not located lies in runs between located functions where Gregar's
code is not Falzar's: the minigames' code most of all, runs of the battle
code, and the code among the map data. The full map's report
(`.build/symbols/located.md`) counts them by bn6f's file, and
`python3 tools/bn6f_match.py --bn6f PATH --report NAME` shows how a
function's bytes differ where its neighbours put it.

### Why bn6f's names are not in these files

bn6f's names for what it found are its contributors' work.
[dism-exe/bn6f](https://github.com/dism-exe/bn6f) carries no license
(GitHub shows none, the repository has no LICENSE file, and its README and
CONTRIBUTE.md say nothing of reuse; checked 4 October 2026), so the right
to share them is theirs alone. This repository therefore commits only
this project's own symbols, the tools and the counts above: no name,
label or address list made from bn6f. The matcher keeps nothing of bn6f's
but names and addresses, and what it writes stays in `.build/`, which git
ignores.

### Making the full map

With a checkout of bn6f and the ROMs (any `.gba` of the SHA-1s above, in
`~/.cache/mmbn-ref/roms` or `--roms DIR`):

```sh
git clone https://github.com/dism-exe/bn6f ~/bn6f    # outside this repository
python3 build.py symbols --bn6f ~/bn6f    # about two minutes, no Docker
```

`tools/bn6f_match.py` writes its tables to `.build/symbols/match/`, then
`tools/symbols.py --full` merges them with this project's symbols into
`.build/symbols/`: `bn6-gregar-us.sym`, `.json` and `.csv` and BN5's
likewise, each function and label under bn6f's name with its Falzar
address and how it was located (the columns `bn6f`, `falzar` and
`confidence`; in the `.sym`, bn6f's name after ours where both name one
address), and `located.md`, the counts by bn6f's file and every check by
name. It also writes the counts above anew, from the bn6f commit you
have. The full map is for your own use: share it only as bn6f's
contributors allow.

## Loading them in a debugger

The `.sym` files are in no$gba's format: a line per symbol, the address as
eight hex digits, a space, the name; a line opening with `;` is a comment.
The full map's `.thumb` and `.arm` lines mark where a function's code
starts and in which instruction set.

- **no$gba**: put the file beside the ROM under the ROM's own name
  (`game.gba`, `game.sym`); no$gba reads it as it loads the ROM and shows
  the names in its disassembly (no$gba's help, "Symbolic Debug Info":
  eight-digit address, a space or TAB, the name; `;` comments; `.arm`,
  `.thumb`, `.byt:`, `.wrd:`, `.dbl:`, `.asc:` and `.pool` reserved).
- **mGBA** (0.10): the same file beside the ROM under its name loads when
  the debugger attaches (`mgba -d game.gba`, or the debugger console of the
  Qt build). Or, in the debugger, `load-symbols path/to/bn6-gregar-us.sym`.
  Then the names show in `disassemble`, `symbol ADDRESS` names an address,
  and a name is an address in any command: `break BN6_GIVE_ITEM`,
  `watch BN6_ZENNY`, `x/4 BN6_TOOLKIT 8`. mGBA reads the format with
  `mDebuggerLoadARMIPSSymbols` (`src/debugger/symbols.c`): eight hex
  digits, spaces, the name up to a comma; it skips lines that open with a
  dot or are not an address, so no$gba's markers and the comments pass by.
  Checked with that loader itself, built alone from mGBA 0.10.5's source:
  both files load, and every name tried gives its address and the address
  its name (`BN6_GIVE_ITEM` `0x0803CD6C`, `BN6_TOOLKIT` `0x020093B0`,
  BN5's `StartBattle` `0x080054F6`); the full map's files too, where two
  names share an address the address gives bn6f's.

## Where our own symbols come from, and how each is verified

- `src/emu/bn6.h` and `src/emu/bn5.h`: every name the engine uses for the
  game, with its comment as its description. A field defined as
  `(BASE + offset)` is a field of BASE; one named by its offset alone
  belongs to its family's structure (`tools/symbols.py`, `FAMILIES`); a
  define whose comment opens with another's name and a colon is a value of
  that one (`BN6_PANEL_HOLE`: `BN6_PANEL_TYPE: a hole`); event flags are
  the `_FLAG_` names.
- `src/core/rom.c`: the ROM offsets of `RomLayout` (BN6) and `XRomLayout`
  (BN5), described by `src/core/rom.h`'s comments.
- `docs/ROM_DATA.md`: the addresses its rows give that the headers and the
  layouts do not, named by the row's own words (a name it gives, else its
  words: "the BugFrag spent" is `bugfrag_spent`, a word alone after the
  row's first words, `rush_gaps.check`), and for every symbol the rows that
  give its address or name it: `verified` lists them, `how` quotes the
  first one's way to verify it.

docs/ROM_DATA.md is where each address's finding is written down (how it
was located, how to verify it), and docs/SOURCES.md says where a claim
about the games is looked up first.

## Keeping them in step

`python3 build.py symbols` (or `python3 tools/symbols.py`) writes the files
and this page's numbers of them. `python3 build.py lint`, in CI too, fails
where a name in bn6.h or bn5.h has no description, or where a file in
`docs/symbols` differs from what the generator writes; it needs no bn6f.
A new define needs its comment: its own, or a "..." that continues the one
above, or a define above it in the same group whose comment names it
(`BN6_PLAYER_X`'s "X, Y and Z"). The counts of bn6f's functions are written
by `build.py symbols --bn6f PATH` alone, from the bn6f commit they name.

## What may be reused

Facts about the ROMs: addresses, names, descriptions and how they were
verified. The files hold no graphics, text, sound or code of the games,
and are under this repository's MIT license; use them, credit welcome. The
games are Capcom's; bring your own ROM. bn6f's names are not in them (see
above).

## Credits

The [bn6f disassembly](https://github.com/dism-exe/bn6f) by dism-exe
(luckytyphlosion, LanHikari22 and their contributors): its source is what
`tools/bn6f_match.py` makes the functions from, its names what the full
map calls them, its structures what the fields' offsets are added up
from, and it is what this project's own addresses were checked against.
Its people keep their notes in
[dism-exe-notes](https://github.com/dism-exe/dism-exe-notes) and talk on
the pret Discord linked from bn6f's README. mGBA's source and no$gba's
documentation for the symbol format.

## Corrections and additions

These files are only as good as what has been verified. A wrong address, a
better name, a description that misleads, a table we have not found: open
an issue or a pull request against the header or docs/ROM_DATA.md row it
comes from (the `source` column), with how you verified it, and
`python3 build.py symbols` writes the files anew. The functions not yet
located, the `calls` and `shift` ones most of all, and BN5's own code are
where help with the matcher goes furthest.
