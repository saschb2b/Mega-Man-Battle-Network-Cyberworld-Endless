/* What MegaMan and Lan say coming home after an act (home_words.c,
 * docs/HOME.md, docs/VOICE.md). */
#ifndef CW_HOME_WORDS_H
#define CW_HOME_WORDS_H

#include <stdbool.h>

/* Their words on arriving home, `beaten` the guardian just deleted (NULL
 * unknown), the run at the next act's first layer; its ports open,
 * `ports[0..ways)` where each is ("squirrel statue"), and `sealed` where a
 * dark way's stands shut (NULL none). */
const char *home_words(const char *beaten, int ways, const char *const ports[3], const char *sealed);
/* MegaMan at a port (docs/HOME.md): where it leads, area `biome` and
 * `navi` its guardian (named where MegaMan has battled him); `dark` the
 * dark way into the Undernet, `sealed` that way still shut. */
const char *home_port_words(int biome, int navi, bool dark, bool sealed);

#endif
