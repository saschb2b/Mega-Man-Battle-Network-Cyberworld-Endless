/* The second screen's state (second.c), read by its parts as they draw. */
#ifndef CW_SECOND_STATE_H
#define CW_SECOND_STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "bn6.h"

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
	/* the folders and the editor (issue #75) */
	uint16_t folder[BN6_FOLDER_ENTRIES];   /* chip | code << 9 */
	bool pack_side;               /* the editor on the pack's side */
	int entry;                    /* the folder's entry under the cursor; -1 none */
	int mega_level, giga_level;   /* the Megas and Gigas a folder may hold */
	int reg, tag[2];              /* the Regular chip's and the TagChips' entries; -1 none */
	uint16_t pack[64];            /* the pack's chips (chip | code << 9) ... */
	uint8_t pack_count[64];       /* ... their copies */
	int npack;
} SecondState;

extern SecondState S2;

#endif
