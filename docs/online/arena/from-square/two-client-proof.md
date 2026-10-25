---
type: Proof
title: "The two-client proof"
description: "The two-client proof of 18 October 2026: two host builds fight a NetBattle from the Square through a local server, stalls, hashes, a desync and a reset caught, a mixed pair before v9, a rematch."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-20T16:36:26Z }
---

# The two-client proof

Two host binaries on this machine (`SDL_VIDEODRIVER=dummy
SDL_AUDIODRIVER=dummy`), each with its own `--data-dir` and a
`network.ini` naming a local server (`python3 server/build.py`, then
`build/server/cyberworld-server.pyz --config` with a free port and its
`state_dir` under `.build/`), BN6 alone in `--rom-dir` (before v9; since
v9 BN5's ROM beside it, or no Battle is offered), `--scene home
--run-depth 7 --seed 3 --dev quiet,arenahp=40` (`arenahp=N` makes each
carried MegaMan's max HP N, so the battle is short). Beta starts first
(session 1, arriving at -24,364), Alpha a second later; after the gate's
steps Alpha is put beside Beta (`place -8 364 5`), reads his card (A
three times) and picks Battle (A); Beta, 635 frames on, presses A, LEFT
(to Yes: the question's options lie side by side) and A. Both then press
RIGHT five times and A six times over (the Custom screen's OK) and fire
charged shots (B held 110 frames). The harness stays in `.build/`.

18 October 2026, the player's Gregar ROM, delay 6 (the server's least):

| Run | Way in | Ready | Battle | Stalls (side 0 / 1) | Hashes | Result |
| --- | --- | --- | --- | --- | --- | --- |
| Two keys a message | 1,817 frames on both | the same hash `fd2fa7d630cf9080` on both | over at frame 1,271 | 636 / 0 | 21 agreed | 2 and 1 (Beta won) |
| A key a message | the same | the same | over at frame 1,270 | 2 / 0 | 21 | 2 and 1 |
| The same, three pairs of games at once | the same | the same | over at frame 1,070 | 0 / 131 | 17 | 2 and 1 |
| `arenadesync=300` on Beta (Alpha's keys from frame 300 changed there) | the same | the same | stopped on both: apart by frame 359 (299 agreed) | | 5 | the stop's note, then MegaMan's word |
| Alpha's reset chord mid-battle | the same | the same | Beta stopped at frame 309: the other player left | | | Alpha at the gate ("We're home,Lan!"), Beta in the Square ("Huh? They're gone...") |

Both games printed the same hashes at every 60th frame. The two
processes' wall clocks put their battles' start within 20 ms and their
ends within 40 ms of each other (15 s of battle). Back in the Square,
both MegaMen stood where they had stood, the loser's word and the
winner's in their chat boxes.

**A mixed pair, BN6 rules, before v9** (the same day, protocol v4).
Beta with BN5's ROM beside BN6's, Alpha with BN6's alone, each with
Folder1 copied into Folder2 (the `folder 2` dev step) and Beta's Folder1
given MrkCan1 M: both named BN6 rules, Beta's Folder1 stayed home and
Folder2 went, and the battle was over at frame 1,568 with 26 hashes
agreeing. v9 offers no NetBattle to a game without BN5's ROM.

**A rematch** (19 October 2026, issue #128, protocol v5 unchanged). Two
BN6 games, `arenahp=10`, the harness above: Alpha challenged, Beta took
it, and the battle was over at frame 848 (Alpha won, 14 hashes
agreeing). Back in the Square each referee called it ("YOU WIN!" on
Alpha's game, "ALPHA WINS!" on Beta's) and each MegaMan asked "Again
with NAME?", the cursor on No. Beta's LEFT and A said Yes and sent the
challenge; Alpha's question was still open, so his game held it
(`arena: Beta asks for a rematch (match 2)`) until his RIGHT and A said
Yes, and took it at once. The second battle's pair hash at the Custom
screen was the first's (`8dc89c5dad9af3e7`: the same rules and folders),
and it was over at frame 1,328 (Beta won, 22 hashes agreeing; all 36 the
same on both games). The referees then called it 1 to 1 on both. Declined:
the same first battle, then Beta's B on the question (No); Alpha's Yes
came after it, and Beta's game turned his challenge down at once (`arena:
Alpha's rematch turned down`), Alpha's MegaMan saying "Aw... No NetBattle
this time.".
