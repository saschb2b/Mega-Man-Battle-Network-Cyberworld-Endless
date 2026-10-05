/* --sheet: sprites drawn into a picture, for a look at them. */
#include "sheets.h"

#include <stdio.h>

#include "gfx.h"
#include "platform.h"
#include "rom.h"

int sheet_run(const char *spec) {
	if (spec[0] == '@') {
		/* --sheet @CAT:FIRST:COUNT:PATH  frame 0 of anim 0 of many sprites, 48x64 cells */
		int cat = 0, first = 0, count = 64;
		char out[256] = "sprites.bmp";
		sscanf(spec + 1, "%i:%i:%i:%255s", &cat, &first, &count, out);
		platform_begin_frame();
		fill_rect(0, 0, P.w, P.h, rgba(96, 96, 96, 255));
		/* 32 x 48 cells, or 96 x 128 when the canvas is wide enough to
		 * show the big overworld objects whole (--size 479xH) */
		int cw = P.w >= 384 ? 96 : 32, ch = P.w >= 384 ? 128 : 48, cols = P.w / cw;
		for (int i = 0; i < count; ++i) {
			int cx = (i % cols) * cw, cy = (i / cols) * ch;
			Sprite *spr = sprite_get(cat, first + i);
			fill_rect(cx + 1, cy + 1, cw - 2, ch - 2, rgba(40, 40, 60, 255));
			if (spr) sprite_draw_frame(spr, 0, 0, cx + cw / 2, cy + ch - 8, false, 0, 0);
			if (cw > 32 && R.data) text_drawf(cx + 2, cy + 1, WHITE, TEXT_LEFT, "%x", first + i);
		}
		platform_save_canvas(out);
		return 0;
	}
	/* --sheet CAT:IDX:ANIM[:PAL]:PATH  every frame of one animation, 64x64 cells */
	int cat = 0, idx = 0, anim = 0, pal = 0;
	char out[256] = "sheet.bmp";
	if (sscanf(spec, "%i:%i:%i:%i:%255s", &cat, &idx, &anim, &pal, out) < 5)
		sscanf(spec, "%i:%i:%i:%255s", &cat, &idx, &anim, out);
	Sprite *spr = sprite_get(cat, idx);
	int n = sprite_frame_count(spr, anim);
	platform_begin_frame();
	fill_rect(0, 0, P.w, P.h, rgba(96, 96, 96, 255));
	for (int f = 0; f < n && f < 12; ++f) {
		int cx = (f % 4) * 64, cy = (f / 4) * 64;
		fill_rect(cx + 1, cy + 1, 62, 62, rgba(40, 40, 60, 255));
		sprite_draw_frame(spr, anim, f, cx + 32, cy + 48, false, pal, 0);
	}
	platform_save_canvas(out);
	printf("sheet %d:%d:%d frames %d -> %s\n", cat, idx, anim, n, out);
	return 0;
}
