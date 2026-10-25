---
type: Engine Mechanism
title: "Hooks"
description: "Hooks on BN6's code (`src/emu/hook.c`): a BKPT over an instruction that calls the engine's C mid-frame, event and answer hooks, one table per core, and every address hooked, from battles and maps to BN5's Souls."
tags: [engine, emulation]
generated: { by: claude-code/unrecorded, at: 2026-10-22T20:25:41Z }
---

# Hooks

A hook lets one instruction of the game's code call the engine's C right
there, mid-frame, with the CPU's registers (`src/emu/hook.c`, issue #27):
the reach a native port's hooks give (Ship of Harkinian's, N64Recomp's
function replacement), while BN6's code stays the ROM's.

- **The BKPT.** A Thumb `BKPT #0xCE` (`0xBECE`), or an ARM one
  (`0xE1200C7E`), is written over the instruction in the core's ROM copy,
  as every patch is, or in RAM. mGBA runs a BKPT's handler at no cost in
  cycles, with the PC already past it (`src/arm/isa-thumb.c`). The board's
  handler, which mGBA installs once in `GBAInit` (a reset keeps ours),
  takes immediate 0 (the debugger) and 1 (the cheat device): ours comes
  first and passes it every other immediate.
- **After the hook.** `ARMRunFake` puts an instruction into the prefetch in
  the BKPT's place, as mGBA's own cheat hooks do: the original, which runs
  as if never replaced (`HOOK_CONTINUE`, with the registers as the hook
  left them); `bx lr` (`HOOK_RETURN`, r0 and r1 the result; at a routine's
  first instruction, before it pushes anything); or `bx r12` (`HOOK_JUMP`,
  on elsewhere).
- **Two kinds.** An *event hook* only queues the registers it met, and the
  director takes the queue up after the frame (`emu_hook_events`,
  `src/director/events.h` its kinds). An *answer hook* returns, or changes,
  what the director prepared before the frame, and may queue an event of
  its own where only some calls are worth one (`hook_post`). Neither
  touches the director's state.
- **Threads.** Where the core has a thread of its own (the New 3DS,
  `CYBERWORLD_EMU_THREAD=1`), a hook runs on it while the main thread
  draws. `emu_read*` and `emu_write*` wait for the frame in progress, so a
  hook reads and writes the game's memory with `hook_read*` and
  `hook_write*`, straight through mGBA's raw access; the queue is written
  only during a frame and read only after one, so it needs no lock. The
  browser build has no threads, and the same rules hold there.
- **States and resets.** A state holds RAM, not the ROM copy, so a hook in
  the ROM stays; one in RAM is written again after a state is loaded or the
  core reset (`hook_reapply`). A patch of RAM the game copies from a ROM
  image patched alike (IWRAM's routines, as the game powers on) is kept
  (`emu_keep`): saved with the game's own bytes, made the engine's again
  as a state is loaded. A hook taken off restores the original; its
  slot stays only while the CPU may hold its BKPT in the prefetch.
- **A table per core.** Each core attached keeps its own hooks and queue,
  found by the CPU that met the BKPT, and a hook reads and writes the core
  it runs on: BN6's (`hook_attach`, which `emu_hook` and the others use)
  and, for the differential alone, the guest's (`hook_core`, through
  `guest_hook`), so its hooks on BN5's code never touch BN6's core
  (`tests/test_emu.c`: one address hooked on both, each core's frames
  running its own hook alone, reading its own memory). Two
  cores' hooks run one core's frame at a time (the differential's
  frames run on the thread BN6's do), but for the Arena's pair under
  full rules ([Full rules](../../online/arena/from-square/full-rules.md)), whose two cores take
  BN5's installs' hooks on tables of their own (`xinstall_log.c`) and
  step an instruction at a time in turn: what a hook keeps between two
  of its calls is kept one a core (`hook_slot`: `xsea.c`'s and
  `xpanels.c`'s way back through their stubs, `soulchaos.c`'s charge
  stepped), and so are the Souls UNITE offers (`soulunite_held_core`).

The hooks in use (`src/director/encounter.c`; [ROM data](../../rom/overview.md)):

| Address | Routine | Kind | Runs |
| --- | --- | --- | --- |
| `0x08005A98`, `0x08005AE2` | checkThenStartBattle: its first test, the branch after the roll | answer: a forced battle, the engine's record (issue #29) | once a frame on the map |
| `0x08005BC8` | StartBattle | event: the record the battle starts from | once a battle |
| `0x080AC180` | the reward pick (bn6f `sub_80AA910`) | answer: its enemies' reward rows rewritten first | once a battle, as it ends |
| `0x0800A4D2` | the battle's deck dealt from the folder (bn6f `sub_800A3E4` at its end, `src/emu/xchips_install.c`) | answer: the chips that sit out (a chip of BN5's whose code BN6 lacks, a DarkChip of BN5's, every one of BN5's without its ROM: [Other games' ROMs](../../multirom/overview.md)) taken out of the deck, the rest in their order, `0xFFFF` after them; the Regular chip's and the TagChips' marks (BattleState `+0x17`, `+0x44`) cleared where theirs sat out | once a battle |
| `0x08010D58` | every chip use's DarkChip check (bn6f `sub_8010D58`, `src/director/darkbn6.c`) | answer, posting an event (`hook_post`) where a DarkChip of MegaMan's runs as its base chip, no BugFrag in the battle's count | once a chip used |
| `0x0800B79A` | a chip's after-effects as it runs (bn6f `sub_800B79A`) | answer, posting an event where a DarkChip's dark power ran for MegaMan (its id 0x11E-0x122 reaches it only then): its price after the battle | once a chip used |
| `0x0800E2D8` | object_subtractHP | answer, posting an event (`hook_post`) where MegaMan's HP falls | in the rival's duel only: once per battle object a frame |
| `0x08007740` | the enemy spawn (bn6f `sub_800768C`), HP and MaxHP set | answer: the netbattle's ProtoMan held to the act's band | in the rival's netbattle only |
| `0x080FDFA0` | ProtoMan's cross landing back on his panel (bn6f `sub_80FCC4C`, `src/emu/protoman_cross.c`) | answer: his collision region given his own panel back where the cross left it cleared (issue #55; `CYBERWORLD_EMU_DEBUG` says each) | once a cross |
| `0x08005152` | EnterMap past its wait for the fade (`src/director/events.c`) | event: a map entered, after a warp or a battle | once a map |
| `0x0802F114` | SetEventFlag | answer, posting an event for a layer's choice flag (`0x1440`-`0x1447`) and a request's at home (`0x1460`-`0x1462`, `0x146A`) | about once a frame on the map |
| `0x0803CD6C` | GiveItem | event: a key item given (a ScrtData, the run's Spin) | once an item |
| `0x080050EC` | cbGameState, the game mode's state update (`src/emu/gamecall.c`) | answer: a queued game call jumps to its routine in the update's place, its return to `EMU_FREE` + `0x100` | once a frame of the game mode |
| `EMU_FREE` + `0x100` | where a game call's routine returns | answer: its r0 and r1 kept, r4-r11 put back, on to the main loop; a call with the update (`game_call_then_update`, issue #187) on into cbGameState instead, lr the main loop's, so the frame's sprites are drawn | once a call |
| `0x08120B90` | the PET's input handler (`src/director/pet.c`) | answer: A on Save taken for the engine ([The PET](../../game/pet.md)) | each frame the PET takes input |
| `0x080003A6` | `main_awaitFrame`'s DISPSTAT loop (`src/emu/idle.c`) | answer: the CPU halted till the VBlank interrupt (The core's time) | about four times a frame (each interrupt wakes it) |
| `0x0801A802`, `0x0801AD3C`, `0x0801AD94`, `0x0801ADC8`, `0x0801AE24`, `0x080DA5FA` | each actor's status step (its pre-steps' first call), four obstacles' pre-steps and one object's routine: where BN5 steps an object on its lava first (`src/emu/xpanels.c`) | answer: an object on lava hurt as BN5 hurts it, then the lava cooled under a spark by a few instructions of the project's own (`EMU_FREE` + `0x4C0040`), which come back to the hooked instruction, run then | where BN5's ROM is read: once per actor and obstacle a frame |
| `0x0801A1AA` | what the panel under an actor does to it, its poison test (`xpanels.c`) | answer: a Fire object on sea taken as on poison | the same: once per actor a frame |
| `0x0800C3B8` | the per-panel update, its conveyors' test (`xpanels.c`) | answer: lava and sea counted down to plain as a conveyor, from BN5's 0x3C0 | the same: once per panel a frame |
| `0x0800CC0A` | object_setPanelType (`xpanels.c`) | answer: a chip's lava or sea made with BN5's 0x3C0 frames | the same: once a panel set |
| `0x0800C5E6` | the panels' draw, past its palettes' step (`src/emu/xpanels_look.c`) | answer: where lava or sea is on the field, its tiles written into VRAM at tile 0x295 on if a word of each says they are not there (a battle's first frame with them), and its colours over the volcano's or the conveyors', by BN5's animations (where those are not on the field) | the same: once a battle frame |
| `0x0802D644` | the delete count raised as a virus of version 0-3 is deleted (`src/emu/xvirus_grow.c`) | answer: returns at once for an AI index of 32 or more (a family moved into the engine, [Other games' ROMs](../../multirom/overview.md)), whose byte would lie past the 32 BN6 counts (`0x02000070`) | where a family is installed: once a virus deleted |
| `0x0802D652` | the delete count's read (the rare versions' check) | answer: 0 for an AI index of 32 or more | the same: once a rare version is looked for |
| `0x0802D65E` | a rare version's deletion (version 4-5: with event flag 0x1D8 set, the AI index put at `0x0203EAEA` for the chat of new virus data after the results) | answer: returns at once for an AI index of 32 or more | the same: once a rare version deleted |
| `0x0800EC56` | an enemy id's name (`BN6_NAME_OF`: the viruses' archive, the Navis' past 0xFF, and the id's low byte; `src/emu/xvirus_grow.c`) | answer: for an id from 0x200, a family's, the families' archive (`EMU_FREE` + `0x341000`) and its entry there, id - 0x200 | where a family is installed: once a name plate, as a battle begins |

The differential (`--xdiff`, dev only; [Dev tools](../../devtools/overview.md)) hooks both cores
on each game's own routines: the four RNG draws, the spawns' common
routine at its start and its end, PlaySoundEffect, an effect's spawner,
setAttack, a panel reserved, the blink and sprite_load ([Virus families moved into BN6's battle engine](../../rom/virus-families.md),
The differential's hooks), and BN5's alone where a hit landed raises a
side's mood (`BN5_MOOD_GAIN`, which BN6 has not: MegaMan as the
differential's subject); `CYBERWORLD_XDRAWS` puts the draws' four on
BN6's core for the control's runs (`tools/xcontrol.py`).

The Souls' proxy form (`--dev soulproxy`, dev only; [The proxy form: where a Soul stands](../../devtools/proxy-form.md), The
proxy form) hooks the three places Gregar's code reads MegaMan's form
itself, each answering form 8 as HeatCross's 1 and running on: the chips'
bonus by form (`0x0800EF62`, r0), his arm flash's look (`0x080EC8DE`, r0)
and the summon as he crosses (`0x08014BB6`, r4). With a Soul united the
first two answer the Soul's (BN5's bonus by form, NumberSoul's +10, on to
the bonuses' end `0x0800F090`; BN5's flash kind and the Soul as its
frame). The dev step `cross F`
hooks CROSSSELECT's commit (`0x08029394`): the next form it writes (r0)
is F, once.

The Souls on it (the same switches, or `--dev souls=MASK`; the Souls
track's S2, `src/emu/soulunite.c` and `soullife.c`; [MegaMan's forms and BN5's Souls in BN6's engine](../../rom/souls.md),
UNITE on BN6's Custom screen, and a Soul's life on the proxy form) add
fourteen, each an answer that leaves BN6's own way alone while the run
holds no Soul:

| Address | Routine | Kind | Runs |
| --- | --- | --- | --- |
| `0x08027EC6` | the Custom screen's set-up, at Beast Out's test for element 0xB (`BN6_UNITE_STAND`) | answer: element 0xB stands for UNITE too (BN5's rule), its role and art chosen; a battle's first screen opens his mood as his HP says; a Soul's screen marked | once a Custom screen |
| `0x0802A57A` | the emblem's lit test (`BN6_EMBLEM_LIT`) | answer: grey for UNITE (the check below lights it), and for the emblem while a Soul is united | as the screen draws element 0xB |
| `0x08028F48` | element 0xB's update after each pick (`BN6_UNITE_LIT`) | answer: UNITE lit for a Soul the last pick lights (BN5's check), else the emblem by BN6's rule or UNITE grey | once a pick |
| `0x08028D6C` | A on element 0xB (`BN6_EMBLEM_A`) | answer: BN5's own code for UNITE, moved to `EMU_FREE` + `0x480012`, into mode 0x24 | once A on it |
| `0x08029072` | B on the last pick (`BN6_EMBLEM_B`) | answer: UNITE's chip given back, its icon redrawn, on to BN6's own end of it (`0x080290F2`) | once B on UNITE |
| `0x08029418` | the picks leaving the deck (`BN6_EMBLEM_GIVEN`) | answer: UNITE's chip leaves it too, as in BN5 | once a screen's OK |
| `0x08029150` | OK's commit of a type 2 pick (`BN6_EMBLEM_COMMIT`) | answer: UNITE's, the proxy form with the Soul's turns and its used bit, on past Beast Out's (`0x08029154`) | once OK on UNITE |
| `0x08008472`, `0x0800847C` | the turn's change, BN6's countdown calls (`BN6_TURN_COUNT`, `BN6_TURN_COUNT_1`) | answer: a Soul's turns one less, as BN5 counts them | once a Custom screen, per side |
| `0x0801488A`, `0x080148B2` | the turn's form check, Beast Out's end asked (`BN6_TURN_END`, `BN6_TURN_END_1`) | answer: a Soul out of turns ends, through BN6's request to leave the form (`0x08014890`, `0x080148B8`) | twice a turn, per side |
| `0x0801B034` | the weakness break (`BN6_FORM_BREAK`) | answer: none for a Soul (on at `0x0801B052`) | once a weakness hit |
| `EMU_FREE` + `0x480000`, + `0x480004` | the proxy's overlay entries 8, entered and left | answer: the Soul committed taken up, or cleared, then on to HeatCross's own; a double of MegaMan set up while a Soul is his (HolyDrem's, Poltrgst's) on to the Soul's own entry, as BN5's `0x0800F01C` | once a form entered or left, a double made |

A Soul's look, abilities and passives (the track's S3-S5, `src/emu/soullook.c`, `soulpower.c`, `soulpassive.c` and `soulcode.c`; [MegaMan's forms and BN5's Souls in BN6's engine](../../rom/souls.md), A Soul's look, abilities and passives) add these, each answering only for MegaMan in the proxy form with a Soul (BN6's own way otherwise):

| Address | Routine | Kind | Runs |
| --- | --- | --- | --- |
| `0x0800FCA4` | the sprite selector's read by the form (`BN6_FORM_SPRITE_READ`) | answer: the Soul's sprite in the grown list 0 | as his sprite is set |
| `0x08010066`, `0x080BF518` | his palette's reads by the form (`BN6_FORM_PALETTE_READ`, `BN6_FORM_PALETTE_READ2`) | answer: palette 0, 2 in Chaos Unison | as his palette is set |
| `0x08014A82` | the crossing's phases by the form (`BN6_CROSSING_PHASES`) | answer: BN6's phases with BN5's phase 1, the Soul's image (`EMU_FREE` + `0x481080`) | each frame of a unison |
| `0x08014C8E` | the crossing's phase 2, its sound (`BN6_CROSSING_SOUND`) | answer: none (BN5's phase 1 played it); TomahawkSoul's landing: the camera shaken, the battle's flag 0x20 | once a unison |
| `0x080C1D40`, `0x080C1D88`, `0x080C1E66` | the image's sprite, its decoration and its end (`BN6_SOUL_IMAGE_SPRITE`, `_DECO`, `_UNDECO`) | answer: the Soul's Navi's battle sprite in list 2; no decoration of BN6's | once a unison |
| `0x0801E6C2`, `0x0801E666` | the emotion window's and the Custom screen's face by the form (`BN6_FACE_OF_FORM`, `BN6_FACE_OF_FORM2`) | answer: the Soul's face and its turns | as the face is drawn |
| `0x0801D814`, `0x0801CBFE` | whether the window counts, its digits (`BN6_FACE_COUNTER`, `BN6_FACE_DIGITS`) | answer: always for a Soul, in BN5's digits | as the face is drawn |
| `0x080EC954` | MegaMan's arm flash routine before its call of BN6's flash spawner (`BN6_ARM_FLASH_SPAWN`) | jump: a Soul's kind (BN5's 0x23, 0x33) to BN5's spawner, moved, on as the call returns: BN5's own flash object with BN5's kinds | once a flash |
| `0x080BA5A2` | a Cross's flash raised in BN6's own flash's set-up (`BN6_FLASH_HEIGHT`) | answer: a Soul's at its kind's height, as BN5's | once a flash of BN6's |
| `0x0800FF3E`, `0x08010884`, `0x080108A8`, `0x0801268C` | his loadout's, element's, weakness's and buster bonus's reads by the form (`BN6_FORM_LOADOUT_READ`, `_ELEMENT_READ`, `_WEAKNESS_READ`, `_BONUS_READ`) | answer: the Soul's row in BN6's ids, its element, no weakness, no bonus | as a form is set up, a shot fired |
| `EMU_FREE` + `0x481000`-`0x48100C` | the moved code's stubs: BN5's pending Soul, a spawn left out, the form by object and by side | answer: from `BN6_SOULS` | as the moved code calls them |
| the moved code's reads of BN5's AI data's Soul fields (`soulcode_bn5.c`'s fields) | in BN5's code moved | answer: from `BN6_SOULS`, where BN6's AI data holds its own | as the moved code reads them |
| `EMU_FREE` + `0x481010`, + `0x481014` | the proxy's passive entries 8 (`BN6_FORM_PASSIVES`, `BN6_FORM_PASSIVES_COLL`) | answer: the Soul's, BN5's routine moved, or none; HeatCross's without a Soul | as a form is set up, its state set again |
| `0x0802A494` | the Custom screen's hand size set (`BN6_HAND_SIZE_SET`) | answer: NumberSoul's ten | once a screen |
| `0x08013236` | the chip charge's test (`BN6_CHARGE_TEST`) | jump: a chip of the Soul's kind charges by BN5's rule (ShadowSoul's swords, KnightSoul's Break chips, ToadSoul's Aqua chips), else none; the answer one of the two `movs` and `bx lr` at `SOULS_THUMB` | as A is held with a chip |
| `0x08012C7C` | a charged chip's bonus (`BN6_CHARGE_BONUS`) | answer: KnightSoul's and ToadSoul's double (0xFF), none for the rest | once a chip used |
| `0x0800FBA4` | the release of a charged A, its power attack (`BN6_CHARGED_RELEASE`) | jump: a Soul's empty power attack to the chip used charged (`0x0800FBEC`) | once a release |
| `0x0801285E` | the chip's damage as its modifier leaves it (`BN6_CHIP_MODIFIED`) | jump: TomahawkSoul's Wood attack chip on grass doubled, to the Thumb that makes the panel plain and plays the modifier's sound | once a chip used |
| `0x080128E8` | the chip use's end (`BN6_CHIP_USED_END`) | answer: BN5's DarkChip use (dark for the battle), BN5's dark gate (a chip refused made the chip of nothing), KnightSoul's 50 frames unharmed from his front column | once a chip used |
| `0x080128EE` | the chip's family read (`BN6_CHIP_FAMILY_READ`) | jump: ShadowSoul's charged sword to BN5's sword family, moved | once a chip used |
| `0x08010340` | the shadow's type by side (`BN6_SHADOW_TYPE_SIDE`) | jump: none for ShadowSoul (BN5's `0x0800E0B6`) | as his shadow is set |

ColonelSoul's button, chip shot and obstacle strike (the track's S5, `src/emu/soulcolonel.c`; [MegaMan's forms and BN5's Souls in BN6's engine](../../rom/souls.md), ColonelSoul's button, chip shot and obstacle strike) add these, each answering only where MegaMan is united with ColonelSoul (BN6's own way, DustCross's, otherwise):

| Address | Routine | Kind | Runs |
| --- | --- | --- | --- |
| `0x08027F10` | DustCross's elements 8 and 9 as a Custom screen opens (`BN6_DUST_ELEMENTS`) | answer: BN5's ColonelSoul's set-up (`0x08023D70`), the button grey | once a screen |
| `0x08028F84` | DustCross's lit rule (`BN6_DUST_LIT`) | answer: BN5's (`0x08024C04`), its chip, slot and place kept | after each pick |
| `0x08027410` | mode 0x38's phases' table (`BN6_DUST_PHASES`) | jump: BN5's two phases, moved | A on the button |
| `0x08028342`, `0x08028782`, `0x0802878C`, `0x080287DE` | the button's tiles and the chip's window's picture and palettes (`BN6_DUST_ART`, `BN6_DUST_INFO_ART`, `BN6_DUST_INFO_PAL`, `BN6_DUST_NAME_PAL`) | jump: BN5's art, copied (`SOULS_COLONEL_ART`) | as each is drawn |
| `0x08026CFA`, `0x08027058` | mode 4's last draw, mode 0x24's end (`BN6_MODE4_LAST_DRAW`, `BN6_MODE24_END`) | jump: BN5's draw of the loaded chip, moved, then on | each frame the button is chosen |
| `0x08029032` | B on the chips (`BN6_CUSTOM_B`) | jump: the button's chip given back first (BN5's `0x08024CEA`), then BN6's end | B with the button the last thing done |
| `0x08029222`, `0x08029458` | OK's end, its picks spent (`BN6_OK_END`, `BN6_OK_SPENT`) | answer: the button's chip committed (the block's +0x14) and its deck entry spent | once OK |
| `0x0801486C` | the turn's form check (`BN6_TURN_FORMS`) | answer: his chip his (the block's +4) and his charged shot the chip shot, ScreenDivide without | as a turn begins |
| `0x0801B3CA`, `0x0801B506`, `0x0801B646`, `0x0801B786` | the obstacles' states at their collision flags' read (`BN6_OBSTACLE_FLAGS_A`-`_D`) | jump: BN5's block (`SOUL_COLONEL_STRIKE`): an obstacle the strike finds turns into BN5's soldier (`xstrike.c`, with the core where BN5's Navis are: either Colonel's side) | each frame of an obstacle while a side is armed |
| `EMU_FREE` + `0x481018`, + `0x48101C` | the moved code's stubs: his side's strike set, BN5's test of an obstacle (`SOUL_STUB_STRIKE`, `SOUL_STUB_STRIKE_CHECK`; `xstrike.c`) | answer: his side's bit in `BN6_COLONEL_SIDES`, where a Navi Colonel's spawn sets his; BN5's searches on BN6's panels | as he unites; as the strike's block runs |

BN5's mood in BN6's battles (the track's S7, `src/emu/soulmood.c`; [MegaMan's forms and BN5's Souls in BN6's engine](../../rom/souls.md), BN5's mood in BN6's battles) adds these where the Souls are installed, each keeping BN5's mood in the Souls' block beside BN6's own:

| Address | Routine | Kind | Runs |
| --- | --- | --- | --- |
| `0x0801A24C` | a hit's mood step, past BN6's own Full Synchro (`BN6_MOOD_HIT`) | answer: the attacker's mood up by the hit's gain (no counter), a counter's Full Synchro, the struck's down by its loss | once a hit |
| `0x08015BEC`, `0x080143A6` | BN6's mood set and calm (`BN6_MOOD_SET`, `BN6_MOOD_CALM`) | answer: BN5's twin writers' callers mirrored (the chip's Full Synchro and Anger, Anger's start and end, a Custom screen's script) | as BN6 writes its own |
| `0x08015C0E`, `0x080143B0` | their writes (`BN6_MOOD_SET_WRITE`, `BN6_MOOD_CALM_WRITE`) | jump: none while a BN5 DarkChip keeps him dark (past to `0x08015C10`, `0x080143B2`) | as BN6 writes its own |
| `0x0802945A` | the deck's compaction as a Custom screen opens (`BN6_COMPACT`) | answer: the battle's opening mood at its first screen; where the deck holds a BN5 DarkChip, BN5's worried rule (the first DarkChip into the deck's place at the hand's count, deck[5] on the first screen) and the run's shelf of BN5's DarkChips, the compaction ours | once a screen |

Chaos Unison (the track's S6, `src/emu/soulchaos.c`; [MegaMan's forms and BN5's Souls in BN6's engine](../../rom/souls.md), Chaos Unison) adds these where the Souls' own code is placed, each answering only for a side united in a Chaos Unison (`BN6_SOUL_CHAOS`) or while its release's request stands (`BN6_SOUL_REQUEST`):

| Address | Routine | Kind | Runs |
| --- | --- | --- | --- |
| `0x08012F62` | a charge's time by its button (`BN6_CHAOS_CHARGE`) | answer: a chaos B's, his chaos power's row of the charge times by the level (BN5's `0x08010654`) | as a charge starts |
| `0x08012E8A`, `0x08012E8E` | the charge's state's step and past it (`BN6_CHAOS_DUE`, `BN6_CHAOS_PHASE`) | answer: the phases' cycle of a full chaos B stepped (BN5's `0x080105F0`), `BN6_SOUL_CYCLE` | each frame of a charge |
| `0x080131D4` | the release's request (`BN6_CHAOS_RELEASE`) | answer: 0x8000 in phase 2, else 0x10000 (BN5's), the request the Souls' | once a release |
| `0x080F180E` | the player's requests read (`BN6_CHAOS_CONSUME`) | jump: the chaos attack (the level one up, the power attack by his AIData +0x11) or the failure (family 0xC7), Chaos Unison's Thumb | once a release |
| `0x080F1870` | the player's idle state past the chip request's test (`BN6_PLAYER_IDLE_FREE`, `darkinvs.c`, where BN5's ROM is read) | jump: BN5's chip AI for the player, through `FREE_CHIP_AI`, where his AI data's +0x48 bit 0x200 stands (BN5's rule; nothing of BN6's sets it) | a frame he idles |
| `0x080172F6` | the player's set-up past his AI data's lay-out (`BN6_PLAYER_INIT_AI`, `darkinvs.c`) | answer: side 0's player's AI data moved into `RAM_PLAYER_AI` (BN5's 0x140 bytes); a NetBattle's other MegaMen move into rooms of their own as the chip AI first takes them (issue #182) | once a battle |
| `0x0801AD10` | the player's frame's end (`BN6_PLAYER_FRAME_END`) | answer: BN5's time under the chip AI counted down (AIData +0x36, BN5's `0x0800DBD8`), at 0 his request 0x80000 | a frame he runs |
| `0x080144A2` | BN6's drain while AIData +0x36 is not 0 (`BN6_AI_DRAIN_TEST`) | jump: none while the chip AI has him (to `0x080144B2`) | a frame |
| `0x080ED12A` | the support family past the chip's routine (`BN6_SUPPORT_MARK`) | jump: DarkInvs's (subfamily 0x65) attack ended in this frame, BN5's (to `0x080ED130`) | a support chip used |
| `0x080EBA9C` | the attack's requests' answer to 0x80000 (`BN6_REACTION_START`) | jump: BN5's family 0x49 (BN6's 0x99) where the chip AI has him, on at the bl `0x080EBA9E` | DarkInvs's end |
| the Navis' copy of BN5's `0x080EBBFE` | its buster arm's read of his Chaos Unison (`BN5_ARM_CHAOS_READ`) | jump: the Souls' block's `BN6_SOUL_CHAOS` | a buster shot of the machine's |
| `XC_STUBS` + 0x24, + 0x28, + 0x2C | DarkInvs's calls BN6 has no twin of (`XC_STUB_DARKINVS`) | answer: the chip AI's state cleared, its entry (time, flag, list), his statuses cleared at BN6's places and BN5's calm of his mood | DarkInvs used |
| `0x08011744` | the attack's end, its requests cleared (`BN6_CHAOS_END`) | answer: BN5's 0x18000 cleared too; the failure's kind his chaos ended | once an attack ends |
| `0x080EBA84`, `0x080ED7B0`, `0x080ED90E`, `0x080EEE32` | BN6's tests of the counter requests 0x8600 (`BN6_COUNTER_TEST`, `_1D`, `_1E`, `_37`) | answer: 0x8000 left out while the Souls' request stands (BN5 tests 0x600) | each frame |
| `0x080E21E4`, `0x080E22A8` | the charge glow's and its sounds' side tests (`BN6_GLOW_SHOW`, `BN6_GLOW_SOUND`) | jump: in chaos, BN5's test by the side | each frame of a glow |
| `0x080E2272`, `0x080E2280`, `0x080E2208` | the glow's sprite past A's, its load, its animation (`BN6_GLOW_PICK`, `BN6_GLOW_LOAD`, `BN6_GLOW_ANIM`) | answer: BN5's dark glow (the Souls' copy in list 0), its animation the phase + 2 once full | as the glow is set and each frame |
| `EMU_FREE` + `0x4833EC` | the failure's stub (`CHAOS_OUT_STUB`, a `bx lr` in Chaos Unison's Thumb) | answer: a Dark Soul already out ended, his side's chip lists where the battle gave none | once a failure |

BN5's green flash (the chips' code, `src/emu/xchips_flash.c`; [The chip registry and BN5's chips' code](../../rom/chips-code.md), BN5's DarkPlus and its green flash) adds two hooks on the moved DarkPlus's stores of it (in its copy, under the chips' claims) and two of BN6's: the player's timers past the time stop's test (`0x080107DC`, `BN6_FLASH_COUNT`: the flash's frames one less) and an object's colours' chain before `0x080168C8` (`0x0801B15A`, `BN6_FLASH_TINT`: the colour shader by BN5's table while it lasts), each frame of a battle where BN5's chips' code is installed.

The duel's two are set as the duel begins and taken off as it ends, so
the other battles carry no hook a frame per object. The hooks replaced
reading the game every frame: the battle's objects for MegaMan's HP and
ProtoMan's, the battle state for its record, every 16 frames the
enemies' reward rows, the flags of the layer's choices, the key items
for a ScrtData or the Spin picked up, and the exit pad's warp-off flag,
written every frame because EnterMap clears the map's flags (`0x1640`-
`0x16FF`): now written as a map is entered and as the guardian's exit
opens.

What stays read a frame at a time, where a hook would only move the read:
whether MegaMan walks the map, battles or reads a menu (`on_map`, the
game mode, GAME OVER among them); the exit pad's warp under way and its
arrival; the frames on a map not the layer's before he is warped back;
the duel's DeleteTime, which its HUD shows as it runs.

Where a table written into the core's copy is simpler than a hook, the
table stays (issue #32): a layer's shop stock goes to the ROM's initial
shop table beside RAM, as the shop screen lists only entries found there
(one write per entry, made again with the layer after a CONTINUE); a
Chip Trader's map takes the first entry of the prize pools and of the
trader kinds (two writes), and its pool's records are rewritten in the
folder's codes either way; the BugFrag Trader's trade waits for its chat
to hold, a few reads while a chat is open, and runs as game calls.

`tests/test_emu.c` runs a ROM of the test's own bytes on mGBA (its
`GBAIsROM` wants `0xEA` at offset 3 and `0x96` at `0xB2`, nothing more):
routines called and their results stored, with a hook of every kind on
them, an ARM-state one, an event posted from an answer hook, the board's
`BKPT #1` passed on, the hooks taken off again, one in RAM written again
after a reset, and a halt from a hook. Then two cores side by side, run in
turn as BN6's and the guest's are (issue #61): BN6's made by `emu_init`
with a hook on it, the other as `src/emu/guest.c` makes one, from a copy
of the ROM that also plays a tone. BN6's hook runs in BN6's frames alone;
the guest's copy of the code stays unpatched, and a `BKPT #0xCE` written
into it goes to its own board, which skips it, never to BN6's hooks (the
hook table is BN6's core's: the guest has none); each one's RAM is its
own; the guest's tone reaches the ring BN6's silence fills, and a frame of
it let go (`emu_audio_drop_from`) does not; its state saves and loads with
BN6's RAM untouched. Then a block of Thumb code of the test's own, made as
if it stood in another game, is moved into BN6's copy with `emu_write`
(`src/emu/thumb_reloc.c`, issue #82): its call within itself left alone,
its two calls out aimed at stubs of the ROM's own 11 MiB away, beyond a
BL's 4 MiB, through 12-byte veneers (`push {r0,r1}; ldr r0,[pc,#4];
str r0,[sp,#4]; pop {r0,pc}` and the target). Each stub, ARM code, keeps
what it met: r0-r12, sp and the flags as the block set them before its
BL, and lr its way back into the block; and the copy's writes outlast the
core's reset that starts it. Then the second-level dispatch's code
(`xvirus_dispatch_code`) run on the core with a driver of the test's own
that stands in for BN6's spawner, its free and its runner: a moved type
spawned as its kind's dispatch type, its number in the side table by the
object's slot and its MOVED routine run with r1 the number times four; a
twin spawned as BN6's own number (its number in the side table too,
where nothing reads it); a type no family named let go as it first runs.
Then RAM kept patched (`emu_keep`): the patch made where the game's own
bytes are, a state saved with the game's own there (another core loading
it reads them), the patch made again as the state is loaded, and bytes
that read neither left alone. Last, the installers' claims through
`emu.c`'s writes and hooks (`src/emu/claims.c`): an installer's bytes its
own, another's write over them told, a write outside any installer
nobody's, and a hook set on an installer's bytes told. Then the
installs' record (`src/emu/xinstall_log.c`): its bytes in another core's
ROM copy (the ROM's alone, the mirror's at its byte, RAM's and those
outside the record left out), its hooks on that core's own table, each
reading the core it runs on, its hash the same once it ends. `build.py
test` runs it with the address and undefined-behaviour sanitizers, as CI
does.
