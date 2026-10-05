/* What a generated map is built of (netmap.c), for its other files (netmap_learn.c, netmap_paste.c, netmap_extra.c). */
#ifndef CW_NETMAP_PARTS_H
#define CW_NETMAP_PARTS_H

#include <stdbool.h>
#include <stdint.h>

#include "decor.h"
#include "netmap.h"
#include "props.h"
#include "rom.h"
#include "tiles.h"

#define MAX_BOOKS (4 * (1 + NET_MORE_MAPS))   /* maps, their mirrors, two heights each */

typedef struct {
	bool tried, ok;
	int ex, ey, tw, th;
	TileBook book[MAX_BOOKS]; /* each source map, then its mirror image */
	int nbooks;
	TileSeams seams;          /* the tiles the maps set side by side */
	DecorBook decor;          /* the scenery of the area's maps */
	uint32_t desc, coord_slot;   /* the map its layers take over (net_area.host) */
	uint32_t src_desc;        /* the learned map's, whose tile set and colours they draw in */
	bool other;               /* its maps are another game's (docs/MULTIROM.md): src_desc is that game's */
	const NetAreaDef *def;
	StairTemplate stairs[STAIR_DIRS];
	PropStamp counter[2];     /* the Net Dealer's counter, facing FACES_X and FACES_Y */
	int counter_dx[2], counter_dy[2];   /* the world offset that sets its tiles on the layer's lattice */
	PropStamp ornament[3];    /* pads' centrepieces: the red gem, the link ring, the cube on its base */
	PropStamp bush[2];        /* Green's potted bushes, plain and in flower */
	PropStamp emblem;         /* the emblem its floors carry (the Graveyard's crosses) */
	PropStamp pad;            /* a whole pad of its maps, for its layers' (Central's framed pads) */
	PropStamp arrow[4];       /* an arrow panel of its maps for each of BN6's ways (0 +X, 1 +Y, 2 -X, 3 -Y; issue #43) */
	uint8_t rebank[2];        /* RomLayout.net_area[].rebank */
	uint8_t rebank_to[128];   /* the first-layer tiles its maps draw in rebank[1] (a bit each) */
	bool pads_seen;           /* its maps have pads, whose look its layers' pads take */
} Learned;

extern const int nm_ornament_tile[3];

/* the last tile map written, both layers (for the dev tools) */
typedef struct { uint16_t *map; uint8_t *seams; uint8_t *pasted; int tw, th; } LastMap;
extern LastMap nm_last;   /* seams: bit 0 right, bit 1 below; pasted: NETMAP_PASTED_* */

/* the current layer's placement */
typedef struct { int gx0, gy0, ex, ey; } MapPlace;
extern MapPlace nm_place;

void nm_grid_to_panel(int x, int y, int *A, int *B);
extern const NetLayout *nm_cur;

/* K_SOLID: floor drawn, walled off (a counter's aisle, net.h C_SOLID) */
enum { K_VOID, K_FLOOR, K_RAISED, K_STAIR, K_SOLID };

int nm_kind(int x, int y);
int nm_kind_at(int A, int B);

#endif
