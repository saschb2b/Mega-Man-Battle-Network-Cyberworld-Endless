#include "network.h"

#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "network_state.h"
#include "network_text.h"
#include "online_visit.h"
#include "platform.h"
#include "text_input.h"
#include "touch.h"

NetworkState N;
static char pending_note[160];

static bool finger_down(void) {
	for (int i = 0; i < SDL_GetNumTouchDevices(); ++i)
		if (SDL_GetNumTouchFingers(SDL_GetTouchDevice(i)) > 0) return true;
	return false;
}

void network_open(void) { scene_set(&scene_network); }

void network_message(const char *message) {
	snprintf(pending_note, sizeof pending_note, "%s", message ? message : "");
	network_open();
}

bool network_store(void) {
	if (!network_prefs_write(g_data_dir, &N.prefs)) {
		snprintf(N.note, sizeof N.note, "%s", network_word(NW_SAVE_FAILED));
		return false;
	}
	platform_persist();
	return true;
}

bool network_back(void) {
	if (N.view != NETWORK_HOME) {
		text_input_end();
		N.view = NETWORK_HOME;
		N.note[0] = 0;
	} else {
		title_menu = true;
		scene_set(&scene_title);
	}
	audio_sfx(SFX_CANCEL);
	return true;
}

static void enter(void) {
	memset(&N, 0, sizeof N);
	N.wait_touch = finger_down();
	N.row = NETWORK_ROW_JOIN;
	N.identity_ok = network_prefs_read(g_data_dir, &N.prefs);
	if (!N.identity_ok) {
		N.identity_ok = network_identity(N.prefs.identity);
		if (N.identity_ok) network_store();
		else snprintf(N.note, sizeof N.note, "%s", network_word(NW_RANDOM_FAILED));
	}
	if (pending_note[0]) snprintf(N.note, sizeof N.note, "%s", pending_note);
	pending_note[0] = 0;
	platform_own_taps(true);
	platform_on_back(network_back);
	platform_second_screen(NULL, NULL);
	touch_release();
	audio_music(MUS_TITLE);
}

static void leave(void) {
	text_input_end();
	platform_own_taps(false);
	platform_on_back(NULL);
}

static void join(void) {
	if (!N.identity_ok) { snprintf(N.note, sizeof N.note, "%s", network_word(NW_RANDOM_FAILED)); return; }
	if (!network_store()) return;
	uint8_t identity[NETWORK_ID_BYTES];
	const char *url = N.prefs.servers[N.prefs.selected];
	if (!network_server_identity(N.prefs.identity, url, identity)) { snprintf(N.note, sizeof N.note, "%s", network_word(NW_URL_INVALID)); return; }
	if (!online_visit_begin(url, N.prefs.name, identity, N.note, sizeof N.note)) return;
	audio_sfx(SFX_SELECT);
}

static void choose(void) {
	N.note[0] = 0;
	switch (N.row) {
	case NETWORK_ROW_NAME: network_edit_begin(true, false); break;
	case NETWORK_ROW_SERVER: network_edit_begin(false, false); break;
	case NETWORK_ROW_ADD:
		if (N.prefs.count < NETWORK_SERVERS) network_edit_begin(false, true);
		else snprintf(N.note, sizeof N.note, "%s", network_word(NW_FULL));
		break;
	case NETWORK_ROW_JOIN:
		if (N.prefs.consent & (1u << N.prefs.selected)) join();
		else { N.view = NETWORK_PRIVACY; N.privacy = 0; }
		break;
	case NETWORK_ROW_BACK: network_back(); break;
	default: break;
	}
}

static void privacy_update(void) {
	if (btn_pressed(BTN_B)) { network_back(); return; }
	if (btn_repeat(BTN_LEFT) || btn_repeat(BTN_RIGHT)) N.privacy ^= 1;
	int x, y;
	bool picked = platform_tap(&x, &y) && y >= P.h - 29 && y < P.h - 9;
	if (picked) N.privacy = x >= P.w / 2;
	if (!picked && !btn_pressed(BTN_A) && !btn_pressed(BTN_START)) return;
	if (!N.privacy) { network_back(); return; }
	N.prefs.consent |= 1u << N.prefs.selected;
	N.view = NETWORK_HOME;
	join();
}

static void update(void) {
	/* The virtual A button's lift belongs to the title that opened us. */
	if (N.wait_touch) {
		int x, y;
		platform_tap(&x, &y);
		N.wait_touch = finger_down();
		return;
	}
	if (N.view == NETWORK_EDIT_NAME || N.view == NETWORK_EDIT_SERVER) { network_edit_update(); return; }
	if (N.view == NETWORK_PRIVACY) { privacy_update(); return; }
	if (btn_pressed(BTN_B)) { network_back(); return; }
	if (btn_repeat(BTN_UP)) { N.row = (N.row + NETWORK_ROWS - 1) % NETWORK_ROWS; N.note[0] = 0; }
	if (btn_repeat(BTN_DOWN)) { N.row = (N.row + 1) % NETWORK_ROWS; N.note[0] = 0; }
	int delta = btn_repeat(BTN_RIGHT) ? 1 : btn_repeat(BTN_LEFT) ? -1 : 0;
	if (N.row == NETWORK_ROW_SERVER && delta) {
		N.prefs.selected = (unsigned)((int)N.prefs.selected + (int)N.prefs.count + delta) % N.prefs.count;
		network_store();
	}
	int x, y;
	if (platform_tap(&x, &y)) {
		for (int i = 0; i < NETWORK_ROWS; ++i) {
			SDL_Rect r = network_home_rect(i);
			if (x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h) { N.row = i; choose(); return; }
		}
	}
	if (btn_pressed(BTN_A) || btn_pressed(BTN_START)) choose();
}

const Scene scene_network = { "network", enter, update, network_draw, leave };
