#include "touch_layout.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "buttons.h"

#define PIC_W 240
#define PIC_H 160

/* Sizes in millimetres, turned into dp (6.3 to a millimetre): a thumb's
 * pad is 10 to 14 mm, so A and B are 14 and the D-pad 30, its arms 11
 * wide; the rest, pressed now and then, smaller. */
#define MM 6.3f
#define DPAD_D 30.f
#define ROUND_D 14.f
#define SHOULDER_W 22.f
#define SHOULDER_H 10.f
#define SMALL_W 13.f
#define SMALL_H 8.f
#define MENU_W 12.f
#define MENU_H 7.f
#define EDGE 4.f       /* from the screen's sides and top */
#define FOOT 5.f       /* from its bottom: clear of a phone's home gesture */
#define GAP 2.f        /* between controls, and the picture */
#define TOP 6.5f       /* the picture's top upright: clear of the status bar */
/* The least of that size the controls are laid out at before the picture
 * gives them room (a smaller scale, or the controls over it): a D-pad of
 * 20 mm, A and B of 9, a fingertip's least. */
#define FIT_MIN 0.66f

static float minf(float a, float b) { return a < b ? a : b; }
static float maxf(float a, float b) { return a > b ? a : b; }

/* How much of the controls' size fits (1 their full size): upright, under
 * a picture whose bottom is `bottom` millimetres down a w x h screen; on
 * its side, beside one in columns `col` wide. Upright, the D-pad and the
 * A and B pair side by side need 57 across, and the MENU row, the
 * shoulder buttons over them and START and SELECT under them 63 down. */
static float fit_below(float w, float h, float bottom) { return minf((w - 2 * EDGE) / 57.f, (h - FOOT - bottom - 2 * GAP) / 63.f); }
static float fit_side(float h, float col) { return minf((col - 2 * GAP) / DPAD_D, (h / 2 - 6.f) / 26.f); }

int touch_picture_top(int sw, int sh, float dp, int scale) {
	if (sh <= sw || dp <= 0) return -1;
	float mm = dp * MM, top = TOP * mm;
	if (fit_below(sw / mm, sh / mm, (top + PIC_H * scale) / mm) < FIT_MIN) return -1;
	return (int)(top + 0.5f);
}

int touch_fit_scale(int sw, int sh, float dp, int scale) {
	if (dp <= 0) return scale;
	float mm = dp * MM;
	for (int s = scale; s >= 1; --s) {
		if (sh > sw) {
			if (PIC_W * s * 2 < sw) break;
			if (touch_picture_top(sw, sh, dp, s) >= 0) return s;
		} else {
			if (PIC_H * s * 2 < sh) break;
			if (fit_side(sh / mm, (sw - PIC_W * s) / 2 / mm) >= FIT_MIN) return s;
		}
	}
	return scale;
}

static TouchBox box(float cx, float cy, float w, float h) { TouchBox b = { cx, cy, w, h }; return b; }

void touch_prefs_default(TouchPrefs *p) {
	memset(p, 0, sizeof *p);
	p->size = p->opacity = 100;
	p->haptics = true;
}

void touch_layout_for(const TouchScreen *s, const TouchPrefs *p, TouchLayout *t) {
	memset(t, 0, sizeof *t);
	float dp = s->dp > 0 ? s->dp : 1, mm = dp * MM, W = (float)s->w, H = (float)s->h;
	float w = W / mm, h = H / mm, bottom = (s->py + s->ph) / mm, col = minf(s->px, W - s->px - s->pw) / mm;
	float f;
	if (s->h > s->w && fit_below(w, h, bottom) >= FIT_MIN) {
		t->shape = TOUCH_BELOW;
		f = fit_below(w, h, bottom);
	} else if (fit_side(h, col) >= FIT_MIN) {
		t->shape = TOUCH_SIDE;
		f = fit_side(h, col);
	} else {
		/* (the D-pad, START over SELECT, and A and B across the foot, the
		 * shoulders over them) */
		t->shape = TOUCH_OVER;
		f = minf((w - 3 * EDGE) / 73.f, (h - EDGE - FOOT - GAP) / 41.f);
	}
	/* the player's size, as far as it fits (a phone's is about all) */
	float size = (p->size >= TOUCH_SIZE_MIN && p->size <= TOUCH_SIZE_MAX ? p->size : 100) / 100.f;
	float u = minf(minf(f, 1.f) * size, f) * mm;
	t->dp = dp;
	t->unit = u / MM;
	t->size_max = (int)(100 * minf(f / minf(f, 1.f), TOUCH_SIZE_MAX / 100.f) + 0.001f);
	TouchBox *b = t->box;
	float edge = EDGE * mm, foot = FOOT * mm, gap = GAP * mm;
	if (t->shape == TOUCH_BELOW) {
		/* the D-pad and A and B low in the thumbs' reach, the shoulder
		 * buttons over them, START and SELECT between them at the foot,
		 * MENU under the picture */
		float dcy = H - foot - 27 * u;
		b[TOUCH_DPAD] = box(edge + 15 * u, dcy, DPAD_D * u, DPAD_D * u);
		b[TOUCH_A] = box(W - edge - 7 * u, dcy - 7 * u, ROUND_D * u, ROUND_D * u);
		b[TOUCH_B] = box(W - edge - 18 * u, dcy + 7 * u, ROUND_D * u, ROUND_D * u);
		b[TOUCH_L] = box(edge + 11 * u, dcy - 24 * u, SHOULDER_W * u, SHOULDER_H * u);
		b[TOUCH_R] = box(W - edge - 11 * u, dcy - 24 * u, SHOULDER_W * u, SHOULDER_H * u);
		b[TOUCH_SELECT] = box(W / 2 - 8 * u, H - foot - 4 * u, SMALL_W * u, SMALL_H * u);
		b[TOUCH_START] = box(W / 2 + 8 * u, H - foot - 4 * u, SMALL_W * u, SMALL_H * u);
		b[TOUCH_MENU] = box(W / 2, s->py + s->ph + gap + 3.5f * u, MENU_W * u, MENU_H * u);
	} else if (t->shape == TOUCH_SIDE) {
		/* the D-pad in the left column, A and B in the right one, the
		 * shoulder buttons at their tops and START and SELECT at their feet */
		float cl = col * mm / 2, cr = W - cl, dcy = H / 2 + 3 * u;
		b[TOUCH_DPAD] = box(cl, dcy, DPAD_D * u, DPAD_D * u);
		b[TOUCH_A] = box(cr + 7 * u, dcy - 7 * u, ROUND_D * u, ROUND_D * u);
		b[TOUCH_B] = box(cr - 8 * u, dcy + 6 * u, ROUND_D * u, ROUND_D * u);
		b[TOUCH_L] = box(cl, edge + 5 * u, SHOULDER_W * u, SHOULDER_H * u);
		b[TOUCH_R] = box(cr, edge + 5 * u, SHOULDER_W * u, SHOULDER_H * u);
		b[TOUCH_SELECT] = box(cl, H - edge - 4 * u, SMALL_W * u, SMALL_H * u);
		b[TOUCH_START] = box(cr, H - edge - 4 * u, SMALL_W * u, SMALL_H * u);
		/* (MENU under R, clear of the picture) */
		b[TOUCH_MENU] = box(maxf(cr - 12 * u, W - col * mm + gap + MENU_W * u / 2), edge + 10 * u + gap + 3.5f * u, MENU_W * u, MENU_H * u);
	} else {
		/* over the picture's corners, see-through */
		float dcy = H - foot - 15 * u;
		b[TOUCH_DPAD] = box(edge + 15 * u, dcy, DPAD_D * u, DPAD_D * u);
		b[TOUCH_A] = box(W - edge - 7 * u, dcy - 9 * u, ROUND_D * u, ROUND_D * u);
		b[TOUCH_B] = box(W - edge - 20 * u, dcy + 4 * u, ROUND_D * u, ROUND_D * u);
		b[TOUCH_L] = box(edge + 11 * u, edge + 5 * u, SHOULDER_W * u, SHOULDER_H * u);
		b[TOUCH_R] = box(W - edge - 11 * u, edge + 5 * u, SHOULDER_W * u, SHOULDER_H * u);
		b[TOUCH_START] = box(W / 2, H - foot - 14 * u, SMALL_W * u, SMALL_H * u);
		b[TOUCH_SELECT] = box(W / 2, H - foot - 4 * u, SMALL_W * u, SMALL_H * u);
		b[TOUCH_MENU] = box(W / 2, edge + 3.5f * u, MENU_W * u, MENU_H * u);
	}
	/* left-handed: the D-pad under the right thumb, A and B under the left
	 * (and MENU clear of the D-pad), the shoulders where the fingers are */
	if (p->left_handed) {
		static const int swap[] = { TOUCH_DPAD, TOUCH_A, TOUCH_B, TOUCH_MENU };
		for (size_t i = 0; i < sizeof swap / sizeof *swap; ++i) b[swap[i]].cx = W - b[swap[i]].cx;
	}
	float alpha = (p->opacity >= TOUCH_ALPHA_MIN && p->opacity <= 100 ? p->opacity : 100) / 100.f;
	if (t->shape == TOUCH_OVER) alpha *= 0.6f;
	for (int k = 0; k < TOUCH_CONTROLS; ++k) {
		const TouchPlace *q = &p->place[t->shape][k];
		TouchBox *m = &b[k];
		if (q->size >= TOUCH_SIZE_MIN && q->size <= TOUCH_SIZE_MAX) { m->w = m->w * q->size / 100; m->h = m->h * q->size / 100; }
		if (q->moved) { m->cx = q->x * W / 1000; m->cy = q->y * H / 1000; }
		/* (whole on the screen) */
		m->cx = maxf(m->w / 2, minf(W - m->w / 2, m->cx));
		m->cy = maxf(m->h / 2, minf(H - m->h / 2, m->cy));
		t->alpha[k] = alpha * (q->alpha >= TOUCH_ALPHA_MIN && q->alpha <= 100 ? q->alpha / 100.f : 1.f);
	}
}

static bool round_control(int c) { return c == TOUCH_DPAD || c == TOUCH_A || c == TOUCH_B; }

int touch_control_at(const TouchLayout *t, float x, float y, int held) {
	int best = -1;
	float best_d = 2;
	for (int c = 0; c < TOUCH_CONTROLS; ++c) {
		const TouchBox *b = &t->box[c];
		if (b->w <= 0) continue;
		float dx = x - b->cx, dy = y - b->cy, d;
		if (round_control(c)) {
			/* (a round one by its distance over its reach) */
			float r = b->w / 2, reach = c == TOUCH_DPAD ? r + 10 * t->dp : r * 4 / 3;
			if (c == held) reach += c == TOUCH_DPAD ? 10 * t->dp : r / 3;
			d = sqrtf(dx * dx + dy * dy) / reach;
		} else {
			/* (a plate by the farther of its two, over its half size) */
			float grow = (c == held ? 20 : 10) * t->dp;
			d = maxf(fabsf(dx) / (b->w / 2 + grow), fabsf(dy) / (b->h / 2 + grow));
		}
		if (d <= 1 && d < best_d) { best = c; best_d = d; }
	}
	return best;
}

/* Eight directions from the right, clockwise (y points down), or four. */
static const uint32_t dirs8[8] = { BTN_RIGHT, BTN_RIGHT | BTN_DOWN, BTN_DOWN, BTN_DOWN | BTN_LEFT,
	BTN_LEFT, BTN_LEFT | BTN_UP, BTN_UP, BTN_UP | BTN_RIGHT };
static const uint32_t dirs4[4] = { BTN_RIGHT, BTN_DOWN, BTN_LEFT, BTN_UP };

uint32_t touch_dpad_steer(const TouchLayout *t, float x, float y, uint32_t last, bool four_way) {
	const TouchBox *b = &t->box[TOUCH_DPAD];
	float r = b->w / 2, dx = x - b->cx, dy = y - b->cy, d = sqrtf(dx * dx + dy * dy);
	/* the middle holds nothing, and a thumb steers once a fifth of the way
	 * out; between, it keeps what it held, so a thumb rolling back toward
	 * the middle neither stops nor, past it, turns round */
	if (d < r / 8) return 0;
	if (d < r / 5) return last;
	int n = four_way ? 4 : 8;
	const uint32_t *dirs = four_way ? dirs4 : dirs8;
	float step = 360.f / n, a = atan2f(dy, dx) * 57.29578f;
	if (a < 0) a += 360;
	/* (the direction held stays 8 degrees past its edge) */
	for (int i = 0; i < n; ++i) {
		if (dirs[i] != last) continue;
		float off = fabsf(a - i * step);
		if (off > 180) off = 360 - off;
		if (off <= step / 2 + 8) return last;
	}
	return dirs[(int)((a + step / 2) / step) % n];
}

uint32_t touch_button(int control) {
	switch (control) {
	case TOUCH_A: return BTN_A;
	case TOUCH_B: return BTN_B;
	case TOUCH_L: return BTN_L;
	case TOUCH_R: return BTN_R;
	case TOUCH_START: return BTN_START;
	case TOUCH_SELECT: return BTN_SELECT;
	default: return 0;
	}
}

static const char *const shape_names[TOUCH_SHAPES] = { "below", "side", "over" };
static const char *const control_names[TOUCH_CONTROLS] = { "dpad", "a", "b", "l", "r", "start", "select", "menu" };

const char *touch_control_name(int control) { return control >= 0 && control < TOUCH_CONTROLS ? control_names[control] : ""; }

static bool on(const char *v) { return !strcmp(v, "on") || !strcmp(v, "yes") || !strcmp(v, "1"); }

void touch_prefs_parse(const char *text, TouchPrefs *p) {
	touch_prefs_default(p);
	while (text && *text) {
		char a[16], b[16], c[16], d[16], e[16], f[16];
		int n = sscanf(text, " %15s %15s %15s %15s %15s %15s", a, b, c, d, e, f);
		int v;
		if (n >= 3 && !strcmp(b, "=")) {
			if (!strcmp(a, "size") && sscanf(c, "%d", &v) == 1 && v >= TOUCH_SIZE_MIN && v <= TOUCH_SIZE_MAX) p->size = v;
			else if (!strcmp(a, "opacity") && sscanf(c, "%d", &v) == 1 && v >= TOUCH_ALPHA_MIN && v <= 100) p->opacity = v;
			else if (!strcmp(a, "haptics")) p->haptics = on(c);
			else if (!strcmp(a, "hand")) p->left_handed = !strcmp(c, "left");
		} else if (n == 6 && a[0] != '#') {
			int s = -1, k = -1;
			for (int i = 0; i < TOUCH_SHAPES; ++i) if (!strcmp(a, shape_names[i])) s = i;
			for (int i = 0; i < TOUCH_CONTROLS; ++i) if (!strcmp(b, control_names[i])) k = i;
			if (s >= 0 && k >= 0) {
				TouchPlace *q = &p->place[s][k];
				int x, y, size, alpha;
				q->moved = sscanf(c, "%d", &x) == 1 && sscanf(d, "%d", &y) == 1 && x >= 0 && x <= 1000 && y >= 0 && y <= 1000;
				q->x = (int16_t)(q->moved ? x : 0);
				q->y = (int16_t)(q->moved ? y : 0);
				q->size = (int16_t)(sscanf(e, "%d", &size) == 1 && size >= TOUCH_SIZE_MIN && size <= TOUCH_SIZE_MAX && size != 100 ? size : 0);
				q->alpha = (int16_t)(sscanf(f, "%d", &alpha) == 1 && alpha >= TOUCH_ALPHA_MIN && alpha < 100 ? alpha : 0);
			}
		}
		const char *nl = strchr(text, '\n');
		text = nl ? nl + 1 : NULL;
	}
}

int touch_prefs_format(const TouchPrefs *p, char *out, int size) {
	int n = snprintf(out, (size_t)size,
		"# Cyberworld Endless: the touch controls, as set in their menu (the MENU button beside them).\n"
		"# size and opacity of them all in percent; haptics on or off; hand = left puts the D-pad on the right.\n"
		"size = %d\nopacity = %d\nhaptics = %s\nhand = %s\n"
		"# A control moved or sized in the editor, per arrangement (below the picture, beside it or over it):\n"
		"# its middle in thousandths of the screen across and down (- where it stays), its size and opacity in percent.\n",
		p->size, p->opacity, p->haptics ? "on" : "off", p->left_handed ? "left" : "right");
	for (int s = 0; s < TOUCH_SHAPES; ++s)
		for (int k = 0; k < TOUCH_CONTROLS && n >= 0 && n < size; ++k) {
			const TouchPlace *q = &p->place[s][k];
			if (!q->moved && !q->size && !q->alpha) continue;
			char xs[8] = "-", ys[8] = "-";
			if (q->moved) { snprintf(xs, sizeof xs, "%d", q->x); snprintf(ys, sizeof ys, "%d", q->y); }
			n += snprintf(out + n, (size_t)(size - n), "%s %s %s %s %d %d\n", shape_names[s], control_names[k], xs, ys,
				q->size ? q->size : 100, q->alpha ? q->alpha : 100);
		}
	return n < size ? n : size - 1;
}
