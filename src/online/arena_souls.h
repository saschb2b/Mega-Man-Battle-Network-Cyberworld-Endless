/* The Souls across the Arena's cable under full rules (issue #120,
 * docs/online/arena/from-square/full-rules.md). UNITE keeps the Soul a player commits
 * in the Souls' block of his own GBA (src/emu/soulunite.c: BN6_SOULS, his
 * side's fields, his side the cable's, the same on both GBAs), and BN6's
 * link carries his turn's form (the proxy, BN6_FORM_PROXY) and not the
 * Soul. The pair holds both GBAs, so it carries the Soul: as a frame of the
 * pair ends, a Soul one core's player has committed goes into the other
 * core's block, his side's fields, once a commit, before either GBA's turn
 * takes it up; so does the chip ColonelSoul's button took at his OK
 * (soulcolonel.c), which only his own GBA writes. Both clients' pairs do
 * the same at the same frame. As the battle begins it gives both GBAs
 * each side's chip list for a Dark Soul, which each GBA can make only for
 * its own player (soulchaos.h). (BN5's mood each GBA keeps for its own
 * player alone, as its own Custom screen opens, and reads for him alone:
 * UNITE's stand, the worried rule's DarkChips. A Soul's end the stand's
 * hook marks for both sides, soulunite.c.) */
#ifndef CW_ARENA_SOULS_H
#define CW_ARENA_SOULS_H

#include <stdint.h>

struct mCore;

/* What was carried last from each core: its side's Soul committed and
 * the Souls it has used (a commit adds its bit) */
typedef struct {
	uint8_t next;
	uint32_t used;
} ArenaSoulCarried;

/* A frame's end: each core's committed Soul, where it is new, and its
 * ColonelSoul's chip, where it differs, into the other's */
void arena_souls_carry(struct mCore *core[2], ArenaSoulCarried carried[2]);
/* As the battle begins (both cores at its first Custom screen): each
 * side's chip list for a Dark Soul (BN6_DARK_LISTS: a failed Chaos
 * Unison's, BN5's DarkInvs's), made from that side's folder on its own
 * player's GBA, into both cores, as BN5's link exchange carries each
 * player's list at the battle's setup; a GBA knows only its own player's
 * folder, and in a link battle fills none (soulchaos.h) */
void arena_souls_lists(struct mCore *core[2]);

#endif
