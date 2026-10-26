---
type: Process
title: "Run it"
description: "`--dev arena`: the Arena started in the title's place against a scripted second player over a local link, and its options (`arenadelay`, `arenalag`, `arenawindow`, `arenabusy`, `arenahp`, `arenaseed`, `arenaagain`, `arenaboth`) for tests and captures."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-21T22:25:49Z }
---

# Run it

```sh
python3 build.py run -- --dev arena                       # a window: you are the first GBA
python3 build.py shot --dev arena --frames 2400 --input "80:,4:RIGHT,8:,4:RIGHT,8:,4:RIGHT,8:,4:RIGHT,8:,4:RIGHT,8:,4:A,200:,110:B,30:,..." --shot 600:/src/.build/arena.bmp
```

`--dev arena` starts on the Arena instead of the title, by full rules as
every NetBattle is: BN5's installs made on the game's core first, as a
run's start makes them, so it needs BN5's ROM beside BN6's (without it
the way in stops, saying so). The run's MegaMan is carried from the data
folder's checkpoint with the first folder that holds chips. The player
is the cable's first GBA from the keyboard or pad; the second is a
script: on the Custom screen it takes the cursor to OK and presses it, in
the battle it stands still. Its keys come over the local link, held back as a
network would hold them. `arenadelay=N` sets the input delay in frames
(default 6, 1-60), `arenalag=N` the local link's one way in ticks
(default 3, 50 ms), `arenawindow=W` the rollback window (0: the delay
alone; without it, what a frame costs here affords: [Rollback](rollback/overview.md)),
`arenabusy` makes the second player step, shoot and OK the Custom screen
by the frame's number alone (a run that rolls back and one that does not
hand him the same keys), `arenahp=N` makes the carried MegaMan's HP N
(the installer seen in the battle), and `arenacarry=N` sends him in with
N HP of his max, as a Team NetBattle's winner goes on ([Team
NetBattles](teams.md)). `arenaseed=N` gives the bout the seed N
and `arenaagain=N` fights N bouts more, each from the pair kept as the
last began, its seed one more ([A rematch in a
second](from-square/rematch-in-a-second.md)). `arenaboth` makes the first player
a script too (the Custom screen's OK, then charged shots), so the battle
plays itself on a device; `quitend` ends the game after it (and after
`--dev rollbench`, which `--dev rollbench,arena` runs first), for a run
sent over 3dslink. The scene shows the Square's screens (the way in's
panel, the name plate and the link's badge, a stall's note, the result
card: [The way in, the battle and the result](from-square/way-in.md)),
the players named "You" and "Beta"; the card's A, or
A+B+SELECT+START, goes back to the title; the log says each step
(`arena:` lines). The badge's three states are seen with `arenabusy`
and `arenadelay=1,arenalag=5` (rolling back) and with
`arenadelay=1,arenalag=110,arenawindow=0` (a stall at the battle's start,
the note after a second); `arenalag=200` overflows the local link's
queue, a stop's card.
