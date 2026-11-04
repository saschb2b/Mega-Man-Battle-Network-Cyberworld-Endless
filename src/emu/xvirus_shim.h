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
 *  - a direction (XVirusFamily.sides): a call of BN4's code to its rule for
 *    the direction from a panel to another, copied with it, whose left and
 *    right are the screen's, made a call to a shim that calls the rule and
 *    numbers them by the object's side, as BN6's collision data holds them;
 *  - an entry (XvRef.entered): a hook of BN4's whose own step is the tail
 *    of its init or update past the engine's work, which BN6's init or
 *    dispatcher does itself, entered through a shim that pushes lr as the
 *    routine's push {lr} did and calls the tail, whose pop {pc} returns;
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
#define XV_SHIM_ENTRY_SIZE   8    /* an entry's */
#define XV_SHIM_SIDE_SIZE    24   /* a direction's (XVirusFamily.sides) */

/* The shim at `at` (word-aligned) that loads lo and hi into r1 and r2
 * (and `side` into r3 where it is 0 or more: BN4's look-up takes its side
 * for itself, BN6's in r3) and branches on to `veneer` (BN6_BY_NAME_RANGE's,
 * within a branch's reach): false where it is not. */
bool xvirus_shim_range(uint8_t out[XV_SHIM_RANGE_SIZE], uint32_t at, uint16_t lo, uint16_t hi, uint32_t veneer, int side);

/* The shim at `at` (word-aligned) that pushes lr and calls `to` (Thumb,
 * within a call's reach: a tail of a routine past its push {lr}), then pops
 * pc: false where it is not. */
bool xvirus_shim_entry(uint8_t out[XV_SHIM_ENTRY_SIZE], uint32_t at, uint32_t to);

/* The shim at `at` (word-aligned) a call to its game's direction rule is
 * aimed at (BN4's, 0 none, 1 up, 2 down, 3 left and 4 right by the screen):
 * it calls `rule` (Thumb, within a call's reach: the rule's copy) and swaps
 * its 3 and 4 where the object's side (r5's +0x16) is not 0, as BN6's rule
 * numbers left and right by it (0x0800E994, r4 the side); r0 and r1 as the
 * rule leaves them but for that, r2 and r3 kept: false where it is not. */
bool xvirus_shim_side(uint8_t out[XV_SHIM_SIDE_SIZE], uint32_t at, uint32_t rule);

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
