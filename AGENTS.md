# AGENTS.md

## Project and scope

Cyberworld Endless is a roguelike that runs Mega Man Battle Network 6:
Cybeast Gregar (USA) from the player's own ROM on an embedded mGBA core and
directs it: generated layers in the game's map formats, the run's structure,
loot and saves. It is written in C11 with SDL2 and ships as a PortMaster port
for ROCKNIX, the primary target, as a Linux desktop build, as a Windows
build, as a macOS app, as a browser build on GitHub Pages and as a
homebrew build for the New 3DS (a CIA for the HOME Menu and a .3dsx) and an
iPhone and iPad app through AltStore and SideStore. Reference
devices: Retroid Nova (1280x960) and Retroid Pocket Flip 2 (1920x1080).

Never commit or publish ROMs, save files, extracted assets, emulator save
states or disassembly files. `.gitignore` covers the usual names; check
`git status` before committing anything. Screenshots of the game running
are the one exception: `docs/screenshots/` and the short videos in
`docs/clips/`, made by `build.py screenshots`, `build.py clips` and
`tools/trailer.py` (the trailers, their music original), for the README and
the site.

## The knowledge base

`docs/` is the project's knowledge, kept as an Open Knowledge Format bundle
(OKF v0.2, the [okf skill](.claude/skills/okf/SKILL.md)). Start at
[docs/index.md](docs/index.md): each folder's `index.md` lists its concepts
with one line each, so open only the ones the task needs. A concept opens
with frontmatter: its `type`, `title`, a one-sentence `description`, `tags`,
who wrote it and when (`generated`) and what it rests on (`sources`). The
old flat docs (`EMULATION.md`, `ROM_DATA.md` and the rest, still named in
commits, issues and old notes) are mapped section by section in
[docs/project/moved.md](docs/project/moved.md).

| Task | Where |
| --- | --- |
| Player experience, controls, install | [docs/GUIDE.md](docs/GUIDE.md); the [README](README.md) is the project's one page of pictures, which links there |
| How the engine drives the game, and the memory it writes | [docs/engine/emulation/](docs/engine/emulation/index.md) |
| What each file of the tree holds | [docs/code/](docs/code/index.md) |
| The dev tools: headless captures, playtests, reports, the differential | [docs/devtools/](docs/devtools/index.md) |
| How a run is paced: acts, battles, areas, guardians, growth | [docs/game/progression/](docs/game/progression/index.md) |
| What carries over between runs: unlocks, the run's setup, threat | [docs/game/meta/](docs/game/meta/index.md) |
| What a run hands out (chips, their rarity and codes, zenny, Mystery Data), measured by act, the plans its curves come in by, and the guard against drift | [docs/game/economy/](docs/game/economy/index.md) |
| Guardians and super bosses | [docs/game/bosses/](docs/game/bosses/index.md) |
| The NaviCust | [docs/game/navicust.md](docs/game/navicust.md) |
| The PET's entries in a run: Save, E-Mail (the lab's mails), KeyItem, the Jukebox | [docs/game/pet.md](docs/game/pet.md) |
| The rival, Chaud and ProtoMan: busting duels, the record, what his respect opens | [docs/game/rival.md](docs/game/rival.md) |
| What is original, generated or adapted | [docs/game/fidelity.md](docs/game/fidelity.md) |
| BN5's Liberation Missions dissected, how they join the run, and the generator that makes them afresh (its prototype in `tools/liberation.py`, BN5's own nine measured by `tools/liberation_bn5.py`) | [docs/game/liberation/](docs/game/liberation/index.md) |
| The net's generated layers: the generator, identity, arrivals, props, set pieces | [docs/world/level-design/](docs/world/level-design/index.md) |
| The real world and the town | [docs/world/overworld.md](docs/world/overworld.md) |
| The town in a run: home after every act, its ports, its people's jobs, its shop, a town that remembers | [docs/world/home/](docs/world/home/index.md) |
| Where ROM data lives and how it was found | [docs/rom/](docs/rom/index.md) |
| What the project has mapped of BN6 and BN5, written out for others: symbol files for debuggers, tables, how they stay in step; bn6f's functions located in Gregar and BN5, its names kept out of git | [docs/rom/symbols.md](docs/rom/symbols.md) |
| Other games' ROMs (Battle Network 5 first): what they can lend a run, and how | [docs/multirom/](docs/multirom/index.md) |
| What the original games have, do and lack: the sources to look in first, how a claim is verified, what players taught us | [docs/research/sources.md](docs/research/sources.md) |
| How reverse engineering is planned, proved and reported | [.claude/skills/reverse-engineering/SKILL.md](.claude/skills/reverse-engineering/SKILL.md); [docs/re/README.md](docs/re/README.md), its [STATUS.md](docs/re/STATUS.md) and [dashboard](docs/re/dashboard.html) |
| Every line a character says (MegaMan, Lan, Mr.Prog, the shops, bystanders, guardians, townsfolk): how BN6's script talks, measured, and the rules; the owner's standing directions | [docs/voice/rules.md](docs/voice/rules.md); the catalog of every line, the owner's review dashboard and a voice pass in [docs/voice/README.md](docs/voice/README.md) |
| The screens the engine draws in BN6's PET look (the Jukebox, Options, Trade Central, the NetPass, the second screen): BN's own screens' hierarchy, keys, cursor, letters and colours, measured, and what ours take from them | [docs/engine/ui.md](docs/engine/ui.md) |
| Moving saves between systems: the `.cwsave` file, import, Undo, the automatic copy | [docs/engine/saves.md](docs/engine/saves.md) |
| Online play (epic #120): the gate in Lan's HP, the Net Square, Trade Central, the Arena, the server and the wire | [docs/online/](docs/online/index.md); the gate in [docs/world/home/square-gate.md](docs/world/home/square-gate.md); hosting in [server/README.md](server/README.md); the wire in [shared/online/README.md](shared/online/README.md); the link cable and the Arena's engine in [docs/online/link.md](docs/online/link.md) and [docs/online/arena/](docs/online/arena/index.md) |
| Shipped changes | [CHANGELOG.md](CHANGELOG.md); the players' notes in [docs/releases/](docs/releases/index.md) |
| The itch.io page: its texts, settings, pictures and files, each release's devlog post, and the record (`docs/itch/state.json`) that `build.py lint` holds it to | [docs/project/itch.md](docs/project/itch.md) |

Keeping it (the okf skill's implicit mode, whenever `docs/` changes):

- A fact goes in the concept that owns it. A new topic is a new concept in
  its folder: frontmatter with `type`, `title`, a one-sentence `description`,
  `tags` and `generated: { by: <agent>/<model>, at: <ISO time> }` (a person
  is `human:<id>`; never `human:` for what an agent wrote), its line in the
  folder's `index.md`, and at least one link to a related concept, the prose
  around it saying what the link means.
- A change to what a concept says refreshes its `generated` (both `by` and
  `at`) and drops any `verified` it carried; `verified` records only a check
  that happened. A changed `description` changes its index line too.
- Each change adds one line to [docs/log.md](docs/log.md) under today's date,
  newest first; past entries are never edited.
- Code, tools and comments cite a concept by its path
  (`docs/engine/emulation/hooks.md`, a section after a comma); a concept
  links another relatively (`../engine/emulation/hooks.md`).
- `build.py lint` validates the bundle (`okf-validate.mjs --strict`: every
  concept typed, none orphaned, no broken link inside it) and that every
  `docs/` path the tree names exists.

## Layout

What each file holds is in [docs/code/](docs/code/index.md), a concept per
part of the tree; a new file gets its line there.

| Path | Its files, in the knowledge base |
| --- | --- |
| `server/`, `deploy/`, `shared/online/` | [docs/code/server.md](docs/code/server.md): the ROM-free server anyone can host, the official server's deployment and a self-hosted Square's, the versioned wire contract |
| `src/online/` | [docs/code/online.md](docs/code/online.md): online play's client |
| `src/core/` | [docs/code/core.md](docs/code/core.md): entry point, platform, Options, ROM access and `RomLayout`, run state and saves |
| `src/emu/` | [docs/code/emu.md](docs/code/emu.md): the mGBA core, hooks, the moved families, Navis, chips and Souls, free space, the `bn6*.h` headers |
| `src/director/` | [docs/code/director.md](docs/code/director.md): the run on the game |
| `src/net/`, `src/map/` | [docs/code/net-and-map.md](docs/code/net-and-map.md): layer generation, layers as game maps |
| `src/layer/` | [docs/code/layer.md](docs/code/layer.md): what stands on a layer |
| `src/world/` | [docs/code/world.md](docs/code/world.md): the real world |
| `src/gfx/`, `src/audio/`, `src/jukebox/` | [docs/code/gfx-and-audio.md](docs/code/gfx-and-audio.md): graphics, the engine's MP2K, the Jukebox |
| `src/launcher/`, `src/saves/`, `src/scenes/`, `src/second/` | [docs/code/screens.md](docs/code/screens.md): the ROMs screen, SAVES, the start and title, the second screen |
| `src/analytics/` | [docs/code/analytics.md](docs/code/analytics.md): the anonymous statistics |
| `src/dev/` | [docs/code/dev.md](docs/code/dev.md): developer tools inside the game |
| `tests/` | [docs/code/tests.md](docs/code/tests.md): the ROM-free tests and lint's baselines |
| `tools/`, `.mcp.json`, `.claude/skills/` | [docs/code/tools.md](docs/code/tools.md): research and test tools, the agents' MCP server, the skills |
| `port/`, `linux/`, `windows/`, `android/`, `macos/`, `ios/`, `3ds/`, `docker/`, `.github/` | [docs/code/platforms.md](docs/code/platforms.md): the platforms, their packaging, the build images, CI |
| `web/`, `docs/screenshots/`, `docs/clips/` | [docs/code/site-and-media.md](docs/code/site-and-media.md): the site, screenshots and clips |
| `docs/` | [docs/index.md](docs/index.md): the knowledge base |

## Rules

- A claim about the original games (what BN6 or BN5 has, does or lacks)
  is looked up in docs/research/sources.md's sources before it shapes a design,
  a doc or code, and written as verified (how) or assumed; a negative
  one names the search that came up empty. Hardcore players know the
  games better than we do: a player's claim is a lead to verify and
  credit there.
- Read data from the ROM through `RomLayout` offsets or the addresses in
  `src/emu/bn6.h`. Do not embed or generate files containing Capcom
  graphics, text, samples or sequences (sprites, tiles, palettes, fonts,
  text dumps, music); tables the engine derives are numbers only.
  Screenshots of whole frames of the game running, for the README and the
  site, are allowed; crops that isolate a sprite, a logo or a mugshot as an
  asset are not.
- Where BN6 has a screen for something (the Custom screen, chip select,
  menus, chat boxes), drive BN6's own code to show it (a spare core
  outside a battle's determinism if need be); never redraw a copy. Our
  own drawing is for what BN6 has no screen for.
- Patch only the core's in-memory ROM copy, and put new data in the free
  space map in `docs/engine/emulation/memory.md`, its place a region of
  `src/emu/free_space.h` (RAM BN6 leaves alone too), which
  `tests/test_core.c` holds apart from every other; an installer writes
  within `claims_begin`/`claims_end` (`src/emu/claims.h`). Never write the
  player's ROM file.
- A new ROM offset needs a row in `docs/rom/` saying how it was
  located and how to verify it.
- The game's 240x160 picture stays whole at a whole-number scale on every
  screen shape. The exceptions: the 3DS's top screen (400x240), where 1x
  is the only whole scale: there it fills the screen's height at 1.5x by
  default, and `screen = whole` in `settings.ini` gives 1x; and screens
  where a whole scale leaves the picture a quarter smaller or more than
  filling would (640x480: 2x against 2.67x, issue #36), which fill it with
  sharp scaling (whole-scaled, then smoothly to size) by default, and
  `screen = whole` or `fill` in `settings.ini` sets it either way; and a
  phone or tablet held upright, its touch controls under the picture,
  which fills the screen's width the same way (1080 wide: 4.5x against
  4x) as far as the controls keep their size, `screen = whole` keeping
  whole scales there too. Beyond these the player alone chooses: Options'
  Display's Fill (`screen = fill`) and Stretch (`screen = stretch`, the
  picture pulled to the screen's shape, never a default).
- MegaMan walks as BN6 walks him, always: no lining-up help, no sliding
  onto a walkway, no movement assist of any kind (one was removed by the
  owner's choice, docs/game/fidelity.md). Where walking drags (walkway mouths,
  lanes), the generated maps change instead, connected as Capcom's are
  (docs/world/level-design/generator.md, Navigation).
- Draw calls must not change game state. Input is read once per frame in
  `platform_poll`.
- The C builds warning-free with the Makefile's `WARN` (`-Wall -Wextra`,
  `-Wshadow`, `-Wcast-qual`, `-Wwrite-strings`, `-Wformat=2` and more; issue
  #19) on every target, `-Werror` in CI. `build.py lint` holds the rest to
  its baselines in `tests/lint/`: no new finding of GCC's analyzer, no new
  function no build reaches, no function joins or grows past CCN 25 or 120
  lines, no ROM offset without its name in `docs/rom/`, no field of
  the game's structures as a bare offset outside `src/emu/bn6.h` (issue
  #34: `BN6_PLAYER_X`, not `BN6_PLAYER + 0x1C`), no new code smell and
  no header that is not whole on its own (Code structure, below), and no
  ROM, save, state or large file in git outside `docs/`, and the itch.io
  page in step with `docs/itch/` (docs/project/itch.md, Keeping it in step). A
  baseline only shrinks: after fixing one, `build.py lint --update`.
- Keep a layer reproducible from `run.layer_seed`: a checkpoint rebuilds the
  layer before it loads `run.state`, so generation and object placement
  must not depend on the game's RAM.
- Run state is saved as a raw struct with a checksum. Changing `Run` breaks
  old saves: bump `RUN_MAGIC` in `save.c` (a `_Static_assert` on its size
  there fails the build until someone does, and sets the new size). Changing
  what a layer seed makes (layouts, object placement, loot and stock rolls):
  bump `LAYER_MAKE` in `src/net/layer_make.h`, so a run saved by an older
  build continues its layer afresh; where generation changed, the unit tests
  fail on `LAYER_MAKE_HASH` beside it and name the new hash to set with
  it. Emulator states (`boot-3.state`, `run.state`) are made on the device
  and never committed.

## Code structure

- One thing per file, named for it. Past 1000 lines (a header 400) a file
  holds more than one thing, and is split by what each part does. A module
  of several files shares its state through a header of its own
  (`src/director/director_state.h` holds the director's `D`); each part's
  header declares what the other parts use, and what the rest of the game
  uses stays in the module's public header (`director.h`).
- What characters say lives apart from the logic that decides when it is
  said, in the feature's words file beside it: `*_words.c` (lines composed
  from the run's facts), `*_lines.c` (tables of lines), `*_text.c` (the
  PET's text). A voice pass (docs/voice/rules.md) reads every line there, and
  the logic reads without them.
- `build.py lint` lists the code's smells in `tests/lint/smells.txt`, none
  of which may join or grow: a file past its lines, a `.c` file including
  more than 20 of `src/`'s headers, a function taking more than 7
  parameters or indented past 5 levels, a block of 10 lines or more
  repeated, and lines of speech outside a words file. Every header has its
  guard and compiles on its own, and a `.c` file includes its own header
  first. A change that touches a listed smell takes it down where it can.

## Reverse engineering

Use `.claude/skills/reverse-engineering/SKILL.md` before investigating a
new ROM behavior, offset, hook, transplant, save format or emulated link.
Addresses stay in `docs/rom/` and claims in `docs/research/sources.md`;
`docs/re/` holds where the work stands (`ledger.json`, written out by
`tools/re_status.py` as `STATUS.md` and the dashboard), not a second
address map. Every ROM fact a piece of work finds, research included, ends
in the project's map: a name in a `src/emu/bn6*.h` header, its
`docs/rom/` row, `tools/symbols.py` run, a ledger entry; notes in
`.build/` alone are lost to it.

## Game design

A change that decides how the game plays (balance values, scaling, drop
rates, prices, rewards, progression, the NaviCust and other loadouts, menus
and screens, "is this fun") goes through the game-design skill first: name
the dialectic it serves, the loop layer it feeds and the patterns it uses,
before the numbers. This holds mid-implementation too.

## Trailers

A release whose news changes how the game looks or plays (a minor
version, 0.N.0, as a rule) gets a trailer of its own (the trailer skill,
`.claude/skills/trailer/`), which replaces the last one everywhere: the project's face changes with the game rather than
piling up. Each one is new, its look and its music, not the last one's
with new shots. A cut is `tools/trailers/vX_Y_Z.py` (its footage, from the
release pictures' scripted runs, and its look) with `vX_Y_Z_music.py` (an
original piece in the manner of a game it shows, written from that game's
habits, never its notes) and `CURRENT` in `tools/trailers/__init__.py`.
`python3 tools/trailer.py` writes `docs/clips/trailer.*`, which the
project page's hero, every page's preview picture, the README, the
AltStore source and the 3DS banner's sound (the piece's `BANNER`) follow
by themselves; the hero's `aria-label` and VideoObject data on the home
page and its Japanese one, and the README's alt text, are rewritten for
it. The release's notes open with it (`../clips/trailer-play.png` linking
`../clips/trailer.mp4`, both kept by the tag). An older trailer stays at
its release's tag and still renders from its cut, into `.build/` only.

## Pull requests

PRs target `dev` and are merged by hand once CI is green; commits carry
only the author (no Co-Authored-By). A PR's description is short and
shows the change: a line or two on what changes, **See it** (pictures or
GIFs, before and after where something visible changed), a short
**Checked** list, and follow-ups if any; the mechanism stays in the
commits and docs. Its pictures (whole frames of the game or pages of the
site, never a sprite cropped out) go on the orphan branch `pr-media`
under `prs/<branch>/`, embedded by their raw URL at that branch's commit;
motion as a GIF, which GitHub plays inline.

A merged branch goes: deleted on GitHub and here, its worktree
(`.claude/worktrees/`, `.build/`) removed once it holds no changes. At
each release, sweep what is left: every branch and worktree already in
`dev`, kept only `main`, `dev` and `pr-media`.

## Build and verify

```sh
python3 build.py            # host + aarch64 binaries (Docker)
python3 build.py test       # ROM-free unit tests
python3 build.py lint       # the code's checks against tests/lint (--update after fixing one)
python3 tools/voice_catalog.py   # docs/voice/catalog.json from the words files (--check: is it in step; build.py lint runs it)
python3 build.py shot --scene emu --run-depth 2 --frames 400 --shot "300:/src/.build/a.bmp"
CYBERWORLD_AUTOPILOT=1 python3 build.py shot --scene emu --frames 15000   # walk layers, fight
python3 build.py package    # build/port/cyberworld (PortMaster-New's layout) and the port's zip
python3 build.py run        # the Linux desktop build, played here in a window
python3 build.py windows    # build/release: the Windows installer and zip (MinGW-w64, NSIS)
python3 build.py android    # build/release/cyberworld-endless.apk (the NDK and Gradle in Docker; android/README.md)
python3 build.py macos      # build/release/cyberworld-endless-macos.dmg, on a Mac only (Apple's SDK)
python3 build.py ios        # build/release/cyberworld-endless.ipa, on a Mac with Xcode (--simulator: the Simulator's app)
python3 build.py flatpak    # build/release/cyberworld-endless.flatpak (flatpak-builder on this machine)
python3 build.py 3ds        # build/3ds: cyberworld-endless.cia and .3dsx (devkitARM in Docker; 3ds/README.md)
python3 tools/cw.py shot options/audio [--do CMDS] [--until COND] [--ref REF]   # one call: a look at a screen (the cyberworld MCP server's shot; docs/devtools/agents-tools.md)
python3 tools/cw.py play NAME --start ROUTE|OPTIONS --do CMDS   # one call: play.py's game, what changed and a picture (the MCP's play)
python3 tools/cw.py build [host|linux|test|lint]   # a build or check, its complaints alone (the MCP's build)
python3 tools/play.py start NAME [--fresh]   # a headless game for a playtest (build.py linux first)
python3 tools/play.py do NAME "press A; hold UP 30" [--every 10] [--keep DIR]   # input, then a picture and the state (--keep: each frame into DIR)
python3 tools/square_tour.py [SEED ...]   # the Net Square on a local server, three games: the plaza, each warp and part, the walks timed (build.py linux and server first)
python3 build.py serve      # the site and the browser build on http://localhost:8080
python3 build.py xdiff [SPEC] [--mutations]   # tools/xdiff.py: a moved virus family against its own game's battle, frame by frame (tests/xdiff; both ROMs)
python3 build.py xrecords [--all]   # tools/xrecords.py: BN5's territories' records as BN6's engine fights them, each fightable or why not (both ROMs)
python3 tools/xproxy.py [--mutations] [--souls]   # the Souls' proxy form (form 8) against HeatCross, every byte of every frame (the ROM; --souls the Souls over it, both ROMs)
python3 tools/heap3ds.py [--seeds 1-12]   # the New 3DS's heap modelled on the host build: long runs' peaks and refused allocations, BN6 alone and with BN5 (both ROMs)
python3 tools/mp2k_verify.py [SONG...]   # the engine's MP2K against BN6's own in romlab, frame by frame (the ROM)
python3 tools/bn5_songs.py [SONG...]   # where BN5 starts each of its songs: its code, map lists, battle records, scripts and text (BN5's ROM; docs/rom/)
python3 build.py economy [RUNS]  # what a run hands out by act, today's plan and the proposal's, against their bands (docs/game/economy/; the ROM)
python3 build.py tiles [SEEDS]  # the tile test: every area's layers drawn, wrong-floor tiles and seams per area
python3 build.py town [SEEDS]  # the town drawn per seed, and the game around it
python3 build.py world        # the real world's original maps, drawn and toured
python3 build.py screenshots [NAMES]   # docs/screenshots from scripted headless runs
python3 build.py clips [NAMES]         # docs/clips: the same runs as 30 fps videos
python3 tools/site_art.py              # web/assets/ui: the site's pixel-art frames and icons
python3 tools/app_icon.py              # linux/icons and src/core/app_icon.h: the application icon
python3 tools/steam_art.py             # linux/steam: Steam's library artwork (capsules, hero, logo); 3ds/banner.png
python3 tools/social_preview.py        # build/social-preview.png: the repository's social preview, uploaded by hand
python3 tools/trailer.py [VERSION] [--keep]   # docs/clips/trailer.*: the current trailer (tools/trailers/; an older VERSION into .build/ only)
python3 tools/intro_chime.py FILE      # src/audio/intro_chime.c: the start's chime (Pixabay's "UI Success Chime", kept out of git) and its two tones
python3 tools/before_after.py v0.5.3 --new 0.6.0   # docs/screenshots/compare-*.png: one spot as the last release and this build draw it, for the notes
python3 build.py release    # build/release/: the PortMaster zip, the Linux AppImage, .deb and tar.gz, the site zip
python3 tools/itch.py push   # the latest release's files to the itch.io page, a butler channel each (docs/project/itch.md; `prepare` lays them out without sending)
python3 tools/itch.py devlog  # the newest release's devlog post for itch's editor; `record --post TAG URL` once posted, `check` after (docs/project/itch.md)
python3 tools/itch_art.py    # docs/itch: the itch.io page's cover, banner and background tile; .build/itch: its screenshots at 4x
```

An agent builds, captures, plays and compares through the `cyberworld` MCP
server (`.mcp.json`; docs/devtools/agents-tools.md) or `tools/cw.py`: a
call each, the host build made first where a source is newer, the build's
output kept to its complaints, the state in words with a `screen` line, and
the game's 240x160 picture inline at 2x, or none where the words answer.
`build.py shot` below stays the way for the release pictures' scripted runs
and for options the tools do not take.

The desktop and browser builds' differences are
[docs/code/builds.md](docs/code/builds.md); `shot`'s options
[docs/devtools/headless-shot.md](docs/devtools/headless-shot.md);
`tools/play.py`'s sessions and dev steps
[docs/devtools/play.md](docs/devtools/play.md); `tools/romlab`
[docs/devtools/romlab.md](docs/devtools/romlab.md).

CI runs without a ROM: it builds every target with `-Werror`
(`WERROR=1`, set when `CI` is), runs `build.py lint` and
`tests/test_core.c`, `tests/test_backup.c`, `tests/test_saves.c`,
`tests/test_transfer.c`, `tests/test_discovery.c` and `tests/test_mapslot.c` under the
sanitizers, then `tests/test_add_to_steam.py`; its android job runs
`tests/test_android_saves.py`. Everything that needs the game (captures, autopilot, atlas,
pacing) stays local.

| Change | Checks |
| --- | --- |
| The knowledge base (`docs/`) | The okf skill's bookkeeping (frontmatter, the folder's `index.md`, a line in `docs/log.md`, a link); `build.py lint` (the bundle validated, every `docs/` path named exists) |
| Engine code | `build.py test`, `build.py lint`, a headless capture of the affected screen (`shot` with its route, `ref` against the base where it was there before), an autopilot run (`shot` from `CYBERWORLD_AUTOPILOT=1 layer` until `layer>=2`) |
| Layer generation or maps | `build.py test` (connectivity over 300 seeds), `build.py tiles` (no layer worse than `tests/atlas_baseline.txt`, the close-ups looked over), captures of every area (`--net-biome 0`-`7`) |
| The town | `build.py town` (its seeds drawn, misses marked, the game around it), an autopilot run from `--scene town` through the jack-in; where Lan's HP changes, MegaMan walked onto the pink pad from the corridor, its last frame in layer 1 (BN6's trigger stops short of the pad's art, session 74): `build.py shot --scene town --seed 2 --frames 1420 --input "$(python3 -c "print('40:,' + '4:A,6:,' * 46 + '30:,0:place -40 -8 7,10:,6:R,500:,' + '4:A,16:,' * 8 + '0:place 171 0 1,10:,25:UP+RIGHT,250:')")" --shot 1410:/src/.build/pad.bmp` (a fresh `--data-dir`), and again with `0:place 171 -5 1,10:,12:RIGHT,250:` for its last walk, MegaMan on the pad's left rim (session 75); where a pad's zone changes (`pad_art.c`), a layer's exit pad from its left rim too, docs/rom/net-maps.md's Warp pads |
| Layer objects, scripts, shops | A capture of the talk or screen with scripted input (`shot` from `layer --talk shop:430`) |
| Lines (a words file: `*_words.c`, `*_lines.c`, `*_text.c`) | docs/voice/rules.md's checks; `python3 tools/voice_catalog.py` (the catalog regenerated and committed with the line, the flags of the lines touched read over), `build.py lint` (fails while it is stale), `build.py test`; a capture of the talk (`--talk`); the dashboard republished with the new catalog, and the reviews and directions acted on marked applied (docs/voice/README.md, A voice pass) |
| Difficulty, encounters, guardians, rewards | `build.py test`, `build.py pacing` (0 past their band, BN5's guardians among them), an autopilot run; where BN5's content changes, over `--net-biome x0`-`x5` and with `--guardian 24`-`29` too, fresh data folders, the run logs looked over |
| Drops, prices, Mystery Data, dealers' stock, the economy's curves (`loot.c`'s rolls, `net_gen.c`'s data, `shop.c`, `economy.c`) | The game-design skill first; `build.py economy` (0 of the plan in play outside its bands; a number moved on purpose sets its band's centre in `economy.c`, the report's before and after in the commit), `build.py test` (`test_economy`, `LAYER_MAKE_HASH`), `build.py pacing` where a battle's reward changes |
| Audio | `--render-song ID:SECONDS:PATH` (`x0/ID:...` BN5's song) and a listen on a device; where the engine's MP2K changes (`src/audio/mp2k*.c`, `gba_sound.c`), `python3 tools/mp2k_verify.py` (every song PASS against the game in romlab) |
| The ROMs screen | `build.py test`, headless captures with `--scene launcher` from 240x160 to a phone's screen (`--size`, `--dpi`) with no ROM, BN6 alone and both, and on Android and iOS the pickers in an emulator or the Simulator (the packaging rows) |
| SAVES | `build.py test`; headless captures of home, compare and result (`--scene saves`) at 240x160, 640x480 and 1080x1920; an export from one data folder imported into a fresh one, CONTINUE on the same layer; UNDO IMPORT puts back the files byte for byte |
| The anonymous statistics | `build.py test` (the answer, the payloads, the queue's cap, the tries); headless captures of the question (`--dev statistics=ask` and a fresh `--data-dir`) and of Options' Statistics line (`--dev statistics=on`, SELECT on the title); runs against a local stand-in for Umami (`--dev statistics=on,statsurl=http://127.0.0.1:PORT/api/send`; Wine's in a container on the host's network), never Umami itself, but for `--dev statscheck`, one `game-dev-check` event to check a system's way there (docs/devtools/statistics.md) |
| Browser page or platform code | `build.py serve` and a run in a browser: ROM choice (BN6 and BN5, and a wrong file refused), a new game, a battle in a BN5 area, CONTINUE after a reload |
| The site | `build.py test` (`tests/test_site.py`); `build.py serve` at desktop and phone widths (agent-browser), the English pages and their Japanese ones; `build.py screenshots` and `build.py clips` again when what they show changed |
| Windows packaging | `build.py windows`; the .exe under Wine (a Debian image with `wine`, `wine32:i386` and `xvfb`): a headless `--scene emu` capture with the ROM, a start in a window, the installer's silent install (`/S`), a start and `uninstall.exe /S` |
| Android packaging | `build.py android`; the APK in an emulator (KVM, the emulator and an image in `.build/android-sdk`; ROM copies pushed with `adb push`): a fresh install opens the ROMs screen, whose BN6 slot asks for the ROM folder; a folder with BN6 and BN5 takes both (logcat `-s Cyberworld SDL/APP`: "BN5 found"), one with only a wrong `.gba` refuses it with its reason, BN5 put in the folder after the first start comes in at the next, and ROMs right in Download go through **FILES INSTEAD** (both picked at once) and the icon's **ROMs** shortcut; the saves' copy (`cyberworld-endless.cwsave`) in the folder after the game saved, an uninstall, and a reinstall whose folder chosen again opens SAVES' comparison of that copy, BRING BACK and CONTINUE on the same layer (the files' hashes as before); a new game by touch, the D-pad walking, a turn to landscape, Back twice and the game opened again |
| macOS packaging | CI's macOS job (`lipo`, `otool -L`, `codesign --verify`, a ROM-less headless start on the ROMs screen); on a Mac, `build.py macos` and a run with the ROM: the ROMs screen's open panel, a new game, CONTINUE |
| iOS packaging | CI's ios job (arm64, iOS 14, only iOS's frameworks, `codesign --verify`, the Simulator's app reaching its ROMs screen: look at the artifact's screenshot); on a Mac, the Simulator with the ROMs: BN6's slot on a folder with BN6 and BN5 (both taken, the title's "BN5 found"), on one with only a wrong `.gba` (refused with its reason), BN5 put in the folder after the first start (taken at the next), FILES INSTEAD with both, the folder's saves' copy opening SAVES' comparison after the app was deleted and installed again, a new game, a turn to landscape, CONTINUE after the app was sent to the background; on an iPhone, AltStore's install from the release's source |
| 3DS build | `build.py 3ds`; `python3 tools/heap3ds.py` where a change allocates more (the 3DS's heap modelled here); on a New 3DS (3ds/README.md): the CIA installed with FBI (its QR code, or from the SD card) and started from the HOME Menu, the .3dsx over ftpd and a start from the Homebrew Launcher, NEW GAME to the first layer and its map on the bottom screen, `log.txt`'s frame lines (`frame_log = on`, the heap's high-water mark among them); with BN5's ROM beside BN6's, 3ds/README.md's BN5 checks (`log.txt`'s BN5 and heap lines, a layer of BN5's areas over 3dslink's `--net-biome x0`-`x5`); the other targets still build, as the 3DS's changes are mostly shared code |
| PortMaster port | `build.py package`; `build/port/cyberworld` copied into a PortMaster-New checkout's `ports/` passes its `tools/build_release.py --do-check` and `tools/build_gameinfo.py`; the binary asks for glibc 2.30 at most (CI checks); a device run; a new PortMaster-New pull request needs the testing its template lists (ArkOS, AmberELEC, ROCKNIX, muOS, Knulli; 640x480 and larger) |
| Linux packaging | `build.py linux`; the AppImage and the `.deb` in a clean distribution container (first start without a ROM, the menu entry, a start with one); `--add-to-steam` with `HOME` at a made-up Steam folder |
| Release | A new trailer where the release earns one (Trailers), made before the tag; `build.py release`, the Linux AppImage, `.deb` and archive each started fresh; tag `vX.Y.Z` on `main`; after the release is published, the itch.io page in step (docs/project/itch.md, Each release): its files (`python3 tools/itch.py push`), its devlog post (`tools/itch.py devlog`, posted on itch, `tools/itch.py record --post`), and `tools/itch.py check` in step; `build.py lint` fails the next release's work while a release lacks its post; runs on real devices are the owner's, after the release, and no blocker |

## Device testing

Use an existing SSH control socket; do not store device credentials here.
Confirm the device is idle (`curl -s localhost:1234/runningGame`) before
launching. `tools/device_run.py HOST --control-path SOCK --shots 10,30 --
--scene emu` does all of the following. Test builds run from a temporary launcher beside the real one
(`ports/zz-cwtest.sh`, reloaded with `GET localhost:1234/reloadgames` and
started with `POST localhost:1234/launch`) that passes `--data-dir` to a
throwaway directory; remove it and reload the games afterwards. Drive the
game with `tools/uinput_keys.py` and capture with `grim`. Update the
installed port in `/storage/roms/ports/cyberworld/` only when asked, never
overwrite a player's `savedata/`, and compare save hashes before and after.

When handing off, state what changed and what ran. Runs on real devices
are the owner's, after a release: not a blocker, and not listed as open.
