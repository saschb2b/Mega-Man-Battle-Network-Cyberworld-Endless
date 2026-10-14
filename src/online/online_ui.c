#include "online_ui.h"

#include <stdio.h>
#include <string.h>

#include "gfx.h"
#include "online_people.h"
#include "online_text.h"
#include "online_transport.h"
#include "online_visit.h"
#include "pixfont.h"
#include "platform.h"

enum { CLOSED, MENU, PLAYERS, PHRASES, EMOTES, CHANNELS, CARD };
static int page, choice;
static uint16_t channel;
static OnlineSnapshot state;
static OnlinePeer selected;
static const char *feedback;
static uint32_t requested_card;

void online_ui_reset(void) { page = choice = 0; feedback = NULL; memset(&state, 0, sizeof state); }
bool online_ui_open(void) { return page != CLOSED; }

static void open_card(void) {
	if ((size_t)choice >= state.peer_count) return;
	selected = state.peers[choice];
	requested_card = state.card_serial;
	online_transport_inspect(selected.id);
	feedback = NULL;
	page = CARD;
}

static void card_input(void) {
	unsigned flag = btn_pressed(BTN_A) ? ONLINE_MUTED : btn_pressed(BTN_R) ? ONLINE_BLOCKED : 0;
	if (flag) feedback = online_people_toggle(selected.identity, flag) ? online_saved : online_people_failed;
}

static void choose(void) {
	if (page == MENU) {
		static const int pages[] = { PLAYERS, PHRASES, EMOTES, CHANNELS };
		if (choice == 4) { online_visit_return(); return; }
		page = pages[choice];
		choice = 0;
		channel = state.channel;
	} else if (page == PLAYERS) open_card();
	else if (page == PHRASES || page == EMOTES) {
		online_transport_social(page == EMOTES ? 1 : 0, (uint8_t)choice);
		page = CLOSED;
	} else if (page == CHANNELS) {
		online_transport_channel(channel);
		page = CLOSED;
	}
}

void online_ui_update(const OnlineSnapshot *snapshot) {
	state = *snapshot;
	if (!state.connected) return;
	if (page == CLOSED) {
		if (btn_pressed(BTN_START)) { page = MENU; choice = 0; }
		else if (btn_pressed(BTN_A)) { page = PLAYERS; choice = 0; }
		return;
	}
	if (btn_pressed(BTN_B)) {
		page = page == MENU ? CLOSED : page == CARD ? PLAYERS : MENU;
		choice = 0; feedback = NULL;
		return;
	}
	if (page == CARD) { card_input(); return; }
	int total = page == MENU ? 5 : page == PLAYERS ? (int)state.peer_count : page == PHRASES ? ONLINE_PHRASE_COUNT : ONLINE_EMOTE_COUNT;
	if (page == CHANNELS) {
		if (btn_pressed(BTN_LEFT)) channel = channel > 1 ? (uint16_t)(channel - 1) : state.channels;
		if (btn_pressed(BTN_RIGHT)) channel = channel < state.channels ? (uint16_t)(channel + 1) : 1;
	} else if (total) {
		if (choice >= total) choice = total - 1;
		if (btn_pressed(BTN_UP)) choice = (choice + total - 1) % total;
		if (btn_pressed(BTN_DOWN)) choice = (choice + 1) % total;
	}
	if (btn_pressed(BTN_A)) choose();
}

static void small(int x, int y, const char *text, SDL_Color color) {
	pixfont_draw(P.core_x + x, P.core_y + y, text, color, BLACK, PIXFONT_LEFT, 1);
}

static void board_draw(const OnlineProfile *profile) {
	static const SDL_Color colors[] = { {22,40,60,255}, {240,240,240,255}, {240,220,80,255},
		{235,100,175,255}, {230,70,70,255}, {75,140,240,255}, {80,210,100,255} };
	for (unsigned i = 0; i < ONLINE_BOARD_CELLS; ++i) {
		int x = P.core_x + 184 + (int)(i % 7) * 6, y = P.core_y + 40 + (int)(i / 7) * 6;
		fill_rect(x, y, 5, 5, colors[profile->board[i]]);
		/* The command line is visible without relying on cell color. */
		if (i / 7 == 3) fill_rect(x, y + 2, 5, 1, rgba(130, 170, 205, 255));
	}
}

static void powers_draw(uint16_t mask, bool souls, int y) {
	char powers[80], wrapped[2][96];
	online_card_powers(powers, sizeof powers, mask, souls);
	int count = pixfont_wrap(powers, 168, 1, wrapped, 2);
	for (int i = 0; i < count; ++i) small(60, y + i * 11, wrapped[i], WHITE);
}

static void card_draw(void) {
	if (state.card_player != selected.id || state.card_serial == requested_card ||
	    memcmp(state.card_identity, selected.identity, ONLINE_IDENTITY_BYTES)) {
		text_draw(P.core_x + 12, P.core_y + 50, online_card_wait, WHITE, TEXT_LEFT);
		return;
	}
	const OnlineProfile *p = &state.card;
	char lines[5][48], stats[60];
	online_card_stats(lines, p->hp, p->max_hp, p->depth, p->act, p->best_depth);
	for (unsigned i = 0; i < 3; ++i) small(12, 37 + (int)i * 12, lines[i], WHITE);
	online_card_buster(stats, sizeof stats, p->attack, p->speed, p->charge);
	small(12, 73, stats, WHITE);
	board_draw(p);
	small(12, 89, lines[3], rgba(100, 210, 245, 255)); powers_draw(p->souls, true, 89);
	small(12, 112, lines[4], rgba(100, 210, 245, 255)); powers_draw(p->crosses, false, 112);
	unsigned marks = 0;
	for (unsigned i = 0; i < 16; ++i) if (p->marks & (1u << i)) ++marks;
	online_card_marks(stats, sizeof stats, marks); small(184, 83, stats, WHITE);
	unsigned flags = online_people_flags(selected.identity);
	const char *status = feedback ? feedback : flags & ONLINE_BLOCKED ? online_blocked : flags & ONLINE_MUTED ? online_muted : "";
	small(12, 136, status, rgba(255, 220, 100, 255));
}

void online_ui_draw(void) {
	if (page == CLOSED) return;
	fill_rect(P.core_x + 5, P.core_y + 19, 230, 139, rgba(8, 28, 50, 245));
	fill_rect(P.core_x + 5, P.core_y + 19, 230, 15, rgba(30, 92, 130, 255));
	const char *title = page == MENU ? online_title : page == PLAYERS ? online_people_title :
		page == PHRASES ? online_phrases_title : page == EMOTES ? online_emotes_title :
		page == CHANNELS ? online_channel_title : selected.name;
	text_draw(P.core_x + 12, P.core_y + 20, title, WHITE, TEXT_LEFT);
	if (page == CARD) card_draw();
	else if (page == CHANNELS) text_drawf(P.core_x + 120, P.core_y + 65, WHITE, TEXT_CENTER, "%u / %u", channel, state.channels);
	else {
		int total = page == MENU ? 5 : page == PLAYERS ? (int)state.peer_count : page == PHRASES ? ONLINE_PHRASE_COUNT : ONLINE_EMOTE_COUNT;
		for (int i = 0; i < total; ++i) {
			const char *label = page == MENU ? online_menu[i] : page == PLAYERS ? state.peers[i].name : page == PHRASES ? online_phrases[i] : online_emotes[i];
			int y = 37 + i * 13;
			if (i == choice) fill_rect(P.core_x + 10, P.core_y + y, 220, 13, rgba(35, 100, 140, 255));
			text_draw(P.core_x + 16, P.core_y + y, label, WHITE, TEXT_LEFT);
		}
		if (!total) small(12, 62, online_waiting, WHITE);
	}
	small(12, 147, page == CARD ? online_card_help : page == CHANNELS ? online_channel_help : online_select_help, WHITE);
}
