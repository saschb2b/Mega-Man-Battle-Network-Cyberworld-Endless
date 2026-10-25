# Liberation Missions

* [Liberation Missions](overview.md) - BN5's Liberation Missions dissected and made afresh from a seed as a dark layer: MegaMan and the run's Soul Navis liberate dark panels toward a Darkloid; the dissection, generator, prototype, stages built and decisions.
* [The decision](decision.md) - Why Liberation Missions became a dark layer from act 2 behind a route fork, not an act's climax or separate mode, and why BN5 Souls lend the team, reasoned with the game-design skill.
* [The generator](generator.md) - The Liberation generator (`tools/liberation.py`): BN5-measured bands per act, six steps from floor to traps, the guarantees and command rule that reject boards, and the par from a planner (`liberation_plan.c`) scaled by BN5's ratio.
* [The quality bar](quality-bar.md) - The Liberation quality bar's eight rules (`liberation_board.c`, `liberation_plan.c`): at most 55% dead phases, a bare board still a puzzle, one new rule per act, honest telegraphs, teammate niches, costed routes, graduated rewards, measured fun.
* [Special routes](special-routes.md) - Special routes: a sealed region's route found on a dark board is kept for the run (`run.route_found`, `run_route_turns`) and opens a third way at the next fork, or the Nest's second guardian.
* [The prototype's measures](prototype.md) - Measures of the ROM-free Liberation prototype (`tools/liberation.py`, `tools/liberation_bn5.py`) beside BN5's nine boards: boards kept and rejected per act, shape and dark bands, par, each command's worth, and the quality bar's numbers.
* [The owner's decisions](decisions.md) - The owner's eight Liberation decisions: dark layer first, co-op later, BN5 Souls only as teammates, acts 5-6 tuned after the team, BN5's par tiers exactly, a found route kept as a third way.

# Subdirectories

* [BN5's missions, dissected](bn5/) - BN5's nine Liberation Missions dissected from its ROM, play in romlab and guides: where a mission lives (sub-mode 0x18, `BN5_LIB_STATE`), each mission's map, Darkloid, par, team, panels and rewards, and how they rise.
* [Building it](building/) - The staged plan for Liberation Missions: the board in the engine, the three-turn battle in BN6's engine, the Soul team, the Darkloids, a separate mode with co-op; and each piece as built.
