---
name: playtest-loop
description: Iterate on Cyberworld Endless with a persona playtester (an agent playing the Linux build headless through tools/play.py) until the player is genuinely satisfied - pin a build, launch a session with patch notes, triage its report (already fixed, misread, vanilla BN6, real bug), fix and commit, and improve this loop itself after every report. Use when asked to playtest the game, run another playtest session, act on a playtester's report, or continue the playtest loop.
---

# The playtest loop

A persona (Kai: a BN6 fan who loves roguelikes, `persona.md`) plays the game
through `tools/play.py`, one batch of input at a time, reading a picture after
each, and reports like a player. You fix what the reports raise, in short
commits, and send the persona back in with patch notes, until it is genuinely
satisfied. Each iteration also improves the loop: `lessons.md` is its log.

Read `lessons.md` first: the satisfaction trend, what the last sessions
raised, and the loop's own lessons.

## One iteration

1. **Build and pin.** `python3 build.py linux`, then
   `.claude/skills/playtest-loop/scripts/pin.sh kai`: the session runs this
   copy (`.build/play/kai/bin/`, kept by `bin.pin`), so rebuilding while it
   plays changes nothing for it, and its replays stay exact.
2. **Patch notes** (player-facing, in the prompt): what changed since the
   last pinned build (`git log` from it), a one-time note when a change made
   CONTINUE restart the layer (`LAYER_MAKE` or `RUN_MAGIC` bumped), and
   "From the developers, after replaying your session: ..." for each report
   that turned out to be a misread. Teaching the player cuts false reports.
3. **Launch** the persona as a background agent (Agent tool,
   `run_in_background: true`) with `persona.md` filled in: the session
   number, last session in two sentences, patch notes, goals. About 260
   `do` calls, 1 to 1.5 hours.
4. **While it plays**, don't wait idle:
   - Read `.build/play/kai/notes-sN.md` now and then; start on its issues
     before the report comes (it runs a pinned build, so commit freely).
   - Sweep ahead: test what the persona will reach next (the next act's
     guardian, shops, traders) with the recipes below. New ground is where
     each session's new problems came from.
5. **Save the report** verbatim to `.build/play/kai/report-sN.md`
   (`scripts/save_report.py AGENT_OUTPUT .build/play/kai/report-sN.md` reads
   it from the agent's output file).
6. **Triage** every problem into one of:
   - *Already fixed* after the pinned build: say so in the next notes.
   - *Misread*: verify by replay (below) before touching code. Seen so far:
     moves "eaten" after the map (the camera follows MegaMan, the floor looks
     alike), an A "eaten" after BATTLE START (it fired the queued Sword into
     a hole), rows misread (BN6 draws Piranhas and chip icons a row up).
   - *Vanilla BN6*: confirm in the disassembly (`~/.cache/mmbn-ref/bn6f`,
     Falzar; Gregar addresses are shifted, find the same bytes) or with
     `tools/romlab`. Change it only where the roguelike breaks it: the Chip
     Trader's "chips the Library has" rule gave one chip every time.
   - *Real bug or design gap*: fix it.
7. **Fix, verify, commit** per AGENTS.md (its check table; `build.py test`;
   a capture of the screen; `LAYER_MAKE` / `RUN_MAGIC` bumps; ROM_DATA.md
   for a new offset; CHANGELOG). A visual fix is verified by looking at
   a picture of the result, never by its code alone (sprite indexes that
   differed in the code drew one navi on the map for two sessions). One commit per change: the subject states
   the new behaviour ("A Chip Trader's prize is a chip new to the Library"),
   the body why and the evidence. No AI co-author trailer. Never commit
   anything from `.build/`.
8. **Improve the loop** (below), then go to 1.

Stop when a report gives 9 or more, the persona wants to keep playing for
reasons of its own (a run it cares about, a build to try), would recommend
it without a major caveat, and two sessions in a row raise nothing major.
Then refresh `docs/screenshots` and `docs/clips` if what they show changed.

## Improve the loop after every report

Append to `lessons.md`: the session, its satisfaction, the fixes it
confirmed, its top problems, the misreads, what the iteration cost, and one
change to the loop. Then act on the pattern, not the instance:

- A misread seen twice goes into `persona.md`'s harness notes, so the next
  persona avoids it.
- A procedure done by hand twice becomes a script in `scripts/`.
- A class of problem found late (a guardian with no warning, a shop line
  that lies) becomes a sweep in step 4 for everything of that class.
- Satisfaction stuck for three sessions: find what each session met for the
  first time; the loop is reacting, so sweep further ahead.
- A step that cost more than it gave: change or cut it here.

## Recipes

**Replay a report's moment** (on the session's own build, or it diverges):

```sh
CYBERWORLD_STATE_POS=1 CYBERWORLD_PLAY_BIN=$PWD/.build/play/kai/bin/cyberworld \
  python3 tools/play.py replay kai rp --until FRAME
CYBERWORLD_PLAY_BIN=$PWD/.build/play/kai/bin/cyberworld \
  python3 tools/play.py do rp "hold DOWN 30; wait 4" --every 5
python3 tools/play.py stop rp
```

`pos X Y Z` in the state is MegaMan's world position: compare before and
after to settle "my input was eaten". To watch memory the state doesn't
show, print it next to `pos` in `director.c` for the replay only (a
throwaway line; the RNG seed is `0x020013F0`), and remove it before
committing.

**Watch a guardian fight** (for its warning line, or its balance):
`scripts/guardian_scan.sh NAVI BIOME` finds a seed whose layer-3 guardian in
that area is NAVI; `scripts/guardian_entry.sh NAVI BIOME SEED` walks the
autopilot to the arena and prints where MegaMan steps in;
`scripts/guardian_watch.sh NAVI BIOME SEED X Y DIR` places MegaMan short of
it, walks in, fights in god mode and saves four sheets;
`scripts/montage.py NAVI` packs them into one picture. The pools are in
`src/core/run.c` (biome indexes by their order there).

**Every act's answer chips**: `build.py pacing` ends with the Net Dealers'
answers per act and element (`+` over the cap). Read it after any change
to the chip pools, the bands or the dealer.

**Talk and shop screens**: `play.py start NAME --fresh -- --scene emu
--run-depth D --talk shop:430` opens a layer's chat on frame 430 (`npc`,
`shop`, `heal`, `programs`, `gift`, `challenge`, `undernet`, `gate`).

## Pitfalls

- The autopilot presses START every 480 frames: it pauses battles. Watch
  fights without it (`guardian_watch.sh`).
- After a battle starts, wait about 200 frames before START and A on the
  Custom screen, or both are lost to its slide-in.
- `play.py` sessions share nothing but the ROM: run several in parallel
  under different names (`scan*`, `w*`), and stop them after.
- The persona's frame numbers are from its own history file; its "f~"
  guesses can be off by hundreds. Use its frame index or `history.txt`.
- A history rewrite of shared commits needs the user: don't force it.
