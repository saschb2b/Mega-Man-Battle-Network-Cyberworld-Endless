# The run: how it plays

* [The core loop, reviewed](core-loop.md) - The core loop reviewed in October 2026 with the game-design skill, a team of six analysts, a devil's advocate and the players' own statistics: what works, what it lacks, what has grown, the next evolution's phases, what it freezes and the owner's decisions.
* [Fidelity](fidelity.md) - Each element the player sees, from the title and town to battles, the PET's look and the second screen, marked original (from the ROM), generated (in the game's formats) or adapted (ours), with known gaps.
* [The NaviCust](navicust.md) - The NaviCust as a run's second build axis: fit it or force it, the board grown by ExpMemry at acts 2 and 4, guardian drafts from 34 programs, readable bugs, and what is built (`src/layer/navicust.c`).
* [The PET](pet.md) - The PET in a run (`src/director/pet.c`): BN6's screens, Save taking a checkpoint, Options in Comm's place, E-Mail as Dad's lab, KeyItem's profile items, the NetPass and the Jukebox, and the hooks that take them over.
* [The rival: Chaud and ProtoMan](rival.md) - The rival Chaud and ProtoMan: busting duels against ProtoMan's DeleteTime on each act's duel layer, terms by rung, a netbattle after two duels won, the record, and Chaud's clearance opening official gates.

# Subdirectories

* [Guardians and super bosses](bosses/) - Guardian Navis and super bosses: how a guardian is met, fought and left behind from approach to next area, staged after Hades with BN6's own scripts and `src/director/cinema.c`, plus Bass and the Cybeast.
* [Economy](economy/) - What a run hands out (chips, their codes, zenny, Mystery Data) measured act by act, and issue #154's opening plan in `src/core/economy.c` that makes act 1 scarcer, opens up deeper and is guarded against drift.
* [Liberation Missions](liberation/) - BN5's Liberation Missions dissected and made afresh from a seed as a dark layer: MegaMan and the run's Soul Navis liberate dark panels toward a Darkloid; the dissection, generator, prototype, stages built and decisions.
* [The meta layer](meta/) - The meta layer, what a run leaves for the next so the tenth starts differently: its design and decisions, the helpers, Spins, compression codes, rumors, DarkChips, Souls, BeastOut, what MegaMan knows and the title's marks.
* [Progression](progression/) - Progression, how a run's difficulty is decided (`src/core/pacing.c`): depth sets difficulty and the area only flavour, per-act HP budgets and damage caps, guardians by HP band, a fixed heal rhythm, and the research behind it.
