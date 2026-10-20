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

## Read before changing

| Task | Source of truth |
| --- | --- |
| Player experience, controls, install | [docs/GUIDE.md](docs/GUIDE.md); the [README](README.md) is the project's one page of pictures, which links there |
| How the engine drives the game, and the memory it writes | [docs/EMULATION.md](docs/EMULATION.md) |
| The real world and the town | [docs/OVERWORLD.md](docs/OVERWORLD.md) |
| The town in a run: home after every act, its ports, its people's jobs, its shop, a town that remembers | [docs/HOME.md](docs/HOME.md) |
| Where ROM data lives and how it was found | [docs/ROM_DATA.md](docs/ROM_DATA.md) |
| What is original, generated or adapted | [docs/FIDELITY.md](docs/FIDELITY.md) |
| What carries over between runs: unlocks, the run's setup, threat | [docs/META.md](docs/META.md) |
| What a run hands out (chips, their rarity and codes, zenny, Mystery Data), measured by act, the plans its curves come in by, and the guard against drift | [docs/ECONOMY.md](docs/ECONOMY.md) |
| The PET's entries in a run: Save, E-Mail (the lab's mails), KeyItem | [docs/PET.md](docs/PET.md) |
| The rival, Chaud and ProtoMan: busting duels, the record, what his respect opens | [docs/RIVAL.md](docs/RIVAL.md) |
| Other games' ROMs (Battle Network 5 first): what they can lend a run, and how | [docs/MULTIROM.md](docs/MULTIROM.md) |
| What the original games have, do and lack: the sources to look in first, how a claim is verified, what players taught us | [docs/SOURCES.md](docs/SOURCES.md) |
| How reverse engineering is planned, proved and reported | [.claude/skills/reverse-engineering/SKILL.md](.claude/skills/reverse-engineering/SKILL.md); [docs/re/README.md](docs/re/README.md), its [STATUS.md](docs/re/STATUS.md) and [dashboard](docs/re/dashboard.html) |
| What the project has mapped of BN6 and BN5, written out for others: symbol files for debuggers, tables, how they stay in step; bn6f's functions located in Gregar and BN5, its names kept out of git | [docs/SYMBOLS.md](docs/SYMBOLS.md) |
| Every line a character says (MegaMan, Lan, Mr.Prog, the shops, bystanders, guardians, townsfolk): how BN6's script talks, measured, and the rules; the owner's standing directions | [docs/VOICE.md](docs/VOICE.md); the catalog of every line, the owner's review dashboard and a voice pass in [docs/voice/README.md](docs/voice/README.md) |
| Moving saves between systems: the `.cwsave` file, import, Undo, the automatic copy | [docs/SAVES.md](docs/SAVES.md) |
| Shipped changes | [CHANGELOG.md](CHANGELOG.md) |
| Online play (epic #120): the gate in Lan's HP, the Net Square, Trade Central, the Arena, the server and the wire | [docs/ONLINE.md](docs/ONLINE.md); the gate in [docs/HOME.md](docs/HOME.md) piece 11; hosting in [server/README.md](server/README.md); the wire in [shared/online/README.md](shared/online/README.md); the link cable and the Arena's engine in [docs/ONLINE_LINK.md](docs/ONLINE_LINK.md) and [docs/ONLINE_ARENA.md](docs/ONLINE_ARENA.md) |

## Layout

| Path | Contents |
| --- | --- |
| `server/` | The ROM-free Python server anyone can host (issue #120): presence and channels (`app.py`), Trade Central's commits and its board's offers (`trade.py`), the week's net's seed and Board (`week.py`), the Arena's pairing and relay (`arena.py`), the stands: a battle kept for its spectators and the channel's battles told (`stands.py`), moderation, what its state keeps and for how long, pruned by itself, and the players' log (`retention.py`), the state's daily archive and its restore (`backup.py`), its tests, zipapp, native and Docker packaging; never import game code or package ROMs/saves here |
| `deploy/` | The official server's deployment (`deploy/coolify/`: its Compose file, the server and its backup service, and configuration, the retention's numbers among it); its policy is server/README.md's The official server |
| `shared/online/` | The versioned wire contract (v8): JSON spec and C constants, held together with the Square's generator version (`SQUARE_MAKE`, `square_plan.h`) and ROM identity by the server's tests |
| `src/online/` | Online play's client (docs/ONLINE.md): the transport on every system (`online_transport.c` and its sockets and streams; snapshots copied across the worker boundary, Trade Central's and the Arena's messages through an outbox and an inbox, `online_mail.c`), a visit from the gate and back on one core (`online_visit.c`), the gate's set-once fields (`online_field.c`), the Square's plan from the seed (`square_plan.c`) built into BN6's map (`online_square.c`), its peers, names and chats in BN6's chat boxes (`online_scene.c`, `online_chat.c`, `square_words.c`), its life (the walking Navis, the log-in beam, the greeter's and the referee's words, what a prop says, the cheer: `square_life.c`, `square_life_words.c`), Trade Central (`trade_central.c`, `online_trade.c`, `trade_words.c`; its two screens, the spares list and the chip trade, in `trade_screen.c`, drawn in `trade_screen_draw.c`, their rows sorted and filtered in `trade_list.c`, their PET text in `trade_screen_text.c`, issue #145; its trade board's offers `online_board.c`), the Arena's two linked cores (`arena*.c`; over the server `arena_link_net.c` and `arena_wire.c`, asked for in the Square and again after a result `square_arena.c`, the MegaMan it carries `arena_format.c` and the folder Lan picks `arena_folder.c`, every battle by full rules (the NetPass's full set), a united Soul carried to the other GBA `arena_souls.c`, the battle rolled back over a wrong guess of the other's keys `arena.c` and the window this system affords `arena_window.c` (issue #128), MegaMan's words `arena_words.c`; docs/ONLINE_ARENA.md), the carried MegaMan (`online_snapshot.c`), the NetPass (issue #131: the full set's table and its stamps `netpass.c`, the pass drawn `netpass_draw.c`, on the game's screen and its stamp moment `netpass_screen.c`, its words `netpass_text.c`; opened from the PET's key item in `src/director/pet.c`, by START in the Square, a strip at the gate, a panel on the second screen), the stands (issue #128: a battle watched, its pair built and run from both players' keys `arena_watch.c`, `watch_keys.c`; the referee's offer and the watching screen `square_watch.c`; the report the cable's first sends `arena_report.c`, `stands_report.c`; the channel's battles `online_stands.c`; the wire `stands_wire.c`; the words `stands_words.c`, `stands_text.c`; docs/ONLINE_ARENA.md, The stands), the week's net (issue #127: its Board on the wire `online_week.c`, the greeter's offer, the post and the Board's lines `square_week.c`, their words `square_week_words.c`); the gate itself is the director's (`src/director/director_gate.c`, `gate_words.c`) |
| `src/core/` | Entry point (`main.c`: the command line and the frame loop; `startup.c`: the data folder, the ROMs and the player's files; `start_3ds.c`: the 3DS's own start; `rng.c`), platform (window, canvas, input, timing; the keyboard's map `keymap.c`, the frame log `frame_log.c`), ROM access and `RomLayout`, chip and virus data, loot, another game's battle records read from its ROM (`xrecords.c`) and fought in BN6's engine (`xbattles.c`, issue #82), where the nets meet (`visitors.c`: the visitors and the deep's coin; each net's families' roles in `visitor_families.c`), the chip registry (BN5's chips in BN6's chip tables, issue #82: `chips.c` every reader's one accessor, `xchips_registry.c`, `xchips_record.c`), run state and saves (`backup.c`: the saves as one `.cwsave` file, its carrier; `backup_info.c` its summary and checks, `backup_manifest.c` its manifest, `backup_restore.c` the staged install, Undo and recovery; `save_layout.h` the saves' records' layout and `save_format.h` their identities; `save_queue.c`: their files written off the frame, by a thread of their own; `ios_saves.m`: iOS's SAVES pickers and folder), the requests at home (`jobs.c`: their kinds, offers, pay and course), the economy's plans (`economy.c`: each lever's curve by act, today's flat, the proposal's waiting for the owner, and the bands the economy report holds them to; docs/ECONOMY.md), the week's net's save slot (`weekly.c`: a dive beside the normal run, its standard profile, its end kept for the Board; issue #127, docs/SAVES.md) |
| `src/gfx/` | Sprite decoding and animation, ROM tiles, the font, the engine's own letters for screens before the game's font is read (`pixfont.c`), the QR code (`qr.c`, as the site's `qr.js`) |
| `src/launcher/` | The ROMs screen before the game on the desktops, Android and iOS (issue #97, docs/GUIDE.md's The ROMs screen): `launcher.c` its course, `launcher_draw.c` its layout and picture, drawn on the game's own screen in the style of the second screen's PET panels (`second_frame.c`; a second display stays dark meanwhile), `launcher_art.c` the cartridges and open spots (the project's own drawing; a label's face drawn from the player's ROM as it opens), `launcher_roms.c` what a chosen file is and the last start's record (`launcher.ini`), its words in `launcher_text.c`; the system's pickers behind `pick.h` (`pick_desktop.c`, `pick_android.c`, `pick_ios.c`, `pick_none.c`; what they share for SAVES in `pick_saves.c`: `saves.ini`, the transfer place where it is a plain folder; their words in `pick_text.c`), and the automatic copy to the transfer folder, a phone's ROM folder or any chosen one (`mirror.c`) |
| `src/saves/` | The SAVES screen (issue #114, docs/SAVES.md), over the title or the ROMs screen: `saves.c` its course, `saves_actions.c` its buttons' work, `saves_draw.c` its picture, `saves_layout.c` where its buttons are, `saves_text.c` its words; the state its parts share in `saves_state.h` |
| `src/analytics/` | The anonymous statistics (issue #104, docs/GUIDE.md's Anonymous statistics): `analytics.c` the player's answer (`settings.ini`'s `statistics`), the events and their Umami payloads and the systems' queue (the game's own website, its host and Umami's endpoint named once in `analytics.h`), `analytics_ask.c` the question at the first start and on the 3DS's SELECT (shown in the controls screen's place, whose row turns them on or off), `analytics_net.c` each system's way to send, off the frame loop (a thread with `analytics_http.c`: libcurl loaded on Linux and the handhelds, linked with mbedTLS and `analytics_roots.c`'s Let's Encrypt roots on the 3DS, WinHTTP loaded on Windows; `analytics_apple.m`'s NSURLSession; Android's `GameActivity`; the browser's fetch), its words in `analytics_text.c`; the run log (`src/director/runlog.c`) hands it the guardians' battles and the runs' ends |
| `src/audio/` | MP2K sequencer and mixer (title music and sounds, and the start's chime: `intro_chime.c`, a Pixabay sound `tools/intro_chime.py` converts); the core's sound during play |
| `src/net/` | Layer generation (rooms, walkways, objects): `net_gen.c` the driver and where the services, data and people stand, `net_plan.c` a layer's layout and signature in its act, `net_signature.c` the signature rooms (docs/LEVEL_DESIGN.md, Identity), `net_detours.c` the detours, `net_landmarks.c` the landmarks and props, `net_set_pieces.c` where the set pieces stand (`net_pieces.c` which a layer holds) |
| `src/scenes/` | The start (`scene_intro.c`: the developer's boot screen, `intro_logo.c`, and MegaMan's word on GitHub, before the title at a plain start), the title (with the run summary), the screen a start without its ROM shows where there is no ROMs screen (`scene_error.c`: PortMaster, the 3DS) and the sprite gallery |
| `src/dev/` | Developer tools inside the game: `--dev` (`devtools.c`), the atlas, the tour, the pacing report, headless runs' scripted input and pictures (`capture.c`: `--input`, `--taps`, `--shot`, remote play), the tools run in the game's place (`tools.c`: `--atlas`, `--pacing`, `--economy` through `economy_report.c`: what a run hands out by act, docs/ECONOMY.md, `--render-song`, `--sheet` through `sheets.c`, `--xdiff` through `xdiff.c`: a moved virus family, or MegaMan, against its own game's battle, issue #82; `--xrecords` through `xrecords_report.c`: BN5's records in BN6's engine) |
| `src/emu/` | The mGBA core, calls into the game through hooks (`gamecall.c`: warps, chat), hooks on its code (`hook.c`), its wait for VBlank halted (`idle.c`), ProtoMan's cross mended (`protoman_cross.c`, issue #55), other games' virus families moved into its battle engine (`xvirus.c`, the recipes in `xvirus_families.c`, the Thumb code's moves in `thumb_reloc.c`; issue #82, docs/MULTIROM.md), their battlefields as BN6's (`xfields.c`), BN5's lava and sea as panels of BN6's engine (`xpanels.c`, their look `xpanels_look.c`; the sea's submerge and hold in `xsea.c`), BN5's Navis moved into it (`xnavi_install.c`: their recipes in `xnavi_bn5.c`, written by `tools/xnavirecipe.py` and checked by `xnavi_recipe.c`; BN6's Navi tables grown, the Navis' names and stubs in `xnavi_room.c`, their battle sprites in `xnavi_sprites.c`, BN5's engine objects they share moved once in `xnavi_objects.c`, the Dark Soul's own rules and chip records in `xnavi_dark.c`; docs/MULTIROM.md, Transplanted Navis), BN6's attack families grown once for every track (`xattack_grow.c`), its chip tables grown for their chips (`xchips_install.c`) and BN5's own code for the chips BN6 has none of (`xchips_code.c`, its units in `xchips_bn5.c`, written by `tools/xchiprecipe.py`), the tracks' installs in their order (`xinstall.c`: the moved objects, the Navis, the chips' code, the families, the sea, then the Souls' code and BN5's obstacle strike for either Colonel, `xstrike.c`; one copy a moved type), the free space past the game's 8 MB and the RAM BN6 leaves alone by one plan (`free_space.h`, docs/EMULATION.md), the installers' writes claimed so that no two write the same bytes (`claims.c`), BN5's installs written down as they are made, for the Arena's cores under full rules (`xinstall_log.c`), the Souls' proxy form (`soulproxy.c`, `--dev soulproxy`) and the Souls on it (UNITE on the Custom screen in `soulunite.c`, a Soul's life in `soullife.c`, BN6's chips' kinds as BN5 reads them in `soulkinds.c`, a Soul's look, moved code, abilities, passives and chips in `soullook.c`, `soulcode.c` (its recipe `soulcode_bn5.c`, written by `tools/xsoul.py`), `soulpower.c`, `soulpassive.c` and `soulchips.c`, ColonelSoul's button in `soulcolonel.c`, BN5's mood and DarkChip rule in `soulmood.c`; `--dev souls=MASK`), BN5's chip AI for the player, DarkInvs's machine (`darkinvs.c`, `--dev chipai`), boot, event flags, debug output, the autopilot and the scene (`docs/EMULATION.md`); `bn6.h` names the game's addresses and the fields of its structures (the PET's in `bn6_pet.h`, the battle engine's per-family tables in `bn6_families.h`, the chip tables and records in `bn6_chips.h`, the Navi tables in `bn6_navis.h`, its battlefield's panels in `bn6_panels.h`, MegaMan's forms in `bn6_forms.h`, which it includes) |
| `src/map/` | Layers as game maps: tiles learned from the original maps (`tiles.c` their checks, `tiles_learn.c`, `tiles_pick.c`), walls and warp-pad triggers, the map tables taken over (`netmap.c`; the originals learned in `netmap_learn.c`, their scenery pasted in `netmap_paste.c`, the set pieces' cells in `netmap_extra.c`); Lan's HP, BN6's own map taken over as the run's warp zone (`lanhp.c`, `docs/HOME.md`) |
| `src/layer/` | What stands on a layer: NPC and text scripts, services, shops, choices, guardians. `layer_objs.c` installs each kind of object, `scripts.c` holds the parts every chat is built of; the words are files of their own (docs/VOICE.md, Where the lines live): `layer_words.c` (the Net Dealer, the NaviCust vendor, ProtoMan's terms, a ScrtData), the chats in `service_words.c`, `reward_words.c` and `lock_words.c`, the guardians' `guardian_lines.c`, the NaviCust's `navicust_words.c`, the bystanders' `npc_lines.c` and `rumor_lines.c` |
| `src/director/` | The run on the game: the town, layers, warps, encounters, bosses, checkpoints, powers; the game's events from its hooks (`events.h`: battles in `encounter.c`, maps, choices and key items in `events.c`), taken up after each frame. `director.c` is the frame's course and the state its parts share (`director_state.h`); its parts are `director_*.c` (the run's end, lost or won, `director_end.c`; layers made and entered, the run's chips, checkpoints, what the drawing reads, the map, the way on, the pad, the NaviCust's board, the duel, DarkChips, BN5's records fought, dev steps, the state in words, home after an act, Lan's HP and its portals: `director_home.c`, the home's frame `director_town.c`, the requests taken, kept and counted `director_jobs.c`), and what they say is in `*_words.c` (L's briefing, the story's beats, Chaud, the NaviCust, DarkChips, where the nets meet, lessons, coming home) |
| `src/second/` | The second screen (issue #72): the 3DS's bottom screen, Android's second display. It follows BN6's screen as the DS games' PET followed theirs: `second.c` reads which screen the player is on once a frame (BN6's modes, docs/ROM_DATA.md) into a state the draw reads (on the 3DS the draw runs while the core's thread runs the next frame), `second_read.c` reads each panel's facts, `second_frame.c` draws the PET's frame (in `tools/site_art.py`'s colours, measured from BN6's PET), each screen's panel has a file of its own, and `second_text.c` holds the panels' words |
| `src/world/` | The real world: Central Town, home, learned from its own tiles and planned (`docs/OVERWORLD.md`), and BN6's own indoor maps taken over as home's places (`indoors.c`, side by side in `home_places.c`): Lan's house and room, where a run begins and his PC jacks in (`lan_house.c`, `docs/HOME.md`), AsterLand (`aster_land.c`) and the Cyber Academy (`academy.c`), their people and what they say in `place_lines.c`, the askers' requests in `job_words.c`; the town's folk and what is said there in `town_lines.c`, at each visit home who stands where (`town_folk.c`) and what they say of the run (`town_words.c`); its other ports in `town_ports.c`, for the jobs' errands |
| `tests/test_core.c` | ROM-free unit tests (with `tests/test_backup.c`, the `.cwsave` carrier and its install); `tests/test_saves.c` the SAVES screen, `tests/test_transfer.c` and `tests/test_discovery.c` the automatic copy and its folders, with made-up progress; `tests/test_mapslot.c` the maps' Mystery Data lists emptied as layers are installed, in a made-up ROM's tables; `tests/test_android_saves.py` `RomLook`'s saves against document-provider doubles on a JVM; `tests/test_emu.c` the hooks on mGBA, and two cores side by side as BN6's and the differential's reference run (each one's hooks, RAM, sound and state its own), with ROMs of its own bytes; `tests/test_add_to_steam.py` runs `linux/steam/add-to-steam.py` against a made-up Steam folder; `tests/test_voice_catalog.py` the voice catalog's reading of C on a words file of its own; `tests/lint/` the baselines `build.py lint` checks against |
| `tools/romlab/` | libmgba research harness (dev only, needs your ROM) |
| `tools/uinput_keys.py` | On-device input injection for testing |
| `tools/play.py` | Playtests: the Linux build headless (`--remote`), played a batch of input at a time, a picture and the state in words after each |
| `tools/voice_catalog.py`, `tools/voice_dashboard/` | The voice review (issue #135, docs/voice/README.md): every line of the words files read as C into `docs/voice/catalog.json` (ids that survive rewording, boxes, speakers, placeholders, the chat box's pages, comments, VOICE.md's flags, our numbers beside BN6's; `--check` for `build.py lint`), and the review page published as an artifact with the catalog beside it, which keeps the owner's verdicts and directions in its db |
| `docs/voice/` | The catalog (`catalog.json`, written by the tool, never by hand) and how the review runs: publishing the dashboard, what it stores, a voice pass (`README.md`) |
| `.claude/skills/playtest-loop/` | The playtest loop as a skill: a persona plays through `tools/play.py`, its reports are triaged and fixed, and the loop's log (`lessons.md`) grows with every session |
| `.claude/skills/game-design/` | The game-design skill (from saschb2b/skills): the lens, pattern catalog and frameworks to reason from before a design decision |
| `.claude/skills/trailer/` | The trailer skill: a release's trailer from the brief to the site, its music's habits read from the ROM (`scripts/song_habits.py`), checked without ears (`scripts/music_check.py`), its frames picked and reviewed on contact sheets (`scripts/sheets.py`), and the log of the trailers made (`lessons.md`) |
| `.claude/skills/agent-browser/` | The agent-browser skill: a real Chrome driven from the shell (`npm i -g agent-browser`), to look over the site at desktop and phone widths |
| `port/` | The PortMaster port: the launcher, `port.json`, `gameinfo.xml` and the README of PortMaster-New's `ports/cyberworld/`; `build.py package` puts them in that folder's layout in `build/port/cyberworld/`, with the screenshot (`docs/screenshots/portmaster.png`), the binary and one `licenses/LICENSE.<part>.txt` per part |
| `linux/` | The Linux release: its README, the tar.gz's menu installer, the `.desktop` entry, AppStream metadata and the icons (`tools/app_icon.py`) that the AppImage, the `.deb` and the Flatpak (`linux/flatpak/`: its manifest, how to build it and bring it to Flathub) carry. The app ID `io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless` is the window's class too (`src/core/platform.h`); `src/core/desktop.c` finds the ROM, opens the file chooser for the ROMs screen (`zenity`, `kdialog`, `osascript`) and adds the AppImage to the menu. `linux/steam/`: `add-to-steam.py` (the game as a non-Steam shortcut, with the library artwork `tools/steam_art.py` draws), which every Linux build carries and `--add-to-steam` runs |
| `windows/` | The Windows release: its README, the NSIS installer script (per user, no administrator), the .exe's manifest (UTF-8 paths, per-monitor DPI), resource script and icon (`tools/app_icon.py`); `src/core/desktop_win.c` is its desktop (the ROM in Windows' file dialog and Downloads) and `src/core/compat.h` what differs from POSIX |
| `android/` | The Android app (`build.py android`): a Gradle project whose `RomActivity` starts the game (the icon's ROMs shortcut asking for the ROMs screen), whose `RomLook` looks at what the ROMs screen's pickers chose (Android's folder picker, or the files in one picker; each `.gba` told by its game code, BN6's and BN5's checked by SHA-1 and copied into the app's files; the folder, kept read and write, looked in again at each start and given the saves' copy; `res/xml/backup_rules.xml` keeps the ROMs out of Android's backups) and whose `GameActivity` is SDL's activity running the game's C in a process of its own (`:game`, ended with the session), built by the NDK from every `src/*/*.c` (`app/jni/CMakeLists.txt`) with SDL2's own source and the GBA core; its README says how releases are signed |
| `macos/` | The macOS app: `deps.sh` (SDL2 and mGBA as universal static libraries, on a Mac), the bundle's `Info.plist`, its icon (`tools/app_icon.py`) and the .dmg's README; `build.py macos` runs only on a Mac, so CI's macOS job builds and checks it |
| `ios/` | The iPhone and iPad app (`build.py ios`, on a Mac with Xcode): its README (AltStore and SideStore, the ROM through Files, building), `deps.sh` (SDL2 and mGBA as static arm64 libraries for the phone or the Simulator), the bundle's `Info.plist` and its icons (`tools/app_icon.py`). `src/core/ios.m` is UIKit's (the Files pickers for the ROM folder or files and the folder's looks at each start, haptics, the safe area, the `main` that hands SDL's to UIKit), `src/core/ios_saves.m` SAVES' pickers and the saves' copy in a folder; `tools/altstore_source.py` writes the AltStore source each release carries. CI's ios job builds the IPA and starts the Simulator's app |
| `3ds/` | The Nintendo 3DS build (`build.py 3ds`, issue #9): its README (playing, building, testing on a 3DS), the icon, the CIA's title settings (`cia.rsf`, for makerom) and the HOME Menu's banner (`banner.png`, `tools/steam_art.py`; its sound a few seconds of the current trailer's music, `tools/trailer_music.py --banner`). `src/core/present_3ds.c` puts the canvas on the top screen through the GPU (citro2d), and the second screen on the bottom one (`src/second/`, drawn by `platform.c` every tenth frame); `src/emu/emu.c` runs the GBA core on the New 3DS's third core where the system gives one, its picture drawn on the main core (mGBA's threaded video); `src/core/start_3ds.c` splits the app's memory, gives back the ROM buffer mGBA's 3DS setup takes and never uses, and reads BN5's ROM where the heap has room for it (docs/EMULATION.md, The 3DS's memory; `tools/heap3ds.py` models that heap on the host build) |
| `web/` | The project site on GitHub Pages, laid out like BN6's PET screens: the home page (`index.html`, `assets/`), the downloads in `download/` (the newest release's files, one system at a time), the player in `play/` (ROM check and storage: BN6's, and BN5's beside it, each told by its game code and checked by its SHA-1; scaling), the FAQ in `faq/` as the PET's E-Mail, its answers taken from the docs, and `404.html`. Each page has its Japanese one in `ja/`, written for Japan's players, who know the game as ロックマンエグゼ6 (the ROM they need said first; BN6's names as Capcom's Japanese releases write them): the same page in all but its words, with the same ids, pictures, clips and scripts, which take their words from the page's `lang`. `tests/test_site.py` holds the two in step, with their hreflang links, canonical addresses and every link between the pages; a change to a page's words changes its Japanese one too. `assets/qr.js` draws the 3DS title's link as a QR code for FBI; `assets/site.js` sends the analytics' events (Umami: what was done, never by whom; links name theirs with `data-umami-event`). Each page has its canonical address, Open Graph tags and schema.org data; `build.py site` adds the sitemap, the FAQ's FAQPage data and the content hash on each page's own files. Its frames and icons (`assets/ui/*.png`) are drawn by `tools/site_art.py`, not taken from the ROM |
| `docs/screenshots/` | Screenshots of the game for the README and the site (`build.py screenshots`) |
| `docs/clips/` | Short videos of the game for the site: WebM, MP4 and a poster each, and a GIF of those the README shows (`build.py clips`, ffmpeg in a pinned image); the current trailer (`trailer.*`: the site's hero and every page's preview picture) and `trailer-play.png`, its poster with a play button, which the README links to the MP4 (`tools/trailer.py`, Trailers below) |
| `docker/` | Build images: `Dockerfile` (host, Debian trixie), `Dockerfile.portmaster` (the PortMaster port's aarch64 binary: Debian bullseye's glibc 2.31 and SDL2 2.0.14, older than any firmware PortMaster serves; `debian/eol` from its snapshot), `Dockerfile.linux` (desktop release, bookworm, SDL2 from source), `Dockerfile.windows` (MinGW-w64, SDL2 and mGBA static, NSIS), `Dockerfile.android` (the SDK, NDK and Gradle, SDL2's source, mGBA per ABI), `Dockerfile.web` (Emscripten, mGBA without threads), `Dockerfile.3ds` (devkitPro's devkitARM, SDL2 and mGBA's library for the 3DS, makerom and bannertool from their sources for the CIA: `ctrtools.sh`) |
| `.github/` | CI (`ci.yml`: checks, every target, Pages from `main`), releases (`release.yml`, on `v*` tags), the cached image build action, Dependabot |

## Rules

- A claim about the original games (what BN6 or BN5 has, does or lacks)
  is looked up in docs/SOURCES.md's sources before it shapes a design,
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
- Patch only the core's in-memory ROM copy, and put new data in the free
  space map in `docs/EMULATION.md`, its place a region of
  `src/emu/free_space.h` (RAM BN6 leaves alone too), which
  `tests/test_core.c` holds apart from every other; an installer writes
  within `claims_begin`/`claims_end` (`src/emu/claims.h`). Never write the
  player's ROM file.
- A new ROM offset needs a note in `docs/ROM_DATA.md` saying how it was
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
  whole scales there too.
- MegaMan walks as BN6 walks him, always: no lining-up help, no sliding
  onto a walkway, no movement assist of any kind (one was removed by the
  owner's choice, docs/FIDELITY.md). Where walking drags (walkway mouths,
  lanes), the generated maps change instead, connected as Capcom's are
  (docs/LEVEL_DESIGN.md, Navigation).
- Draw calls must not change game state. Input is read once per frame in
  `platform_poll`.
- The C builds warning-free with the Makefile's `WARN` (`-Wall -Wextra`,
  `-Wshadow`, `-Wcast-qual`, `-Wwrite-strings`, `-Wformat=2` and more; issue
  #19) on every target, `-Werror` in CI. `build.py lint` holds the rest to
  its baselines in `tests/lint/`: no new finding of GCC's analyzer, no new
  function no build reaches, no function joins or grows past CCN 25 or 120
  lines, no ROM offset without its name in `docs/ROM_DATA.md`, no field of
  the game's structures as a bare offset outside `src/emu/bn6.h` (issue
  #34: `BN6_PLAYER_X`, not `BN6_PLAYER + 0x1C`), no new code smell and
  no header that is not whole on its own (Code structure, below), and no
  ROM, save, state or large file in git outside `docs/`. A baseline only
  shrinks: after fixing one, `build.py lint --update`.
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
  PET's text). A voice pass (docs/VOICE.md) reads every line there, and
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
Addresses stay in `docs/ROM_DATA.md` and claims in `docs/SOURCES.md`;
`docs/re/` holds where the work stands (`ledger.json`, written out by
`tools/re_status.py` as `STATUS.md` and the dashboard), not a second
address map.

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
python3 tools/play.py start NAME [--fresh]   # a headless game for a playtest (build.py linux first)
python3 tools/play.py do NAME "press A; hold UP 30" [--every 10] [--keep DIR]   # input, then a picture and the state (--keep: each frame into DIR)
python3 build.py serve      # the site and the browser build on http://localhost:8080
python3 build.py xdiff [SPEC] [--mutations]   # tools/xdiff.py: a moved virus family against its own game's battle, frame by frame (tests/xdiff; both ROMs)
python3 build.py xrecords [--all]   # tools/xrecords.py: BN5's territories' records as BN6's engine fights them, each fightable or why not (both ROMs)
python3 tools/xproxy.py [--mutations] [--souls]   # the Souls' proxy form (form 8) against HeatCross, every byte of every frame (the ROM; --souls the Souls over it, both ROMs)
python3 tools/heap3ds.py [--seeds 1-12]   # the New 3DS's heap modelled on the host build: long runs' peaks and refused allocations, BN6 alone and with BN5 (both ROMs)
python3 build.py economy [RUNS]  # what a run hands out by act, today's plan and the proposal's, against their bands (docs/ECONOMY.md; the ROM)
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
```

The host, Linux, Windows and macOS builds are desktop builds
(`CW_DESKTOP`): a resizable window, saves in
`~/.local/share/cyberworld-endless` (Windows:
`%LOCALAPPDATA%\cyberworld-endless`, macOS: `~/Library/Application
Support/cyberworld-endless`), the ROM looked for there, beside the binary
and in `./rom`. The ROCKNIX build fills the screen
and takes both folders from its launcher. `build.py run` is the quickest way
to play a change; the headless `shot` stays the way to capture one.

The browser build (`__EMSCRIPTEN__`) has no threads: the page drives one
game frame per 1/60 s (`emscripten_set_main_loop`), and writes reach
IndexedDB through `platform_persist()`, which saves and states call. Keep
the frame loop free of blocking waits. Every battle is BN6's core's on
every build, BN5's areas', Navis' and chips' too (docs/MULTIROM.md, One
net): BN5's ROM is read as data, and its own core (`src/emu/guest.c`) is
the differential's reference alone, never started in a run. `build.py serve` also serves the
developer's ROMs, told by their SHA-1, for tests in a local browser:
BN6's at `/.dev/rom.gba`, BN5's at `/.dev/bn5.gba`; the page itself only
takes the ROMs the player chooses.

CI runs without a ROM: it builds every target with `-Werror`
(`WERROR=1`, set when `CI` is), runs `build.py lint` and
`tests/test_core.c`, `tests/test_backup.c`, `tests/test_saves.c`,
`tests/test_transfer.c`, `tests/test_discovery.c` and `tests/test_mapslot.c` under the
sanitizers, then `tests/test_add_to_steam.py`; its android job runs
`tests/test_android_saves.py`. Everything that needs the game (captures, autopilot, atlas,
pacing) stays local.

`shot` runs headless in the build image with the repository at `/src` and
`~/.cache/mmbn-ref/roms` (override with `CYBERWORLD_ROM_DIR`) mounted
read-only; `--data-dir` defaults to `.build/data`. `--scene emu` starts a new
run on the game, `--run-depth N` at depth N, `--net-biome N` in one area (`xN` another game's area N, its ROM beside BN6's: docs/MULTIROM.md),
`--guardian N` with navi N guarding every area,
`--seed S` with a given seed, `--scene town` from the town as NEW GAME does,
`--scene launcher` the ROMs screen (the ROMs in `--rom-dir`; `--launcher auto`, `open` or `off` sets a start's own),
`--scene saves` the SAVES screen,
`--scene summary` the title's run summary (a win's with `--setup short` at
`--run-depth 10`), `--setup NET,FOLDER,THREAT,HELPERS[,CROSS]` the run's setup (CROSS the navi whose Cross it brings, 1-5),
`--marks HEX` the title's marks, `--touch` the touch controls from the
start (as on a phone; with `--size` a phone's screen and `--dpi N` its density), `--talk NAME:FRAME,...` opens a
layer's chats at its frames (npc, shop, heal, programs, gift, challenge,
undernet, gate, navigate for a Navi gate, vault, duel for ProtoMan's terms, official for an official gate, trader and bugtrader for a Chip or BugFrag Trader; intro, defeat, reward for the guardian; status for L; rumor for the layer's whisper (rumor_lines.c); dark for a flame of darkness (docs/META.md);
fragment for MegaMan's words at a ScrtData; bugfrags gives 50 BugFrags, keys an Unlocker, rushfood three RushFood, wwwid a WWW-ID, zenny 10000 zenny, regup a RegUP3). `--input "FRAMES:BUTTONS,..."` scripts the
buttons (`UP+RIGHT`, `A`; play.py's dev steps too, `0:place X Y FACE`, `0:flags FROM TO 1`, `0:sig` (MegaMan in the layer's signature, docs/LEVEL_DESIGN.md, Identity) and `0:battle`, the layer's next random battle as soon as MegaMan is free on its map, BN5's on its territory; `300:battle` waits its 300 frames first; `0:foes 200@52/01@43` a battle in BN6's engine of the enemy ids named at their panels, column then row, `0:xrecord 811DE10` one of BN5's battle records in BN6's engine (`build.py xrecords` lists them), and `0:status freeze 90` its enemies given a status as a hit leaves it, docs/DEVTOOLS.md), `--taps "FRAME:X,Y[>X2,Y2];..."` fingers at screen pixels (a tap, or a drag over 20 frames: the touch controls, their menu and editor), `--shot FRAME:PATH,...` and `--shot-range A:B:PREFIX`
save frames (the canvas; `--screen-shot FRAME:PATH,...` the whole screen, the touch controls on it; `--second-shot FRAME:PATH,...` the second screen, the 3DS's bottom one with its panel, at `--second-size WxH` an Android display's, and `--second-shot-range A:B:PREFIX` its frames as `--shot-range` the picture's), and `--sheet CAT:IDX:ANIM[:PAL]:PATH` or `--sheet
@CAT:FIRST:COUNT:PATH` draw sprites. Environment variables reach the image
only when `build.py` lists them (`CYBERWORLD_EMU_DEBUG`, `CYBERWORLD_AUTOPILOT`
and the audio ones).

`tools/play.py` keeps one game running per NAME (`.build/play/NAME`: its
saves in `data/`, every command in `history.txt`, the seed in its first
line), so a playtester, person or agent, plays it like the handheld,
reading each picture; a session replays from the seed, `data0/` and the
history. Each start copies the current build into the session's `bin/`;
with `NAME/bin.pin` present its restarts keep that copy, so a rebuild
during a playtest leaves its CONTINUE alone. Dev steps beside the
player's: `place X Y FACING` puts MegaMan somewhere, `flags FROM TO 1`
sets a block of event flags and `flags FROM TO 0` puts them back,
`stuck NAME` saves the run with BN6 holding MegaMan so (a CONTINUE's
recovery tested, issue #23), `foes 200@52 01@43` a battle of the foes named, `xrecord 811DE10` one of BN5's records in BN6's engine and `status freeze 90` a status on its enemies (docs/DEVTOOLS.md), `battle` starts the layer's next random battle once he is free on its
map (on BN5 territory one of its records, in BN6's engine), `sig` puts MegaMan in the layer's signature (the state
names it and where it lies), `second` saves the second screen's picture beside the
top one's, and `dump NAME` the video memory in romlab's format for
`tools/romlab/labtrace.py`.

`tools/romlab` runs the plain ROM in libmgba for research: scripted input,
memory peeks and pokes, states and recordings. `labtrace.py` traces captured
tiles, palettes and OBJs back to ROM offsets.

| Change | Checks |
| --- | --- |
| Engine code | `build.py test`, `build.py lint`, a headless capture of the affected screen, an autopilot run |
| Layer generation or maps | `build.py test` (connectivity over 300 seeds), `build.py tiles` (no layer worse than `tests/atlas_baseline.txt`, the close-ups looked over), captures of every area (`--net-biome 0`-`7`) |
| The town | `build.py town` (its seeds drawn, misses marked, the game around it), an autopilot run from `--scene town` through the jack-in |
| Layer objects, scripts, shops | A capture of the talk or screen with scripted input |
| Lines (a words file: `*_words.c`, `*_lines.c`, `*_text.c`) | docs/VOICE.md's checks; `python3 tools/voice_catalog.py` (the catalog regenerated and committed with the line, the flags of the lines touched read over), `build.py lint` (fails while it is stale), `build.py test`; a capture of the talk (`--talk`); the dashboard republished with the new catalog, and the reviews and directions acted on marked applied (docs/voice/README.md, A voice pass) |
| Difficulty, encounters, guardians, rewards | `build.py test`, `build.py pacing` (0 past their band, BN5's guardians among them), an autopilot run; where BN5's content changes, over `--net-biome x0`-`x5` and with `--guardian 24`-`29` too, fresh data folders, the run logs looked over |
| Drops, prices, Mystery Data, dealers' stock, the economy's curves (`loot.c`'s rolls, `net_gen.c`'s data, `shop.c`, `economy.c`) | The game-design skill first; `build.py economy` (0 of the plan in play outside its bands; a number moved on purpose sets its band's centre in `economy.c`, the report's before and after in the commit), `build.py test` (`test_economy`, `LAYER_MAKE_HASH`), `build.py pacing` where a battle's reward changes |
| Audio | `--render-song ID:SECONDS:PATH` and a listen on a device |
| The ROMs screen | `build.py test`, headless captures with `--scene launcher` from 240x160 to a phone's screen (`--size`, `--dpi`) with no ROM, BN6 alone and both, and on Android and iOS the pickers in an emulator or the Simulator (the packaging rows) |
| SAVES | `build.py test`; headless captures of home, compare and result (`--scene saves`) at 240x160, 640x480 and 1080x1920; an export from one data folder imported into a fresh one, CONTINUE on the same layer; UNDO IMPORT puts back the files byte for byte |
| The anonymous statistics | `build.py test` (the answer, the payloads, the queue's cap, the tries); headless captures of the question (`--dev statistics=ask` and a fresh `--data-dir`) and of the controls screen's row (`--dev statistics=on`, SELECT on the title); runs against a local stand-in for Umami (`--dev statistics=on,statsurl=http://127.0.0.1:PORT/api/send`; Wine's in a container on the host's network), never Umami itself, but for `--dev statscheck`, one `game-dev-check` event to check a system's way there (docs/DEVTOOLS.md) |
| Browser page or platform code | `build.py serve` and a run in a browser: ROM choice (BN6 and BN5, and a wrong file refused), a new game, a battle in a BN5 area, CONTINUE after a reload |
| The site | `build.py test` (`tests/test_site.py`); `build.py serve` at desktop and phone widths (agent-browser), the English pages and their Japanese ones; `build.py screenshots` and `build.py clips` again when what they show changed |
| Windows packaging | `build.py windows`; the .exe under Wine (a Debian image with `wine`, `wine32:i386` and `xvfb`): a headless `--scene emu` capture with the ROM, a start in a window, the installer's silent install (`/S`), a start and `uninstall.exe /S` |
| Android packaging | `build.py android`; the APK in an emulator (KVM, the emulator and an image in `.build/android-sdk`; ROM copies pushed with `adb push`): a fresh install opens the ROMs screen, whose BN6 slot asks for the ROM folder; a folder with BN6 and BN5 takes both (logcat `-s Cyberworld SDL/APP`: "BN5 found"), one with only a wrong `.gba` refuses it with its reason, BN5 put in the folder after the first start comes in at the next, and ROMs right in Download go through **FILES INSTEAD** (both picked at once) and the icon's **ROMs** shortcut; the saves' copy (`cyberworld-endless.cwsave`) in the folder after the game saved, an uninstall, and a reinstall whose folder chosen again opens SAVES' comparison of that copy, BRING BACK and CONTINUE on the same layer (the files' hashes as before); a new game by touch, the D-pad walking, a turn to landscape, Back twice and the game opened again |
| macOS packaging | CI's macOS job (`lipo`, `otool -L`, `codesign --verify`, a ROM-less headless start on the ROMs screen); on a Mac, `build.py macos` and a run with the ROM: the ROMs screen's open panel, a new game, CONTINUE |
| iOS packaging | CI's ios job (arm64, iOS 14, only iOS's frameworks, `codesign --verify`, the Simulator's app reaching its ROMs screen: look at the artifact's screenshot); on a Mac, the Simulator with the ROMs: BN6's slot on a folder with BN6 and BN5 (both taken, the title's "BN5 found"), on one with only a wrong `.gba` (refused with its reason), BN5 put in the folder after the first start (taken at the next), FILES INSTEAD with both, the folder's saves' copy opening SAVES' comparison after the app was deleted and installed again, a new game, a turn to landscape, CONTINUE after the app was sent to the background; on an iPhone, AltStore's install from the release's source |
| 3DS build | `build.py 3ds`; `python3 tools/heap3ds.py` where a change allocates more (the 3DS's heap modelled here); on a New 3DS (3ds/README.md): the CIA installed with FBI (its QR code, or from the SD card) and started from the HOME Menu, the .3dsx over ftpd and a start from the Homebrew Launcher, NEW GAME to the first layer and its map on the bottom screen, `log.txt`'s frame lines (`frame_log = on`, the heap's high-water mark among them); with BN5's ROM beside BN6's, 3ds/README.md's BN5 checks (`log.txt`'s BN5 and heap lines, a layer of BN5's areas over 3dslink's `--net-biome x0`-`x5`); the other targets still build, as the 3DS's changes are mostly shared code |
| PortMaster port | `build.py package`; `build/port/cyberworld` copied into a PortMaster-New checkout's `ports/` passes its `tools/build_release.py --do-check` and `tools/build_gameinfo.py`; the binary asks for glibc 2.30 at most (CI checks); a device run; a new PortMaster-New pull request needs the testing its template lists (ArkOS, AmberELEC, ROCKNIX, muOS, Knulli; 640x480 and larger) |
| Linux packaging | `build.py linux`; the AppImage and the `.deb` in a clean distribution container (first start without a ROM, the menu entry, a start with one); `--add-to-steam` with `HOME` at a made-up Steam folder |
| Release | A new trailer where the release earns one (Trailers), made before the tag; device run on the Nova and the Flip 2, `build.py release`, the Linux AppImage, `.deb` and archive each started fresh; tag `vX.Y.Z` on `main` |

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

When handing off, state what changed, what ran, and what is unverified on
hardware.
