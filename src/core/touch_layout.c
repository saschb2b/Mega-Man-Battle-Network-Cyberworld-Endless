#include "touch_layout.h"

#include <stdio.h>
#include <string.h>

#include "buttons.h"

#define PIC_W 240
#define PIC_H 160
#define BELOW_MIN 150   /* canvas rows under the picture that take the controls */
#define SIDE_MIN 72     /* columns beside it that do: a D-pad of radius 30, A and B of 15 (in picture
                         * pixels, so a thumb's share of the screen's short side) */

static int min_i(int a, int b) { return a < b ? a : b; }

static TouchBox round_at(int cx, int cy, int r) { TouchBox b = { cx - r, cy - r, 2 * r + 1, 2 * r + 1 }; return b; }
static TouchBox box_at(int cx, int cy, int w, int h) { TouchBox b = { cx - w / 2, cy - h / 2, w, h }; return b; }

int touch_fit_scale(int sw, int sh, int scale) {
	if (sh > sw) return scale;
	while (scale > 1 && sw / scale - PIC_W < 2 * SIDE_MIN) --scale;
	return scale;
}

int touch_picture_top(int w, int h) {
	if (h <= w || h - PIC_H < BELOW_MIN) return -1;
	/* (clear of a phone's status bar or notch) */
	return h * 3 / 100 + 4;
}

void touch_layout_for(int w, int h, int px, int py, TouchLayout *t) {
	memset(t, 0, sizeof *t);
	int below = h - (py + PIC_H), side = min_i(px, w - (px + PIC_W));
	if (below >= BELOW_MIN && below >= side) {
		/* a tall screen: under the picture, in thumbs' reach, and clear of a
		 * phone's bottom edge (its home gesture) */
		t->below = true;
		int top = py + PIC_H + 8, bot = h - 6 - h * 3 / 100, ah = bot - top;
		int rd = min_i(w * 21 / 100, ah * 23 / 100), rb = min_i(w * 9 / 100, ah * 11 / 100);
		int lw = min_i(w * 30 / 100, 72);
		t->box[TOUCH_L] = box_at(8 + lw / 2, top + ah * 12 / 100 + 10, lw, 20);
		t->box[TOUCH_R] = box_at(w - 8 - lw / 2, top + ah * 12 / 100 + 10, lw, 20);
		t->box[TOUCH_DPAD] = round_at(w * 27 / 100, top + ah * 54 / 100, rd);
		t->box[TOUCH_A] = round_at(w * 85 / 100, top + ah * 47 / 100, rb);
		t->box[TOUCH_B] = round_at(w * 64 / 100, top + ah * 61 / 100, rb);
		t->box[TOUCH_SELECT] = box_at(w * 38 / 100, bot - 10, 46, 16);
		t->box[TOUCH_START] = box_at(w * 62 / 100, bot - 10, 46, 16);
	} else if (side >= SIDE_MIN) {
		/* a wide screen: the D-pad left of the picture, A and B right of it */
		int rd = min_i(side * 42 / 100, h * 24 / 100), rb = min_i(side * 21 / 100, h * 10 / 100);
		int bw = min_i(side - 8, 56);
		t->box[TOUCH_L] = box_at(side / 2, 14, bw, 18);
		t->box[TOUCH_R] = box_at(w - side / 2, 14, bw, 18);
		t->box[TOUCH_DPAD] = round_at(side / 2, h * 56 / 100, rd);
		t->box[TOUCH_A] = round_at(w - side * 29 / 100, h * 50 / 100, rb);
		t->box[TOUCH_B] = round_at(w - side * 69 / 100, h * 63 / 100, rb);
		t->box[TOUCH_SELECT] = box_at(side / 2, h - 12, min_i(bw, 46), 14);
		t->box[TOUCH_START] = box_at(w - side / 2, h - 12, min_i(bw, 46), 14);
	} else {
		/* no room: over the picture's corners, see-through */
		t->over = true;
		t->box[TOUCH_L] = box_at(px + 26, py + 22, 44, 14);
		t->box[TOUCH_R] = box_at(px + PIC_W - 26, py + 22, 44, 14);
		t->box[TOUCH_DPAD] = round_at(px + 38, py + PIC_H - 44, 30);
		t->box[TOUCH_A] = round_at(px + PIC_W - 22, py + PIC_H - 52, 14);
		t->box[TOUCH_B] = round_at(px + PIC_W - 52, py + PIC_H - 34, 14);
		t->box[TOUCH_SELECT] = box_at(px + PIC_W / 2 - 24, py + PIC_H - 10, 40, 12);
		t->box[TOUCH_START] = box_at(px + PIC_W / 2 + 24, py + PIC_H - 10, 40, 12);
	}
}

static bool round_control(int c) { return c == TOUCH_DPAD || c == TOUCH_A || c == TOUCH_B; }

int touch_shape(const TouchLayout *t) { return t->below ? TOUCH_SHAPE_BELOW : t->over ? TOUCH_SHAPE_OVER : TOUCH_SHAPE_SIDE; }

void touch_layout_custom(TouchLayout *t, int w, int h, const TouchCustom *c) {
	if (!c) return;
	const TouchPlace *p = c->place[touch_shape(t)];
	for (int k = 0; k < TOUCH_CONTROLS; ++k) {
		TouchBox *b = &t->box[k];
		if (b->w <= 0 || (!p[k].moved && !p[k].size)) continue;
		int cx = b->x + b->w / 2, cy = b->y + b->h / 2;
		if (p[k].size) {
			int s = p[k].size < TOUCH_SIZE_MIN ? TOUCH_SIZE_MIN : p[k].size > TOUCH_SIZE_MAX ? TOUCH_SIZE_MAX : p[k].size;
			if (round_control(k)) {
				int r = b->w / 2 * s / 100;
				*b = round_at(cx, cy, r < 6 ? 6 : r);
			} else {
				int bw = b->w * s / 100, bh = b->h * s / 100;
				*b = box_at(cx, cy, bw < 16 ? 16 : bw, bh < 10 ? 10 : bh);
			}
		}
		if (p[k].moved) {
			cx = p[k].x * w / 1000;
			cy = p[k].y * h / 1000;
		}
		/* (whole on the canvas, where it fits) */
		int x = cx - b->w / 2, y = cy - b->h / 2;
		if (x > w - b->w) x = w - b->w;
		if (y > h - b->h) y = h - b->h;
		b->x = x < 0 ? 0 : x;
		b->y = y < 0 ? 0 : y;
	}
}

static const char *const shape_names[TOUCH_SHAPES] = { "below", "side", "over" };
static const char *const control_names[TOUCH_CONTROLS] = { "dpad", "a", "b", "l", "r", "start", "select" };

void touch_custom_parse(const char *text, TouchCustom *c) {
	memset(c, 0, sizeof *c);
	while (text && *text) {
		char shape[16], name[16], xs[16], ys[16];
		int size;
		if (sscanf(text, " %15s %15s %15s %15s %d", shape, name, xs, ys, &size) == 5) {
			int s = -1, k = -1;
			for (int i = 0; i < TOUCH_SHAPES; ++i) if (!strcmp(shape, shape_names[i])) s = i;
			for (int i = 0; i < TOUCH_CONTROLS; ++i) if (!strcmp(name, control_names[i])) k = i;
			if (s >= 0 && k >= 0) {
				TouchPlace *p = &c->place[s][k];
				int x, y;
				p->moved = sscanf(xs, "%d", &x) == 1 && sscanf(ys, "%d", &y) == 1 && x >= 0 && x <= 1000 && y >= 0 && y <= 1000;
				p->x = (int16_t)(p->moved ? x : 0);
				p->y = (int16_t)(p->moved ? y : 0);
				p->size = (int16_t)(size >= TOUCH_SIZE_MIN && size <= TOUCH_SIZE_MAX && size != 100 ? size : 0);
			}
		}
		const char *nl = strchr(text, '\n');
		text = nl ? nl + 1 : NULL;
	}
}

int touch_custom_format(const TouchCustom *c, char *out, int size) {
	int n = snprintf(out, (size_t)size, "# Cyberworld Endless: the touch controls as you arranged them on the title's\n"
		"# EDIT CONTROLS. Per screen shape (below or beside the picture, or over it):\n"
		"# a control's middle in thousandths of the screen (- where it stays), its size in percent.\n");
	for (int s = 0; s < TOUCH_SHAPES; ++s)
		for (int k = 0; k < TOUCH_CONTROLS && n >= 0 && n < size; ++k) {
			const TouchPlace *p = &c->place[s][k];
			if (!p->moved && !p->size) continue;
			char xs[8] = "-", ys[8] = "-";
			if (p->moved) { snprintf(xs, sizeof xs, "%d", p->x); snprintf(ys, sizeof ys, "%d", p->y); }
			n += snprintf(out + n, (size_t)(size - n), "%s %s %s %s %d\n", shape_names[s], control_names[k], xs, ys, p->size ? p->size : 100);
		}
	return n < size ? n : size - 1;
}

int touch_control_at(const TouchLayout *t, int x, int y) {
	/* the round buttons first, each with a quarter of its radius more */
	static const int order[TOUCH_CONTROLS] = { TOUCH_A, TOUCH_B, TOUCH_DPAD, TOUCH_L, TOUCH_R, TOUCH_START, TOUCH_SELECT };
	int best = -1;
	long best_d = 0;
	for (int k = 0; k < TOUCH_CONTROLS; ++k) {
		int c = order[k];
		const TouchBox *b = &t->box[c];
		if (b->w <= 0) continue;
		if (round_control(c)) {
			int r = b->w / 2, cx = b->x + r, cy = b->y + r, reach = r + r / 4 + 2;
			long d = (long)(x - cx) * (x - cx) + (long)(y - cy) * (y - cy);
			/* (between A and B, the nearer) */
			if (d <= (long)reach * reach && (best < 0 || d < best_d)) { best = c; best_d = d; }
		} else if (best < 0 && x >= b->x - 4 && x < b->x + b->w + 4 && y >= b->y - 4 && y < b->y + b->h + 4) {
			best = c;
		}
	}
	return best;
}

uint32_t touch_dpad_steer(const TouchLayout *t, int x, int y, uint32_t last) {
	const TouchBox *b = &t->box[TOUCH_DPAD];
	int r = b->w / 2, dx = x - (b->x + r), dy = y - (b->y + r);
	long d2 = (long)dx * dx + (long)dy * dy, stop = r / 5 + 1, start = r / 3 + 1;
	/* the middle holds nothing, and a thumb steers once it is a third of
	 * the way out; between, it keeps what it held, so a thumb rolling back
	 * toward the middle neither stops nor, past it, turns round (a
	 * player's MegaMan went the opposite way now and then) */
	if (d2 < stop * stop) return 0;
	if (d2 < start * start) return last;
	int ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
	uint32_t h = dx < 0 ? BTN_LEFT : BTN_RIGHT, v = dy < 0 ? BTN_UP : BTN_DOWN;
	/* eight directions of 45 degrees each: BN6's walkways run along the
	 * diagonals, which had 30 (tan 22.5 is 0.414) */
	if (ay * 1000 <= ax * 414) return h;
	if (ax * 1000 <= ay * 414) return v;
	return h | v;
}

static uint32_t dpad(const TouchLayout *t, int x, int y) { return touch_dpad_steer(t, x, y, 0); }

uint32_t touch_hit(const TouchLayout *t, int x, int y, int from) {
	if (from == TOUCH_DPAD) return dpad(t, x, y);
	switch (touch_control_at(t, x, y)) {
	case TOUCH_DPAD: return 0;   /* (a finger that went down on a button does not steer) */
	case TOUCH_A: return BTN_A;
	case TOUCH_B: return BTN_B;
	case TOUCH_L: return BTN_L;
	case TOUCH_R: return BTN_R;
	case TOUCH_START: return BTN_START;
	case TOUCH_SELECT: return BTN_SELECT;
	default: return 0;
	}
}
