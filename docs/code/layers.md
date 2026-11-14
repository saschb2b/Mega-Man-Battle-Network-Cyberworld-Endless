---
type: Measurement
title: "The code's layers"
description: "Which part of `src/` may include which: the dependencies between its folders measured (`tools/layers.py`), the intended order as 24 units lowest first (`tests/lint/layers.txt`), the rule `build.py lint` holds with a baseline that only shrinks, how to fix a violation, what the six widest headers' includers use of them, the files a tool writes, the first ten cuts and the numbers a release records."
tags: [code, lint, architecture, process]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-14T03:30:00Z }
sources:
  - { id: measure, title: "python3 tools/layers.py and its --cuts, --uses and --generated on dev at 2744367e (12 November 2026)", resource: "tools/layers.py" }
  - { id: rule, title: "tests/lint/layers.txt, the units and what each may include; tests/lint/layer_violations.txt, the includes it does not allow as they stood", resource: "tests/lint/layers.txt, tests/lint/layer_violations.txt" }
---

# The code's layers

`build.py lint` has long held the files (their size, their functions, their
smells: [the tests](tests.md), `tests/lint/`). Nothing held the level above
them: every folder of `src/` is on the include path, so any file may include
any header, and by November 2026 most folders included each other. This
page is that level's map: what was measured, the order the code is meant to
have, the rule that keeps it from getting worse, and the first steps to
make it better. What each folder holds, file by file, is in
[the code's concepts](overview.md).

## What was measured

`python3 tools/layers.py` resolves every `#include "x.h"` in `src/` to the
folder that owns `x.h` (a header's name is its address: no two folders own
the same one) and reports the graph. On `dev` at `2744367e`, 12 November
2026 [^measure]:

- **1,118 files, 179,378 lines**: 153,684 written by hand and 25,694 by a
  tool (17 files, [Generated code](#generated-code)).
- **18 folders, 129 edges between them, 34 pairs that include each other.**
  17 of the 18 are one knot (each reaches every other); only `scenes` is
  outside it, because nothing includes it. There is no longer cycle to
  find beyond the pairs: no three folders form a cycle without a pair
  among them.
- 2,039 includes cross a folder's edge. The rule below forbids 147 of
  them: the knot is wide, but most of what crosses already goes downward.

| Folder | Files | By hand | By a tool | Includes | Included by |
| --- | ---: | ---: | ---: | ---: | ---: |
| `analytics` | 16 | 1,912 | | 7 | 4 |
| `audio` | 21 | 3,070 | 2,150 | 2 | 11 |
| `core` | 165 | 24,997 | 1,360 | 10 | 17 |
| `dev` | 56 | 8,579 | | 14 | 5 |
| `director` | 163 | 24,090 | | 14 | 9 |
| `emu` | 221 | 27,993 | 22,184 | 11 | 12 |
| `gfx` | 26 | 4,251 | | 3 | 13 |
| `jukebox` | 6 | 695 | | 3 | 3 |
| `launcher` | 20 | 2,883 | | 6 | 2 |
| `layer` | 60 | 9,065 | | 5 | 12 |
| `library` | 6 | 637 | | 6 | 4 |
| `map` | 39 | 7,671 | | 5 | 7 |
| `net` | 29 | 5,754 | | 2 | 10 |
| `online` | 201 | 22,450 | | 13 | 6 |
| `saves` | 11 | 1,002 | | 6 | 2 |
| `scenes` | 18 | 2,058 | | 7 | 0 |
| `second` | 29 | 2,526 | | 9 | 8 |
| `world` | 31 | 4,051 | | 6 | 4 |

(Includes, included by: the count of other folders.) `core` is the hub:
it includes 10 folders and all 17 others include it. `emu`, `director`,
`online` and `dev` each include more than ten.

The pairs, with the includes each way, are the report's second table; the
widest are `director` and `emu` (221 down, 15 back), `core` and `director`
(3 up, 195 down), `emu` and `online` (4, 125), `core` and `emu` (41, 109).
In nearly every pair one direction is heavy and the other a handful of
includes: the order is already there, with leaks.

## The map

Two folders do not sit at one level, so the map's unit is a folder **or a
part of one**, named in `tests/lint/layers.txt` [^rule] by its files:

- `core` holds five levels ([core's files](#cores-files-by-level)).
- `emu`'s `bn6*.h`, `bn5*.h`, `bn4*.h`, `bytes.h` and `free_space.h` are
  the games' addresses and record layouts, numbers alone, which everything
  from audio to the second screen reads: the lowest level, `formats`.
- `emu`'s `scene_emu`, `emu_picture`, `game_hooks`, `fast_forward_game`,
  `feel_game`, `autopilot` and `showcase` are not the emulator but the
  game's scene: the frame that runs the core, the director and the screens
  over it. They include the director, the Square and the second screen
  because putting those together is their job: `emu.game`, near the top.

The 24 units, lowest first. A unit may include only the units its line
names, and a line names only units above it:

| Unit | What it is | Files | May include |
| --- | --- | ---: | --- |
| `formats` | The games' addresses, record layouts, byte packing, the free space's plan; headers alone | 39 | nothing |
| `core.base` | The platform, input, settings, the ROM file and `RomLayout`, the scenes' switch | 56 | `formats` |
| `core.data` | The ROM's tables: chips, programs, foes, the other games' records and registry | 27 | `formats` `core.base` |
| `emu` | The mGBA core, hooks, gamecalls, the game's memory, the moved families, Navis, chips and Souls | 171 | `formats` `core.base` `core.data` |
| `core` | The run: its state, rules, loot, pacing, saves, the profile | 57 | `formats` `core.base` `core.data` |
| `net` | Layer generation | 29 | `formats` `core.base` `core.data` `core` |
| `audio` | The engine's MP2K, the device, the sound filter | 21 | `formats` `core.base` `emu` |
| `gfx` | Sprites, tiles, fonts, the PET's look | 26 | `formats` `core.base` `core.data` `emu` |
| `analytics` | The anonymous statistics: a sink the levels above report to | 16 | `formats` `core.base` `core.data` `core` `net` `audio` `gfx` |
| `map` | Layers as the game's maps | 39 | `formats` `core.base` `core.data` `core` `emu` `net` `gfx` |
| `layer` | What stands on a layer | 60 | `formats` `core.base` `core.data` `core` `emu` `net` `map` |
| `world` | The real world and the town | 31 | the same, and `layer` |
| `jukebox` | The Jukebox | 6 | `formats` `core.base` `audio` `gfx` |
| `library` | The Library | 6 | `formats` `core.base` `core.data` `core` `audio` `gfx` |
| `core.options` | Options and its panels | 15 | `formats` `core.base` `emu` `audio` `gfx` `analytics` |
| `director` | The run on the game | 163 | everything above this line |
| `online` | Online play's client | 201 | everything above the director's line but `world` and `core.options`, and `director` |
| `second` | The second screen: a view of all of it | 29 | everything above this line |
| `launcher` | The ROMs screen | 20 | `formats` `core.base` `core` `audio` `gfx` `core.options` |
| `saves` | SAVES | 11 | `formats` `core.base` `core` `audio` `gfx` `launcher` |
| `scenes` | The start, the title, the error screen | 18 | `formats` `core.base` `core.data` `core` `audio` `gfx` `layer` `core.options` `director` `second` `launcher` `saves` |
| `emu.game` | The game's scene | 12 | everything above this line |
| `dev` | Developer tools inside the game | 56 | everything above this line |
| `core.app` | The entry point and each system's start | 9 | everything |

What the order says, beyond the obvious:

- **The run does not know the emulator, and the emulator does not know the
  run.** `core` (the run) and `emu` stand side by side on `core.data`;
  neither may include the other. `tests/test_core.c` already builds the
  run's rules and the generator without mGBA; the rule keeps that true.
  It costs 8 baseline entries (7 from the run: which virus families and
  fields are installed; 1 from the emulator: a debug line's depth).
- **Generation is pure.** `net` may not include `emu`, `gfx` or `map`; it
  has no violation today.
- **A layer's objects do not draw.** `layer` and `world` include neither
  `gfx` nor `audio` today, and may not start.
- **The statistics are a sink, not a watcher.** `analytics` stands low so
  that the director, the Square and Options may report to it; what it
  wants to know from above it is handed in (4 entries reach up today).
- **The launcher and SAVES work before a game exists**: they may not
  include `layer`, `emu` or the director.

### Core's files by level

| Unit | Files of `src/core/` |
| --- | --- |
| `formats` | `chip_entry.h` (a chip's entry as the game packs it; `bn6_chips.h` includes it) |
| `core.base` | `platform*`, `present_3ds`, `second_android`, `sdl_old.h`, `app_icon.h`; input: `buttons.h`, `pads`, `padmap`, `keymap`, `touch`, `touch_layout`, `feel`, `feel_out`, `fast_forward`; `display`, `frame_log`; `compat`, `save_queue`, `settings_line`, `text_input`, `network_3ds`; `desktop*`, `ios*`, `ps5_*`; `rom` (the ROM files and `RomLayout`), `rng.c`, `game.h` |
| `core.data` | `data`, `chips`, `chip_pool`, `foes.h`, `library_pages*`, `xchips*` (the registry, the moved chips, the Program Advances), `xprograms`, `xrecords` |
| `core` (the run) | Everything else: `run*`, `save*`, `backup*`, `meta*`, `loot*`, `pacing`, `economy`, `jobs`, `rivals`, `weekly`, `groups`, `group_*`, `formations`, `visitors`, `visitor_families`, `xbattles`, `super_boss`, `battle_data`, `extras*` |
| `core.options` | `options*`, `controls*`, `touch_art`, `pet_panel.h` |
| `core.app` | `main.c`, `startup`, `start_3ds`, `start_console.h`, `start_ps4`, `start_switch`, `start_vita` |

A new file of `core` that no pattern takes is the run's. `game.h` stands
in `core.base` because `net` and `emu` include it for the PRNG and the
data folder; what else it declares belongs higher
([The wide headers](#the-wide-headers)).

## The rule

`build.py lint` runs `python3 tools/layers.py --lint` (a fifth of a
second, no build, no ROM):

- **`tests/lint/layers.txt`** holds the units and what each may include:
  `unit = folder/pattern ...` names a part of a folder (the first line
  that matches a file takes it; what no line takes is its folder's own
  unit), `unit: units` what it may include (`*`: every unit on the lines
  above). A line names only units above it, so the rules themselves can
  hold no cycle.
- **`tests/lint/layer_violations.txt`** lists the includes the rule does
  not allow as they stood when it came, `src/folder/file -> folder/header`
  a line: 147. It only shrinks. A new one fails lint; one that is gone is
  reported, and `build.py lint --update` drops it, as with the other
  baselines.
- A folder or a unit without a line, a header's name owned by two
  folders, a pattern that matches nothing: each fails with what to do.

A failure names the file, the include, the rule and the way out:

```
  new: src/net/net_gen.c includes "director.h" (src/director/director.h): net may not include director
         the rule (tests/lint/layers.txt): director is above net; net may include: formats core.base core.data core
         the way out: move what is needed into a header the lower part owns, pass it in as an
         argument, or have the upper part register a callback (docs/code/layers.md, Fixing a violation)
```

The rule reads includes, not calls: a function declared by hand to dodge
its header would pass. The headers' own rule (every `.c` file includes its
header first, every header compiles alone) and `-Wmissing-prototypes` in
the warnings make that an awkward thing to do by accident.

### Fixing a violation

`python3 tools/layers.py --why A B` lists each include from A to B with
the names its file uses of that header, which is usually one or two. Then,
in this order:

1. **The include is not needed** (the file uses no name of it, or one a
   lower header also gives): drop it.
2. **The name is at the wrong level.** A constant, a name table or a small
   type that upper and lower parts both need belongs to the lowest part
   that needs it: move the declaration (and its definition) down. No
   include line changes when a file moves between folders, as every
   folder is on the include path.
3. **The lower part wants a fact from above**: have the caller pass it in
   as an argument (the statistics wanting a guardian's name: the director
   hands the name over with the event).
4. **The lower part must start something above** (the platform flushing
   the saves' copy as the app goes to the background): the lower part
   owns a small header with a hook (`platform_on_background(fn)`), and the
   upper part registers its function at its start. The call still
   happens; the include points down.
5. **The rule is wrong**: a unit truly needs one below it that its line
   does not name. Add it to the line, in the pull request that needs it,
   with the reason in the commit. A unit never names one below its line.

Do not add the include to the baseline by hand: the list only shrinks.

One case adds to it. A branch cut before the rule came (12 November 2026)
does not have it in its own lint, and its checks may have run before the
rule reached `dev`: merged, its new include turns `dev`'s lint red for
everyone. Where that include is used and belongs to a group the plan
already ends by a move ([Where the code calls upward](#where-the-code-calls-upward)),
it joins the list as it would have stood, in a pull request that says so.
Two did: `src/emu/xnavi_bn4_chip.c`'s `xsong.h` and `xnavi.h` (pull request
#403), which makes 149. `python3 tools/layers.py` on a pull request merged
with `dev` shows such a one before it lands; a branch cut since has the
rule in its own lint and cannot bring one.

## Where the code calls upward

The 147 by what causes them (`python3 tools/layers.py --cuts` has every
edge with its files). These are real: the mechanism is what would turn
each around, and the count what it would take off the list.

| What reaches up | Entries | What turns it around |
| --- | ---: | --- |
| The dev switches read from below: `devtools.h` in `director` (12), `online` (14), `emu` and its scene (5), `library` (1) | 32 | The switches' struct in a header `core.base` owns (`dev_switch.h`, `dev` filled by `src/dev/`); the dev menu's own functions stay in `devtools.h` |
| The second screen's drawing kit: `second_frame.h` in `online` (8), `launcher` (2), `saves` (1); `second_art.h` and `second_card.h` in `online`, `library`, `director` (4) | 15 | The kit is graphics: `second_art.*` to `gfx` as it is; `second_frame.*` after its colours and frame are parted from `second_state.h` |
| The platform starting things above it: `platform.c` including `audio.h`, `analytics_net.h`, `mirror.h`, `emu.h`, `gfx.h`; `touch.c` asking the director and fast forward; `frame_log.c`, `platform_second.c`, `display.h`, `desktop*.c` | 13 | Hooks the platform owns, registered at startup (pause, background, quit, each frame's end); `touch` told its state (four-way, fast forward shown) instead of asking; the LCD's two types declared with the display's settings |
| `core.base` including Options and the saves' carrier: `options.h` (2), `options_text.h`, `touch_art.h`, `backup.h` (4) | 8 | `BACKUP_NAME` and the settings Options edits declared in `core.base`; Options reads and writes them |
| The run reading generation's types: `net.h` in `loot.c`, `meta.c`, `run.c`, `run_glitch.c`, `xbattles.c` and `rom.c`; `layer_make.h` in the backup (2) | 8 | The areas' and looks' numbers in a header of the run's (`areas.h`); the layers' make in `core.base` |
| The run asking the emulator what is installed: `xvirus.h` (3), `xvirus_bn4.h` (2), `soulkinds.h`, `xfields.h` | 7 | A table in `core.data` the installers fill, as the chip registry already is |
| The installers using `audio`'s song installer: `xsong.h` in seven of `emu`'s files | 7 | `xsong.*` writes the game's song table: it is the emulator's, and moves there |
| The emulator's installers reading `layer`'s `xnavi.h` | 7 | The other games' Navis' table is data: its header to `core.data` |
| The director opening the Square: `online_visit.h` (3), `netpass_screen.h`, `network_prefs.h`, `online_field.h`, `square_pet.h` | 7 | The one that is genuinely upward: `online` registers its gate and screens with a small table the director owns |
| `layer` reading the director's numbers and names: `powers.h` (2), `darkchips.h`, `liberation_board.h`, `reward_pick.h`, `souls.h` | 6 | Names and prices are the run's: the crosses' names and `DARK_PRICE` to `core`, the reward's kinds to `layer` |
| The run reading `layer`: `guardians.h` (2), `xguardian.h` (2); `text.h` in `chips.c` | 5 | The guardians' table is the run's data; the game's text encoding is a format |
| The statistics looking up: `guardians.h`, `powers.h`, `online_visit.h`, `pet_panel.h`; the run calling it (`meta.c`) | 5 | Names handed in with the event; the run's start reported by the director, which starts it |
| `start_3ds.h`'s two questions asked from `frame_log.c`, the scene, `second`, `dev` | 4 | Declared by the platform, answered by the 3DS's start |
| The scene's parts from below: `autopilot.h` and `feel_game.h` in the director, `showcase.h` in Options; `tour.h` from the scene | 4 | Each a flag or a per-frame call the scene makes instead of being asked |
| The rest: `lz.h` (3), `startup.h` in the launcher (2), `audio.h` in `save.c` and `weekly.c` (2), and twelve of one each | 19 | [The first cuts](#the-first-cuts) take nine of them |

## The wide headers

`python3 tools/layers.py --uses HEADER` lists who includes a header and
which of its names each includer uses. "Reach" counts the files that get
it through another header too, which is what a change to it rebuilds: of
562 compiled files, `bn6.h` reaches 256, `free_space.h` 239, `hook.h`
237, `emu.h` 227, `rom.h` 212, `run.h` 136. Proposals, none built:

**`bn6.h`** (181 include it, 316 reach it; 400 lines, the limit, with 13
headers it includes in turn). 38 of its includers use none of its own 336
names: 27 only names of the headers it includes (`bn6_chips.h` 6,
`bn6_families.h` 6, the rest spread), 11 nothing at all.
1. Those 38 include what they use: 181 to 143.
2. Its own names parted as the 13 already are: the battle's (`BN6_T1_*`,
   `BN6_BATTLE_*`, `BN6_PANEL_*`, the phases), the world's and the PET's
   (maps, the chat box, the folder), the system's (`BN6_EWRAM`,
   `BN6_TOOLKIT`, the modes). By prefix, 26 includers use only the
   battle's, 19 only the world's, 4 only the system's: about 49 more
   would include one part, and `bn6.h` stays as the header that includes
   them all.
A new address then rebuilds the files of its concern; `bn6*.h` changed in
162 commits in the month to 12 November.

**`emu.h`** (178, 259). It includes `free_space.h` and `hook.h` and uses
no name of the first: 283 files reach `free_space.h` and 69 use it, and
that header is written anew with every region a session claims (47
commits in the month), so each claim rebuilds 239 files.
1. `emu.h` stops including `free_space.h`; the 39 files that use its
   names without including it add the line. About 214 files stop seeing
   it. This is the cheapest large win on this page.
2. Memory access in a header of its own (`emu_mem.h`: `emu_read8/16/32`,
   `emu_write*`, `emu_ready`): 92 of the 178 use nothing else of `emu.h`.
   `emu.h` keeps the core's life, keys, sound and picture: 178 to 86.
3. `hook.h` for those who hook: 55 use its names, 21 include it already.

**`rom.h`** (170, 219) holds three things: BN6's ROM (`R`, `ROM_SIZE`,
`rom_u32`, `rom_find`), the other games' (`XR`, `XROM_*`, `xrom_*`) and
the net areas' table (`NetAreaDef`, `NET_AREAS`, `net_area_def`,
`XLOOK_*`).
1. The areas' table to a header of its own: 24 includers use it, 6 of
   them nothing else. It also ends `rom.c`'s include of `net.h` upward.
2. `xrom.h` for the other games' ROMs: 31 includers use only those names,
   52 only BN6's, 60 both. `rom.h`: 170 to 133.

**`platform.h`** (97, 99). 73 includers use the global `P`, and 43 of
those for `P.core_x` and `P.core_y`, where the picture sits; 21 use
`btn_pressed`, 18 `platform_persist`.
1. The picture's place, `CORE_W`, `btn_pressed`, `back` and
   `platform_tap` in a small header (`screen.h`): 30 includers use
   nothing else. 8 use only `platform_persist`, which is the saves'
   business (`save_queue.h`).
2. `P`'s fields are written by `core.base` (28 of them) and, a few each,
   from `scenes`, the game's scene, `main.c`, `emu`, `online` and `dev`;
   those few become calls.

**`run.h`** (94, 146: 52 files get it only through another header,
`save.h` above all, and 26 of them use no name of it). 67 includers use the global `run`; 26
of `Run`'s fields are touched outside `run.c`, in 88 files (`depth` in
53, `side_kind` 31, `biome` 30, `seed` 26).
1. The areas' numbers (`BIOME_*`, `CYCLE_LAYERS`) to their own header,
   with `rom.h`'s areas' table: 13 includers use nothing else.
2. An opaque `Run` with accessors is not proposed: `Run` is the save's
   format (a raw struct with a checksum, `AGENTS.md`'s Rules), 88
   files read its fields, and an accessor for each would be churn without
   a gain. What an opaque type would buy is held more cheaply: **who
   writes it**. Today only the run itself (19 fields), the director (17),
   the entry point (6) and `dev` (6) assign to `run.*`; a check in
   `tools/layers.py` that no other unit does keeps that true.
3. `save.h` includes `run.h` for the prototypes that take a `Run *`
   (`peek_run`, `load_run`, `save_run`): those in a header of their own
   (`save_run.h`) let the 26 stop reaching it.

**`game.h`** (77, 83) declares seven things: the PRNG, the data folder,
the scenes' switch, the title's state, the launcher's entry, the game
scene's start and `error_show`. 29 includers use only the PRNG, 24 only
`g_data_dir`, 7 only the scenes' switch: 60 of the 77.
1. `rng.h`, the data folder with `compat.h`, `scene.h`.
2. The title's state (`title_*`) to `src/scenes/`, the launcher's entry to
   `launcher.h`, the scene's start to `scene_emu.h`: they are declared in
   `core.base` and defined four levels up, which the rule cannot see.

## Generated code

17 files of `src/` are written by a tool, 25,694 lines, a seventh of the
tree (`python3 tools/layers.py --generated`):

| File | Lines | Written by |
| --- | ---: | --- |
| `src/emu/xnavi_bn5.c` | 5,250 | `tools/xnavirecipe.py` |
| `src/emu/xchips_bn5.c` | 5,092 | `tools/xchiprecipe.py` |
| `src/emu/xnavi_bn4.c` | 3,644 | `tools/xnavirecipe_bn4.py` |
| `src/emu/xvirus_bn5.c` | 3,066 | `tools/xrecipe.py` |
| `src/audio/intro_chime.c`, `.h` | 2,150 | `tools/intro_chime.py` |
| `src/emu/xchips_bn4.c` | 2,021 | `tools/xchiprecipe_bn4.py` |
| `src/emu/xvirus_bn4.c` | 1,970 | `tools/xrecipe_bn4.py` |
| `src/core/library_pages_data.c` | 528 | `tools/library_pages.py` |
| `src/emu/soulcode_bn5.c` | 427 | `tools/xsoul.py` |
| `src/emu/xbracket_bn4.c` | 420 | `tools/xrecipe_bracket.py` |
| `src/emu/free_space.h` | 294 | `tools/free_space.py`, from `free_space.plan` |
| `src/core/app_icon.h` | 263 | `tools/app_icon.py` |
| `src/core/xchips_moved.c`, `xchips_pas.c` | 249 | `tools/xchiprecipe.py` |
| `src/core/xchips_registry_bn4.c` | 213 | `tools/xreg_bn4.py` |
| `src/core/group_tags.c` | 107 | `tools/encounter_tags.py` |

Nothing but each file's opening comment tells them from hand-written
code, and the comments differ: `build.py lint`'s own test (the first
three lines say "written by tools/") knows 12 of the 17 and misses
`free_space.h`, `group_tags.c`, `xchips_moved.c`, `xchips_pas.c` and
`xchips_registry_bn4.c`, whose word comes later or in capitals. A review
sees a 5,000-line diff beside a 20-line one. Proposed, in two steps:

1. **A first line every generated file carries, which lint requires**
   (no file moves): `/* GENERATED by tools/NAME.py: do not edit. ... */`.
   - The generators (fourteen tools) write it; `tools/layers.py`'s table and
     `build.py`'s `generated()` both become "the first line says so".
   - Lint: a file with the line names a tool that exists; a file whose
     comment says a tool wrote it has the line.
   - `.gitattributes` gets `linguist-generated=true` for each, so GitHub
     folds their diffs in a pull request. (GitHub reads this attribute,
     unlike `merge=union`, [Merging in parallel](../project/merging.md).)
2. **A `gen/` folder per folder** (`src/emu/gen/xnavi_bn5.c`), once the
   chip and Navi sessions that write these files have merged. What
   changes: the Makefile's `SRCS` and `SRC_DIRS` (`src/*/*.c` becomes two
   patterns, and the objects' folders are made one deeper), Android's
   `CMakeLists.txt` (the same two globs), the tests' source lists that
   name a generated file, every generator's output path, `build.py lint`'s
   globs (`lint_headers`, the smells, the unreached functions' pattern
   `folder/file.o`), `tools/layers.py` (a file's folder is its first
   path part) and the rows in [the emulator's](emu.md),
   [the core's](core.md) and [graphics and sound's](gfx-and-audio.md)
   concepts. No include line changes.

Step 1 gives the reader and the review what they need; step 2 gives the
file listing the same, and is the owner's call.

## The first cuts

Ten baseline entries or small groups, cheapest first, each one small pull
request. "Ends" names the pair of units that would no longer include each
other. None is started.

| # | The cut | Entries | Files | Risk |
| ---: | --- | ---: | --- | --- |
| 1 | `chips.h` includes `free_space.h` (for `FREE_CHIPS`), not `emu.h`. Ends `core.data` and `emu`; 38 files stop reaching `emu.h` and `hook.h` through it | 1 | `core/chips.h`, and the includers that used `emu.h` unasked | Low: the compiler names each missing include |
| 2 | `core.data`'s two calls into the run: `data.c`'s `run_all_star()` becomes its caller's argument; `library_pages.c` takes the "held" test it already passes on as a parameter. Ends `core.data` and `core` | 2 | `core/data.c`, `data.h`, `library_pages.c`, their callers | Low |
| 3 | `emu/debug.c` prints `run.depth` in a debug line: the line takes a note the director sets. The emulator then includes nothing of the run | 1 | `emu/debug.c`, `debug.h`, `director/director.c` | Low: a debug line |
| 4 | `library.c` reads `dev.library` to fill the Library for pictures: `dev` calls `meta_library_add` itself at its start. Ends `library` and `dev` | 1 | `library/library.c`, `dev/devtools.c` | Low; the Library's captures (`--dev library=N`) looked at |
| 5 | `order_sort.c` calls `director_pack_now`: the sorter takes the pack's reader as a function the director gives it at its start. Ends `world` and `director` | 1 | `world/order_sort.c`, `order_sort.h`, the director's start | Low |
| 6 | `launcher.c` calls `saves_import` and `saves_tick`: the launcher owns two hooks, SAVES registers at its start. Ends `launcher` and `saves` | 1 | `launcher/launcher.c`, `launcher.h`, `saves/saves.c` | Low to medium: a dropped `.cwsave` and the automatic copy tried again (`tests/test_transfer.c`, a capture) |
| 7 | The statistics are handed names: `analytics.c` no longer includes `guardians.h` and `powers.h`, `analytics_online.c` not `online_visit.h`; `meta.c`'s `analytics_run_start()` moves to the director, which starts the run. Ends `analytics` with `director`, `layer`, `online`, and `core` with `analytics` | 4 | `analytics/analytics.c`, `analytics.h`, `analytics_online.c`, `director/runlog.c`, `core/meta.c` | Medium: the payloads must stay byte for byte (`build.py test` holds them) |
| 8 | `lz.[ch]` (29 lines: data packed for the game's LZ77) from `src/map/` to `src/core/`, in `formats`' neighbour `core.base`. Ends `gfx` and `map`, and `emu`'s include of `map` | 3 | Two files moved, the Makefile's `TEST_MAPSLOT_SRCS`, two rows in the concepts | Low; a move, so after a look at the open pull requests |
| 9 | `xsong.*` from `src/audio/` to `src/emu/`: it installs songs into the game's table, and seven installers include it. `audio` then needs nothing of the emulator's but `formats` | 7 | Two files moved, rows in the concepts; `audio`'s line in `layers.txt` loses `emu` | Low to medium; a move, and the BN4 music sessions touch it: after they merge |
| 10 | `second_art.*` (a chip's icon and art from the ROM) from `src/second/` to `src/gfx/`. Ends `library` and `second` | 3 | Two files moved, rows in the concepts | Low; a move |

After these ten the list is at 123 and twelve of the 41 pairs of units
are gone.
Then, larger and each worth more: the dev switches' header (32), the
second screen's frame (12), the platform's hooks (13), the installed
families' table (7), `xnavi.h` (7), the Square's registration with the
director (7).

## The numbers

`python3 tools/layers.py --record LABEL` adds a row here; the release
skill runs it with the version at each release. Three to watch: the
**hand-written lines**, the **pairs of units that include each other**
and the **baseline's size**. The last two reach zero together: the rules
hold no cycle, so every pair has a baselined include in it.

| At | By hand | By a tool | Files | Folder pairs | Unit pairs | Baseline |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| v0.10.0 (5 October 2026) | 54,266 in all | | 330 | | | |
| dev, 12 November 2026 | 153684 | 25694 | 1118 | 34 | 41 | 147 |
| v0.21.1 | 161610 | 27320 | 1156 | 31 | 35 | 142 |
| v0.21.2 | 162075 | 27328 | 1158 | 31 | 35 | 142 |
<!-- numbers: tools/layers.py --record adds a row above this line -->

(v0.10.0's row is the count of that tag's `src/`, before the tool: its
lines by hand and by a tool were not told apart.)

## The tool

`tools/layers.py` ([tools](tools.md)), standard library only:

| Call | Gives |
| --- | --- |
| `python3 tools/layers.py` | The report: folders with their lines and fans, the pairs, the knots and cycles of three, the units, the 25 headers most included |
| `--edges` (`--units`) | Every edge between folders (or units) with the headers and files that cause it |
| `--why A B` | Each include from A to B with the names its file uses |
| `--uses HEADER` | Who includes a header, which of its names and of the headers it includes they use |
| `--cuts` | The baseline by edge, the fewest first, with the pair each would end |
| `--generated` | The files a tool writes, and any whose comment says so that the table lacks |
| `--lint`, `--update` | The rule, as `build.py lint` runs it |
| `--record LABEL` | A row into the table above |

Its output is sorted throughout, so two runs can be laid side by side.

[^measure]: `python3 tools/layers.py`, and its `--cuts`, `--uses` and `--generated`, on `dev` at `2744367e`, 12 November 2026.
[^rule]: `tests/lint/layers.txt` and `tests/lint/layer_violations.txt` as this page's pull request wrote them.
