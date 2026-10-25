---
type: Feature
title: "A partner's UNITE"
description: "A partner's UNITE with his own Souls (`arena_team_soul.c`, `soullife.c`): BN5's installs' hooks on his screen copy, `TEAM_SOUL` and `TEAM_SOUL_CHIP` in his stream, and his Soul's life, turns and powers read by object."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T15:28:38Z }
---

# A partner's UNITE

A partner unites with his own Souls on his own Custom screen, as his lead
does (22 October 2026, issue #181; `arena_team_soul.c`, `soullife.c`,
the Souls' modules by object):

1. **UNITE on his copy.** UNITE and ColonelSoul's button are hooks of
   BN5's installs on the Custom screen (`soulunite.c`, `soulcolonel.c`).
   His side's GBA of his copy now takes the recorded installs' hooks
   (`xlog_hook_core`) and his own Souls held (`soulunite_held_core`,
   the loadout's `souls`), without the pair's two-a-side hooks
   (`arena_pair_open_screen`); a copy whose partner holds no Soul stays
   plain, no hook on it. That is a hook core more on a partner's game,
   two where the dev flag stands in for both partners: `HOOK_CORES` is 5
   (BN6's, the pair's two, a copy's one or two). Each turn his own Souls
   block (`BN6_SOULS_PARTNERS`) is copied into his side's block there,
   as his record is into his side's NaviStats, so UNITE reads his mood,
   the Souls he has used and a Soul he is in.
2. **Crossing as his Cross does.** His OK on UNITE commits the proxy form
   (8) in his copy's record and the Soul in its side's block, as BN5's
   OK does a lead's (`BN6_SOUL_NEXT`, `_NEXT_CHAOS`, `_NEXT_CHIP`); his
   client says his Cross (the proxy form) and then the Soul
   (`TEAM_SOUL`: the Cross count's index 1, the Soul and its chaos; and
   `TEAM_SOUL_CHIP`, index 2 or 3, ColonelSoul's chip). Each GBA takes
   them from his side's packet into his own block, with the Soul's used
   bit and his turns (BN5's three and his own record's
   `BN6_NAVI_TURN_EXTRA`, one for a Chaos Unison), and answers his
   crossing's pending record with its chaos and turns
   (`arena_team_cross.c`'s `pending`). A Soul said for a turn that never
   crossed is dropped as his next turn opens.
3. **His Soul's life by object.** `soullife.c` takes a Soul up as a
   MegaMan enters the proxy form and lets it go as he leaves it by his
   own Souls state (`soul_field_of`): a side's player's in his side's
   block as before, a partner's in his own (his own mood calmed, BN5's
   chip AI left, a Colonel's side bit his lead's alone). BN6 counts turns
   and asks a form's end for the sides' players alone, so the partners'
   turn hooks call the Soul's countdown (`soul_count_of`, from the
   countdown's body run for him) and its end (`soul_ended_of`, BN6's
   request to leave the form in place of Beast Out's end check, as for a
   lead); a Custom screen opening marks the partners' blocks too
   (`BN6_SOUL_SCREEN`).
4. **His Soul's own powers.** Every read of a Soul where its MegaMan is
   at hand reads his own Souls state (`soul_id_of`, `soul_field_of`): the
   Chaos Unison's charge, phases, release, request, counter and glow
   (`soulchaos.c`), the chip powers (`soulchips.c`), the abilities and
   loadout (`soulpower.c`), the passives and the chip bonus
   (`soulpassive.c`, `soulproxy.c`), the look, palette, crossing's sound,
   face and arm flash (`soullook.c`), BN5's moved code's reads of its AI
   data's Soul fields and the form getter (`soulcode.c`), BN5's DarkChip
   ending a Soul (`soulmood.c`) and ColonelSoul's chip at the turn
   (`soulcolonel.c`). For a side's player each reads what it read
   before: the Souls' differential (`build.py xdiff`, the 72 specs of the
   Souls, their chips, BN5's mood and DarkChips, the Dark Soul and
   ColonelSoul) passes as before, and a NetBattle of two hashes as
   before.

`teamunite=1` ([Dev tools](../../../devtools/overview.md)) gives each stood-in partner's copy the
carried MegaMan's Souls and has his client take the chip under the
cursor and UNITE at his first turn; on a partner's own game in a
side-by-side match it gives the MegaMan he carries NumberSoul and Atk+10
`*` as his folder's first chip, a UNITE in his first hand. Proved, 22
October 2026, both ROMs, `--dev
arena,arenaboth,arena2v2=1,souls=0x04,teamunite=1,teamhp=500,arenadelay=1,quitend`
from the rules branch's checkpoint of thirty DarkPlus (each partner's
first chip DarkPlus: NumberSoul's Chaos Unison), `CYBERWORLD_EMU_DEBUG`:

| Run | End | Hashes | The two GBAs |
| --- | --- | --- | --- |
| `arenalag=4` (388 rollbacks) | over at frame 4846 | 80, all the same as `arenalag=0,arenawindow=8`'s | 3,600 iterations, 0 apart |
| `arenalag=4,arenaroll=8` | over at 4857 | all 80 the same as the delay alone's (`arenawindow=0`) | 3,600, 0 apart; 607 rollbacks of 8 frames, the game's state the same after all 607 |
| `teamunite=-1,arenalag=4` (his Soul left out on the second GBA, the mutation) | | | apart from iteration 1 |
| `teamunite=2,arenalag=4` (each partner's folder opening with Atk+10 `*`: NumberSoul, three turns; 671 rollbacks) | over at frame 7766 | 129, all the same as `arenalag=0,arenawindow=8`'s | 3,226 iterations, 0 apart |
| `teamunite=2,arenalag=4,arenaroll=8` | over at 7777 | all 129 the same as the delay alone's | 3,226, 0 apart; 972 rollbacks of 8 frames, the game's state the same after all 972 |
| `teamunite=-2,arenalag=4` (the mutation) | | | apart from iteration 1 |
| `teamunite=3,arenalag=4` (RockCube `*` first, ColonelSoul held: ColonelSoul, three turns; 552 rollbacks) | over at frame 6204, side 1 won | 103, all the same as `arenalag=0,arenawindow=8`'s | 2,117 iterations, 0 apart |
| `teamunite=3,arenalag=4,arenaroll=8` | over at 6354 | all 105 the same as the delay alone's | 2,263, 0 apart; 794 rollbacks of 8 frames, the game's state the same after all 794 |

On both GBAs: "side 0's partner united with Soul 9 (chaos 1, turns 1),
his own", his chaos B "released, out of phase 2: the failure", then "side
0's partner's Soul 9 ended, his own". With `teamunite=2` side 1's
partner's NumberSoul of three turns: "united with Soul 9 (chaos 0,
turns 3), his own" on both GBAs, its turns counted at each Custom screen
from the countdown's body run for him, and its end asked through the
turn check as a lead's ("a Soul ended 2 times", both GBAs), "Soul 9
ended, his own"; side 0's partner, his lead's folder of DarkPlus behind
the Atk+10, took his UNITE screen but never pressed OK at his next two
turns (his stood-in script's keys, not the Soul's), his client sending
his picks at the wait's limit. With `teamunite=3` side 1's partner's
ColonelSoul: "united with Soul 7 (chaos 0, turns 3), his own" on both
GBAs, his turn's charged shot set by ColonelSoul's turn hook for him (no
chip on his button: ScreenDivide). From his seat (`teamseat=2`, the
pictures kept out of the repository): his own screen of DarkPlus, his
OK, NumberSoul's image over him as he crosses, then his MegaMan in the
Chaos Unison's palette beside his lead, his chaos B's glow charging.
Over the network (the four games of [Over the
server](over-the-server.md), `teamunite=1` on Beta's game,
Delta's HeatCross beside it, Epsilon watching): Beta's UNITE of
NumberSoul on his own screen, his Soul said on every game ("said 2
times", both GBAs), his MegaMan in NumberSoul's look on the field; over
at frame 1846 on all four, 30 hashes agreed and the spectator's 30
checked, 960 iterations 0 apart on all five.

Not exercised yet: ColonelSoul's button on his copy (a chip taken onto
it at a later turn, its chip shot: built, the stood-in client's script
never presses it), and his Chaos Unison's failure's Dark Soul, which plays his side's lists, made from his lead's folder
(`arena_souls.c`): his own folder's would need lists past the sides'
two in `RAM_DARK_LISTS`.
