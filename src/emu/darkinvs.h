/* BN5's DarkInvs in BN6's engine (issue #82; docs/MULTIROM.md, The chips'
 * code, What remains; the owner's call of 13 October 2026: BN5's rule
 * wherever its ROM is read). BN5's DarkInvs puts MegaMan under its chip
 * AI for the player (BN5_CHIP_AI_PLAYER) for 600 frames, invisible, the
 * game driving him: BN5's idle states run the machine while his AI data's
 * flag BN5_UNDER_CHIP_AI stands (0x080F04CC-0x080F04E2).
 *
 * In place: the machine, moved with the Dark Soul's chip AI
 * (tools/bn5_navi_twins.py), run from BN6's idle state where it stands
 * (BN6_PLAYER_IDLE_FREE, a hook into a stub in FREE_CHIP_AI that calls it
 * and leaves the state as BN5's does); and its entry as BN5's DarkInvs
 * makes it (0x0802C03E, 0x0802C114): his AI data moved into a block of
 * BN5's 0x140 bytes (RAM_PLAYER_AI), its chip AI's state cleared, the
 * flag and the timer set, his own side's list made from his folder (as a
 * failed Chaos Unison's Dark Soul's). Not yet: BN5's DarkInvs unit (its
 * support 0x3B, the object that enters it), its timer's end and the
 * engine's exits from the chip AI, his look; until then only `--dev
 * chipai` enters it (two seconds into each battle, 600 frames). */
#ifndef CW_DARKINVS_H
#define CW_DARKINVS_H

#include <stdbool.h>
#include <stdint.h>

/* The machine's hook and stub, where BN5's ROM is read and the Dark Soul
 * is installed (it holds the machine's code); false, none set, where not.
 * `dev` enters the machine in each battle (--dev chipai). */
bool darkinvs_install(bool dev);
/* MegaMan's object `me` put under the chip AI for `frames` frames, as
 * BN5's DarkInvs does (its AI data moved first where it is BN6's 0x100);
 * from a hook (hook_read and hook_write). */
void darkinvs_enter(uint32_t me, uint16_t frames);

#endif
