/* The Arena's PET text (arena_text.h) */
#include "arena_text.h"

#include <stdio.h>

/* the panel's and the card's headings and labels */
const char arena_title[] = "NETBATTLE", arena_practice[] = "Practice", arena_vs[] = "VS";
const char arena_hp[] = "HP", arena_cross[] = "Cross", arena_soul[] = "Soul", arena_none[] = "---";
const char arena_set[] = "Set", arena_time[] = "Time", arena_hp_left[] = "HP left", arena_chips_used[] = "Chips used";
const char arena_you[] = "You", arena_other[] = "2nd GBA";
/* whose wait */
const char arena_coming[] = "Sending...", arena_linking[] = "Linking...", arena_ready[] = "Ready!";
/* the card's headline */
const char arena_won[] = "WIN", arena_lost[] = "LOSE", arena_ended[] = "OVER", arena_broke[] = "BROKE OFF", arena_stopped[] = "STOPPED";
/* the rematch, the keys */
const char arena_yes[] = "Yes", arena_no[] = "No";
const char arena_keys_ask[] = "A: choose  B: back to the Square", arena_keys_back[] = "A: back to the Square";
const char arena_back[] = "A: back to the title", arena_leave_key[] = "B: leave the NetBattle";

/* a Team NetBattle's */
const char arena_team[] = "Team", arena_standing[] = "Standing";

const char *arena_team_end(unsigned result) {
	static const char *const ends[5] = { "TEAM OVER", "TEAM WIN!", "TEAM LOSE", "TEAM BROKE OFF", "TEAM DRAW" };
	return ends[result <= 4 ? result : 0];
}

/* (each line in a buffer of its own: a screen draws one as it gets the next) */
const char *arena_team_next(const char *ours, const char *theirs) { static char s[64]; snprintf(s, sizeof s, "Next: %s vs %s", ours, theirs); return s; }
const char *arena_vs_name(const char *name) { static char s[48]; snprintf(s, sizeof s, "VS %s", name); return s; }
const char *arena_score(unsigned won, unsigned lost) { static char s[24]; snprintf(s, sizeof s, "%u - %u", won, lost); return s; }

/* the names of a mask's bits, in a row */
static const char *named(unsigned mask, const char *const *names, unsigned n, char *s, size_t size) {
	int k = 0;
	s[0] = 0;
	for (unsigned i = 0; i < n; ++i)
		if (mask >> i & 1u) k += snprintf(s + k, size - (size_t)k, "%s%s", k ? " " : "", names[i]);
	return k ? s : arena_none;
}

const char *arena_crosses_named(unsigned mask) {
	static const char *const crosses[] = { "Heat", "Elec", "Slash", "Erase", "Charge" };
	static char s[64];
	return named(mask, crosses, 5, s, sizeof s);
}

const char *arena_souls_named(unsigned mask) {
	static const char *const souls[] = { "Colonel", "Shadow", "Number", "Tomahawk", "Knight", "Toad" };
	static char s[64];
	return named(mask, souls, 6, s, sizeof s);
}

const char *arena_again_asked(const char *name) { static char s[64]; snprintf(s, sizeof s, "Again with %s?", name); return s; }
const char *arena_again_wanted(const char *name) { static char s[64]; snprintf(s, sizeof s, "%s wants another!", name); return s; }
const char *arena_again_waiting(const char *name) { static char s[64]; snprintf(s, sizeof s, "Waiting for %s...", name); return s; }
const char *arena_again_gone(const char *name) { static char s[64]; snprintf(s, sizeof s, "No rematch with %s.", name); return s; }
const char *arena_waiting_for(const char *name) { static char s[64]; snprintf(s, sizeof s, "Waiting for %s's PET...", name); return s; }

const char *arena_stop_line(ArenaStop why, const char *name) {
	static char s[96];
	switch (why) {
	case ARENA_STOP_APART: return "Communication error! The two PETs saw the battle apart.";
	case ARENA_STOP_LEFT: snprintf(s, sizeof s, "%s left the NetBattle.", name); return s;
	case ARENA_STOP_WE_LEFT: return "We left the NetBattle.";
	case ARENA_STOP_SILENT: snprintf(s, sizeof s, "%s's PET stopped answering.", name); return s;
	case ARENA_STOP_LOST: return "The link to the Net Square was lost.";
	case ARENA_STOP_NOT_READY: snprintf(s, sizeof s, "%s's PET never got ready.", name); return s;
	case ARENA_STOP_NEVER_CAME: snprintf(s, sizeof s, "%s's MegaMan never came.", name); return s;
	case ARENA_STOP_CANT: snprintf(s, sizeof s, "%s's MegaMan can't go into a NetBattle.", name); return s;
	case ARENA_STOP_INSTALLS: return "Communication error! The two PETs' programs don't match.";
	case ARENA_STOP_MEMORY: return "Not enough room in this PET for a NetBattle.";
	default: return "Communication error!";
	}
}
