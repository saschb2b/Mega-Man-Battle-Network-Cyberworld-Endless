/* second_text.h. The Crosses' weaknesses and gifts are powers.c's, from
 * BN6's own Cross tutorials; the Cybeast's turns and its Attack+30 are
 * BN6's Cybeast tutorial's (bn6f TextScriptDadCybeastTut: "Each turn you
 * spend as a Cybeast will decrease your EmotionCounter by 1", tired at 0,
 * and "don't press the CybeastButton" then: he would BeastOver), Full
 * Synchro's x2 its battle tutorial's. Each verified in a battle played
 * through tools/play.py (docs/ROM_DATA.md, the second screen's battle). */
#include "second_text.h"

#include <stdio.h>

#include <string.h>

#include "bn6.h"
#include "navicust.h"
#include "powers.h"

void second_cross_lines(int navi, char *out, size_t n) {
	const char *strong = powers_cross_strength(navi), *navis = powers_cross_on_navis(navi);
	snprintf(out, n, "Weak to %s%s%s%s%s", powers_cross_weakness(navi) ? powers_cross_weakness(navi) : "?",
		strong ? "|" : "", strong ? strong : "", navis ? "|" : "", navis ? navis : "");
}

void second_beast_lines(int turns, char *out, size_t n) {
	if (turns > 0)
		snprintf(out, n, "%d turn%s as a Cybeast|Elementless attack chips that don't dim the screen: Attack+30",
			turns, turns == 1 ? "" : "s");
	else snprintf(out, n, "MegaMan is tired:|a Beast Out now ends in BeastOver");
}

/* his Cross or Beast Out, as BN6_BATTLE_FORM names it */
static int form_line(int form, int turns, SecondStateLine *out) {
	const char *cross = powers_cross_name(form > BN6_FORM_CROSS_BEAST && form < BN6_FORM_BEAST_OVER ? form - BN6_FORM_CROSS_BEAST : form);
	if (form == BN6_FORM_BEAST_OVER) {
		snprintf(out->name, sizeof out->name, "BeastOver!");
		snprintf(out->line, sizeof out->line, "The Cybeast runs wild");
	} else if (form == BN6_FORM_BEAST || (form > BN6_FORM_CROSS_BEAST && cross)) {
		snprintf(out->name, sizeof out->name, "Beast Out");
		if (turns > 0) snprintf(out->line, sizeof out->line, "%d more turn%s", turns, turns == 1 ? "" : "s");
		else snprintf(out->line, sizeof out->line, "The last turn");
	} else if (form >= 1 && cross) {
		snprintf(out->name, sizeof out->name, "%s", cross);
		snprintf(out->line, sizeof out->line, "Weak to %s", powers_cross_weakness(form));
	} else return 0;
	return 1;
}

int second_state_lines(int form, int turns, bool beast, bool synchro, SecondStateLine *out, int most) {
	int k = most > 0 ? form_line(form, turns, out) : 0;
	bool beastly = form == BN6_FORM_BEAST || (form > BN6_FORM_CROSS_BEAST && form <= BN6_FORM_BEAST_OVER);
	if (k < most && beast && turns == 0 && !beastly) {
		snprintf(out[k].name, sizeof out[k].name, "Tired");
		snprintf(out[k].line, sizeof out[k].line, "No Beast Out now!");
		++k;
	}
	if (k < most && synchro) {
		snprintf(out[k].name, sizeof out[k].name, "Full Synchro");
		snprintf(out[k].line, sizeof out[k].line, "The next chip x2");
		++k;
	}
	return k;
}

void second_program_lines(const SecondState *s, char *out, size_t n) {
	static const char *const kinds[] = { [NAVI_PART] = "program part|Goes on the command line",
		[NAVI_PLUS] = "plus part|Stays off the command line", [NAVI_EITHER] = "part|On the command line or off it" };
	const char *color = navicust_color_name(s->nc_shape.color);
	char where[40];
	if (s->nc_on == NC_LIST) snprintf(where, sizeof where, "%d to place%s", s->nc_copies, s->nc_fits ? ": it fits" : ": no room now");
	else snprintf(where, sizeof where, "%s", s->nc_on == NC_PLACED ? "On the board" : s->nc_fits ? "Held: it fits" : "Held: no room now");
	snprintf(out, n, "%c%s %s|%s|%s", *color ? *color - 'a' + 'A' : ' ', *color ? color + 1 : "",
		kinds[s->nc_shape.kind <= NAVI_EITHER ? s->nc_shape.kind : NAVI_EITHER], where,
		s->nc_turns ? "L and R turn it" : "No Spin of its colour: won't turn");
}

const char *second_program_does(int program) {
	static char does[160];
	const char *about = navicust_about(program), *colon = about ? strchr(about, ':') : NULL;
	snprintf(does, sizeof does, "%s", colon ? colon + 1 + (colon[1] == ' ') : "");
	if (*does >= 'a' && *does <= 'z') *does = (char)(*does - 'a' + 'A');
	return does;
}

const char *second_run_line(bool bug) { return bug ? "RUN now would bring a bug:" : "RUN now: no bug"; }

const char *second_board_rules(void) {
	return "Program parts on the command line, plus parts off it, no colour beside its own";
}
