/* The crew's display on a dive (issue #717; docs/online/shared-dive.md,
 * The crew's display): while MegaMan is in a crew, a small plate a
 * crewmate under BN6's HP box on the map, each his name, his HP and max
 * HP as his game last said and a bar of it, and his layer where it is
 * not MegaMan's; and MegaMan's own HP and max HP told to the server as
 * they change (DIVE vitals, protocol v24), which tells his crew alone.
 * At a guardian's door while a party forms there, the plates list the
 * party instead, its leader first (issue #753): a crewmate as ever,
 * another diver his name alone, in white. Alone, or with the link down,
 * nothing. Our own drawing over the map;
 * it changes nothing. In a party's battle a partner's HP stands under his
 * name on the field instead (party_marks.h). */
#ifndef CW_DIVE_CREW_VIEW_H
#define CW_DIVE_CREW_VIEW_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "protocol.h"

/* A dive's frame: `linked` the link up; `me` this game's session; `now`
 * SDL_GetTicks's. MegaMan's HP sent where it changed, ONLINE_CREW_VITALS_MS
 * apart at most */
void dive_crew_view_frame(bool linked, uint32_t me, uint32_t now);
/* The crewmates shown, in the order they came, or the door's party (none
 * alone or with the link down), each as the plates write him; the second screen's crew
 * rows read them too (second_crew.h) */
typedef struct {
	uint32_t session;
	bool crewmate;                      /* in MegaMan's crew (else a diver of the door's party alone, his name only) */
	char full[ONLINE_NAME_BYTES + 1];   /* his name */
	char name[ONLINE_NAME_BYTES + 1];   /* ... cut to a plate (party_marks_cut) */
	char hp[12];                        /* "120/200", "--" not told yet */
	char layer[6];                      /* "L3", "" on MegaMan's */
	uint16_t now, most;                 /* his HP and max HP (0 0 not told) */
} CrewRow;
int dive_crew_view_rows(CrewRow out[ONLINE_PARTY_MOST]);
/* a row's HP as a bar w wide at (x, y), a pixel high: orange from a quarter down */
void dive_crew_view_bar(int x, int y, int w, const CrewRow *row);
/* Over the map (at P.core_x, P.core_y) */
void dive_crew_view_draw(void);
/* ... in the state's words: `crewview N` (the plates shown; `crewview N
 * party` the door's party) and a line a plate, `mateK NAME hp H/M|--|none
 * layer D|here`, ` stranger` after a diver outside the crew */
void dive_crew_view_describe(FILE *f);

#endif
