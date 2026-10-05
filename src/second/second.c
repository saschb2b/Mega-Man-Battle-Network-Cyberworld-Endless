/* second.h. The screen the player is on, from BN6's state once a frame
 * (docs/ROM_DATA.md, the second screen's contexts), and the panel drawn
 * for it in the PET's frame (issue #72): the map on a layer, as Operate
 * Shooting Star keeps it up, the folder, the battle, the NaviCustomizer,
 * MegaMan's status, the Library, and the PET at home in the town and on
 * the PET's other screens; second_read.c reads each one's facts. */
#include "second.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "director.h"
#include "emu.h"
#include "guardians.h"
#include "guest.h"
#include "platform.h"
#include "run.h"
#include "second_battle.h"
#include "second_folder.h"
#include "second_frame.h"
#include "second_home.h"
#include "second_navicust.h"
#include "second_pet.h"
#include "second_read.h"
#include "second_shop.h"
#include "second_state.h"
#include "town.h"

SecondState S2;

/* What the second screen draws: a panel, which several screens may share */
typedef enum { PANEL_DARK, PANEL_MAP, PANEL_FOLDER, PANEL_BATTLE, PANEL_NAVICUST, PANEL_HOME, PANEL_STATUS, PANEL_LIBRARY, PANEL_SHOP, PANEL_TRADER } Panel;
static Panel panel;
static int panel_since;

/* A PET screen by the first byte of its state */
static SecondContext pet_screen(int screen) {
	switch (screen) {
	case BN6_SUBMENU_FOLDERS: return SECOND_FOLDERS;
	case BN6_SUBMENU_EDIT: return SECOND_EDIT;
	case BN6_SUBMENU_NAVICUST: return SECOND_NAVICUST;
	case BN6_SUBMENU_STATUS: return SECOND_STATUS;
	case BN6_SUBMENU_LIBRARY: return SECOND_LIBRARY;
	case BN6_SUBMENU_MAIL: return SECOND_MAIL;
	case BN6_SUBMENU_KEYITEM: return SECOND_KEYITEM;
	case BN6_SUBMENU_SUBCHIP: return SECOND_SUBCHIP;
	case BN6_SUBMENU_COMM: return SECOND_COMM;
	case BN6_SUBMENU_SAVE: return SECOND_SAVE;
	default: return SECOND_OTHER;
	}
}

static SecondContext context_now(void) {
	if (!director_on_layer() && !director_in_town()) return SECOND_DARK;
	if (guest_active()) return SECOND_BATTLE;
	switch (emu_read8(emu_read32(BN6_TOOLKIT))) {
	case BN6_MODE_SUBMENU: return pet_screen(emu_read8(emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_SUBMENU)));
	case BN6_MODE_SHOP: return SECOND_SHOP;
	case BN6_MODE_TRADER: return SECOND_TRADER;
	case BN6_MODE_MAIL: return SECOND_MAIL;
	case BN6_MODE_GAME: break;
	default: return SECOND_OTHER;
	}
	int sub = emu_read8(BN6_GAMESTATE);
	if (sub == BN6_SUB_PET) return SECOND_PET;
	if (sub == BN6_SUB_BATTLE || sub == BN6_SUB_BATTLE_INIT) return SECOND_BATTLE;
	/* (a trader talks on the map, from BN6's trader archive: a BugFrag
	 * Trader's whole trade, a Chip Trader's before its screen) */
	if (emu_read8(BN6_CHATBOX) && emu_read32(BN6_CHATBOX_ARCHIVE) == BN6_TRADER_TEXT) return SECOND_TRADER;
	return director_in_town() ? SECOND_TOWN : SECOND_NET;
}

/* The panel for a screen: its own where it has one; the PET at home on
 * its menu and its other screens and in the town; else the map */
static Panel panel_for(SecondContext c) {
	switch (c) {
	case SECOND_FOLDERS: case SECOND_EDIT: return PANEL_FOLDER;
	case SECOND_NAVICUST: return PANEL_NAVICUST;
	case SECOND_STATUS: return PANEL_STATUS;
	case SECOND_LIBRARY: return PANEL_LIBRARY;
	case SECOND_SHOP: return PANEL_SHOP;
	case SECOND_TRADER: return PANEL_TRADER;
	case SECOND_PET: case SECOND_MAIL: case SECOND_KEYITEM: case SECOND_SUBCHIP: case SECOND_COMM: case SECOND_SAVE: return PANEL_HOME;
	case SECOND_DARK: return PANEL_DARK;
	/* (an older net's battle runs in BN5's memory: the map then) */
	case SECOND_BATTLE: if (!guest_active()) return PANEL_BATTLE; break;
	default: break;
	}
	return S2.town ? PANEL_HOME : PANEL_MAP;
}

void second_update(void) {
	SecondContext c = context_now();
	if (c != S2.context) S2.since = 0;
	else if (S2.since < 1 << 20) ++S2.since;
	S2.context = c;
	S2.town = director_in_town();
	director_megaman_hp(&S2.hp, &S2.max_hp);
	S2.zenny = emu_read32(BN6_ZENNY);
	S2.bugfrags = emu_read32(BN6_BUGFRAGS);
	S2.depth = run.depth;
	snprintf(S2.area, sizeof S2.area, "%s", S2.town ? town_info()->name : guardian_area_in_text(run.biome, run.side_kind));
	Panel p = panel_for(c);
	if (p == PANEL_FOLDER) second_read_folder();
	if (p == PANEL_BATTLE) second_read_battle();
	if (p == PANEL_NAVICUST) second_read_navicust();
	if (p == PANEL_HOME) second_read_home();
	if (p == PANEL_STATUS) second_read_status();
	if (p == PANEL_SHOP) second_read_shop();
	if (p == PANEL_TRADER) second_read_trader();
	if (p != panel) panel_since = 0;
	else if (panel_since < 1 << 20) ++panel_since;
	panel = p;
	/* (a new panel at once, its title sliding in over two draws) */
	if (panel_since < 2) platform_second_screen_soon();
}

/* The frame's name for the panel: the PET's screen's where the PET at
 * home stands in for it */
static const char *panel_title(void) {
	static const char *const title[] = { [PANEL_MAP] = "NET", [PANEL_FOLDER] = "FOLDER", [PANEL_BATTLE] = "BATTLE", [PANEL_NAVICUST] = "NAVICUST",
		[PANEL_HOME] = "PET", [PANEL_STATUS] = "MEGAMAN", [PANEL_LIBRARY] = "LIBRARY", [PANEL_SHOP] = "SHOP", [PANEL_TRADER] = "TRADER" };
	switch (S2.context) {
	case SECOND_EDIT: return "FOLDER EDIT";
	case SECOND_MAIL: return "E-MAIL";
	case SECOND_KEYITEM: return "KEYITEM";
	case SECOND_SUBCHIP: return "SUBCHIP";
	case SECOND_COMM: return "COMM";
	case SECOND_SAVE: return "SAVE";
	default: return title[panel];
	}
}

/* what the last picture showed: the panel and the state it drew from */
static Panel drawn_panel;
static SecondState drawn;

bool second_changed(void) {
	if (panel == PANEL_MAP || panel != drawn_panel || panel_since < 2) return true;
	SecondState now = S2;
	now.since = drawn.since;
	return memcmp(&now, &drawn, sizeof now) != 0;
}

bool second_draw(int w, int h) {
	drawn_panel = panel;
	drawn = S2;
	if (panel == PANEL_DARK) return false;
	SDL_Rect body = second_frame(w, h, panel_title(), panel_since == 0 ? 24 : panel_since == 1 ? 8 : 0);
	if (panel == PANEL_FOLDER) second_folder_draw(body);
	else if (panel == PANEL_BATTLE) second_battle_draw(body);
	else if (panel == PANEL_NAVICUST) second_navicust_draw(body);
	else if (panel == PANEL_HOME) second_home_draw(body);
	else if (panel == PANEL_STATUS) second_status_draw(body);
	else if (panel == PANEL_LIBRARY) second_library_draw(body);
	else if (panel == PANEL_SHOP) second_shop_draw(body);
	else if (panel == PANEL_TRADER) second_trader_draw(body);
	else director_draw_layer_map(body.x, body.y, body.w, body.h);
	return true;
}
