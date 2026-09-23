/* The layers' service and choice scripts, in the game's text language. */
#ifndef CW_SCRIPTS_H
#define CW_SCRIPTS_H

#include "text.h"

/* The ScrtData key item: three open the Secret Area's gate. */
#define SCRIPTS_SECRET_DATA 0x31

/* Service NPCs on the game's own commands: heal to full HP, the Chip Trader
 * (3 chips) and the BugFrag trader. */
int ta_heal(TextArchive *t);
int ta_chip_trader(TextArchive *t);
int ta_bug_trader(TextArchive *t);
/* A shopkeeper: `greeting`, then shop `shop`'s screen. */
int ta_shop(TextArchive *t, int shop, const char *greeting);

/* Choices: Yes sets event flag `flag`, which the director acts on. A
 * challenge answers only once; the gate first wants three ScrtData. */
int ta_challenge(TextArchive *t, int flag);
int ta_undernet(TextArchive *t, int flag);
int ta_secret_gate(TextArchive *t, int flag);
/* The layer's guardian Navi: Yes starts its battle. */
int ta_boss(TextArchive *t, int flag);

#endif
