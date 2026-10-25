# Online

* [Online: the Net Square](overview.md) - The online folder (epic #120): the Net Square on a server, reached from a gate in Lan's HP, giving variety never power, with the NetPass, Trade Central, the week's net, the Arena and the wire.
* [The link cable and the Arena](link.md) - The link cable research behind the Arena: two cores per client on mGBA's `GBASIOLockstep` with only inputs crossing, why raw serial over a network stalls, the measurements, the pair's snapshot, and `build.py online-pair`.
* [Where things are](where-things-are.md) - Where online play lives: the game's side in `src/online/` with the gate in `director_gate.c`, the server in `server/`, the wire in `shared/online/`, each part's builds and tests; the server imports no game code.
* [The server and the wire](server-and-wire.md) - Protocol v15, binary over a WebSocket: what each version since v7 added, what the server refuses and bounds (NOTICE 28 without the full set), and the official server's moderation and how long it keeps what.
* [Not yet](not-yet.md) - What online play has not yet done or tried: emotes, the strip and palette, a dropped link, the week's net, the NetPass, Trade Central's screens, the Arena on the New 3DS, rollback, the stands, hardware.
* [Two games at once](two-games.md) - Running two games in one Net Square for a test: a local server (`build.py server-run`), a `network.ini` or `--dev online=URL`, two `tools/play.py` sessions, and `tools/online_browser.html` for a browser beside a native game.
* [The week's net](weekly-net.md) - The week's net (issue #127): each server's weekly seed (`week.py`), a dive in a save slot of its own from a standard profile (`src/core/weekly.c`), the greeter's offer, WEEK_POST and the Board's reported depths.
* [What has been shown](shown.md) - The dated record of what online play has been shown doing: visits, trades, the trade board, NetBattles across networks and systems, full rules and Souls, rematches, the stands, the NetPass, dropped links, emotes, crowds.
* [The Arena, in brief](arena-in-brief.md) - The Arena, the stands and Team NetBattles in brief: a challenge, BN6's Practice NetBattle on two linked cores per game by full rules, rematches, the referee, watching for real, relay teams and side by side.
* [Trade Central](trade-central.md) - Trade Central (`trade_central.c`): one spare chip for another player's, kept in `trade.pending` and settled at home, one trade a visit, the spares list and chip trade screens (`trade_screen.c`, issue #145), and the trade board.
* [The NetPass](netpass.md) - The NetPass (issue #131, `src/online/netpass.c`): the full set of ROMs, BN6's and BN5's, that opens the Arena, trades and the DeepNet dive, its four stamps, its screen, the stamp moment and protocol v9's NOTICE 28.

# Subdirectories

* [The Arena](arena/) - The Arena folder: BN6's Practice NetBattle between players, each client running both linked cores with only keys crossing: its engine, smoothed delay, rollback, the way in, the stands, Team NetBattles, two a side.
* [A visit](square/) - A visit to the Net Square: the gate in Lan's HP and its first questions (`director_gate.c`, `online_field.c`), the checkpoint at the gate, the Square built on the game's own core (`online_visit.c`), how visits end.
