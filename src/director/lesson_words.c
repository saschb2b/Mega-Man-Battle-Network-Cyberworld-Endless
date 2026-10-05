/* MegaMan's lessons, each said once a profile (docs/VOICE.md): where a
 * chip come to the Pack went, and Rush's bone panels. */
#include "lesson_words.h"

#include <stdio.h>
#include <stdlib.h>

#include "bn6_fields.h"
#include "cinema.h"
#include "director_folder.h"
#include "director_state.h"
#include "netmap.h"
#include "save.h"
#include "talk.h"
#include "town.h"

/* Rush's gap (issue #14), named the first time MegaMan comes near its
 * stand on a layer: BN6 answers A there without RushFood by a sound alone,
 * which teaches nothing (what calls Rush, how many he needs, where to buy
 * them), so MegaMan says it, and says it as it stands: with enough held,
 * where to press A. */
void rush_hint(void) {
	if (!layer.ngaps || flag_get(LAYER_RUSH_TOLD_FLAG) || !on_map() || cinema_busy() || talk_busy() || emu_read8(BN6_CHATBOX)) return;
	int sx, sy;
	netmap_world(layer.gap[0].x, layer.gap[0].y, &sx, &sy);
	if (abs(bn6_player_x() - sx) > 40 || abs(bn6_player_y() - sy) > 40) return;
	uint32_t items = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS);
	int held = items >= BN6_EWRAM && items < BN6_EWRAM_END ? emu_read8(items + ITEM_RUSH_FOOD) : 0;
	char words[260];
	if (held >= layer.gap[0].len)
		snprintf(words, sizeof words, "@M Bone panels,Lan! Rush can bridge this gap!|@M Press A at the edge to toss him RushFood!");
	else if (layer.gap[0].len == 1)
		snprintf(words, sizeof words, "@M Bone panels,Lan! Rush could bridge this gap!|@M He needs one RushFood.|@M A Net Dealer might have some.");
	else
		snprintf(words, sizeof words, "@M Bone panels,Lan! Rush could bridge this gap!|@M He comes when we hold %d RushFood,and eats one.|"
			"@M A Net Dealer might have some.", layer.gap[0].len);
	if (talk_start(words, FACE_MEGAMAN)) flag_set(LAYER_RUSH_TOLD_FLAG);
}

/* where a chip come to the pack went, the first time (pack_due) */
void pack_words(void) {
	if (!D.pack_due || talk_busy() || emu_read8(BN6_CHATBOX) || cinema_busy() || !on_map()) return;
	if (talk_start("@M That chip went to our Pack,Lan!|@M To fight with it,add it to our Folder.|@M In the PET,go to Folder,then EDIT!", FACE_MEGAMAN)) {
		D.pack_due = false;
		profile.pack_taught = 1;
		profile_save();
	}
}

/* What the Mystery Data on a battlefield was, said the first time, after
 * it was met, kept or broken, not before */
const char *gem_words(void) {
	return "@M Lan! Mystery Data on the battlefield!|@M Any hit breaks it,theirs or ours.|"
		"@M If it's still there when we win,it's ours!";
}

/* MegaMan's word on R pressed in the town away from the port: almost at a
 * jack-in cell (`way` the way to it), close to the port (`way` the walk's),
 * or nowhere near it */
const char *port_words(int how, const char *way) {
	static char buf[160];
	if (how == PORT_ROOM)
		snprintf(buf, sizeof buf, "@M The PC's %s,Lan!|@M Step up to it and press R!", way);
	else if (how == PORT_HOUSE)
		snprintf(buf, sizeof buf, "@M Your room's %s,Lan!|@M The PC's up there!", way);
	else if (town_is_home())
		snprintf(buf, sizeof buf, "%s", how == PORT_AWAY ? "@M No port here,Lan!|@M I jack in from your PC at home!" :
			"@M Home's right here,Lan!|@M The PC's up in your room!");
	else if (how == PORT_ALMOST_CELL)
		snprintf(buf, sizeof buf, "@M Almost,Lan! The %s's %s.|@M Step up to it and press R!", town_info()->landmark, way);
	else if (how == PORT_ALMOST)
		snprintf(buf, sizeof buf, "@M Almost,Lan! The %s's %s.|@M Step right up to it and press R!", town_info()->landmark, way);
	else
		snprintf(buf, sizeof buf, "@M No port here,Lan!|@M It's by the %s!", town_info()->landmark_at);
	return buf;
}
