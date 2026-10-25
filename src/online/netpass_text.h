/* The NetPass's PET text (netpass_text.c; docs/online/netpass.md):
 * the one screen that names which game earns which stamp. Characters
 * speak of stamps alone (docs/voice/rules.md); the pass, a PET screen, may name
 * the cartridges. */
#ifndef CW_NETPASS_TEXT_H
#define CW_NETPASS_TEXT_H

#include "netpass.h"

extern const char netpass_title[];
/* the pass's level: the Square's alone, or every stamp */
extern const char netpass_level_square[];
extern const char netpass_level_full[];
/* the operator card */
extern const char netpass_operator_label[];
extern const char netpass_operator[];
extern const char netpass_navi_label[];
extern const char netpass_navi[];
extern const char netpass_stamps_label[];
/* "1 of 4" into out */
void netpass_count_words(char *out, int n, int held, int all);
/* each stamp's name and what it opens */
extern const char *const netpass_stamp_names[STAMP_COUNT];
extern const char *const netpass_stamp_captions[STAMP_COUNT];
/* the cartridge slots' words: in, or open */
extern const char netpass_cart_in[];
extern const char netpass_cart_open[];
/* the line beside the cartridges: what the game still out (`name`, its
 * title) stamps; every game in; every game in on a system whose NetBattles
 * are not offered (the New 3DS: square_arena_offered) */
const char *netpass_missing_words(const char *name);
extern const char netpass_complete[];
extern const char netpass_complete_no_ring[];

#endif
