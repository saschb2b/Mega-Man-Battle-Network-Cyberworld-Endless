/* What the game's code tells the director through hooks (hook.h; docs/
 * EMULATION.md, Hooks): the kinds of event the hooks queue, which
 * director_update takes up after each frame. */
#ifndef CW_EVENTS_H
#define CW_EVENTS_H

enum {
	EV_BATTLE_START = 1,   /* StartBattle, as any battle begins: r0 its record */
	EV_MEGAMAN_HIT,        /* MegaMan's HP lowered, in a watched battle (emu_battle_watch) */
};

#endif
