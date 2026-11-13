/* Where a layer's breaks lie (break_places.c, docs/devtools/tile-test.md,
 * Where the breaks lie): the breaks in the last layer's picture, each by
 * the kind of place on the layer's floor its tile lies on. */
#ifndef CW_BREAK_PLACES_H
#define CW_BREAK_PLACES_H

/* A room's inside (floor all round), its edge, its corner (void on two
 * sides that meet) and its notch (void at a corner alone); where a walkway
 * and a room meet (the last panel of each); a walkway's straight run, its
 * bend, its fork (three ways or four) and its end; a pad; raised floor and
 * stairs. */
enum { BP_ROOM_IN, BP_ROOM_EDGE, BP_ROOM_CORNER, BP_ROOM_NOTCH, BP_JOIN, BP_WALK_RUN, BP_WALK_BEND, BP_WALK_FORK, BP_WALK_END, BP_PAD, BP_RAISED, BP_PLACES };

/* The last layer built: its breaks by place (a break off the floor counts
 * for the floor panel beside it) and its panels by place. */
typedef struct { int breaks[BP_PLACES], cells[BP_PLACES]; } BreakPlaces;
void break_places(BreakPlaces *out);
extern const char *const break_place_names[BP_PLACES];

#endif
