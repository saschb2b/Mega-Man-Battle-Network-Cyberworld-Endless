/* The engine's hooks on BN6's code, put in as the core starts (game_hooks.c),
 * for scene_emu.c. */
#ifndef CW_GAME_HOOKS_H
#define CW_GAME_HOOKS_H

/* Those the boot already needs: game calls, the idle VBlank wait, the NPCs'
 * reach, the chat's marks, the PET's entries and their text */
void game_hooks_install(void);
/* Those after it, on a booted game or a resumed run: its battles (ProtoMan's
 * cross mended among them) and its events */
void game_hooks_after_boot(void);

#endif
