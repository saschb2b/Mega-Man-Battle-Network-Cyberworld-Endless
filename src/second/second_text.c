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
#include "chip_pool.h"
#include "guardians.h"
#include "jobs.h"
#include "meta.h"
#include "navicust.h"
#include "net.h"
#include "powers.h"
#include "run.h"
#include "save.h"
#include "shop.h"
#include "trader.h"

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

void second_home_next(int where, const char *landmark_at, char *out, size_t n) {
	if (where == SECOND_HOME_HP) snprintf(out, n, "The pink pad leads on. R jacks out to Lan's room");
	else if (where == SECOND_HOME_ROOM) snprintf(out, n, "R at Lan's PC jacks MegaMan in");
	else if (where == SECOND_HOME_UP) snprintf(out, n, "Lan's PC, up in his room, jacks MegaMan in");
	else snprintf(out, n, "R at the %s jacks MegaMan in", landmark_at && *landmark_at ? landmark_at : "port");
}

void second_home_setup(char *out, size_t n) {
	const FolderInfo *f = meta_folder(run.folder);
	int k = snprintf(out, n, "%s net", run.mode == RUN_SHORT ? "Short" : "Endless");
	if (k > 0 && (size_t)k < n && run.threat) k += snprintf(out + k, n - (size_t)k, ", threat %d", run.threat);
	if (k > 0 && (size_t)k < n && f && f->name) k += snprintf(out + k, n - (size_t)k, "|%s folder", f->name);
	if (k > 0 && (size_t)k < n && powers_cross_name(run.cross)) snprintf(out + k, n - (size_t)k, ", %s", powers_cross_name(run.cross));
}

void second_home_clock(char *out, size_t n) {
	if (!run.clock) { if (n) out[0] = 0; return; }
	snprintf(out, n, "%d notch%s: guardians' HP +%d%%", run.clock, run.clock == 1 ? "" : "es", run.clock * RUN_CLOCK_PERCENT);
}

void second_home_ways(const DirectorWay *ways, int n, char *out, size_t size) {
	size_t k = 0;
	int backs = 0;
	if (size) out[0] = 0;
	for (int i = 0; i < n && k + 1 < size; ++i) {
		const char *area = guardian_area_in_text(ways[i].biome, LAYER_NORMAL);
		const char *who = ways[i].navi && guardian_known(ways[i].navi) ? guardian(ways[i].navi)->name : "???";
		/* (on or back, as Lan's HP's tags tell them: issue #110; the ways
		 * back on one line, which the panel cut as a fourth, session 73) */
		int w = !ways[i].back ? snprintf(out + k, size - k, "%sOn: %s (%s), %s", k ? "|" : "", area, who, i ? "link" : "pink pad")
			: backs++ ? snprintf(out + k, size - k, ", %s", area) : snprintf(out + k, size - k, "%sBack: %s", k ? "|" : "", area);
		if (w < 0 || (size_t)w >= size - k) break;
		k += (size_t)w;
	}
	if (backs && k + 1 < size) snprintf(out + k, size - k, ", a notch%s", backs > 1 ? " each" : "");
}

void second_home_job(char *out, size_t n) {
	static const char *const who[JOB_ASKERS] = { "NetBattler", "The club", "The lab" };
	static const char *const elements[] = { "", "Fire", "Aqua", "Elec", "Wood" };
	const Job *j = &run.job;
	if (!j->kind || j->asker >= JOB_ASKERS) { if (n) out[0] = 0; return; }
	char what[64];
	switch (j->kind) {
	case JOB_CLEAN: snprintf(what, sizeof what, "%u wins without damage, %u so far", j->need, j->got); break;
	case JOB_QUICK: snprintf(what, sizeof what, "%u wins in 10 s each, %u so far", j->need, j->got); break;
	case JOB_EXPLORE: snprintf(what, sizeof what, "every Mystery Data of a layer"); break;
	case JOB_BRING: snprintf(what, sizeof what, "%s chip from the Pack", j->need <= 4 ? elements[j->need] : "a"); break;
	default: snprintf(what, sizeof what, "no Mr.Prog patch-ups"); break;
	}
	snprintf(out, n, "%s: %s%s", who[j->asker], what, j->state == JOB_DONE ? ". Done: paid at home" : j->state == JOB_FAILED ? ". Broken" : "");
}

const char *second_hour_name(int hour) {
	static const char *const names[4] = { "Morning", "Afternoon", "Evening", "Night" };
	return names[hour < 0 ? 0 : hour > 3 ? 3 : hour];
}

/* rows' values as numbers */
static int rows_of(SecondRow *out, int most, const char *const *names, const int *values, int n, int keep) {
	int k = 0;
	for (int i = 0; i < n && k < most; ++i)
		if (values[i] || i < keep) {
			out[k].name = names[i];
			snprintf(out[k++].value, sizeof out[0].value, "%d", values[i]);
		}
	return k;
}

int second_record(SecondRow *out, int most) {
	static const char *const names[] = { "Runs", "Best layer", "Guardians beaten", "Viruses deleted", "Short nets won" };
	const int values[] = { profile.runs, profile.best_depth, profile.bosses, profile.viruses, profile.short_wins };
	return rows_of(out, most, names, values, 5, 2);
}

int second_status(SecondRow *out, int most, int max_hp, int base_hp) {
	static const char *const names[] = { "Layer", "Guardians beaten", "Viruses deleted", "Max HP", "Programs' HP" };
	const int values[] = { run_reached(), run.bosses_beaten, run.viruses_deleted, max_hp, max_hp - base_hp };
	int k = rows_of(out, most, names, values, 5, 4);
	if (k && !strcmp(out[k - 1].name, "Programs' HP")) snprintf(out[k - 1].value, sizeof out[0].value, "+%d", max_hp - base_hp);
	if (k < most && powers_cross_name(run.cross)) {
		out[k].name = "Cross brought";
		snprintf(out[k++].value, sizeof out[0].value, "%s", powers_cross_name(run.cross));
	}
	return k;
}

int second_library(SecondRow *out, int most) {
	/* (BN6's own tabs' names: two to a line beside the Library's card) */
	static const char *const cls[] = { "StdChip", "MegaChip", "GigaChip" };
	int k = 0;
	for (int c = 0; c < 3 && k < most; ++c) {
		out[k].name = cls[c];
		snprintf(out[k++].value, sizeof out[0].value, "%d/%d", meta_library_count(c), chip_pool_class_count(c));
	}
	if (k < most) {
		out[k].name = "New this run";
		snprintf(out[k++].value, sizeof out[0].value, "%d", meta_library_new());
	}
	return k;
}

const char *second_library_line(void) { return "Every chip MegaMan held in any run stays in the Library, and each run begins with it"; }

int second_mail_rows(SecondRow *out, int most, const char *subject, int unread, int shown) {
	int k = 0;
	if (k < most && *subject) {
		out[k].name = "Subject";
		snprintf(out[k++].value, sizeof out[0].value, "%s", subject);
	}
	if (k < most) {
		out[k].name = "New mail";
		snprintf(out[k++].value, sizeof out[0].value, "%d of %d", unread, shown);
	}
	return k;
}

const char *second_mail_none(void) { return "No mail"; }

const char *second_accessing(void) { return "ACCESSING"; }

void second_layer_next(int guardian_navi, char *out, size_t n) {
	/* (a super boss unnamed until met, as MegaMan senses him: docs/
	 * BOSSES.md, Super bosses) */
	if (super_boss(guardian_navi) && !guardian_known(guardian_navi))
		snprintf(out, n, "%s waits at the end", guardian_navi == SUPER_BASS ? "A dark signal" : "Something huge");
	else if (guardian_navi) snprintf(out, n, "The exit opens once %s is deleted", guardian(guardian_navi)->name);
	else snprintf(out, n, "Find the exit pad");
}

/* (on the 3DS BN5 is never looked for, its memory holding no second core:
 * "not found beside BN6" sent a player to check his ROM, issue #107) */
const char *second_bn5_line(bool found) {
#ifdef __3DS__
	(void)found;
	return "BN5's older net: on PC and phones, not the 3DS";
#else
	return found ? "BN5 found: the older net joins the runs" : "BN5 not found beside BN6";
#endif
}

/* (where a chip's copies are, a shop's and the Library's) */
static const char *const copies_in[2] = { "In the folder", "In the pack" };

int second_copies(SecondRow *out, int most, int folder, int pack) {
	const int values[2] = { folder, pack };
	return rows_of(out, most, copies_in, values, 2, 2);
}

void second_held_lines(const SecondState *s, char *out, size_t n) {
	if (s->sh_kind == BN6_SHOP_KIND_CHIP) snprintf(out, n, "%s: %d|%s: %d", copies_in[0], s->sh_folder, copies_in[1], s->sh_pack);
	else if (s->sh_kind == BN6_SHOP_KIND_PROGRAM) snprintf(out, n, "Held: %d|On the board: %d", s->sh_held, s->sh_placed);
	else if (s->sh_kind == BN6_SHOP_KIND_ITEM) snprintf(out, n, "Held: %d", s->sh_held);
	else if (n) *out = 0;
}

/* (BN6's own trader lines: "Insert 3 BtlChips?", the Special's 10, the
 * BugFrag Trader's 10 BugFrags, each from the Pack; the prize a chip new
 * to the Library, trader.c) */
const char *second_trader_name(int kind) {
	return kind == TRADER_BUGFRAG ? "BugFrag Trader" : kind == TRADER_SPECIAL ? "Chip Trader Special" : "Chip Trader";
}

void second_trader_lines(int kind, int pack_chips, char *out, size_t n) {
	if (kind == TRADER_BUGFRAG) snprintf(out, n, "Takes 10 BugFrags|Gives a chip new to the Library");
	else snprintf(out, n, "Takes %d chips from the Pack|Gives a chip new to the Library|The Pack holds %d", kind == TRADER_SPECIAL ? 10 : 3, pack_chips);
}

/* (the run's own: docs/PROGRESSION.md's 20-HP HPMemory, shop.h's keys) */
const char *second_item_does(int id) {
	switch (id) {
	case 0x70: return "Max HP +20, kept for the run";
	case ITEM_RUSH_FOOD: return "Calls Rush over a gap: one held for each of its panels, one eaten";
	case ITEM_WWW_ID: return "Opens every skull door of the Undernet";
	case SUB_UNLOCKER: return "Opens a purple Mystery Data";
	default: return "";
	}
}
