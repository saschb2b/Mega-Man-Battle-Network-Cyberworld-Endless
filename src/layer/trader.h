/* Chip Traders on the game's own trade screen and text (bn6f
 * TextScriptChipTrader): the screen takes its prizes and its lines from
 * tables keyed by the map it stands on, which the layer's map takes over. */
#ifndef CW_TRADER_H
#define CW_TRADER_H

/* The trader's script in BN6_TRADER_TEXT, which is also its kind. */
typedef enum {
	TRADER_CHIPS = 0,     /* 3 chips for 1 */
	TRADER_SPECIAL = 6,   /* Chip Trader Special: 10 chips for 1 */
	TRADER_BUGFRAG = 12,  /* 10 BugFrags for 1 */
} TraderKind;

/* Makes the trade screen on map (group, number) serve `kind`, with prizes
 * for layer `depth`. One trader per map. */
void trader_install(int group, int number, TraderKind kind, int depth);

#endif
