/* second.h. The screen the player is on, from BN6's state once a frame
 * (docs/ROM_DATA.md, the second screen's contexts), and the panel drawn
 * for it in the PET's frame. Until a screen has a panel of its own (issue
 * #72's stories), a layer's screens show the layer's map, as Operate
 * Shooting Star keeps its map up through a battle, and the town and the
 * title are dark. */
#include "second.h"

#include <stdio.h>

#include "bn6.h"
#include "director.h"
#include "emu.h"
#include "guardians.h"
#include "guest.h"
#include "platform.h"
#include "run.h"
#include "second_frame.h"
#include "second_state.h"

SecondState S2;

/* What the second screen draws: a panel, which several screens may share */
typedef enum { PANEL_DARK, PANEL_MAP } Panel;
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
	return director_in_town() ? SECOND_TOWN : SECOND_NET;
}

/* The panel for a screen: the map on a layer, dark in the town */
static Panel panel_for(SecondContext c) {
	return c == SECOND_DARK || S2.town ? PANEL_DARK : PANEL_MAP;
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
	snprintf(S2.area, sizeof S2.area, "%s", S2.town ? "" : guardian_area_in_text(run.biome, run.side_kind));
	Panel p = panel_for(c);
	if (p != panel) panel_since = 0;
	else if (panel_since < 1 << 20) ++panel_since;
	panel = p;
	/* (a new panel at once, its title sliding in over two draws) */
	if (panel_since < 2) platform_second_screen_soon();
}

bool second_draw(int w, int h) {
	if (panel == PANEL_DARK) return false;
	SDL_Rect body = second_frame(w, h, "NET", panel_since == 0 ? 24 : panel_since == 1 ? 8 : 0);
	director_draw_layer_map(body.x, body.y, body.w, body.h);
	return true;
}
