---
type: Engine Mechanism
title: "Guest battles"
description: "History of BN5's battles fought on a second mGBA core (`src/emu/guest.c`, epic #57) before issue #82 moved every battle into BN6's engine; the guest core now serves only as the differential's reference."
tags: [multirom, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-12T21:51:29Z }
---

# Guest battles

What runs now: none. Every battle of a run is BN6's engine's, BN5's
areas' too ([One net](one-net.md)), and the guest core (`src/emu/guest.c`) is
the differential's alone: BN5's own battle as BN5 has it, booted once on
the main thread and kept as `xdiff-bn5.state` in the differential's data
folder (`build.py xdiff`, [The differential: a moved family against its own game](../devtools/differential.md)). A data
folder a run's guest kept its boot state in (`guest-bn5-1.state`) has it
removed once at the next start (`src/core/startup.c`). What it took a
run, gone with it: BN5's first boot (7210 of its frames: five seconds of
a Retroid Nova's, a thread of its own on native builds, the browser's
frames' spare time), the switch of engines and its flash, the older
net's wait screen, the chips that sat out of its battles for want of a
BN5 namesake and the codes it read its own way, the guest's ROM copy in
the browser's memory (below), and some 2,500 lines of run code.

The history (epic #57, issues #61-#69 and #102, until issue #82's last
round), for what it taught the engine's port:

- **When and which**: where BN5 dressed an area and the build could run a
  second core (every build but the 3DS's), BN6's roll decided a battle
  and a hook gave BN6 none; the director began a guest battle instead,
  BN6's core waiting while BN5's ran. Its record was one of the area's
  own maps' in BN5's numbers (ACDC Area 0x90:0, 0x90:1 and 0x86:0 for an
  act's first, middle and guardian's layers; Oran Area 0x91:0, 0x91:1,
  0x86:1; SciLab 0x92:0, 0x92:1, 0x86:2; End Area 0x93:0, 0x86:3, 0x93:1;
  the Undernet 0x94:0, 0x94:1, 0x86:5; Nebula Area 0x86:6, 0x94:3,
  0x86:7), held to the act's band from BN5's own enemy tables; BN5's
  story battles never. The same maps' records are a territory's now, in
  BN6's engine (`NetAreaDef.xbattles`).
- **MegaMan as the run had him**: his HP, the run's folder as BN5's chips
  of the same name (114 of BN6's 313 paired; a chip BN5 had none of sat
  out, a code its chip lacked fought as its * or first), his buster as
  the NaviCust made it, the run's DarkChips by BN5's worried rule
  (a shelf behind the deck's padding), the run's Souls as BN5's event
  flags so its own Custom screen offered UNITE. Its random numbers were
  the run's: both words written from the battle's seed as it began
  (issue #102; [BN5's battles, records and rewards](../rom/bn5-battles.md), BN5's random numbers), which the
  differential's reference still does.
- **Its results**: BN5's result, the HP the battle left and its results
  screen's rewards (the busting level's and a green Mystery Data's
  find) went into the run as BN6's chips of their names, zenny and
  BugFrags. BN5's own rows are BN6's now, through the chip registry
  ([One net](one-net.md)).
- **Its guardians and Souls** (issue #69): BN5's Navis guarded the acts
  its areas dress in half the runs, fought in BN5's engine at three
  quarters of BN6's guardians' band (no Cross could come in), never on
  the short net's last act or Nebula Area, whose Souls would have served
  no battle of BN5's after them; their Souls woke only in BN5's battles.
  Since the last round they fight in BN6's engine at BN6's band, and
  their Souls serve every battle ([BN5's Navis](../game/bosses/bn5-navis.md);
  [Souls](../game/meta/souls.md)).
- **Its boot and wait**: BN5's first boot ran in the background from the
  game's start (a thread of its own on native builds, measured 3.4 to 3.6
  s on an Intel Core Ultra X7 358H; the browser's frames' spare time),
  and a battle that came first waited behind the older net's wait, a
  screen designed through the game-design skill's interface layer: the
  switch's white, MegaMan's three lines in BN6's chat box, a gauge in the
  battle HUD's form filling with the boot.
- **In the browser** (issue #68): the page's WebAssembly memory was 97 MB
  on a first layer with BN6 alone, 153 MB on BN5's first layer with the
  guest booted (BN5's ROM, 8 MB, and the guest core's ROM copy, which
  mGBA copies to 32 MB at its first patch), 197 MB at most; without the
  second core a run's page holds BN6's core and BN5's ROM file alone
  (measured: [The browser's memory](../engine/emulation/browser-memory.md)).
