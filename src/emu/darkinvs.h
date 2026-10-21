/* BN5's DarkInvs in BN6's engine (issue #82; docs/MULTIROM.md, The chips'
 * code, DarkInvs, moved; the owner's call of 13 October 2026: BN5's rule
 * wherever its ROM is read). BN5's DarkInvs puts MegaMan under its chip
 * AI for the player (BN5_CHIP_AI_PLAYER) for 600 frames, invisible, the
 * game driving him: BN5's idle states run the machine while his AI data's
 * flag BN5_AI_UNDER stands (0x080F04CC-0x080F04E2).
 *
 * Here: the machine, moved with the Dark Soul's chip AI
 * (tools/bn5_navi_twins.py), run from BN6's idle state where the flag
 * stands (BN6_PLAYER_IDLE_FREE, into a stub in FREE_CHIP_AI that calls it
 * and leaves the state as BN5's does), its state cleared on his other idle
 * frames; his AI data BN5's 0x140 bytes from his set-up (RAM_PLAYER_AI);
 * the calls of BN5's DarkInvs unit (tools/xchiprecipe.py) BN6 has no twin
 * of, at the chips' stubs (XC_STUB_DARKINVS); its attack ended in BN5's
 * frame; its time counted down at his frame's end and BN5's end family
 * started at 0; BN6's drain of AIData +0x36 held back meanwhile; the
 * Navis' buster arm's Chaos Unison from the Souls' block. Not ported: BN5's
 * five other exits from the chip AI (0x0802C12A's callers). `--dev chipai`
 * enters it in each battle without the chip. */
#ifndef CW_DARKINVS_H
#define CW_DARKINVS_H

#include <stdbool.h>
#include <stdint.h>

/* The machine's hooks and stub, and BN5's DarkInvs's (its unit's stubs),
 * where BN5's ROM is read and the Dark Soul is installed (it holds the
 * machine's code), with the Souls (soulchaos.c); false, none set, where
 * not. */
bool darkinvs_install(void);
/* --dev chipai: the machine entered in each battle, two seconds in, for
 * DarkInvs's 600 frames */
void darkinvs_dev(bool on);
/* --dev arena2v2 (arena_team.h): two MegaMen a side, the block of BN5's
 * size (RAM_PLAYER_AI) side 0's first's alone, the second keeping BN6's */
void darkinvs_two_a_side(bool on);
/* MegaMan's object `me` put under the chip AI for `frames` frames, as
 * BN5's DarkInvs does (its AI data moved first where it is BN6's 0x100);
 * from a hook (hook_read and hook_write). */
void darkinvs_enter(uint32_t me, uint16_t frames);
/* ... and out of it, as BN5's 0x0802C12A takes him out where a Soul's
 * unison begins or ends (0x0801214A, 0x08012276): its time and state
 * cleared, the flag dropped (soullife.c); where the chip AI has him. From
 * a hook. */
void darkinvs_leave(uint32_t me);

#endif
