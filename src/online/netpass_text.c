#include "netpass_text.h"

#include <stdio.h>

const char netpass_title[] = "NetPass";
const char netpass_level_square[] = "Square pass";
const char netpass_level_full[] = "Full pass";

const char netpass_operator_label[] = "OPERATOR";
const char netpass_operator[] = "Lan";
const char netpass_navi_label[] = "NAVI";
const char netpass_navi[] = "MegaMan";
const char netpass_stamps_label[] = "STAMPS";

void netpass_count_words(char *out, int n, int held, int all) { snprintf(out, (size_t)n, "%d of %d", held, all); }

const char *const netpass_stamp_names[STAMP_COUNT] = { "Square", "Arena", "Trade", "DeepNet" };
const char *const netpass_stamp_captions[STAMP_COUNT] = { "meet,cheer", "NetBattle", "chip swap", "the Board" };

const char netpass_cart_in[] = "IN";
const char netpass_cart_open[] = "OPEN";

const char *netpass_missing_words(const char *name) {
	static char s[96];
	snprintf(s, sizeof s, "%s's ROM stamps Arena, Trade and DeepNet", name);
	return s;
}

const char netpass_complete[] = "Every game in. The whole Net is yours.";
const char netpass_complete_no_ring[] = "Every game in! NetBattles need a PC or a phone.";
