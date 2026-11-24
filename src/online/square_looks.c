/* square_looks.h. The peers' MegaMen draw a copy of BN6's field MegaMan
 * (look_sprite.h) whose palettes, one a peer slot, change as looks are
 * picked (docs/rom/megaman-colours.md). */
#include "square_looks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "look_sprite.h"
#include "looks.h"
#include "online_square.h"
#include "online_transport.h"
#include "peer_looks.h"
#include "square_looks_pick.h"

#define STALE_FRAMES 600             /* how long the room is held smaller after a peer drew in stale colours */

static struct {
	LookSprite sprite;                /* the peers' copy of BN6's MegaMan */
	bool shown[SQUARE_PEERS];         /* drawn in his own look */
	uint32_t ids[SQUARE_PEERS];       /* the peers the pick was made for, by slot */
	Look own;                         /* MegaMan's own, as the pick saw it */
	int part, held;                   /* the part the pick was made in, and the frames it has held */
	int cap_down, cap_down_for;       /* looks taken off the part's room where a peer showed stale colours, and how long */
	int stale;                        /* peers BN6 drew in a stale slot's colours this frame */
} S;
static Look sent;                     /* MegaMan's own as last sent */
static bool ever_sent;

/* (the measured free OBJ palette slots each part leaves, docs/rom/megaman-colours.md:
 * the plaza 3, Trade Central 9, the stage 10; the Arena's ring as the plaza until measured) */
static const int caps[SQ_PARTS] = { [SQ_PART_PLAZA] = 3, [SQ_PART_TRADE] = 9, [SQ_PART_ARENA] = 3, [SQ_PART_STAGE] = 10 };

static int square_looks_cap(int part) { return part >= 0 && part < SQ_PARTS ? caps[part] : caps[SQ_PART_PLAZA]; }

int square_looks_install(void) {
	memset(&S, 0, sizeof S);
	S.part = -1;
	return look_sprite_install(&S.sprite, SQUARE_PEERS, "square");
}

static Look peer_look(const OnlinePeer *p) { return look_sprite_wire(p->look); }

static void pick(const OnlineSnapshot *shown, int n, int x, int y, int part, const Look *own) {
	LookPeer peers[SQUARE_PEERS];
	for (int i = 0; i < n; ++i) {
		const OnlinePeer *p = &shown->peers[i];
		int dx = p->x - x, dy = p->y - y;
		peers[i] = (LookPeer){ peer_look(p), abs(dx) + abs(dy), S.ids[i] == p->id && S.shown[i] };
	}
	int cap = square_looks_cap(part) - S.cap_down;
	square_looks_pick(peers, n, own, cap < 0 ? 0 : cap, SQUARE_LOOKS_KEEP, S.shown);
	for (int i = 0; i < SQUARE_PEERS; ++i) S.ids[i] = i < n ? shown->peers[i].id : 0;
	S.part = part;
	S.own = *own;
	S.held = 0;
}

/* a slot's palette in the copy's block: his look where the pick drew it, else BN6's own */
static void colour_slot(int i, const OnlinePeer *p) {
	Look l = S.shown[i] ? peer_look(p) : look_plain();
	look_sprite_colour(&S.sprite, i, &l);
}

void square_looks_frame(const OnlineSnapshot *shown, const uint32_t scripts[SQUARE_LOOKS_PEERS], int x, int y, int part) {
	if (!S.sprite.block) return;
	int n = shown->connected && shown->peer_count < SQUARE_PEERS ? (int)shown->peer_count : shown->connected ? SQUARE_PEERS : 0;
	Look own = looks_worn();
	bool again = part != S.part || !square_looks_same(&own, &S.own) || ++S.held >= SQUARE_LOOKS_HOLD;
	for (int i = 0; i < SQUARE_PEERS; ++i) again |= S.ids[i] != (i < n ? shown->peers[i].id : 0u);
	uint32_t objects[SQUARE_PEERS] = { 0 };
	S.stale = 0;
	for (int i = 0; i < n; ++i) S.stale += (objects[i] = look_sprite_object(scripts[i])) && look_sprite_stale(objects[i]);
	bool over = S.stale > 0;
	/* (BN6 ran out of slots after all, an effect or a walker more than measured: one look fewer a while) */
	if (over && S.cap_down < square_looks_cap(part)) {
		++S.cap_down;
		S.cap_down_for = STALE_FRAMES;
		again = true;
		fprintf(stderr, "square looks: a peer drawn in stale colours; %d looks fewer a while\n", S.cap_down);
	} else if (S.cap_down && --S.cap_down_for <= 0) {
		--S.cap_down;
		S.cap_down_for = STALE_FRAMES;
	}
	if (again) pick(shown, n, x, y, part, &own);
	for (int i = 0; i < n; ++i) {
		colour_slot(i, &shown->peers[i]);
		look_sprite_paint(objects[i], i);
	}
}

void square_looks_send(bool again) {
	Look worn = looks_worn();
	if (!again && ever_sent && square_looks_same(&worn, &sent)) return;
	uint8_t bytes[2 * ONLINE_LOOK_CHANNELS];
	memcpy(bytes, &worn, sizeof bytes);
	if (!online_transport_look(bytes)) return;
	sent = worn;
	ever_sent = true;
}

int square_looks_shown(uint32_t peer) {
	for (int i = 0; S.sprite.block && peer && i < SQUARE_PEERS; ++i)
		if (S.ids[i] == peer) return S.shown[i];
	return -1;
}

int square_looks_stale(void) { return S.stale; }

int square_looks_room(void) { return S.sprite.block ? square_looks_cap(S.part) - S.cap_down : 0; }

bool square_looks_sent(Look *out) {
	*out = ever_sent ? sent : look_plain();
	return ever_sent;
}

void square_looks_begin(void) {
	peer_looks_reset();
	square_looks_send(true);
}

void square_looks_update(const OnlineSnapshot *snapshot, bool up) {
	peer_looks_note(snapshot);
	if (up) square_looks_send(false);
}

_Static_assert(SQUARE_LOOKS_PEERS == SQUARE_PEERS, "a look a peer slot (online_square.h)");
