/* online_chat.h. The Net Square's chats (docs/ONLINE.md), all in BN6's own
 * chat boxes: L asks MegaMan to say something or do an emote (the
 * protocol's quick phrases and emotes, a page of two at a time); A facing
 * another player's MegaMan reads his card, and waves, mutes or blocks him;
 * R, or the pad home, asks to head home; the channel keeper moves MegaMan
 * to the next or last channel. Each option's script sets an event flag of
 * the Square's (its RAM is thrown away on the way home), acted on as the
 * chat closes. */
#include "online_chat.h"

#include <string.h>

#include "arena_words.h"
#include "bn6_fields.h"
#include "emu.h"
#include "flags.h"
#include "netpass.h"
#include "netpass_screen.h"
#include "online_people.h"
#include "online_square.h"
#include "online_text.h"
#include "online_transport.h"
#include "platform.h"
#include "script_kit.h"
#include "square_arena.h"
#include "square_team.h"
#include "square_life.h"
#include "square_week.h"
#include "square_words.h"
#include "talk.h"
#include "trade_central.h"

/* The Square's event flags (its own map's, free while MegaMan is there) */
#define SAY_FLAG     0x1480   /* + phrase 0-7 */
#define EMOTE_FLAG   0x1488   /* + emote 0-5 */
#define NEXT_FLAG    0x148E
#define LAST_FLAG    0x148F
#define WAVE_FLAG    0x1490
#define MUTE_FLAG    0x1491
#define BLOCK_FLAG   0x1492
#define HOME_FLAG    0x1493
#define LOOK_FLAG    0x14C5   /* L's Look around (after square_watch.c's 0x14C4: Trade Central clears 0x1494-0x149F) */
#define DIVE_FLAG    SQUARE_DIVE_FLAG   /* the greeter's dive (square_week.h): 0x14AF */
#define BATTLE_FLAG  0x14A0   /* (the Arena's, apart: square_arena.c) */
#define TEAM_FLAG    0x14C9   /* "Team up" or "Split up" (square_team.c's own are 0x14C6-0x14C8) */
#define FIRST_FLAG SAY_FLAG
#define LAST_SQUARE_FLAG HOME_FLAG

#define CARD_WAIT 60     /* frames A waits for a peer's card */
#define WAVE 0           /* the protocol's emote "Wave" */

static struct {
	OnlineSnapshot s;
	struct { uint32_t id; OnlineProfile card; } cards[SQUARE_PEERS + 1];
	uint32_t card_serial, asked[SQUARE_PEERS];
	OnlinePeer peer;          /* the one being talked to */
	int waiting;              /* frames a peer's talk has waited for his card */
	bool home, open;          /* the question home asked; a chat of ours open */
	int stamp;                /* the stamp moment: 1 the greeter's word open, 2 the pass up (netpass_screen.h) */
	unsigned keeper_said;     /* channel, channels and navis the keeper's words name, packed */
	bool on_exit;
} C;

void online_chat_reset(void) {
	memset(&C, 0, sizeof C);
	C.on_exit = true;   /* (arrived beside it: it asks once he steps on it) */
	trade_central_reset();
	square_life_reset();
	square_week_reset();
	for (int f = FIRST_FLAG; f <= LAST_SQUARE_FLAG; ++f) flag_clear(f);
	flag_clear(BATTLE_FLAG);
	flag_clear(TEAM_FLAG);
	flag_clear(DIVE_FLAG);
	flag_clear(LOOK_FLAG);
}

static const OnlineProfile *card_of(uint32_t id) {
	for (size_t i = 0; i < sizeof C.cards / sizeof *C.cards; ++i)
		if (C.cards[i].id == id) return &C.cards[i].card;
	return NULL;
}

/* each card as it comes, kept by its player; each peer's asked for once */
static void cards(void) {
	if (C.s.card_serial != C.card_serial && C.s.card_player) {
		C.card_serial = C.s.card_serial;
		size_t slot = C.s.card_serial % (sizeof C.cards / sizeof *C.cards);
		for (size_t i = 0; i < sizeof C.cards / sizeof *C.cards; ++i) if (C.cards[i].id == C.s.card_player) slot = i;
		C.cards[slot].id = C.s.card_player;
		C.cards[slot].card = C.s.card;
	}
	for (size_t i = 0; i < C.s.peer_count && i < SQUARE_PEERS; ++i) {
		uint32_t id = C.s.peers[i].id;
		if (C.asked[i] == id || card_of(id)) continue;
		if (online_transport_inspect(id)) C.asked[i] = id;
	}
}

/* L's chat into the Square's talk room: the question, then pages of two
 * phrases or emotes, each with More (or Back to the first), or a look
 * around (square_life_look) */
#define SAY_PAGES 4
#define EMOTE_PAGES 3
static void menu(void) {
	static TextArchive t;
	static uint8_t out[TEXT_ARCHIVE_MAX];
	enum { TOP, SAY = 1, EMOTE = SAY + SAY_PAGES, PHRASE = EMOTE + EMOTE_PAGES, DO = PHRASE + ONLINE_PHRASE_COUNT,
		LOOK = DO + ONLINE_EMOTE_COUNT, END };
	ta_begin(&t);
	ta_script(&t);
	ta_page(&t, FACE_MEGAMAN, square_menu_question(), true);
	ta_mugshot(&t, FACE_MEGAMAN);
	ta_clear(&t);
	ta_options(&t, square_menu_options, SQUARE_MENU_OPTIONS);
	int top[SQUARE_MENU_OPTIONS] = { SAY, EMOTE, LOOK };
	ta_choose(&t, top, SQUARE_MENU_OPTIONS, END);
	for (int kind = 0; kind < 2; ++kind) {
		int pages = kind ? EMOTE_PAGES : SAY_PAGES, first = kind ? EMOTE : SAY, item = kind ? DO : PHRASE;
		const char *const *names = kind ? online_emotes : online_phrases;
		for (int p = 0; p < pages; ++p) {
			ta_script(&t);
			ta_mugshot(&t, FACE_MEGAMAN);
			ta_clear(&t);
			const char *labels[3] = { names[2 * p], names[2 * p + 1], p + 1 < pages ? square_more() : square_back() };
			ta_options(&t, labels, 3);
			int to[3] = { item + 2 * p, item + 2 * p + 1, p + 1 < pages ? first + p + 1 : TOP };
			ta_choose(&t, to, 3, END);
		}
	}
	for (int k = 0; k < ONLINE_PHRASE_COUNT + ONLINE_EMOTE_COUNT; ++k) {
		ta_script(&t);
		ta_flag_set(&t, k < ONLINE_PHRASE_COUNT ? SAY_FLAG + k : EMOTE_FLAG + k - ONLINE_PHRASE_COUNT);
		ta_end(&t);
	}
	ta_script(&t);
	ta_flag_set(&t, LOOK_FLAG);
	ta_end(&t);
	ta_closing(&t);
	int n = ta_build(&t, out);
	if (n <= SQUARE_TALK_ROOM) {
		emu_write(online_square_talk_room(), out, (size_t)n);
		C.open = talk_script(online_square_talk_room(), TOP);
	}
}

/* ... where the Arena is built, Battle and Team up first, in three boxes:
 * [Battle][Team up][More...], [Wave][Mute][More...], then [Block][Back]
 * (script 0 the card, 1-3 the boxes, 4-8 their flags, 9 the end) */
static void peer_talk_arena(TextArchive *t, uint8_t *out, bool muted) {
	ta_begin(t);
	ta_script(t);
	bool first = true;
	ta_pages(t, square_peer_words(C.peer.name, card_of(C.peer.id), netpass_peer_full(C.peer.roms)), FACE_MEGAMAN, &first);
	ta_jump(t, 1);
	const char *boxes[3][3] = {
		{ arena_battle_option(), square_team_option(&C.peer), square_more() },
		{ square_peer_option(0, muted), square_peer_option(1, muted), square_more() },
		{ square_peer_option(2, muted), square_back(), NULL },
	};
	static const int to[3][3] = { { 4, 5, 2 }, { 6, 7, 3 }, { 8, 1, 0 } };
	static const int counts[3] = { 3, 3, 2 };
	for (int b = 0; b < 3; ++b) {
		ta_script(t);
		ta_mugshot(t, FACE_MEGAMAN);
		ta_clear(t);
		ta_options(t, boxes[b], counts[b]);
		ta_choose(t, to[b], counts[b], 9);
	}
	static const int flags[5] = { BATTLE_FLAG, TEAM_FLAG, WAVE_FLAG, MUTE_FLAG, BLOCK_FLAG };
	for (int k = 0; k < 5; ++k) {
		ta_script(t);
		ta_flag_set(t, flags[k]);
		ta_end(t);
	}
	ta_closing(t);
	int n = ta_build(t, out);
	if (n <= SQUARE_TALK_ROOM) {
		emu_write(online_square_talk_room(), out, (size_t)n);
		C.open = talk_script(online_square_talk_room(), 0);
	}
}

/* A facing a peer: his card, then Wave, Mute or Unmute, Block */
static void peer_talk(void) {
	static TextArchive t;
	static uint8_t out[TEXT_ARCHIVE_MAX];
	bool muted = online_people_flags(C.peer.identity) & ONLINE_MUTED;
	if (square_arena_offered() && netpass_has(STAMP_ARENA) && netpass_peer_full(C.peer.roms)) { peer_talk_arena(&t, out, muted); return; }
	ta_begin(&t);
	ta_script(&t);
	bool first = true;
	ta_pages(&t, square_peer_words(C.peer.name, card_of(C.peer.id), netpass_peer_full(C.peer.roms)), FACE_MEGAMAN, &first);
	ta_mugshot(&t, FACE_MEGAMAN);
	ta_clear(&t);
	const char *labels[SQUARE_PEER_OPTIONS];
	for (int k = 0; k < SQUARE_PEER_OPTIONS; ++k) labels[k] = square_peer_option(k, muted);
	ta_options(&t, labels, SQUARE_PEER_OPTIONS);
	int to[SQUARE_PEER_OPTIONS] = { 1, 2, 3 };
	ta_choose(&t, to, SQUARE_PEER_OPTIONS, 4);
	static const int flags[SQUARE_PEER_OPTIONS] = { WAVE_FLAG, MUTE_FLAG, BLOCK_FLAG };
	for (int k = 0; k < SQUARE_PEER_OPTIONS; ++k) {
		ta_script(&t);
		ta_flag_set(&t, flags[k]);
		ta_end(&t);
	}
	ta_closing(&t);
	int n = ta_build(&t, out);
	if (n <= SQUARE_TALK_ROOM) {
		emu_write(online_square_talk_room(), out, (size_t)n);
		C.open = talk_script(online_square_talk_room(), 0);
	}
}

/* the keeper's words as the channel stands, rewritten as it changes */
static void keeper(void) {
	unsigned navis = (unsigned)C.s.peer_count + 1, said = C.s.channel | C.s.channels << 9 | navis << 18;
	if (!C.s.channel || said == C.keeper_said || emu_read8(BN6_CHATBOX)) return;
	static TextArchive t;
	static uint8_t out[TEXT_ARCHIVE_MAX];
	ta_begin(&t);
	ta_script(&t);
	bool first = true;
	ta_pages(&t, square_keeper_words(C.s.channel, C.s.channels, navis), FACE_PROG, &first);
	ta_mugshot(&t, FACE_PROG);
	ta_clear(&t);
	ta_options(&t, square_keeper_options, 2);
	int to[2] = { 1, 2 };
	ta_choose(&t, to, 2, 3);
	for (int k = 0; k < 2; ++k) {
		ta_script(&t);
		ta_flag_set(&t, k ? LAST_FLAG : NEXT_FLAG);
		bool f = true;
		ta_pages(&t, square_keeper_moving(), FACE_PROG, &f);
		ta_end(&t);
	}
	ta_closing(&t);
	if (online_square_keeper_say(out, ta_build(&t, out))) C.keeper_said = said;
}

/* the flags of the chat just closed, acted on; true: MegaMan heads home */
static bool chosen(void) {
	bool home = false;
	for (int k = 0; k < ONLINE_PHRASE_COUNT; ++k) if (flag_get(SAY_FLAG + k)) online_transport_social(0, (uint8_t)k);
	for (int k = 0; k < ONLINE_EMOTE_COUNT; ++k) if (flag_get(EMOTE_FLAG + k)) online_transport_social(1, (uint8_t)k);
	if (flag_get(BATTLE_FLAG)) { flag_clear(BATTLE_FLAG); square_arena_challenge(&C.peer, C.s.latency_ms); }
	if (flag_get(TEAM_FLAG)) { flag_clear(TEAM_FLAG); square_team_chosen(&C.peer); }
	if (flag_get(WAVE_FLAG)) online_transport_social(1, WAVE);
	if (flag_get(LOOK_FLAG)) { flag_clear(LOOK_FLAG); square_life_look(); }
	if (flag_get(MUTE_FLAG)) online_people_toggle(C.peer.identity, ONLINE_MUTED);
	if (flag_get(BLOCK_FLAG) && online_people_toggle(C.peer.identity, ONLINE_BLOCKED)) talk_start(square_blocked_words(C.peer.name), FACE_MEGAMAN);
	if (C.s.channels > 1 && (flag_get(NEXT_FLAG) || flag_get(LAST_FLAG))) {
		unsigned step = flag_get(NEXT_FLAG) ? 1u : C.s.channels - 1u;
		online_transport_channel((uint16_t)((C.s.channel - 1u + step) % C.s.channels + 1u));
	}
	/* (the greeter's Yes to the week's net: home to dive, square_week.c;
	 * the way home's flag beside it, his "Start over") */
	if (flag_get(DIVE_FLAG)) { square_week_choose(flag_get(SQUARE_DIVE_OVER_FLAG)); home = true; }
	else if (C.home && flag_get(HOME_FLAG)) home = true;
	flag_clear(DIVE_FLAG);
	for (int f = FIRST_FLAG; f <= LAST_SQUARE_FLAG; ++f) flag_clear(f);
	C.home = false;
	return home;
}

bool online_chat_update(const OnlineSnapshot *s) {
	C.s = *s;
	talk_update();
	cards();
	keeper();
	trade_central_frame(s);
	square_week_frame(s);
	square_life_frame(s);
	bool busy = talk_busy() || emu_read8(BN6_CHATBOX);
	if (busy) { C.open = C.open || talk_busy(); return false; }
	/* (BN6's own talk with the keeper sets its flags too) */
	if (chosen()) return true;
	C.open = false;
	/* (the stamp moment, once MegaMan is free: the greeter's word, then the pass with its new stamps landing) */
	if (C.stamp == 1) { C.stamp = 2; netpass_stamp_begin(); return false; }
	if (!C.stamp && netpass_stamp_due() && talk_start(square_greeter_stamp_words(), FACE_PROG)) { C.stamp = 1; return false; }
	if (C.waiting && (card_of(C.peer.id) || --C.waiting <= 0)) { C.waiting = 0; peer_talk(); return false; }
	bool on = online_square_on_exit(bn6_player_x(), bn6_player_y());
	if (on && !C.on_exit && talk_ask("", FACE_MEGAMAN, square_home_question(), HOME_FLAG)) C.home = true;
	C.on_exit = on;
	return false;
}

uint32_t online_chat_keys(uint32_t keys) {
	/* (the NetPass up over the Square: the game held under it) */
	if (netpass_screen_update()) return 0;
	if (talk_busy() || emu_read8(BN6_CHATBOX) || C.waiting) return keys & ~(uint32_t)(KEY_L | KEY_R | KEY_START | KEY_SELECT);
	keys &= ~(uint32_t)(KEY_START | KEY_SELECT);
	/* (START, the PET's button: the NetPass, BN6's PET staying shut in the Square, whose RAM goes) */
	if (btn_pressed(BTN_START)) { netpass_screen_open(); return 0; }
	if (btn_pressed(BTN_L)) { menu(); return 0; }
	if (btn_pressed(BTN_R)) {
		flag_clear(HOME_FLAG);
		if (talk_ask("", FACE_MEGAMAN, square_home_question(), HOME_FLAG)) C.home = true;
		return 0;
	}
	if (btn_pressed(BTN_A)) {
		int x = bn6_player_x(), y = bn6_player_y(), face = emu_read8(BN6_PLAYER_FACING) & 7;
		int slot = online_square_peer_ahead(x, y, face);
		if (slot >= 0 && (size_t)slot < C.s.peer_count) {
			C.peer = C.s.peers[slot];
			C.waiting = card_of(C.peer.id) ? 1 : CARD_WAIT;
			if (!card_of(C.peer.id)) online_transport_inspect(C.peer.id);
			return keys & ~(uint32_t)KEY_A;
		}
		/* (the pad home, as R and a step onto it ask: session 74 pressed A on it and nothing came) */
		if (slot == -1 && online_square_exit_ahead(x, y, face)) {
			flag_clear(HOME_FLAG);
			if (talk_ask("", FACE_MEGAMAN, square_home_question(), HOME_FLAG)) C.home = true;
			return keys & ~(uint32_t)KEY_A;
		}
		/* (a walker, or a prop: the Square's own words) */
		if (slot == -1 && square_life_talk(x, y, face)) return keys & ~(uint32_t)KEY_A;
	}
	return keys & ~(uint32_t)(KEY_L | KEY_R);
}
