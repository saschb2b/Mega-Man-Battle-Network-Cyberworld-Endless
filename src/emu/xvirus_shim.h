/* A moved family's shims (issue #82's A2b; docs/rom/virus-families.md, The families'
 * shims and pre-steps): what its game's code asks of the engine that a
 * changed immediate cannot give BN6's way, made as a few instructions of
 * the project's own Thumb in its home (XV_HOME_SHIMS), each from the
 * recipe's numbers:
 *
 *  - a NameID range (XvRange): its call to look an enemy up by a range of
 *    NameIDs made a call to a shim that loads BN6's bounds (past an
 *    immediate's 255) into r1 and r2 and goes on to BN6's routine;
 *  - a chip compare (XvCmp): a compare of a held chip's id with a number
 *    and the beq or bne after it made a call to a shim that compares with
 *    BN6's id (the chip registry's, past 255) and goes on where the branch
 *    would, the flags the compare's;
 *  - a pre-step (XVirusFamily.prestep): BN5's virus dispatcher acts for two
 *    families' NameIDs before their state each frame, as BN6's does for
 *    its own ids alone. A hook on BN6's dispatcher (BN6_VIRUS_NAMEID_CASES)
 *    jumps to the family's pre-step by its AI index (XV_PRESTEP), which runs
 *    BN5's case and the routine it calls, their collision data's offsets
 *    BN6's, then goes on to BN6's state step (BN6_VIRUS_STATE_STEP).
 *
 * Pure functions on byte buffers: the installer (xvirus.c) stages what they
 * make where they say. */
#ifndef CW_XVIRUS_SHIM_H
#define CW_XVIRUS_SHIM_H

#include <stdbool.h>
#include <stdint.h>

#define XV_SHIM_RANGE_SIZE   16   /* a NameID range's shim */
#define XV_SHIM_CMP_SIZE     28   /* a chip compare's */
#define XV_SHIM_PRESTEP_MAX  0x70 /* a pre-step's, at most */

/* The shim at `at` (word-aligned) that loads lo and hi into r1 and r2 and
 * branches on to `veneer` (BN6_BY_NAME_RANGE's, within a branch's reach):
 * false where it is not. */
bool xvirus_shim_range(uint8_t out[XV_SHIM_RANGE_SIZE], uint32_t at, uint16_t lo, uint16_t hi, uint32_t veneer);

/* The shim at `at` (word-aligned) a call replaces `cmp reg,#imm; beq to`
 * (eq) or `...; bne to` with: it compares register `reg` (r0-r7) with
 * `now`, returns where the branch is not taken and goes on to `to` (Thumb,
 * its bit 0 clear) where it is, every register but lr and the flags as the
 * two instructions would leave them. */
bool xvirus_shim_cmp(uint8_t out[XV_SHIM_CMP_SIZE], uint32_t at, int reg, uint16_t now, bool eq, uint32_t to);

/* The pre-step of BN5's dispatcher's case `which` (BN5_PRESTEP_APPLEY,
 * BN5_PRESTEP_DOMINERD) at `at` (word-aligned), its calls through the
 * veneers to BN6's object_clearFlag2 and PlaySoundEffect: its size, 0
 * where `which` is no case ported or it does not fit in `max`. */
int xvirus_shim_prestep(uint8_t *out, int max, uint32_t at, uint32_t which, uint32_t clear_veneer, uint32_t sound_veneer);

#endif
