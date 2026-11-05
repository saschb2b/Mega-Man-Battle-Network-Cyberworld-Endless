/* The JACK-IN SETUP after NEW GAME (docs/game/meta/, The run's setup): the
 * net's length, the starting folder, a Cross and the threat rung, the
 * helpers, and the extras (docs/game/meta/extras.md), the last run's
 * choices to start from, in BN6's PET look as Options (docs/engine/ui.md):
 * three pages under tabs, L and R between them, each page's rows in a
 * window with NEXT at its foot as NaviCust's RUN!, JACK IN! at the last's,
 * START jacking in from any, and the row under the cursor in the
 * information box. */
#ifndef CW_TITLE_SETUP_H
#define CW_TITLE_SETUP_H

#include <stdbool.h>

/* The choices a run starts with where no setup was shown: a short net,
 * BN6's folder, nothing else */
void title_setup_reset(void);
/* Opens it on the last run's choices, the cursor on what the last run
 * opened, else on JACK IN! */
void title_setup_open(void);
bool title_setup_shown(void);
/* A frame of its keys: true when it jacks in (the choices kept in the
 * profile, the setup closed); B closes it */
bool title_setup_update(void);
void title_setup_draw(int x0, int y0);
/* The run just made given the choices jacked in with (run_setup, run_setup_extras) */
void title_setup_apply(void);

#endif
