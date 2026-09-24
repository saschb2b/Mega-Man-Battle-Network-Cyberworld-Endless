/* The layers' service and choice scripts, in the game's text language. */
#ifndef CW_SCRIPTS_H
#define CW_SCRIPTS_H

#include "text.h"

/* The ScrtData key item: three open the Secret Area's gate. */
#define SCRIPTS_SECRET_DATA 0x31
/* HPMemory, and how many a beaten guardian leaves */
#define SCRIPTS_HP_MEMORY 0x70
#define SCRIPTS_BOSS_HP_MEMORIES 3

/* Service NPCs on the game's own commands: heal to full HP. (Chip Traders
 * speak the game's own lines, see trader.h.) */
int ta_heal(TextArchive *t);
/* A shopkeeper: `greeting`, then shop `shop`'s screen. */
int ta_shop(TextArchive *t, int shop, const char *greeting);

/* Choices: Yes sets event flag `flag`, which the director acts on. A
 * challenge answers only once; the gate first wants three ScrtData. */
int ta_challenge(TextArchive *t, int flag);
int ta_undernet(TextArchive *t, int flag);
int ta_secret_gate(TextArchive *t, int flag);
/* The layer's guardian Navi: Yes starts its battle. */
int ta_boss(TextArchive *t, int flag);
/* After the guardian's battle: `power` (a Cross, Beast Out; NULL for none),
 * then HPMemory through the game's own item (+20 max HP each). */
int ta_boss_reward(TextArchive *t, const char *power);

#endif
