/* The crew's display on a dive (issue #717; docs/online/shared-dive.md,
 * The crew's display): while MegaMan is in a crew, a small plate a
 * crewmate under BN6's HP box on the map, each his name, his HP and max
 * HP as his game last said and a bar of it, and his layer where it is
 * not MegaMan's; and MegaMan's own HP and max HP told to the server as
 * they change (DIVE vitals, protocol v24), which tells his crew alone.
 * Alone, or with the link down, nothing. Our own drawing over the map;
 * it changes nothing. In a party's battle a partner's HP stands under his
 * name on the field instead (party_marks.h). */
#ifndef CW_DIVE_CREW_VIEW_H
#define CW_DIVE_CREW_VIEW_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* A dive's frame: `linked` the link up; `me` this game's session; `now`
 * SDL_GetTicks's. MegaMan's HP sent where it changed, ONLINE_CREW_VITALS_MS
 * apart at most */
void dive_crew_view_frame(bool linked, uint32_t me, uint32_t now);
/* Over the map (at P.core_x, P.core_y) */
void dive_crew_view_draw(void);
/* ... in the state's words: `crewview N` (the plates shown) and a line a
 * plate, `mateK NAME hp H/M layer D|here` */
void dive_crew_view_describe(FILE *f);

#endif
