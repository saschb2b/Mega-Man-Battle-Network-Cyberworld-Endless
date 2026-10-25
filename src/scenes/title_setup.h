/* The JACK-IN SETUP after NEW GAME (docs/game/meta/, The run's setup): the
 * net's length, the starting folder, a Cross, the threat rung and the
 * helpers, the last run's choices to start from, in BN6's PET look as
 * Options (docs/engine/ui.md): the rows in a window, JACK IN! as NaviCust's RUN!
 * at its foot, the row under the cursor in the information box, and the
 * helpers on a page of their own as Options' sections. */
#ifndef CW_TITLE_SETUP_H
#define CW_TITLE_SETUP_H

#include <stdbool.h>

typedef struct {
	int net, folder, cross, threat, helpers;   /* run_setup's */
} SetupChoice;

/* The choices a run starts with where no setup was shown: a short net,
 * BN6's folder, nothing else */
void title_setup_reset(void);
/* Opens it on the last run's choices, the cursor on what the last run
 * opened, else on JACK IN! */
void title_setup_open(void);
bool title_setup_shown(void);
/* A frame of its keys: true when it jacks in (the choices kept in the
 * profile, the setup closed); B on the rows closes it */
bool title_setup_update(void);
void title_setup_draw(int x0, int y0);
const SetupChoice *title_setup_choice(void);

#endif
