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

/* What the Custom screen's card shows: the chip under the cursor, the
 * Cross under CROSSSELECT's, Beast Out on its emblem, the picks on OK */
typedef enum { CARD_CHIP, CARD_CROSS, CARD_BEAST, CARD_PICKS } SecondCard;

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
	int pack_entry;               /* ... the pack's, in its list by chip ID; -1 none */
	int mega_level, giga_level;   /* the Megas and Gigas a folder may hold */
	int reg, tag[2];              /* the Regular chip's and the TagChips' entries; -1 none */
	uint16_t pack[64];            /* the pack's chips (chip | code << 9) ... */
	uint8_t pack_count[64];       /* ... their copies */
	int npack;
	/* a battle (issue #74): its Custom screen, MegaMan's hand, the enemies */
	bool custom;                  /* the Custom screen open */
	SecondCard card;              /* ... the card it shows */
	uint16_t hand[10];            /* ... its chips (chip | code << 9) */
	int nhand;
	int cursor;                   /* ... the slot under its cursor (a chip's card) */
	int cross_under;              /* ... CROSSSELECT's Cross under its cursor (navi 1-5) */
	int picks[5], npicks;         /* ... the slots picked, in order */
	uint16_t queue[BN6_HAND_MAX], queue_power[BN6_HAND_MAX], queue_bonus[BN6_HAND_MAX];   /* the chips held after OK */
	int nqueue, queue_at;         /* ... and the one up next */
	int form;                     /* MegaMan's Cross or Beast Out (BN6_BATTLE_FORM) */
	int beast_turns;              /* ... the EmotionCounter */
	bool beast;                   /* ... Beast Out his (BN6_FLAG_BEAST_OUT) */
	bool synchro;                 /* ... Full Synchro */
	int guardian;                 /* the guardian fought (navi), 0 none */
	const char *tip;              /* ... what MegaMan knows of him, chat boxes; NULL none */
	struct { uint16_t name; int hp, max_hp, element; } foe[8];
	int nfoes;
} SecondState;

extern SecondState S2;

#endif
