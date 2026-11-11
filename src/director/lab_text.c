/* The Battle Lab's builder's words (lab_text.h). */
#include "lab_text.h"

#include <stdio.h>

#include "bn6.h"
#include "bn6_panels.h"

const char lab_title[] = "BATTLE LAB";
const char lab_go[] = "BATTLE!";
const char lab_info_label[] = "INFORMATION";
const char *const lab_tabs[3] = { "FOES", "FIELD", "STAGE" };
const char lab_row_preset[] = "Start from";
const char lab_row_area[] = "Background";
const char lab_row_song[] = "Song";
const char lab_own_song[] = "The battle's own";
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
	if (foes >= most) snprintf(out, n, "%d foes are set. Take one off first.", most);
	else snprintf(out, n, "%s: set it on a panel.", name);
}

void lab_info_panel(char *out, size_t n, const char *panel, const char *foe) {
	if (foe && foe[0]) snprintf(out, n, "%s. %s stands here.", panel, foe);
	else snprintf(out, n, "%s.", panel);
}

void lab_info_empty(char *out, size_t n) { snprintf(out, n, "No foes are set."); }

void lab_info_result(char *out, size_t n, const LabResult *r) {
	int cs = r->frames * 100 / 60, m = cs / 6000, sec = cs / 100 % 60;
	cs %= 100;
	if (r->end == LAB_WON && r->bust >= 11) snprintf(out, n, "Time %d:%02d:%02d. Level S. HP lost %d.", m, sec, cs, r->hp_lost);
	else if (r->end == LAB_WON) snprintf(out, n, "Time %d:%02d:%02d. Level %d. HP lost %d.", m, sec, cs, r->bust, r->hp_lost);
	else if (r->end == LAB_DELETED) snprintf(out, n, "MegaMan fell at %d:%02d:%02d.", m, sec, cs);
	else snprintf(out, n, "Left at %d:%02d:%02d. HP lost %d.", m, sec, cs, r->hp_lost);
}
