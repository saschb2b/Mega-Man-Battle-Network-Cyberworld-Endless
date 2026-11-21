/* The crew on the second screen (issue #729; docs/online/shared-dive.md,
 * The crew on the second screen): while MegaMan dives in a crew, the
 * second screen's map panel gives its top to a row a crewmate, his full
 * name, HP over max HP and a bar of it, his layer where it is not
 * MegaMan's, and his link's bars where he stands on MegaMan's layer; the
 * layer's map below them. The rows are the top screen's plates' words
 * (dive_crew_view.h), more room for them. */
#ifndef CW_SECOND_CREW_H
#define CW_SECOND_CREW_H

#include <SDL.h>

/* Once a frame, while the map's panel is up: the crew copied for the draw */
void second_read_crew(void);
/* The rows at the top of `body`; what is left of it for the map */
SDL_Rect second_crew_draw(SDL_Rect body);

#endif
