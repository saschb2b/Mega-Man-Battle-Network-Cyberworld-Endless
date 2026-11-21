/* The engine's hooks on BN6's code, put in as the core starts (game_hooks.c),
 * for scene_emu.c. */
#ifndef CW_GAME_HOOKS_H
#define CW_GAME_HOOKS_H

/* (BN6's save's mask, marked by one of them: the scene waits through it) */
#include "save_mask.h"

/* Those the boot already needs: BN6's chip tables (xchips_install.h), game
 * calls, the idle VBlank wait, the NPCs' reach, the chat's marks, the PET's
 * entries and their text */
void game_hooks_install(void);
/* Those after it, on a booted game or a resumed run: its battles (ProtoMan's
 * cross mended among them) and its events */
void game_hooks_after_boot(void);

#endif
