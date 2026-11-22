/* director.h's director_cover: what of BN6's, or a card of ours, stands
 * over the layer's field in the picture now (issue #781). One answer for
 * everything we draw over the field (the divers' names and rings, the
 * crew's plates, the dive's band, MegaMan's arrival mark), so none of it
 * lands on a screen of BN6's own or on our cards. Reads; changes nothing. */
#include "director.h"

#include "bn6.h"
#include "cinema.h"
#include "director_state.h"
#include "emu.h"

int director_cover(void) {
	/* (not the field: a battle, BN6's menus and PET, a transition's black, SELECT's map) */
	if (!D.active || !on_map() || director_map_over()) return COVER_AWAY;
	/* (an act's, an area's or a guardian's title card) */
	if (cinema_busy()) return COVER_CARD;
	/* (BN6's fade, a warp under way) */
	if (emu_read8(BN6_FADE_ACTIVE) == 1 || emu_read8(BN6_WARP_PENDING) != 0 || D.warping) return COVER_FADE;
	if (emu_read8(BN6_CHATBOX)) return COVER_CHAT;
	return COVER_NONE;
}

const char *director_cover_name(int cover) {
	static const char *const names[] = { "none", "chat", "card", "fade", "away" };
	return cover >= 0 && cover <= COVER_AWAY ? names[cover] : "?";
}
