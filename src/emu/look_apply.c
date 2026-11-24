/* MegaMan's look onto his palettes in the ROM copy (look_apply.h). Step 2's
 * first part: the interface, BN6's colours standing (no palette given yet). */
#include "look_apply.h"

void look_apply(const Look *look) {
	uint16_t pal[16];
	for (int w = 0; w < LOOK_SPRITE_COUNT; ++w)
		if (look_palette_of(look, w, pal)) return;
}
