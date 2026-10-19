# Voice sweep: the story and its people

Group: the story and its people (`src/layer/guardian_lines.c`,
`src/layer/super_lines.c`, `src/director/duel_words.c`,
`src/director/story_words.c`, `src/scenes/title_lines.c`,
`src/director/pet_text.c`). Swept 19 October 2026 by the rules of
[docs/VOICE.md](../../VOICE.md), What nobody says, after the owner's
direction of that day: a line that does not need to be said is cut, not
rewritten. Ids are the catalog's as they were before the sweep
(`docs/voice/catalog.json` at 93568943); a cut inside a function moves the
ids after it (`guardian_intro:3` is now `:2`, `guardian_soul_words:3`-`:7`
now `:2`-`:6`, `super_intro:1` now `:0`).

| File | Read | Kept | Cut | Noted |
| --- | --- | --- | --- | --- |
| guardian_lines.c | 205 | 175 | 3 (1 in part) | 27 |
| super_lines.c | 34 | 29 | 3 | 2 |
| duel_words.c | 37 | 36 | 1 (in part) | 0 |
| story_words.c | 43 | 43 | 0 | 0 |
| title_lines.c | 7 | 7 | 0 | 0 |
| pet_text.c | 53 | 50 | 1 (in part) | 2 |
| **All** | **379** | **340** | **8** | **31** |

Kept, in short: the guardians' and Bass's meetings, rematches and last
words (personality), the Cybeast's roars, every arrival and call of the
story (Dad, Chaud, MegaMan sensing), Chaud's calls and verdicts (he gives
the clearance, so he says what it opens: rule 4), the net's rumors of the
guardians (rule 7), the BBS (strategy where BN6 keeps it), the title's
summary, the PET's screens. No LAYER_MAKE change: no table the layer's
rolls pick from lost an entry.

## Cut

| Id | The line as it was | Why |
| --- | --- | --- |
| `guardian_lines:guardian_intro:2` | `\|@M Lan,remember... {when}` (the tip's last box, "strike while his lightning falls!", before every known guardian's battle call) | Rule 5: MegaMan coaching at the fight. The tip stays in Dad's mail, the briefing and the second screen |
| `super_lines:super_intro:0` | `\|@M Lan,remember... {when}` (Bass's and the Cybeast's) | Rule 5, as the guardians' |
| `guardian_lines:guardian_netbattle_terms:0` (in part) | `@M {guardian_tip(11)}`: ProtoMan's whole tip, five boxes, recited by MegaMan between ProtoMan's challenge and "Take the duel?" | Rule 5. His tells are BN6's own ProtoMan's (rule 3), and Dad's mail holds them once his copy is fought. The rest of the line is noted below |
| `guardian_lines:guardian_soul_words:2` | `\|@M His Soul,Lan! It's with us every battle.` (every Soul after the first) | Rule 4: a power is taught once (the first Soul's words still do); the narration "MegaMan got: KnightMan's Soul!!" before it already says it |
| `super_lines:super_reward_words:4` | `\|@M And its battle data!\|@M Next time,we'll know how it fights!` | Rule 5: explains the run's battle-data system; the briefing simply knows the Cybeast next time. The guardians' same line was cut earlier (guardian_objs.c) |
| `super_lines:super_reward_words:5` | `\|@M And his battle data,Lan!\|@M Next time,we'll know how he fights!` | As above, for Bass |
| `duel_words:verdict_words:1` (in part) | `@C Tag two chips with SELECT in your Folder's EDIT.\|@C They'll come to your hand together.\|@C As long as they total under 60 MB.` | Rules 2 and 3: BN6's own TagChip system (its key item, its EDIT entry) and a button. Chaud still names it: "And a NetBattler's trick. The TagChip system." |
| `pet_text:codes_text:0` (in part) | `In the NaviCust,hold RIGHT on a program. Then press its code.` (the Compression mail's second page) | Rules 2 and 3: BN6's own compression codes and a button; the mail comes only after the player has entered one |

## Noted (kept now; for a later rewrite or move)

| Id | Line | What the player needs from it | Where BN6 would put it |
| --- | --- | --- | --- |
| `guardian_lines:guardian_tip:case1.0`-`case29.0` (23) | MegaMan's battle data, four or five boxes a guardian: "His fire tower gives no yellow warning!\|It turns into our row. Sidestep it late!\|..." | The tells playtesters were deleted by unwarned (EraseMan's erase at low HP, CircusMan's tent, ProtoMan's lone panel); shown in the briefing, Dad's mail and the second screen | Strategy is the BBS's, in its posters' voices (rule 4), or a guardian's mail as the lab's data, not MegaMan coaching (rule 5). Two boxes coach outright: "Heal up before we face him!" (EraseMan), "Keep a Recover chip ready!" (DustMan) |
| `super_lines:super_tip:0`, `:1` | The same for Bass and the Cybeast ("His cape stops our shots cold!\|...") | As above | As above; they have no mail (Dad's are for BN6's guardians 1-18), so the BBS |
| `guardian_lines:guardian_netbattle_terms:0` (the rest) | `@M If he deletes us,our dive's over!\|@L We can still run if it goes bad.` | That the netbattle is a real fight (a playtester took it at 100 HP) and that running is allowed (a loss, not deleted) | The stake as feeling, not as the run's end said aloud (rule 1: "our dive's over" is game over in the world's words): "He's not holding back,Lan!". `layer_words.c`'s `duel_terms` says the same for the races ("If they delete us,the dive's over.") |
| `guardian_lines:guardian_soul_words:4`, `:5`, `:6` | `\|@M Pick {a Fire} chip last,like {HeatSht}.` / `\|@M Pick {a Fire} chip last.` / `\|@M Then UNITE! Once a battle.` | How BN5's Soul Unison works in BN6's Custom Screen, the run's own; said once, at the first Soul | Rule 4: a system by mail from whoever provides it (as BN6's NaviCust Support Center mails), or a power by whoever gave it; not MegaMan explaining the screen (rule 5) |
| `guardian_lines:guardian_soul_words:7` | `\|@M A Soul or a Cross,though. Not both!` | The run's rule that a Soul and a Cross are not worn together, said once a profile | As above: the Soul mail; the second screen's card says it too |
| `pet_text:cross_words:0`, `:1` | Dad's report: " and HeatCross.\fIn it,fire chips +50,Buster +1.\fAqua attacks do double to it,and break it." | What the brought Cross gives and fears (a playtester chose by weakness alone; three met EraseCross's erase unexplained) | Rule 3: BN6's own Cross rules, taught in BN6's own Cross tutorials. A menu outside the fiction: the setup screen where the Cross is chosen, in its own plain style |
