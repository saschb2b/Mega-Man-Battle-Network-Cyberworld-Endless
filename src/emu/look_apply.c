/* MegaMan's look onto his palettes in the ROM copy (look_apply.h). Step 2's
 * first part: the interface, doing nothing yet (BN6's colours stand). */
#include "look_apply.h"

void look_apply(const Look *look) { (void)look; }

bool look_palette_of(const Look *look, int which, uint16_t out16[16]) {
	(void)look; (void)which; (void)out16;
	return false;
}
