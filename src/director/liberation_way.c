/* The way to a dark layer marked at home (liberation.h; docs/LIBERATION.md,
 * Where it appears): in Lan's HP, the portal of the way whose act holds a
 * dark layer shows BN5's dark panels' colours before MegaMan steps on it.
 * Its own colours where it is drawn (the pink pad's magenta, a link
 * square's glass), within its zone (its cells and its art, lanhp.c), are
 * taken shade for shade into the swirl's ramp of BN5's palette 9 (its
 * colours 1-9, light to dark), read from BN5's ROM, and stepped as BN5's
 * board steps it (BN5_LIB_DARK_FRAMES' timing): the frame alone is
 * recoloured, so BN6's own palettes, its scripts and the sprites over the
 * portal stay as they are. With MegaMan on or at the portal the mark goes,
 * his sprite over it left whole. Reads only. */
#include "liberation.h"

#include <stdint.h>

#include "bn5_liberation.h"
#include "bn6.h"
#include "bn6_fields.h"
#include "bn6_forms.h"
#include "director_home.h"
#include "director_state.h"
#include "emu.h"
#include "lanhp.h"
#include "rom.h"

#define CAM_X 120                 /* where the camera's point shows */
#define CAM_Y 80
#define NEAR  12                  /* MegaMan this near the portal's zone (world units): the mark gone */
#define RAMP  9                   /* palette 9's swirl, its colours 1 (light) to 9 (dark) */

/* Palette 9 as BN5's board shows it at frame `t`: at rest 120 frames, then
 * three frames down and back, 16, 16, 32, 16 and 16 frames (traced) */
static const uint8_t *swirl(const uint8_t *rom, int t) {
	static const int held[6] = { 120, 16, 16, 32, 16, 16 }, at[6] = { 3, 2, 1, 0, 1, 2 };
	t %= 216;
	int i = 0;
	while (i < 5 && t >= held[i]) t -= held[i++];
	return rom + (BN5_LIB_DARK_FRAMES - 0x08000000u) + 0x20u * (uint32_t)at[i];
}

/* A pixel of the portal's own colours: its shade on the swirl's ramp, its
 * lightest the ramp's middle, its darkest the ramp's end (so the portal
 * reads dark, as BN5's dark panels do); -1 not one of them (the pink pad's
 * magenta has no green; a link's glass is pale blue to cyan) */
static int shade(uint32_t c, bool pad) {
	int r = c & 0xFF, g = c >> 8 & 0xFF, b = c >> 16 & 0xFF, s;
	if (pad) {
		if (g >= 24 || r < 160 || b < 96) return -1;
		s = 3 + (247 - r) / 8;
	} else {
		if (b < 224 || g < 120 || r > 190) return -1;
		s = 2 + (255 - g) / 25;
	}
	return s < 2 ? 2 : s > RAMP - 1 ? RAMP - 1 : s;
}

void lib_way_tint(uint32_t *px) {
	const XRom *x = &XR[XROM_BN5_COLONEL_US];
	if (!x->data || !D.active || !home_in_hp() || !on_map()) return;
	int n, k = -1;
	bool sealed;
	const RunWay *ways = home_ways(&n, &sealed);
	for (int i = 0; i < n; ++i) if (ways[i].board) k = i;
	if (k < 0 || !(home_lit() >> k & 1)) return;
	int mx = bn6_player_x(), my = bn6_player_y();
	if (lanhp_portal_near(mx, my, NEAR) == k) return;
	/* (BN6's camera: the world point at the picture's middle) */
	uint32_t cam = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CAMERA);
	int cx = (int32_t)emu_read32(cam + BN6_CAMERA_RENDER_X) >> 16, cy = (int32_t)emu_read32(cam + BN6_CAMERA_RENDER_Y) >> 16;
	int cz = (int32_t)emu_read32(cam + BN6_CAMERA_RENDER_Z) >> 16;
	int cam_u = cx + cy, cam_v = (cy - cx) / 2 - cz;
	const uint8_t *pal = swirl(x->data, D.frame);
	uint32_t ramp[RAMP];
	for (int i = 0; i < RAMP; ++i) {
		uint16_t c = (uint16_t)(pal[2 * (i + 1)] | pal[2 * (i + 1) + 1] << 8);
		uint32_t r = c & 31, g = c >> 5 & 31, b = c >> 10 & 31;
		ramp[i] = (r << 3 | r >> 2) | (g << 3 | g >> 2) << 8 | (b << 3 | b >> 2) << 16;
	}
	/* (the portal's zone on the screen: around its spot, the pink pad's
	 * ring the widest, 62 by 42 pixels) */
	int sx, sy;
	lanhp_portal_spot(k, &sx, &sy);
	int px0 = sx + sy - cam_u + CAM_X, py0 = (sy - sx) / 2 - cam_v + CAM_Y;
	for (int y = py0 - 32; y < py0 + 32; ++y) {
		if (y < 0 || y >= EMU_H) continue;
		for (int xx = px0 - 48; xx < px0 + 48; ++xx) {
			if (xx < 0 || xx >= EMU_W) continue;
			uint32_t c = px[y * EMU_W + xx];
			int s = shade(c, k == 0);
			if (s < 0) continue;
			int u = xx - CAM_X + cam_u, v = 2 * (y - CAM_Y + cam_v);
			if (lanhp_portal_near((u - v) / 2, (u + v) / 2, 0) != k) continue;
			px[y * EMU_W + xx] = (c & 0xFF000000u) | ramp[s];
		}
	}
}
