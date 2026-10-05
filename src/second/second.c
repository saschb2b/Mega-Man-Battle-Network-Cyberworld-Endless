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
#include "second_folder.h"
#include "second_frame.h"
#include "second_state.h"

SecondState S2;

/* What the second screen draws: a panel, which several screens may share */
typedef enum { PANEL_DARK, PANEL_MAP, PANEL_FOLDER } Panel;
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

/* The panel for a screen: the folder's in the folders' screens; else the
 * map on a layer, dark in the town */
static Panel panel_for(SecondContext c) {
	if (c == SECOND_FOLDERS || c == SECOND_EDIT) return PANEL_FOLDER;
	return c == SECOND_DARK || S2.town ? PANEL_DARK : PANEL_MAP;
}

/* an entry of the folder's (0xFF none) as an index, -1 for none */
static int entry_of(int v) { return v < BN6_FOLDER_ENTRIES ? v : -1; }

/* The folder, its limits and its marks; in the editor its cursor, and the
 * pack (read every quarter second: 314 chips' counts) */
static void read_folder(void) {
	director_folder_now(S2.folder);
	S2.mega_level = emu_read8(BN6_NAVI_MEGA_LEVEL);
	S2.giga_level = emu_read8(BN6_NAVI_GIGA_LEVEL);
	S2.reg = entry_of(emu_read8(BN6_NAVI_FOLDER1_REG));
	S2.tag[0] = entry_of(emu_read8(BN6_NAVI_FOLDER1_TAG));
	S2.tag[1] = entry_of(emu_read8(BN6_NAVI_FOLDER1_TAG2));
	uint32_t edit = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_SUBMENU);
	bool editing = S2.context == SECOND_EDIT;
	S2.pack_side = editing && emu_read8(edit + BN6_EDIT_SIDE) == BN6_EDIT_PACK;
	S2.entry = editing && !S2.pack_side ? entry_of(emu_read8(edit + BN6_EDIT_SCROLL) + emu_read8(edit + BN6_EDIT_ROW)) : -1;
	S2.pack_entry = S2.pack_side ? emu_read8(edit + BN6_EDIT_PACK_SCROLL) + emu_read8(edit + BN6_EDIT_PACK_ROW) : -1;
	if (S2.since % 15 == 0) S2.npack = director_pack_now(S2.pack, S2.pack_count, (int)(sizeof S2.pack / sizeof *S2.pack));
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
	if (p == PANEL_FOLDER) read_folder();
	if (p != panel) panel_since = 0;
	else if (panel_since < 1 << 20) ++panel_since;
	panel = p;
	/* (a new panel at once, its title sliding in over two draws) */
	if (panel_since < 2) platform_second_screen_soon();
}

bool second_draw(int w, int h) {
	if (panel == PANEL_DARK) return false;
	static const char *const title[] = { [PANEL_MAP] = "NET", [PANEL_FOLDER] = "FOLDER" };
	const char *t = S2.context == SECOND_EDIT ? "FOLDER EDIT" : title[panel];
	SDL_Rect body = second_frame(w, h, t, panel_since == 0 ? 24 : panel_since == 1 ? 8 : 0);
	if (panel == PANEL_FOLDER) second_folder_draw(body);
	else director_draw_layer_map(body.x, body.y, body.w, body.h);
	return true;
}
