/* The second screen's state (second.c), read by its parts as they draw. */
#ifndef CW_SECOND_STATE_H
#define CW_SECOND_STATE_H

#include <stdbool.h>

/* The screen the player is on, as BN6's state tells it (docs/ROM_DATA.md,
 * the second screen's contexts) */
typedef enum {
	SECOND_DARK,         /* no game: the title, the start's screens */
	SECOND_TOWN,         /* the real world */
	SECOND_NET,          /* a layer, MegaMan on its map */
	SECOND_BATTLE,       /* a battle: its Custom screen, its action */
	SECOND_PET,          /* the PET's menu */
	SECOND_FOLDERS,      /* the PET's screens: the folders' list, */
	SECOND_EDIT,         /* ... the folder editor, */
	SECOND_NAVICUST,     /* ... the NaviCustomizer, */
	SECOND_STATUS,       /* ... MegaMan's status, */
	SECOND_LIBRARY,      /* ... the Library, */
	SECOND_MAIL,         /* ... E-Mail, */
	SECOND_KEYITEM,      /* ... KeyItem, */
	SECOND_SUBCHIP,      /* ... SubChip, */
	SECOND_COMM,         /* ... Comm, */
	SECOND_SAVE,         /* ... Save */
	SECOND_SHOP,         /* a shop */
	SECOND_TRADER,       /* the Chip Trader */
	SECOND_OTHER,        /* a screen of BN6's not named here */
	SECOND_CONTEXTS
} SecondContext;

typedef struct {
	SecondContext context;
	int since;                    /* frames since it came */
	bool town;                    /* in the real world (else on a layer) */
	int hp, max_hp;
	unsigned zenny, bugfrags;
	int depth;                    /* the run's layer */
	char area[32];                /* the place's name */
} SecondState;

extern SecondState S2;

#endif
