/* reward_pick_text.h. The screen speaks as the PET's own screens do: a
 * name on the band, and the information box in KeyItem's voice, a fact a
 * line (docs/voice/rules.md); what a program does is MegaMan's words of
 * the NaviCust (navicust_words.c), a chip's its card's, BN6's own. */
#include "reward_pick_text.h"

#include <stdio.h>

#include "navicust.h"
#include "reward_pick.h"

const char *reward_pick_title(int kind) {
	static const char *const titles[PICK_KINDS] = { "PROGRAM DATA", "VAULT", "CHIP ORDER", "OFFICIAL VAULT", "SERVER DATA", "GLITCH DATA", "GIFT" };
	return kind >= 0 && kind < PICK_KINDS ? titles[kind] : "";
}

const char *reward_pick_label(void) { return "INFORMATION"; }

const char *reward_pick_part(int part) { return part == NAVI_PLUS ? "PLUS PART" : part == NAVI_EITHER ? "ANY PART" : "PROG PART"; }

const char *reward_pick_items(void) { return "ITEMS"; }

const char *reward_pick_new(void) { return "NEW"; }

void reward_pick_hp_info(char *out, size_t n, int count, int max_hp) { snprintf(out, n, "Max HP %d now.|%d with these.", max_hp, max_hp + count * 20); }

void reward_pick_skip_info(char *out, size_t n, int frags) { snprintf(out, n, "The data breaks into %d BugFrags.", frags); }

const char *reward_pick_skip_name(void) { return "BugFrags"; }

const char *reward_pick_hp_name(void) { return "HPMemory"; }
