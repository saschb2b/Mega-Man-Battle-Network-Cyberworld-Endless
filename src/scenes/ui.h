/* Modal UI shared by the overworld: messages, menus and chip choices. */
#ifndef UI_H
#define UI_H

#include <stdbool.h>

#include "gfx.h"

#define UI_MAX_ITEMS 12

typedef struct {
	char label[40];
	char detail[40];
	int chip;       /* chip id to show as icon/art, or -1 */
	char code;
	bool disabled;
} UiItem;

typedef void (*UiMenuDone)(int choice); /* -1 = cancelled */

/* Queue a message. mugshot < 0 for none. */
void ui_message(const char *text, int mugshot);
void ui_messagef(int mugshot, const char *fmt, ...);
/* Show a menu; done is called with the chosen index. */
void ui_menu(const char *title, const UiItem *items, int n, bool cancellable, UiMenuDone done);
/* Chip cards side by side (rewards, trades). */
void ui_cards(const char *title, const UiItem *items, int n, bool cancellable, UiMenuDone done);

/* The original FOLDER EDIT screen: chip card on the left, the list on the
 * right. `marked` rows show as picked (used by the Chip Trader). */
void ui_folder(const char *title, const uint16_t *ids, const char *codes, int n, const bool *marked, bool cancellable, UiMenuDone done);

typedef struct {
	int hp, max_hp, zenny, bugfrags;
	char place[32];
} PetInfo;

/* The PET menu: ChipFolder, SubChip, Library, MegaMan, E-Mail, KeyItem,
 * Comm, Save. done() gets the item index, -1 when closed. */
void ui_pet(const PetInfo *info, int start, UiMenuDone done);

bool ui_active(void);
/* True while a full-screen menu (PET, folder) covers the scene. */
bool ui_fullscreen(void);
void ui_update(void);
void ui_draw(void);
void ui_clear(void);

/* Box drawn in the game's menu colours. */
void ui_box(int x, int y, int w, int h);

#endif
