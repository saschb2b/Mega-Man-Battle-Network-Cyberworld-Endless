# The loop's log

Newest last. One entry per report: score, what the session met, what it
raised, misreads, and one change to the loop. The persona's own files are in
`.build/play/kai/` (not committed): `diary.md`, `notes-sN.md`,
`report-sN.md`, `history*.txt`.

## Satisfaction

| Session | Score | What it met |
| --- | --- | --- |
| 1 | 4 | A soft-lock in the first five minutes |
| 2 | 5 | Reached layer 2; the first Net Dealer |
| 3 | 6 | A build plan, a heal to find, a close death |
| 4 | 7 | Fights won by thinking, loot kept, saves trusted |
| 5 | 8 | A build that paid off against a guardian; map and L trusted |
| 6 | 7 | A four-session run lost to a random pair after the guardian |
| 7 | 8 | Act 1 as wanted; calls spent on fights, not walls |
| 8 | 9 | All three wishes answered; guardian beaten by shop-prep-boss |
| 9 | 7 | EraseMan: a wall never dented, a move nobody warned of |
| 10 | 8 | The prep loop works; AquaSwrd one-shot BlastMan (too good) |
| 11 | 7 | CircusMan a real fight; the dealer's answer (SumnBlk1) a dud |
| 12 | 8 | BlastMan beaten with the dealer's BblStar1; R presses lost |
| 13 | 8 | HeatMan "the fairest, tensest boss yet", lost at 111/700; the R buffer works |

From session 5 the score swings 7 to 9: each session reaches ground no one
had tested (act 2's guardians, answer chips against them, traders) and finds
its problems there. The loop was reacting.

## Lessons so far

- **Pin the build.** Replaying a session on a newer build diverges. Each
  session runs its own copy (`scripts/pin.sh`), and replays name it with
  `CYBERWORLD_PLAY_BIN`.
- **Verify before fixing.** The persona sees stills. "Eaten" moves were the
  camera following MegaMan; an "eaten" A was a Sword swung into a hole.
  Replays with `CYBERWORLD_STATE_POS=1` settled both in minutes. Telling the
  persona in the patch notes ("From the developers, ...") and in
  `persona.md`'s harness notes stops the same report coming back.
- **Check "vanilla" in the disassembly.** Lost L/R presses near a full gauge
  were BN6's, and still worth a buffer. The Chip Trader giving BlastMan B
  every time was BN6's own rule (prizes the Library has, three in four) on a
  run's near-empty Library.
- **Sweep a class, not an instance.** EraseMan (s9) and CircusMan (s11)
  each surprised the persona with no warning; the other eight guardians had
  none either, found only when watched one by one (s13, `guardian_watch.sh`).
  The dealer's answer chip failed three ways over three sessions (a dud
  needing a hole, a sword that couldn't reach, a one-shot); the sweep of
  every act's answers per element (`build.py pacing`, s13) found three
  more at once: navi chips sold in twos, act 1's Elec answers over the
  cap, layers with none.
- **Budget the persona.** Sessions ran 320+ calls against 240: boss fights
  one action per call. The prompt now asks for bursts of two or three moves.
- **Where the engine writes a game record, carry every field the original
  has.** The battle record always put MegaMan on column 2 row 2; the user,
  playing, found him in a hole: the originals start him beside a hole or
  poison. Swept the record's other bytes after (only byte 1 varies, and
  ours is one of the originals'). And a record rewritten in place can
  change under a battle the game has already rolled: write in turn.
- **Save the report verbatim** the moment it arrives
  (`scripts/save_report.py`); the notes and diary are the persona's, the
  report is the developers'.

## Session 13 (8/10, keep playing: yes, new run started at once)

Build b24aaa3. Confirmed: the L/R buffer (4 times, the fight "flowed for
the first time"), "Back for more?", "my pick for the job", the Chip Trader
in L, a steadier arrow, and both developer notes (no eaten moves after the
map; the A after BATTLE START fires the chip at the bottom left).

Raised: the shops looked like the bystanders still (the s12 fix picked
sprites 64, 65 and 87, which are the dealer's green navi on the map: it was
never looked at, only its faces differed), no HeatMan warning, the trader's
three BlastMan B, services in walkway mouths, L's words against the arrow,
A turning to a vendor behind, the vendor's full chat every time, rows
misread in battle. All but the misreads were fixed before the report came
(875c7da, fa81690, d3f3265, 81e468d) or right after (3b13355, 5ecf688, the
vendor's repeat line); the misreads got a harness aid (b59dcc7: the battle
state names MegaMan's panel). Swept ahead: every guardian's warning, every
act's dealer answers (4c6bad0). The user, playing, found MegaMan starting a
battle in a hole (0d7d36b).

Cost: 257 calls, 82 minutes. Loop change: **a visual fix is verified in a
picture of the result**, not by its code: draw the sprites
(`build.py shot --scene gallery --sheet @6:56:48:/src/.build/l6.png`) or
capture the layer, and look.

## Session 14 (restarted)

Launched on 872ad88, whose new bystander and vendor sprites (Roll, GutsMan,
Glyde: compressed, unlisted) drew as white dots on the map, one of them
ElecMan's. The in-game capture after the launch caught it; the session was
stopped ten minutes in, Kai's profile put back from `data0` and relaunched
on 62a748e (EvilNavi bystanders, GirlNavi vendor, looked at on a layer).
Loop change: look **before** pinning. bn6f's `npcSpritePtrs` names list 6.
