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
| ROM data (tables, text, literals) | 77 | 21 |
| ROM, code or data not told | 72 | 19 |
| RAM (variables, structures) | 47 | 21 |
| I/O registers | 3 | 0 |
| Fields of structures | 84 | 26 |
| Event flags | 21 | 3 |
| Values of fields | 15 | 12 |
| Constants, sizes and counts | 35 | 22 |
| All | 383 | 126 |

| Part of the game | BN6 Gregar | BN5 Team Colonel |
| --- | ---: | ---: |
| PET, mail and key items | 36 | 1 |
| Shops and traders | 14 | 0 |
| NaviCust | 14 | 0 |
| Chips and folders | 55 | 12 |
| Battle | 83 | 86 |
| Text and fonts | 25 | 1 |
| Sound | 8 | 2 |
| Maps and the overworld | 103 | 15 |
| Events and progress | 18 | 2 |
| Engine core | 27 | 7 |

<!-- end of what tools/symbols.py writes -->

## Loading them in a debugger

The `.sym` files are in no$gba's format: a line per symbol, the address as
eight hex digits, a space, the name; a line opening with `;` is a comment.

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
  BN5's `StartBattle` `0x080054F6`).

## Where they come from, and how each is verified

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
and this page's numbers. `python3 build.py lint`, in CI too, fails where
a name in bn6.h or bn5.h has no description, or where a file here differs
from what the generator writes. A new define needs its comment: its own,
or a "..." that continues the one above, or a define above it in the same
group whose comment names it (`BN6_PLAYER_X`'s "X, Y and Z").

## What may be reused

Facts about the ROMs: addresses, names, descriptions and how they were
verified. The files hold no graphics, text, sound or code of the games,
and are under this repository's MIT license; use them, credit welcome. The
games are Capcom's; bring your own ROM.

## Corrections and additions

These files are only as good as what has been verified. A wrong address, a
better name, a description that misleads, a table we have not found: open
an issue or a pull request against the header or docs/ROM_DATA.md row it
comes from (the `source` column), with how you verified it, and
`python3 build.py symbols` writes the files anew.
