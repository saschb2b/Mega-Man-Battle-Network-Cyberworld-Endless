# The Arena

* [The Arena](overview.md) - The Arena folder: BN6's Practice NetBattle between players, each client running both linked cores with only keys crossing: its engine, smoothed delay, rollback, the way in, the stands, Team NetBattles, two a side.
* [Run it](run-it.md) - `--dev arena`: the Arena started in the title's place against a scripted second player over a local link, and its options (`arenadelay`, `arenalag`, `arenawindow`, `arenabusy`, `arenahp`, `arenaseed`, `arenaagain`, `arenaboth`) for tests and captures.
* [Measured](measured.md) - The Arena's first measurements on the Linux host, 18 October 2026: `--dev arena` battles at 50, 83 and 150 ms one way finishing with every hash agreed, stalls only where the delay fell short.
* [What runs](what-runs.md) - The Arena's parts file by file (`arena_pair.c`, `arena_boot.c`, `arena_input.c`, `arena_sync.c`, `arena.c` and more), the lockstep's frame timing and 128-frame cap, the pair hash every 60 frames, and the `ArenaLink` interface.
* [Smoothed delay](smoothed-delay.md) - Smoothed delay (issue #128's first step): a wait moves the game's clock, the leading game holds a tick (`arena_sync.c`), the server's `delay_for` fits the delay to both games' pings, measured through a relay.
* [The stands](stands.md) - The stands (issue #128, protocol v7): a NetBattle watched for real by rebuilding its pair from both players' keys (`arena_watch.c`, `stands.py`), the catch-up, the referee's report and cheers every game gets, and the three-game proof.
* [Team NetBattles](teams.md) - Team NetBattles (epic #167, protocol v12): relay teams of two, Team up and Split up on a card, the captains' blind order, the winner staying on with his HP, the wire (`teams.py`), the four-game proof.
* [Not yet](not-yet.md) - What the Arena has not yet done or tried: two a side's open points, teams of three, battle sound, NaviCust effects, rollback between devices, the stands on phones, Battle mode, the browser with a screen.

# Subdirectories

* [From the Net Square](from-square/) - Challenging a player from his card in the Net Square: Battle and Team up in BN6's chat boxes, offered where both NetPasses hold the Arena stamp, the question and answer, and the New 3DS declining.
* [Rollback](rollback/) - Rollback in every NetBattle (issue #128): the other player's missing keys guessed as his last held, the pair's snapshot kept, frames run again after a wrong guess, the input delay down to 2 frames.
* [Two MegaMen a side](two-a-side/) - Two MegaMen a side (epic #167, issue #181): four MegaMen in one BN6 battle via hooks at `BN6_SPAWN_ENTITIES` and `BN6_KEY_COPY`, `--dev arena2v2`, and the rule that both GBAs run the same battle iteration by iteration.
