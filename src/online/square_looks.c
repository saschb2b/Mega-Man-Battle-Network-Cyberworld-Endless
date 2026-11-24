/* square_looks.h. The peers' MegaMen draw a copy of BN6's field MegaMan
 * (list 6's 0x37): its header and every animation's frames copied into the
 * map's room, each frame's offsets turned to reach BN6's own tiles and
 * objects from the copy (the renderer adds them to the sprite's address as
 * u32s) and its palettes the copy's own block: palette 0 BN6's own colours,
 * then one a peer slot, which the slot's puppet draws in (its NPC object's
 * PaletteIndex, which BN6 sets into its sprite each frame). The block's
 * colours change as looks are picked; BN6 copies a sprite's palette into an
 * OBJ slot by its content each frame, so a peer's look shows the next frame
 * and two peers in one look share a slot (docs/rom/megaman-colours.md). */
#include "square_looks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "look_apply.h"
#include "looks.h"
#include "mapslot.h"
#include "online_square.h"
#include "online_transport.h"
#include "peer_looks.h"
#include "puppet.h"
#include "rom.h"
#include "square_looks_pick.h"

#define BUS 0x08000000u
#define LIST_NAVIS 6
#define PALETTES (1 + SQUARE_PEERS)   /* BN6's own, then a peer slot's each */
#define COPY_MOST 0x1800              /* the copy's bytes at most (BN6's: 0x1530 and its block) */
#define STALE_FRAMES 600             /* how long the room is held smaller after a peer drew in stale colours */

static struct {
	uint32_t block;                   /* the copy's palettes: the first's address (0 none) */
	uint16_t colours[PALETTES][16];   /* as written */
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

static uint32_t list_entry(int number) {
	return emu_read32(BUS + R.layout->sprite_lists + LIST_NAVIS * 4) + (uint32_t)number * 4;
}

/* one offset of a frame, from BN6's sprite's data to the copy's (a u32's wrap: the copy lies past it) */
static void reach(uint8_t *frame, int at, uint32_t from, uint32_t to) { put32(frame + at, from + get32(frame + at) - to); }

/* BN6's sprite's header, animations and frames into `out` as they stand:
 * the animations' count, 0 where it is not as measured or past the room;
 * `*size` the bytes */
static uint32_t copy_frames(uint32_t sprite, uint8_t *out, uint32_t room, uint32_t *size) {
	uint32_t data = sprite + 4, anims = emu_read32(data) / 4;
	uint32_t at = 4 + anims * 4;
	if (!anims || anims > 128 || at > room) return 0;
	put32(out, emu_read32(sprite));
	for (uint32_t a = 0; a < anims; ++a) {
		put32(out + 4 + a * 4, at - 4);
		uint32_t f = data + emu_read32(data + a * 4);
		for (int k = 0;; ++k, f += BN6_LOOK_FRAME_SIZE) {
			if (k == 64 || at + BN6_LOOK_FRAME_SIZE > room) return 0;
			for (uint32_t b = 0; b < BN6_LOOK_FRAME_SIZE; ++b) out[at + b] = emu_read8(f + b);
			at += BN6_LOOK_FRAME_SIZE;
			if (out[at - BN6_LOOK_FRAME_SIZE + BN6_LOOK_FRAME_FLAGS] & BN6_LOOK_FRAME_LAST) break;
		}
	}
	*size = at;
	return anims;
}

int square_looks_install(void) {
	memset(&S, 0, sizeof S);
	S.part = -1;
	static uint8_t copy[COPY_MOST];
	uint32_t block_room = 4 + PALETTES * BN6_LOOK_PAL_SIZE;
	uint32_t sprite = emu_read32(list_entry(PUPPET_MEGAMAN)), frames = 0, anims;
	uint16_t bn6[16];
	for (int i = 0; i < 16; ++i) bn6[i] = (uint16_t)get16(R.data + (BN6_LOOK_FIELD_PALS - BUS) + 2 * i);
	if (sprite >> 31 || !(anims = copy_frames(sprite, copy, COPY_MOST - block_room, &frames))) {
		fprintf(stderr, "square looks: BN6's MegaMan not as measured; the peers in BN6's colours\n");
		return PUPPET_MEGAMAN;
	}
	/* (the block: a size word, then the palettes, BN6's own colours in each) */
	put32(copy + frames, BN6_LOOK_PAL_SIZE);
	for (int p = 0; p < PALETTES; ++p) {
		for (int i = 0; i < 16; ++i) put16(copy + frames + 4 + (uint32_t)(p * BN6_LOOK_PAL_SIZE + 2 * i), bn6[i]);
		memcpy(S.colours[p], bn6, sizeof bn6);
	}
	uint32_t size = frames + block_room, to = mapslot_alloc(copy, (int)size);
	if (!to) return PUPPET_MEGAMAN;
	/* (every frame's offsets from the copy: BN6's tiles, sub-animations and
	 * objects where they were, the palettes the copy's block) */
	uint32_t from = sprite + 4, base = to + 4;
	for (uint32_t at = 4 + anims * 4; at < frames; at += BN6_LOOK_FRAME_SIZE) {
		uint8_t *frame = copy + at;
		reach(frame, BN6_LOOK_FRAME_TILES, from, base);
		reach(frame, BN6_LOOK_FRAME_SUB, from, base);
		reach(frame, BN6_LOOK_FRAME_OAM, from, base);
		put32(frame + BN6_LOOK_FRAME_PALS, to + frames - base);
	}
	emu_write(to, copy, size);
	emu_write32(list_entry(BN6_LOOK_FIELD_SPARE), to);
	S.block = to + frames + 4;
	fprintf(stderr, "square looks: BN6's MegaMan's %u animations copied at %08x, %u bytes, list 6's %#x\n",
		(unsigned)anims, (unsigned)to, (unsigned)size, BN6_LOOK_FIELD_SPARE);
	return BN6_LOOK_FIELD_SPARE;
}

/* the NPC object a puppet's script runs (its AnimationScriptPtr within it), 0 none */
static uint32_t object_of(uint32_t script) {
	for (unsigned i = 0; script && i < BN6_NPC_COUNT; ++i) {
		uint32_t o = BN6_NPC_OBJECTS + i * BN6_NPC_SIZE;
		if (!(emu_read8(o + BN6_NPC_IN_USE) & 1u)) continue;
		uint32_t at = emu_read32(o + BN6_NPC_SCRIPT_AT);
		if (at >= script && at < script + PUPPET_BYTES) return o;
	}
	return 0;
}

/* whether BN6 drew the object's sprite past the slots it had, in a stale slot's colours */
static bool stale(uint32_t o) {
	uint32_t sprite = o + (uint32_t)(emu_read8(o + BN6_OBJ_SPRITE_AT) >> 4) * 16;
	return emu_read8(sprite + BN6_SPRITE_PAL_STALE_AT) & BN6_SPRITE_PAL_STALE;
}

static Look peer_look(const OnlinePeer *p) {
	Look l;
	memcpy(&l, p->look, sizeof l);
	return look_valid(&l) ? l : look_plain();
}

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
	uint16_t c[16];
	/* (BN6's own where the look has no field palette: palette 0's, as installed) */
	if (!look_palette_of(&l, LOOK_SPRITE_FIELD, c)) memcpy(c, S.colours[0], sizeof c);
	if (!memcmp(c, S.colours[1 + i], sizeof c)) return;
	uint8_t b[BN6_LOOK_PAL_SIZE];
	for (int k = 0; k < 16; ++k) put16(b + 2 * k, c[k]);
	emu_write(S.block + (uint32_t)(1 + i) * BN6_LOOK_PAL_SIZE, b, sizeof b);
	memcpy(S.colours[1 + i], c, sizeof c);
}

void square_looks_frame(const OnlineSnapshot *shown, const uint32_t scripts[SQUARE_LOOKS_PEERS], int x, int y, int part) {
	if (!S.block) return;
	int n = shown->connected && shown->peer_count < SQUARE_PEERS ? (int)shown->peer_count : shown->connected ? SQUARE_PEERS : 0;
	Look own = looks_worn();
	bool again = part != S.part || !square_looks_same(&own, &S.own) || ++S.held >= SQUARE_LOOKS_HOLD;
	for (int i = 0; i < SQUARE_PEERS; ++i) again |= S.ids[i] != (i < n ? shown->peers[i].id : 0u);
	uint32_t objects[SQUARE_PEERS] = { 0 };
	S.stale = 0;
	for (int i = 0; i < n; ++i) S.stale += (objects[i] = object_of(scripts[i])) && stale(objects[i]);
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
		if (objects[i] && emu_read8(objects[i] + BN6_NPC_PALETTE) != 1 + i) emu_write8(objects[i] + BN6_NPC_PALETTE, (uint8_t)(1 + i));
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
	for (int i = 0; S.block && peer && i < SQUARE_PEERS; ++i)
		if (S.ids[i] == peer) return S.shown[i];
	return -1;
}

int square_looks_stale(void) { return S.stale; }

int square_looks_room(void) { return S.block ? square_looks_cap(S.part) - S.cap_down : 0; }

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
