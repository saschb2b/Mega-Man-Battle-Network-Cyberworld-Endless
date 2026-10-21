/* online_chat.h. The Net Square's chats (docs/ONLINE.md), all in BN6's own
 * chat boxes: L asks MegaMan to say something or do an emote (the
 * protocol's quick phrases and emotes, a page of two at a time), L held
 * with a direction sends one of four favourite emotes at once
 * (square_emote.h), and L's More asks to look around or to quiet everyone
 * else's; A facing
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
#include "square_emote.h"
#include "square_team.h"
#include "square_life.h"
#include "square_week.h"
#include "square_words.h"
#include "talk.h"
#include "trade_central.h"

/* The Square's event flags (its own map's, free while MegaMan is there) */
#define SAY_FLAG     0x1480   /* + phrase 0-7 */
#define EMOTE_FLAG   0x1488   /* + emote 0-5 */
#define EMOTE_FLAG2  0x14CA   /* + emote 6-11 (issue #176) */
#define QUIET_FLAG   0x14D0   /* L's Quiet please or Let them talk (issue #176) */
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
#define EMOTES_LOW 6     /* the emotes of EMOTE_FLAG's; the rest EMOTE_FLAG2's */

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
	bool card_open;           /* a peer's card open in the chat box (online_chat_card_open) */
	bool l_held, l_used;      /* L down, and a favourite sent with it (its release opens no menu) */
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
	flag_clear(QUIET_FLAG);
	for (int k = EMOTES_LOW; k < ONLINE_EMOTE_COUNT; ++k) flag_clear(EMOTE_FLAG2 + k - EMOTES_LOW);
	square_emote_reset();
}

static const OnlineProfile *card_of(uint32_t id) {
	for (size_t i = 0; i < sizeof C.cards / sizeof *C.cards; ++i)
		if (C.cards[i].id == id) return &C.cards[i].card;
	return NULL;
}

const OnlineProfile *online_chat_card(uint32_t id) { return id ? card_of(id) : NULL; }

uint32_t online_chat_card_open(void) { return C.card_open && (talk_busy() || emu_read8(BN6_CHATBOX)) ? C.peer.id : 0; }

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

static int emote_flag(int k) { return k < EMOTES_LOW ? EMOTE_FLAG + k : EMOTE_FLAG2 + k - EMOTES_LOW; }

/* L's chat into the Square's talk room: the question, then pages of two
 * phrases or emotes, each with More (or Back to the first); More's look
 * around (square_life_look) and the quiet (square_emote.h) */
#define SAY_PAGES (ONLINE_PHRASE_COUNT / 2)
#define EMOTE_PAGES (ONLINE_EMOTE_COUNT / 2)
static void menu(void) {
	static TextArchive t;
	static uint8_t out[TEXT_ARCHIVE_MAX];
	enum { TOP, SAY = 1, EMOTE = SAY + SAY_PAGES, MORE = EMOTE + EMOTE_PAGES, PHRASE, DO = PHRASE + ONLINE_PHRASE_COUNT,
		LOOK = DO + ONLINE_EMOTE_COUNT, QUIET, END };
	ta_begin(&t);
	ta_script(&t);
	ta_page(&t, FACE_MEGAMAN, square_menu_question(), true);
	ta_mugshot(&t, FACE_MEGAMAN);
	ta_clear(&t);
	const char *top_labels[3] = { square_menu_options[0], square_menu_options[1], square_more() };
	ta_options(&t, top_labels, 3);
	int top[3] = { SAY, EMOTE, MORE };
	ta_choose(&t, top, 3, END);
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
	/* (More: Look around, the quiet, Back) */
	ta_script(&t);
	ta_mugshot(&t, FACE_MEGAMAN);
	ta_clear(&t);
	const char *more[3] = { square_menu_options[2], square_quiet_option(square_emote_quiet()), square_back() };
	ta_options(&t, more, 3);
	int to_more[3] = { LOOK, QUIET, TOP };
	ta_choose(&t, to_more, 3, END);
	for (int k = 0; k < ONLINE_PHRASE_COUNT + ONLINE_EMOTE_COUNT; ++k) {
		ta_script(&t);
		ta_flag_set(&t, k < ONLINE_PHRASE_COUNT ? SAY_FLAG + k : emote_flag(k - ONLINE_PHRASE_COUNT));
		ta_end(&t);
	}
	ta_script(&t);
	ta_flag_set(&t, LOOK_FLAG);
	ta_end(&t);
	ta_script(&t);
	ta_flag_set(&t, QUIET_FLAG);
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
	/* (our challenge to him open: Call off in Battle's place, protocol v13) */
	const char *battle = square_arena_open_to(C.peer.id) ? arena_call_off_option() : arena_battle_option();
	const char *boxes[3][3] = {
		{ battle, square_team_option(&C.peer), square_more() },
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
	C.card_open = true;
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

/* the session of the MegaMan this one faces, near enough to talk to, 0
 * none: an emote's aim */
static uint32_t facing_peer(void) {
	int slot = online_square_peer_ahead(bn6_player_x(), bn6_player_y(), emu_read8(BN6_PLAYER_FACING) & 7);
	return slot >= 0 && (size_t)slot < C.s.peer_count ? C.s.peers[slot].id : 0;
}

/* the flags of the chat just closed, acted on; true: MegaMan heads home */
static bool chosen(void) {
	bool home = false;
	uint32_t now = SDL_GetTicks();
	for (int k = 0; k < ONLINE_PHRASE_COUNT; ++k) if (flag_get(SAY_FLAG + k)) square_emote_say(k, now);
	for (int k = 0; k < ONLINE_EMOTE_COUNT; ++k)
		if (flag_get(emote_flag(k))) { flag_clear(emote_flag(k)); square_emote_send(k, facing_peer(), now); }
	if (flag_get(BATTLE_FLAG)) { flag_clear(BATTLE_FLAG); square_arena_challenge(&C.peer, C.s.latency_ms); }
	if (flag_get(TEAM_FLAG)) { flag_clear(TEAM_FLAG); square_team_chosen(&C.peer); }
	/* (the card's Wave: at him) */
	if (flag_get(WAVE_FLAG)) square_emote_send(WAVE, C.peer.id, now);
	if (flag_get(LOOK_FLAG)) { flag_clear(LOOK_FLAG); square_life_look(); }
	if (flag_get(QUIET_FLAG)) { flag_clear(QUIET_FLAG); square_emote_set_quiet(!square_emote_quiet()); }
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
	C.card_open = false;
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

/* L down: a direction sends its favourite emote (MegaMan stands while L
 * is held); L let go without one opens L's chat */
static uint32_t held_l(uint32_t keys) {
	static const uint32_t ways[4] = { BTN_UP, BTN_RIGHT, BTN_DOWN, BTN_LEFT };
	for (int k = 0; k < 4; ++k)
		if (btn_pressed(ways[k])) {
			C.l_used = true;
			square_emote_send(square_emote_favourite(k), facing_peer(), SDL_GetTicks());
		}
	if (!btn_held(BTN_L)) {
		C.l_held = false;
		if (!C.l_used) menu();
		return 0;
	}
	return keys & ~(uint32_t)(KEY_L | KEY_R | KEY_A | KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT);
}

uint32_t online_chat_keys(uint32_t keys) {
	/* (the NetPass up over the Square: the game held under it) */
	if (netpass_screen_update()) return 0;
	if (talk_busy() || emu_read8(BN6_CHATBOX) || C.waiting) { C.l_held = false; return keys & ~(uint32_t)(KEY_L | KEY_R | KEY_START | KEY_SELECT); }
	keys &= ~(uint32_t)(KEY_START | KEY_SELECT);
	/* (MegaMan's own emote under way: he stands still, as BN6 holds him for its own) */
	if (square_emote_busy()) return 0;
	/* (START, the PET's button: the NetPass, BN6's PET staying shut in the Square, whose RAM goes) */
	if (btn_pressed(BTN_START)) { netpass_screen_open(); return 0; }
	if (btn_pressed(BTN_L)) { C.l_held = true; C.l_used = false; }
	if (C.l_held) return held_l(keys);
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
