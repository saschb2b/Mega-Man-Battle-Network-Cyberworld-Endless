/* town.c's styles (town_styles.c, docs/world/overworld.md): the original
 * each town is cut from or copied whole, its plan and its places; for
 * town.c and town_styles.c alone. */
#ifndef CW_TOWN_STYLE_H
#define CW_TOWN_STYLE_H

#include <stdbool.h>
#include <stdint.h>

#include "town.h"
#include "townsrc.h"

#define TOWN_STYLES 5
#define HOME_STYLE 0   /* Central Town, every run's (docs/world/home/) */
enum { F_JACK_IN = 1 };   /* a copied piece brings its jack-in cells */

/* What a style's plan lays out, town.c's: a rectangle of the original
 * copied, moved by (dx, dy); a strip of it repeated; a tree added; a coin
 * of the plan's seed. */
typedef struct {
	void (*copy)(int sx0, int sy0, int sx1, int sy1, int dx, int dy, int flags);
	void (*tile)(int x0, int y0, int x1, int y1, int sx, int sy, int sw, int sh);
	void (*tree)(int x, int y);
	int (*coin)(void);
} TownKit;

/* A town's style: the original it is cut from (and whose map it takes
 * over), its plan, and what is said there. Points are in the original's
 * world units, moved with the piece they lie in. */
typedef struct TownStyle {
	int group, number, song;
	unsigned jack_ins;                  /* the original's jack-in points kept: bit n for 0x40 + n */
	int front[4];                       /* the landmark's front (the original's cells x0, y0, x1, y1):
	                                     * R jacks in anywhere Lan stands there */
	int statue[4];                      /* the statue's own cells: R jacks in all round it too */
	TownMatOf mat_of;
	void (*design)(const TownKit *k);
	int start_x, start_y, start_face;   /* where Lan arrives */
	int look[TOWN_SPOTS][2];            /* places worth a look */
	bool whole;                         /* copied whole, with its own walls and heights (not the
	                                     * plan's walls round flat walkable cells) */
	const char *name, *landmark, *landmark_at;
	int port_src[2];                    /* the act's second and third ways' ports: the original's
	                                     * jack-in point (0x40 + n) or check (0xF0 + n) made one */
	int door;                           /* the original's warp trigger into Lan's house, home's front door
	                                     * (docs/world/home/: Lan jacks in from his PC there); 0 none */
	unsigned doors;                     /* its other doors kept open, bit n its warp trigger n:
	                                     * home's AsterLand and Cyber Academy (indoors.c) */
	int rom, src_group, src_number;     /* another game's town (1 + XRomId, xtown.c): its map, in BN6's (group, number) */
} TownStyle;
extern const TownStyle town_styles[TOWN_STYLES];

/* An environment variable's number, else `value` (the dev switches) */
int town_env_or(const char *name, int value);
/* Style `s`'s original learned (its own game's map) */
TownBook *town_style_learn(const TownStyle *s);
/* The BN6 map `s` takes over: its descriptor and coordinate slot (ROM
 * offsets), its tiles and colours another game's where it is one (xtown.c,
 * BN6's own put back where it is not); false where they could not be had */
bool town_style_take(const TownStyle *s, const TownBook *book, uint32_t *desc, uint32_t *coord_slot);
/* Its song, a BN6 song id (another game's copied in) */
int town_style_song(const TownStyle *s);
/* Folk `i`'s talk made by the style's own (a stop's people: oran.c), -1
 * none: then their words */
int town_style_folk(const TownStyle *s, void *text, int i, int face);

#endif
