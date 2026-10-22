/* A Soul's look in BN6's engine (issue #82, the Souls track's S3; docs/
 * MULTIROM.md, Transplanted Souls; docs/ROM_DATA.md, A Soul's look): the
 * proxy form drawn as BN5 draws a Soul. MegaMan's sprite is the Soul's
 * whole sprite, BN5's list 0 by the Soul (BN6's list 0 grown with them),
 * its palette 0, 2 in Chaos Unison; no Cross's overlay but what BN5's
 * Soul spawns as he enters it (NumberSoul's); his unison's phase 1 is
 * BN5's, the Soul's image (BN6's own copy of BN5's object, which Gregar
 * keeps and never spawns, drawn from the Soul's Navi's battle sprite in
 * BN6's list 2, xnavi_sprites.h), in place of a Cross's summon; his face
 * in the emotion window and on the Custom screen BN5's of the Soul, its
 * turns counted under it; his arm's flash BN5's in the Soul, at its kind's
 * own height (BN6 raises a Cross's); TomahawkSoul's landing shakes the
 * camera as it unites, as BN5's phase 2 does. */
#ifndef CW_SOULLOOK_H
#define CW_SOULLOOK_H

#include <stdbool.h>
#include <stdint.h>

/* Who the Souls' look's bytes are (claims.h) */
#define SOULLOOK_CLAIMS "the Souls' look (soullook.c)"

/* (soulbreak's look, soulunite.h's SOULBREAK_LOOK: the differential's own test, tests/xdiff/mutations.json) */
enum {
	SOULLOOK_BREAK_SPRITE = 1,   /* the next Soul's sprite drawn for each Soul */
	SOULLOOK_BREAK_PALETTE = 2,  /* palette 1 where BN5 draws 0 */
	SOULLOOK_BREAK_SHAKE = 4,    /* TomahawkSoul's landing left still (no camera shake as it unites) */
	SOULLOOK_BREAK_FLASH = 8,    /* MegaMan's buster's flash in a Soul spawned as BN6's object (BN5's kind in BN6's table) */
};

/* The look as the core starts, after the Souls' code (soulcode_install):
 * the sprites and faces copied from BN5's ROM, BN6's list 0, faces and
 * their palettes grown, the hooks. Once a core; false (nothing written)
 * where a byte it stands on is not as expected. */
bool soullook_install(int breaks);
/* (soulproxy.c's arm flash hook) the flash of MegaMan's buster in the
 * proxy form where a Soul is united on `side`: BN6's kind for BN5's
 * (*kind) and the Soul as its frame (*frame); false where none is */
bool soullook_flash(uint32_t obj, int *kind, int *frame);

#endif
