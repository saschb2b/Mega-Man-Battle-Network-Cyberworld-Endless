/* The Battle Lab's builder's state, shared by its course (lab.c) and its
 * picture (lab_draw.c). */
#ifndef CW_LAB_STATE_H
#define CW_LAB_STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "lab.h"
#include "lab_battle.h"

enum { LAB_PAGE_FOES, LAB_PAGE_FIELD, LAB_PAGE_STAGE, LAB_PAGE_SETS, LAB_PAGES };
/* where the cursor is: the page's list, the field's panels, the button */
enum { LAB_AT_LIST, LAB_AT_FIELD, LAB_AT_GO };
#define LAB_ROSTER 128       /* families listed at most */
#define LAB_ROWS_SHOWN 3     /* the list's rows in sight */
#define LAB_STAGE_ROWS 2     /* STAGE's rows: the background, the song */
#define LAB_LOCK 20          /* frames before it takes a key: a press that opened it is not its first */

/* A family on the FOES page */
typedef struct {
	uint8_t family;      /* a virus family's AI index, or a Navi's number */
	uint8_t navi;        /* 1: a Navi (lab_navis.h) */
	uint8_t versions;    /* the versions that can be set, a bit each; 0 while its data is not complete */
	uint8_t version;     /* the one its row shows */
	uint8_t count;       /* its deletions, to 255; a Navi's: MegaMan's wins over him */
} LabRow;

typedef struct {
	bool up;
	bool fighting;       /* its battle is on: the screen is away until it ends */
	bool all;
	int t;               /* frames since it opened or came back */
	uint32_t was;        /* the keys of the frame before */
	int page, at;        /* LAB_PAGE_*, LAB_AT_* */
	int row[LAB_PAGES], top;   /* the list's row under the cursor a page, the FOES list's first row in sight */
	int col, line;       /* the field's panel under the cursor */
	bool lifted;         /* FOES: a family lifted, to be set on a panel */
	int preset;          /* FIELD: the field of BN6's the panels were last taken from, 0 plain */
	LabRow roster[LAB_ROSTER];
	int nroster;
	LabSet set;
	LabResult last;      /* how the last battle of this visit ended (LAB_NONE: none yet) */
} LabState;

extern LabState LS;

/* The list's rows on the page shown */
int lab_rows(void);
/* Whether the set holds a Navi (who fights alone) */
bool lab_set_navi(void);
/* The foe set on a panel, -1 none */
int lab_foe_at(int col, int line);
/* The panel types a panel turns through, in order */
extern const uint8_t lab_panel_types[];
extern const int lab_panel_type_count;

#endif
