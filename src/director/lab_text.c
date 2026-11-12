/* The Battle Lab's builder's words (lab_text.h). */
#include "lab_text.h"

#include <stdio.h>

#include "bn6.h"
#include "bn6_panels.h"

const char lab_title[] = "BATTLE LAB";
const char lab_go[] = "BATTLE!";
const char *const lab_tabs[4] = { "FOES", "FIELD", "STAGE", "SETS" };
const char lab_row_preset[] = "Preset";
const char lab_row_area[] = "Background";
const char lab_row_song[] = "Song";
const char lab_own_song[] = "Battle's own";
const char lab_plain_field[] = "Plain";

const char *lab_panel_name(int type) {
	static const char *const names[BN6_PANEL_TYPES] = {
		[BN6_PANEL_HOLE] = "Hole", [BN6_PANEL_PLAIN] = "Plain", [BN6_PANEL_CRACKED] = "Cracked", [BN6_PANEL_POISON] = "Poison",
		[BN6_PANEL_HOLY] = "Holy", [BN6_PANEL_GRASS] = "Grass", [BN6_PANEL_ICE] = "Ice", [BN6_PANEL_VOLCANO] = "Volcano",
		[BN6_PANEL_CONVEYOR_UP] = "Conveyor up", [BN6_PANEL_CONVEYOR_DOWN] = "Conveyor down",
		[BN6_PANEL_CONVEYOR_LEFT] = "Conveyor left", [BN6_PANEL_CONVEYOR_RIGHT] = "Conveyor right",
	};
	return type >= 0 && type < BN6_PANEL_TYPES && names[type] ? names[type] : "";
}

const char *lab_version_word(int version) {
	static const char *const words[6] = { "V1", "V2", "V3", "SP", "Rare", "Rare2" };
	return version >= 0 && version < 6 ? words[version] : "";
}

void lab_count_words(char *out, size_t n, int foes, int most) { snprintf(out, n, "%d/%d", foes, most); }

void lab_preset_words(char *out, size_t n, int field) { snprintf(out, n, "Field %d", field); }

void lab_info_family(char *out, size_t n, int hp, int deleted) { snprintf(out, n, "HP %d. Deleted %d.", hp, deleted); }

void lab_info_gauge(char *out, size_t n, int deleted, int complete) { snprintf(out, n, "Battle data %d of %d.", deleted, complete); }

void lab_info_lifted(char *out, size_t n, const char *name, int foes, int most) {
	(void)name;
	if (foes >= most) snprintf(out, n, "Take one off first.");
	else snprintf(out, n, "Set it on a panel.");
}

void lab_info_panel(char *out, size_t n, const char *panel, const char *foe) { snprintf(out, n, "%s.", foe && foe[0] ? foe : panel); }

void lab_info_empty(char *out, size_t n) { snprintf(out, n, "No foes are set."); }

void lab_info_result(char *out, size_t n, const LabResult *r) {
	int cs = r->frames * 100 / 60, m = cs / 6000, sec = cs / 100 % 60;
	cs %= 100;
	if (r->end == LAB_WON && r->bust >= 11) snprintf(out, n, "%d:%02d:%02d. Lv S. HP -%d.", m, sec, cs, r->hp_lost);
	else if (r->end == LAB_WON) snprintf(out, n, "%d:%02d:%02d. Lv %d. HP -%d.", m, sec, cs, r->bust, r->hp_lost);
	else if (r->end == LAB_DELETED) snprintf(out, n, "Deleted at %d:%02d:%02d.", m, sec, cs);
	else snprintf(out, n, "Left at %d:%02d:%02d.", m, sec, cs);
}

void lab_set_words(char *out, size_t n, int slot) { snprintf(out, n, "%d", slot + 1); }

void lab_set_value(char *out, size_t n, int foes) {
	if (!foes) snprintf(out, n, "Empty");
	else if (foes == 1) snprintf(out, n, "1 foe");
	else snprintf(out, n, "%d foes", foes);
}

void lab_info_set(char *out, size_t n, int foes, int bust, int frames, int hp_lost) {
	int cs = frames * 100 / 60, m = cs / 6000, sec = cs / 100 % 60;
	cs %= 100;
	if (!foes) snprintf(out, n, "Nothing set here.");
	else if (!bust) snprintf(out, n, "No win yet.");
	else if (bust >= 11) snprintf(out, n, "Best %d:%02d:%02d. Lv S. HP -%d.", m, sec, cs, hp_lost);
	else snprintf(out, n, "Best %d:%02d:%02d. Lv %d. HP -%d.", m, sec, cs, bust, hp_lost);
}

const char *lab_navi_word(int kind, int version) {
	static const char *const words[3][6] = { { "V1", "EX", "SP" }, { "V1", "V2", "V3", "SP" }, { "1", "2", "3", "4", "5", "6" } };
	const char *w = kind >= 0 && kind < 3 && version >= 0 && version < 6 ? words[kind][version] : NULL;
	return w ? w : "";
}

void lab_info_navi(char *out, size_t n, int hp, int wins) { snprintf(out, n, "HP %d. Beaten %d.", hp, wins); }

void lab_info_alone(char *out, size_t n) { snprintf(out, n, "A Navi fights alone."); }

