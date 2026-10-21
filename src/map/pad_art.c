/* pad_art.h. A pad's art on the screen is an ellipse round its middle (the
 * link marker's diamond inside its own), and MegaMan reads as standing on
 * it while his feet are on it, or just in front of it with his sprite over
 * its front: a playtester stood so on the pink pad's left rim, his feet
 * 16 pixels left of its middle and 20 below it, and on a layer's exit pad,
 * 23 left and 22 below, for 150 frames each, and nothing warped (session
 * 75). The front's allowance is the 14 and 12 world units Lan's HP's
 * portals were stretched by (session 73), 13 pixels down the screen. */
#include "pad_art.h"

#define PAD_SHOE 6    /* his shoes and his sprite, either side of the middle of his feet (pixels across) */
#define PAD_FRONT 13  /* his sprite over the art from in front of it (pixels down) */

bool pad_art_on(const PadArt *p, int x, int y, int reach) {
	long long dx = x - p->x, dy = y - p->y;
	long long across = dx + dy, down2 = dy - dx;   /* (twice the pixels down) */
	if (down2 > 0) down2 = down2 > 2 * PAD_FRONT ? down2 - 2 * PAD_FRONT : 0;
	long long w = p->across + PAD_SHOE + reach, h2 = 2LL * (p->down + reach);
	if (w <= 0 || h2 <= 0) return false;
	return across * across * h2 * h2 + down2 * down2 * w * w <= w * w * h2 * h2;
}
