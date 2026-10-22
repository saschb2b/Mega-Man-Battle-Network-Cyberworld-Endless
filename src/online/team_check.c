/* Two MegaMen a side: a partner's messages checked (team_check.h). */
#include "team_check.h"

#include <stddef.h>

#define CODES        26      /* A-Z */
#define FORM_CROSSES 5       /* Gregar's Crosses, forms 1-5 */
#define FORM_PROXY   8       /* the Souls' proxy form (BN6_FORM_PROXY) */
#define FORM_BEAST   11      /* Beast Out (BN6_FORM_BEAST); */
#define FORM_CROSS_BEAST 12  /* ... this + a Cross, Beast Out in it (BN6_FORM_CROSS_BEAST) */
#define SOULS        12      /* BN5's Souls 1-12 */
#define RECIPES_MAX  128     /* a list's recipes walked at most */
#define KIND_IN_A_ROW 0      /* a recipe's kind: one chip n times, codes in a row */
#define KIND_CHIPS   4       /* ... its chips in order */

static unsigned id_of(uint16_t e) { return e & 0x1FFu; }
static unsigned code_of(uint16_t e) { return e >> 9; }

const char *team_check_picks(const uint16_t *hand, unsigned dealt, const uint8_t *slots, unsigned n) {
	if (n > TEAM_CHECK_PICKS) return "more than five picks";
	if (dealt > TEAM_CHECK_DEALT) dealt = TEAM_CHECK_DEALT;
	unsigned seen = 0, id = 0, code = TEAM_CHECK_STAR;
	bool one_chip = true, one_code = true, first = true;
	for (unsigned i = 0; i < n; ++i) {
		unsigned s = slots[i];
		if (s == TEAM_CHECK_EMBLEM) continue;
		if (s >= dealt) return "a pick past the hand dealt";
		if (seen >> s & 1u) return "a slot picked twice";
		seen |= 1u << s;
		unsigned c = code_of(hand[s]);
		if (first) id = id_of(hand[s]);
		else if (id_of(hand[s]) != id) one_chip = false;
		first = false;
		if (c == TEAM_CHECK_STAR) continue;
		if (code == TEAM_CHECK_STAR) code = c;
		else if (c != code) one_code = false;
	}
	return one_chip || one_code ? NULL : "picks of neither one code nor one chip";
}

const char *team_check_form(unsigned form, const TeamAllowed *a) {
	if (form >= 1 && form <= FORM_CROSSES) return a->crosses >> (form - 1) & 1u ? NULL : "a Cross his MegaMan lacks";
	if (form == FORM_BEAST) return a->beast ? NULL : "Beast Out, which his MegaMan lacks";
	if (form > FORM_CROSS_BEAST && form <= FORM_CROSS_BEAST + FORM_CROSSES)
		return a->beast && a->crosses >> (form - FORM_CROSS_BEAST - 1) & 1u ? NULL : "Beast Out in a Cross his MegaMan lacks";
	if (form == FORM_PROXY) return a->souls ? NULL : "UNITE, with no Soul held";
	return "a form no CROSSSELECT offers";
}

const char *team_check_soul(unsigned soul, unsigned chaos, int bit, const TeamAllowed *a, uint32_t used) {
	if (!soul || soul > SOULS || bit < 0 || bit >= TEAM_CHECK_SOULS) return "no Soul of BN5's held";
	if (!(a->souls >> bit & 1u)) return "a Soul his MegaMan does not hold";
	if (used & (1u << soul) << (chaos ? 16 : 0)) return "a Soul already united this battle";
	return NULL;
}

const char *team_check_soul_chip(unsigned chip, bool colonel, const uint16_t *hand, unsigned dealt) {
	if (!colonel) return "ColonelSoul's chip without ColonelSoul";
	if (dealt > TEAM_CHECK_DEALT) dealt = TEAM_CHECK_DEALT;
	for (unsigned i = 0; i < dealt; ++i)
		if (id_of(hand[i]) == chip) return NULL;
	return "ColonelSoul's chip not in the hand dealt";
}

/* `n` entries of chip `part` in codes in a row, '*' standing in for one code at most */
static bool in_a_row(unsigned part, const uint16_t *e, unsigned n) {
	int first = -1, stars = 0;
	for (unsigned i = 0; i < n; ++i) {
		if (id_of(e[i]) != part) return false;
		if (code_of(e[i]) == TEAM_CHECK_STAR) ++stars;
		else if (first < 0) first = (int)code_of(e[i]) - (int)i;
	}
	if (stars > 1 || first < 0 || first + (int)n > CODES) return false;
	for (unsigned i = 0; i < n; ++i)
		if (code_of(e[i]) != TEAM_CHECK_STAR && (int)code_of(e[i]) != first + (int)i) return false;
	return true;
}

static bool recipe_forms(TeamRead read, void *context, uint32_t at, unsigned id, const uint16_t *entries, unsigned took) {
	unsigned n = read(context, at, 1), kind = read(context, at + 1, 1);
	if (read(context, at + 2, 2) != id || n != took) return false;
	if (kind == KIND_IN_A_ROW) return in_a_row(read(context, at + 4, 2) & 0x1FFu, entries, n);
	if (kind != KIND_CHIPS) return false;
	for (unsigned i = 0; i < n; ++i)
		if (id_of(entries[i]) != (read(context, at + 4 + 2 * i, 2) & 0x1FFu)) return false;
	return true;
}

const char *team_check_pa(TeamRead read, void *context, const uint32_t lists[2], unsigned id, const uint16_t *entries,
	unsigned took) {
	if (took < 2 || took > TEAM_CHECK_PICKS) return "a Program Advance of no number of picks";
	for (unsigned l = 0; l < 2; ++l)
		for (unsigned i = 0; lists[l] && i < RECIPES_MAX; ++i) {
			uint32_t at = read(context, lists[l] + 4 * i, 4);
			if (!at) break;
			if (recipe_forms(read, context, at, id, entries, took)) return NULL;
		}
	return "a Program Advance its picks do not form";
}
