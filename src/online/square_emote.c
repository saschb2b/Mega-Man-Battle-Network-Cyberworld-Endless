#include "square_emote.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "debug.h"
#include "emu.h"
#include "game.h"
#include "gamecall.h"
#include "network_prefs.h"
#include "npc.h"
#include "online_people.h"
#include "online_square.h"
#include "online_text.h"
#include "online_transport.h"
#include "platform.h"
#include "square_life.h"
#include "talk.h"
#include "text.h"

/* An emote as MegaMan performs it: BN6's animation of his overworld
 * sprite (BN6_OW_MEGAMAN; four ways: + the way of four he faces; a turn:
 * his eight standing facings in turn, TURN_STEP frames each, from the one
 * after his own round to it), held so many frames from its beat, BN6's
 * sound with it, and BN6's "!!" over his head too. The proposed mapping
 * (issue #176), the owner's to adjust. */
typedef struct {
	uint8_t anim;
	bool four_ways;
	uint8_t frames;
	uint16_t sound;
	bool burst, turn;
} Pose;
#define TURN_STEP 6

static const Pose poses[ONLINE_EMOTE_COUNT] = {
	{ BN6_OW_ANIM_GOT, false, 60, BN6_SOUND_DOORBELL, false },       /* Wave: a hand raised, "ding, dong" */
	{ BN6_OW_ANIM_ARMS_UP, false, 72, BN6_SOUND_POPS, false },       /* Cheer: both arms up, "pop,pop,pop!!" */
	{ BN6_OW_ANIM_CROUCH, false, 60, BN6_SOUND_TYPE, false },        /* Laugh: doubled over, shaking ("Haha!") */
	{ BN6_OW_ANIM_STARTLED, true, 45, BN6_SOUND_BEEP, true },         /* Surprised: a step back under BN6's "!!" */
	{ BN6_OW_ANIM_PONDER, false, 90, BN6_SOUND_CURSOR, false },      /* Thinking: crouched, looking about ("Hmm...") */
	{ BN6_OW_ANIM_KNEEL, false, 60, BN6_SOUND_CHIME, false },        /* Bow: on one knee, the chime */
	{ BN6_OW_ANIM_GOT, false, 70, BN6_SOUND_GOT, false },            /* Got it!: the item-get pose and its jingle */
	{ BN6_OW_ANIM_AIM, true, 60, BN6_SOUND_BUSTER_CHARGE, false },   /* Buster: held out the way he faces, charging */
	{ BN6_OW_ANIM_PANTING, false, 90, BN6_SOUND_FADE, false },       /* Phew...: on one knee, heaving */
	{ BN6_OW_ANIM_COLLAPSE, false, 90, BN6_SOUND_FALL, false },      /* I'm beat: down, "Waaa!! *thud*" */
	{ 0, false, 8 * TURN_STEP, BN6_SOUND_LOG_IN, false, true },      /* Spin: turning round once on the spot, the log-in's whoosh */
	{ BN6_OW_ANIM_SHOCKED, false, 60, BN6_SOUND_HIT, false },        /* Zapped: sparks beside him */
};

/* L held with up, right, down, left: Wave, Cheer, Bow, Laugh */
static const int default_favourites[4] = { 0, 1, 5, 2 };

#define PENDING 16
#define INVITE_MS (ONLINE_ANSWER_MS - 500)   /* an answer offered so long (the server's window, less the trip) */
#define SEND_MS (ONLINE_SOCIAL_REFILL_MS + 100)   /* a hair slower than the server's refill: its jitter never trips it */
#define LOCAL_SLACK 6   /* frames past an emote's own before MegaMan's keys come back */
#define AIM_REACH 56    /* world units an aimed emote reaches, in a cone of his facing (about three panels) */
#define BURST_Z (-30)   /* BN6's "!!" over a MegaMan's head (square_life.c's at a walker's, taller, less this) */
#define ANSWER_Z (-16)  /* ... between two MegaMen, over both */

typedef struct { uint32_t id, target, start; uint8_t emote, flags; } Pending;
typedef struct { uint32_t id, until; uint8_t emote; bool gap, sound; } Act;

static struct {
	uint32_t event_serial, credit, credit_at, invited, invited_until, local_until;
	Pending pend[PENDING];
	int npend;
	Act acts[ONLINE_MAX_PLAYERS];
	int favourites[4];
	bool quiet, ready;
} E;

void square_emote_reset(void) {
	memset(&E, 0, sizeof E);
	E.credit = ONLINE_SOCIAL_BURST * SEND_MS;
	NetworkPrefs p;
	network_prefs_read(g_data_dir, &p);
	E.quiet = p.quiet != 0;
	for (int k = 0; k < 4; ++k)
		E.favourites[k] = p.favourites[k] && p.favourites[k] <= ONLINE_EMOTE_COUNT ? p.favourites[k] - 1 : default_favourites[k];
}

bool square_emote_quiet(void) { return E.quiet; }

void square_emote_set_quiet(bool on) {
	E.quiet = on;
	NetworkPrefs p;
	if (!network_prefs_read(g_data_dir, &p)) return;
	p.quiet = on;
	if (!network_prefs_write(g_data_dir, &p)) fprintf(stderr, "online: the quiet could not be kept\n");
}

int square_emote_favourite(int way) { return E.favourites[way & 3]; }

/* a peer of the view by his session, NULL gone (or blocked out of it) */
static const OnlinePeer *peer_of(const OnlineSnapshot *s, uint32_t id, int *slot) {
	for (size_t i = 0; i < s->peer_count; ++i)
		if (s->peers[i].id == id) { if (slot) *slot = (int)i; return &s->peers[i]; }
	return NULL;
}

/* whether this game shows what player `id` does: MegaMan's own always;
 * another's unless muted, blocked or the quiet is on */
static bool shown_from(const OnlineSnapshot *s, uint32_t id) {
	if (id == s->player_id) return true;
	const OnlinePeer *p = peer_of(s, id, NULL);
	return p && !E.quiet && !(online_people_flags(p->identity) & (ONLINE_MUTED | ONLINE_BLOCKED));
}

static bool where(const OnlineSnapshot *s, uint32_t id, int *x, int *y) {
	if (id == s->player_id) { *x = bn6_player_x(); *y = bn6_player_y(); return true; }
	const OnlinePeer *p = peer_of(s, id, NULL);
	if (p) { *x = p->x; *y = p->y; }
	return p != NULL;
}

/* world (x, y) on MegaMan's screen, by BN6's projection about him
 * (online_labels.c's, the camera on him) */
static bool on_screen(int x, int y) {
	int dx = x - bn6_player_x(), dy = y - bn6_player_y();
	int sx = 120 + dx + dy, sy = 82 + (dy - dx) / 2;
	return sx >= -16 && sx < EMU_W + 16 && sy >= -16 && sy < EMU_H + 32;
}

uint32_t square_emote_faced(const OnlineSnapshot *s) {
	int x = bn6_player_x(), y = bn6_player_y(), dx, dy;
	npc_probe(emu_read8(BN6_PLAYER_FACING) & 7, &dx, &dy);
	long probe = (long)dx * dx + (long)dy * dy, best_d = (long)AIM_REACH * AIM_REACH + 1;
	uint32_t best = 0;
	for (size_t i = 0; i < s->peer_count; ++i) {
		long px = s->peers[i].x - x, py = s->peers[i].y - y, d = px * px + py * py, dot = px * dx + py * dy;
		/* (within 60 degrees of his way: the dot at least half the lengths' product, squared) */
		if (d >= best_d || dot <= 0 || 4 * dot * dot < d * probe) continue;
		best = s->peers[i].id;
		best_d = d;
	}
	return best;
}

/* the phrases' and emotes' events since the last frame: an emote waits
 * for its beat; one aimed at MegaMan invites his answer */
static void collect(const OnlineSnapshot *s) {
	uint32_t newest = E.event_serial;
	for (unsigned i = 0; i < ONLINE_EVENT_HISTORY; ++i) {
		const OnlineEvent *e = &s->events[i];
		if (!e->serial || (int32_t)(e->serial - E.event_serial) <= 0) continue;
		if ((int32_t)(e->serial - newest) > 0) newest = e->serial;
		if (!E.ready || e->kind != 1 || !shown_from(s, e->player_id) || E.npend == PENDING) continue;
		E.pend[E.npend++] = (Pending){ e->player_id, e->target, e->start, e->value, e->flags };
		if (e->target == s->player_id && e->player_id != s->player_id && !(e->flags & ONLINE_EVENT_ANSWERED)) {
			E.invited = e->player_id;
			E.invited_until = e->start + INVITE_MS;
			if (emu_debug_on()) fprintf(stderr, "online: player %u aims at MegaMan: an answer until %u (ticks %u)\n", e->player_id, E.invited_until, e->ticks);
		}
		/* (his answer to MegaMan's: nothing more to answer) */
		if (e->target == s->player_id && (e->flags & ONLINE_EVENT_ANSWERED) && E.invited == e->player_id) E.invited = 0;
	}
	E.event_serial = newest;
	E.ready = true;
}

/* MegaMan's own pose through BN6's text commands (bn6f ts_player_lock,
 * ts_player_animate_scene, ts_sound_play00, ts_wait, ts_player_reset_scene,
 * ts_player_unlock), as BN6's item-get plays his 0x18: no chat box opens */
static void perform_local(const Pose *p, int anim, uint32_t now) {
	if (talk_busy() || emu_read8(BN6_CHATBOX)) return;
	static TextArchive t;
	static uint8_t out[TEXT_ARCHIVE_MAX];
	ta_begin(&t);
	int i = ta_script(&t);
	const uint8_t lock[] = { BN6_TEXT_PLAYER, 0x00, BN6_TEXT_SOUND, 0x00, (uint8_t)p->sound, (uint8_t)(p->sound >> 8) };
	ta_bytes(&t, lock, sizeof lock);
	/* (a turn: each facing in turn; else the pose held) */
	int steps = p->turn ? 8 : 1, wait = p->turn ? TURN_STEP : p->frames;
	for (int k = 1; k <= steps; ++k) {
		uint8_t pose[] = { BN6_TEXT_PLAYER, 0x01, (uint8_t)(p->turn ? (anim + k) & 7 : anim), 0xEE, 0x00, (uint8_t)wait, 0x00 };
		ta_bytes(&t, pose, sizeof pose);
	}
	/* (his own animation back, F8 04: the one before the last F8 01, so a
	 * turn, whose last step is his own facing again, goes without it) */
	const uint8_t unlock[] = { BN6_TEXT_PLAYER, 0x04, BN6_TEXT_PLAYER, 0x02 };
	ta_bytes(&t, p->turn ? unlock + 2 : unlock, p->turn ? 2 : (int)sizeof unlock);
	ta_end(&t);
	int n = ta_build(&t, out);
	if (n > SQUARE_TALK_ROOM) return;
	emu_write(online_square_talk_room(), out, (size_t)n);
	game_call(BN6_CHAT_RUN_SCRIPT, online_square_talk_room(), (uint32_t)i);
	E.local_until = now + (uint32_t)(p->frames + LOCAL_SLACK) * 1000u / 60u;
}

/* the wall clock in ms, for the debug log: games on one machine compared
 * (their emotes on one beat) */
static unsigned long long wall_ms(void) {
#ifdef TIME_UTC
	struct timespec ts;
	if (timespec_get(&ts, TIME_UTC)) return (unsigned long long)ts.tv_sec * 1000u + (unsigned long long)ts.tv_nsec / 1000000u;
#endif
	return 0;
}

static Act *act_of(uint32_t id) {
	Act *free_one = NULL;
	for (int i = 0; i < ONLINE_MAX_PLAYERS; ++i) {
		if (E.acts[i].id == id) return &E.acts[i];
		if (!E.acts[i].id && !free_one) free_one = &E.acts[i];
	}
	return free_one;
}

/* emote `p` begun on its beat: his act (a gap of a frame where the same
 * pose plays again: BN6 starts an animation only as it changes), the
 * "!!" of an answer between the two, a Surprised's over him */
static void begin(const OnlineSnapshot *s, const Pending *p, uint32_t now) {
	Act *a = act_of(p->id);
	if (!a) return;
	bool again = a->id && now < a->until && poses[a->emote].anim == poses[p->emote].anim;
	*a = (Act){ p->id, now + poses[p->emote].frames * 1000u / 60u, p->emote, again, true };
	int x, y, tx, ty;
	if ((p->flags & ONLINE_EVENT_ANSWERED) && where(s, p->id, &x, &y) && where(s, p->target, &tx, &ty))
		square_life_mark((x + tx) / 2, (y + ty) / 2, ANSWER_Z, BN6_SOUND_APPEAR);
	else if (poses[p->emote].burst && where(s, p->id, &x, &y)) square_life_mark(x, y, BURST_Z, 0);
	if (emu_debug_on()) fprintf(stderr, "online: player %u performs emote %d (%s)%s%s at %llu ms (ticks %u)\n", p->id, p->emote, online_emotes[p->emote],
		p->target ? " aimed" : "", p->flags & ONLINE_EVENT_ANSWERED ? ", an answer" : "", (unsigned long long)wall_ms(), now);
}

/* the animation of pose `p` for a MegaMan facing `facing`, `into` frames
 * into it (a turn's facing by then; the local one's script turns itself) */
static int pose_anim(const Pose *p, int facing, int into) {
	if (p->turn) return (facing + 1 + into / TURN_STEP) & 7;
	return p->anim + (p->four_ways ? (facing & 7) / 2 : 0);
}

/* each act this frame: a peer's pose over his puppet's walk, MegaMan's own
 * once; a sound a frame at most, from one in view, his own or aimed at him */
static void perform(const OnlineSnapshot *s, uint32_t now) {
	bool sounded = false;
	for (int i = 0; i < ONLINE_MAX_PLAYERS; ++i) {
		Act *a = &E.acts[i];
		if (!a->id) continue;
		if ((int32_t)(now - a->until) >= 0) { a->id = 0; continue; }
		const Pose *p = &poses[a->emote];
		if (a->gap) { a->gap = false; continue; }
		int x, y, slot = -1;
		bool want = a->sound && !sounded && where(s, a->id, &x, &y) && (a->id == s->player_id || on_screen(x, y));
		int into = (int)(now + (uint32_t)p->frames * 1000u / 60u - a->until) * 60 / 1000;
		if (a->id == s->player_id) {
			int facing = emu_read8(BN6_PLAYER_FACING) & 7;
			if (a->sound && !square_emote_busy()) perform_local(p, p->turn ? facing : pose_anim(p, facing, 0), now);
		} else {
			const OnlinePeer *peer = peer_of(s, a->id, &slot);
			if (peer) online_square_peer_pose((unsigned)slot, pose_anim(p, peer->facing, into), want ? p->sound : 0);
		}
		if (want && emu_debug_on()) fprintf(stderr, "online: emote %d's sound %#x from player %u\n", a->emote, p->sound, a->id);
		sounded |= want;
		a->sound = false;
	}
}

void square_emote_frame(const OnlineSnapshot *shown, uint32_t now) {
	collect(shown);
	/* (the beat come: begun in the order they came) */
	int kept = 0;
	for (int i = 0; i < E.npend; ++i) {
		if ((int32_t)(now - E.pend[i].start) >= 0) begin(shown, &E.pend[i], now);
		else E.pend[kept++] = E.pend[i];
	}
	E.npend = kept;
	perform(shown, now);
}

bool square_emote_busy(void) { return E.local_until && (int32_t)(SDL_GetTicks() - E.local_until) < 0; }

const char *square_emote_caption(uint32_t id, uint32_t now) {
	for (int i = 0; i < ONLINE_MAX_PLAYERS; ++i)
		if (E.acts[i].id == id && (int32_t)(now - E.acts[i].until) < 0) return online_emote_captions[E.acts[i].emote];
	return NULL;
}

uint32_t square_emote_invited(uint32_t now) {
	if (E.invited && (int32_t)(now - E.invited_until) >= 0) E.invited = 0;
	return E.invited;
}

/* the burst: ONLINE_SOCIAL_BURST at once, one back each SEND_MS */
static bool spend(uint32_t now) {
	uint32_t most = ONLINE_SOCIAL_BURST * SEND_MS, grown = E.credit + (now - E.credit_at);
	E.credit = grown > most || grown < E.credit ? most : grown;
	E.credit_at = now;
	if (E.credit < SEND_MS) return false;
	E.credit -= SEND_MS;
	return true;
}

bool square_emote_send(int emote, uint32_t facing, uint32_t now) {
	if (emote < 0 || emote >= ONLINE_EMOTE_COUNT || !spend(now)) return false;
	bool aims = ONLINE_EMOTE_AIMED >> emote & 1;
	uint32_t invited = square_emote_invited(now), target = aims ? (invited ? invited : facing) : 0;
	if (target && target == invited) E.invited = 0;
	if (emu_debug_on()) fprintf(stderr, "online: emote %d sent%s%s (ticks %u)\n", emote, target ? " aimed" : "", target && target == invited ? ", answering" : "", now);
	return online_transport_social(1, (uint8_t)emote, target);
}

bool square_emote_say(int phrase, uint32_t now) {
	return phrase >= 0 && phrase < ONLINE_PHRASE_COUNT && spend(now) && online_transport_social(0, (uint8_t)phrase, 0);
}
