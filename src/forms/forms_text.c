/* forms_text.h. The box says what the row is and what it costs, never how
 * the screen works (docs/voice/rules.md, What nobody says). */
#include "forms_text.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

const char *forms_word(FormsWord w) {
	/* (in FormsWord's order) */
	static const char *const words[FT_COUNT] = { "FORMS", "CROSS", "SOUL", "CROSSSELECT", "SPARE", "INFORMATION",
		"No Crosses or Souls yet.", "FORMS", "Weak to", "Kind" };
	return (unsigned)w < FT_COUNT ? words[w] : "";
}

void forms_count_words(char *out, size_t n, int five) { snprintf(out, n, "%d/5", five); }

void forms_cross_words(char *out, size_t n, const char *name, const char *weak, const char *strong) {
	snprintf(out, n, "%s. Weak to %s.%s%s", name, weak ? weak : "nothing", strong ? "|" : "", strong ? strong : "");
}

void forms_taken_words(char *out, size_t n, const char *taken, const char *there) { snprintf(out, n, "Swap %s|for %s?", taken, there); }

void forms_soul_words(char *out, size_t n, const char *name, const char *kind) {
	bool an = kind && kind[0] && strchr("AEIOU", kind[0]);
	snprintf(out, n, "%s.|Unites with %s %s chip.", name, an ? "an" : "a", kind ? kind : "");
}

const char *const forms_bn5_soul_names[FORMS_GAME_SOULS] = {
	"ProtoSoul", "GyroSoul", "SearchSoul", "NapalmSoul", "MagnetSoul", "MeddySoul",
	"ColonelSoul", "ShadowSoul", "NumberSoul", "TomahawkSoul", "KnightSoul", "ToadSoul",
};
const char *const forms_bn5_soul_kinds[FORMS_GAME_SOULS] = {
	"Sword", "Wind", "Cursor", "Fire", "Elec", "Recovery",
	"Obstacle", "Invisible", "Plus", "Wood", "Break", "Aqua",
};
const char *const forms_bn4_soul_names[FORMS_GAME_SOULS] = {
	"RollSoul", "GutsSoul", "WindSoul", "SrchSoul", "FireSoul", "ThunSoul",
	"ProtSoul", "NumSoul", "MetlSoul", "JunkSoul", "AquaSoul", "WoodSoul",
};
const char *const forms_bn4_soul_kinds[FORMS_GAME_SOULS] = {
	"Recovery", "Panel crack", "Wind", "Invisible", "Fire", "Elec",
	"Sword", "Plus", "Break", "Obstacle", "Aqua", "Wood",
};
