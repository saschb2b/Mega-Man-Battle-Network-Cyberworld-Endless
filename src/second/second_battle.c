/* second_battle.h. BN6's Custom screen shows the chip under the cursor as
 * a small card (its picture, code, element and power) and lists the
 * enemies' names; the field shows their HP under them. Here the card is
 * large, as the DS games' chip pictures are, with its text from the
 * Library, and the chips picked under it; CROSSSELECT's Cross and the
 * Beast Out emblem get cards of their own (what BN6's tutorials told of
 * them), and OK the picks in order. While MegaMan fights: the chips he
 * holds in their order (the top screen names the next alone), his Cross,
 * Beast Out and Full Synchro, what he knows of a guardian, and each enemy
 * by name with its HP. Nothing BN6 keeps from the player: not the
 * folder's draw, not an enemy's next move. */
#include "second_battle.h"

#include <stdio.h>
#include <string.h>

#include "data.h"
#include "powers.h"
#include "second_art.h"
#include "second_frame.h"
#include "second_state.h"
#include "second_text.h"

#define ART_SCALE 2
#define CARD_H    (48 * ART_SCALE + 4)   /* the card's height, its picture's frame */
#define ROW_H     18                     /* a chip's line */
#define FOE_H     16                     /* an enemy's line */
#define LINE_H    13                     /* a line of text */
#define WRAP_CHARS  96

/* `text` broken into lines no wider than `w` (a '|' breaks one too), at
 * most `most` of them into `lines`: how many */
static int wrap(const char *text, int w, char lines[][WRAP_CHARS], int most) {
	int n = 0;
	char line[WRAP_CHARS] = "";
	for (const char *p = text; *p && n < most;) {
		if (*p == '|') {
			snprintf(lines[n++], WRAP_CHARS, "%s", line);
			*line = 0;
			++p;
			continue;
		}
		while (*p == ' ') ++p;
		size_t k = strcspn(p, " |");
		if (!k) continue;
		char next[WRAP_CHARS * 2];
		snprintf(next, sizeof next, "%s%s%.*s", line, *line ? " " : "", (int)(k < WRAP_CHARS ? k : WRAP_CHARS - 1), p);
		p += k;
		if (*line && text_width(next) > w) {
			snprintf(lines[n++], WRAP_CHARS, "%s", line);
			snprintf(line, sizeof line, "%.*s", (int)(k < WRAP_CHARS ? k : WRAP_CHARS - 1), p - k);
		} else snprintf(line, sizeof line, "%.*s", WRAP_CHARS - 1, next);
	}
	if (*line && n < most) snprintf(lines[n++], WRAP_CHARS, "%s", line);
	return n;
}

/* `text` wrapped from (x, y) in `w`, at most `most` lines: the y under it */
static int wrapped(const char *text, int x, int y, int w, int most, SDL_Color c) {
	char lines[8][WRAP_CHARS];
	int n = wrap(text, w, lines, most < 8 ? most : 8);
	for (int i = 0; i < n; ++i) text_draw(x, y + i * LINE_H, lines[i], c, TEXT_LEFT);
	return y + n * LINE_H;
}

/* The chip under the cursor as a card: its picture twice as large, its
 * name, code, element and power beside it, its text under them */
static void card_chip(uint16_t e, int x, int y, int w) {
	int chip = e & 0x1FF;
	ChipInfo ci;
	chip_info(chip, &ci);
	fill_rect(x, y, 56 * ART_SCALE + 4, CARD_H, PET_SLOT_EDGE);
	second_chip_art(chip, x + 2, y + 2, ART_SCALE);
	int tx = x + 56 * ART_SCALE + 12;
	text_draw(tx, y + 2, ci.name, PET_WHITE, TEXT_LEFT);
	char s[16];
	snprintf(s, sizeof s, "%c", (e >> 9) >= 26 ? '*' : 'A' + (e >> 9));
	text_draw_scaled(tx, y + 18, s, PET_GOLD, TEXT_LEFT, 2);
	second_element_icon(second_chip_element(chip), tx + 24, y + 22);
	if (ci.power > 0) {
		snprintf(s, sizeof s, "%d", ci.power);
		text_draw_scaled(x + w - 4, y + 18, s, PET_WHITE, TEXT_RIGHT, 2);
	}
	char desc[160];
	chip_desc(chip, desc, sizeof desc);
	wrapped(desc, tx, y + 52, x + w - tx - 4, 3, PET_WHITE);
}

/* A card of words alone: its name twice as large, its lines under it */
static void card_words(const char *name, const char *lines, int x, int y, int w) {
	text_draw_scaled(x + 2, y + 2, name, PET_GOLD, TEXT_LEFT, 2);
	wrapped(lines, x + 2, y + 34, w - 4, 5, PET_WHITE);
}

/* a chip's power as the HUD writes it, with what Atk+ chips add ("80",
 * "80+10"); empty for none */
static void power_text(char *s, size_t n, int power, int bonus) {
	if (power <= 0) *s = 0;
	else if (bonus > 0) snprintf(s, n, "%d+%d", power, bonus);
	else snprintf(s, n, "%d", power);
}

/* a chip's line at (r.x, r.y), r.w wide: its icon, name, `code` where
 * known (0 none) and `power` */
static void chip_row(uint16_t chip, char code, const char *power, SDL_Rect r, SDL_Color c) {
	ChipInfo ci;
	chip_info(chip, &ci);
	fill_rect(r.x, r.y, 18, 18, PET_SLOT_EDGE);
	second_chip_icon(chip, r.x + 1, r.y + 1);
	text_draw(r.x + 24, r.y + 3, ci.name, c, TEXT_LEFT);
	if (code) {
		char s[2] = { code, 0 };
		text_draw(r.x + r.w - 64, r.y + 3, s, c, TEXT_LEFT);
	}
	text_draw(r.x + r.w, r.y + 3, power, c, TEXT_RIGHT);
}

static char code_of(uint16_t e) { return (e >> 9) >= 26 ? '*' : (char)('A' + (e >> 9)); }

/* On OK: the chips picked, a line each in their order */
static void card_picks(int x, int y, int w) {
	if (!S2.npicks) return;
	text_draw(x, y, "Picked", PET_GOLD, TEXT_LEFT);
	for (int k = 0; k < S2.npicks; ++k) {
		uint16_t e = S2.hand[S2.picks[k]];
		ChipInfo ci;
		char power[16];
		chip_info(e & 0x1FF, &ci);
		power_text(power, sizeof power, ci.power, 0);
		chip_row(e & 0x1FF, code_of(e), power, (SDL_Rect){ x, y + 16 + k * ROW_H, w, ROW_H }, PET_WHITE);
	}
}

/* the chips picked so far, their icons in a row */
static void picks_row(int x, int y) {
	if (!S2.npicks) return;
	text_draw(x, y + 3, "Picked", PET_GOLD, TEXT_LEFT);
	x += text_width("Picked") + 8;
	for (int k = 0; k < S2.npicks; ++k, x += 22) {
		fill_rect(x, y, 18, 18, PET_SLOT_EDGE);
		second_chip_icon(S2.hand[S2.picks[k]] & 0x1FF, x + 1, y + 1);
	}
}

/* The Custom screen: its card, and the picks under it */
static void custom(int x, int y, int w) {
	char lines[200];
	switch (S2.card) {
	case CARD_CHIP: card_chip(S2.hand[S2.cursor], x, y, w); break;
	case CARD_CROSS:
		second_cross_lines(S2.cross_under, lines, sizeof lines);
		card_words(powers_cross_name(S2.cross_under), lines, x, y, w);
		break;
	case CARD_BEAST:
		second_beast_lines(S2.beast_turns, lines, sizeof lines);
		card_words("Beast Out", lines, x, y, w);
		break;
	case CARD_PICKS: card_picks(x, y, w); return;
	}
	picks_row(x, y + CARD_H + 6);
}

/* MegaMan's hand while he fights: the turn's chips in their order, each
 * with its power as it will hit, the used ones dim and the next lit: the
 * y under it */
static int hand(int x, int y, int w) {
	if (S2.nqueue == 0) return y;
	text_draw(x, y, "Hand", PET_GOLD, TEXT_LEFT);
	y += 16;
	for (int k = 0; k < S2.nqueue; ++k, y += ROW_H) {
		char power[16];
		power_text(power, sizeof power, S2.queue_power[k], S2.queue_bonus[k]);
		chip_row(S2.queue[k] & 0x1FF, 0, power, (SDL_Rect){ x, y, w, ROW_H },
			k < S2.queue_at ? PET_DIM : k == S2.queue_at ? PET_GOLD : PET_WHITE);
	}
	return y;
}

/* His Cross, Beast Out, Full Synchro: a name and a line each; the y under */
static int megaman(int x, int y) {
	SecondStateLine s[3];
	int n = second_state_lines(S2.form, S2.beast_turns, S2.beast, S2.synchro, s, 3);
	for (int i = 0; i < n; ++i, y += 2 * LINE_H + 4) {
		text_draw(x, y, s[i].name, PET_GOLD, TEXT_LEFT);
		text_draw(x, y + LINE_H, s[i].line, PET_WHITE, TEXT_LEFT);
	}
	return y;
}

/* What MegaMan knows of the guardian, from `y` down to `bottom`: his
 * boxes, those whole ones that fit */
static void tip(int x, int y, int w, int bottom) {
	if (!S2.tip || y + 2 * LINE_H > bottom) return;
	text_draw(x, y, "MegaMan knows", PET_GOLD, TEXT_LEFT);
	y += LINE_H + 2;
	char box[WRAP_CHARS * 3], lines[6][WRAP_CHARS];
	for (const char *p = S2.tip; *p;) {
		size_t k = strcspn(p, "|");
		const char *t = k > 3 && p[0] == '@' && p[2] == ' ' ? p + 3 : p;
		snprintf(box, sizeof box, "%.*s", (int)(p + k - t), t);
		p += k + (p[k] == '|');
		int n = wrap(box, w, lines, 6);
		if (y + n * LINE_H > bottom) break;
		for (int i = 0; i < n; ++i, y += LINE_H) text_draw(x, y, lines[i], PET_WHITE, TEXT_LEFT);
	}
}

/* The enemies, two to a line, each in a slot: its name and HP (a Navi's
 * element before his name, as the guardian's card told it): the y they
 * start at */
static int foes(SDL_Rect b) {
	if (!S2.nfoes) return b.y + b.h;
	int rows = (S2.nfoes + 1) / 2, col = (b.w - 8) / 2, y0 = b.y + b.h - 4 - rows * (FOE_H + 2);
	for (int i = 0; i < S2.nfoes; ++i) {
		int x = b.x + 4 + (i % 2) * col, y = y0 + (i / 2) * (FOE_H + 2);
		second_slot(x, y, col - 2, FOE_H);
		char name[24], hp[16];
		enemy_name(S2.foe[i].name, name, sizeof name);
		snprintf(hp, sizeof hp, "%d", S2.foe[i].hp);
		text_draw(x + col - 6, y + 2, hp, S2.foe[i].hp * 4 <= S2.foe[i].max_hp ? PET_GOLD : PET_WHITE, TEXT_RIGHT);
		/* (BN6's elements 1-4, Heat to Wood: its icons' 0-3) */
		int el = S2.foe[i].element;
		if (S2.foe[i].name > 0xFF && el >= 1 && el <= 4) {
			second_element_icon(el - 1, x + 2, y);
			x += 18;
		}
		text_draw(x + 4, y + 2, name, PET_WHITE, TEXT_LEFT);
	}
	return y0;
}

void second_battle_draw(SDL_Rect b) {
	fill_rect(b.x, b.y, b.w, b.h, PET_NAVY);
	int x = b.x + 6, w = b.w - 12, top = foes(b);
	if (S2.custom) {
		custom(x, b.y + 6, w);
		return;
	}
	int half = w / 2;
	int y = hand(x, b.y + 6, half - 8);
	int y2 = megaman(x + half + 4, b.y + 6);
	tip(x, (y > y2 ? y : y2) + 4, w, top - 4);
}
