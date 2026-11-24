/* look_sprite.h. The copy is BN6's field MegaMan's header and every
 * animation's frames, each frame's offsets turned to reach BN6's own tiles
 * and objects from the copy (the renderer adds them to the sprite's address
 * as u32s) and its palettes the copy's own block. */
#include "look_sprite.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "emu.h"
#include "look_apply.h"
#include "mapslot.h"
#include "puppet.h"
#include "rom.h"

#define BUS 0x08000000u
#define LIST_NAVIS 6
#define COPY_MOST 0x1800   /* the copy's bytes at most (BN6's: 0x1530 and its block) */

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

int look_sprite_install(LookSprite *s, int puppets, const char *who) {
	memset(s, 0, sizeof *s);
	if (puppets > LOOK_SPRITE_PUPPETS) puppets = LOOK_SPRITE_PUPPETS;
	int palettes = 1 + puppets;
	static uint8_t copy[COPY_MOST];
	uint32_t block_room = 4 + (uint32_t)palettes * BN6_LOOK_PAL_SIZE;
	uint32_t sprite = emu_read32(list_entry(PUPPET_MEGAMAN)), frames = 0, anims;
	uint16_t bn6[16];
	for (int i = 0; i < 16; ++i) bn6[i] = (uint16_t)get16(R.data + (BN6_LOOK_FIELD_PALS - BUS) + 2 * i);
	if (sprite >> 31 || !(anims = copy_frames(sprite, copy, COPY_MOST - block_room, &frames))) {
		fprintf(stderr, "%s looks: BN6's MegaMan not as measured; the others in his colours\n", who);
		return PUPPET_MEGAMAN;
	}
	/* (the block: a size word, then the palettes, BN6's own colours in each) */
	put32(copy + frames, BN6_LOOK_PAL_SIZE);
	for (int p = 0; p < palettes; ++p) {
		for (int i = 0; i < 16; ++i) put16(copy + frames + 4 + (uint32_t)(p * BN6_LOOK_PAL_SIZE + 2 * i), bn6[i]);
		memcpy(s->colours[p], bn6, sizeof bn6);
	}
	uint32_t size = frames + block_room, to = mapslot_alloc(copy, (int)size);
	if (!to) {
		fprintf(stderr, "%s looks: no room in the map for BN6's MegaMan's copy (%u bytes); the others in his colours\n", who,
			(unsigned)size);
		return PUPPET_MEGAMAN;
	}
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
	s->block = to + frames + 4;
	s->puppets = puppets;
	fprintf(stderr, "%s looks: BN6's MegaMan's %u animations copied at %08x, %u bytes, list 6's %#x\n", who, (unsigned)anims,
		(unsigned)to, (unsigned)size, BN6_LOOK_FIELD_SPARE);
	return BN6_LOOK_FIELD_SPARE;
}

void look_sprite_colour(LookSprite *s, int k, const Look *l) {
	if (!s->block || k < 0 || k >= s->puppets) return;
	uint16_t c[16];
	/* (BN6's own where the look has no field palette: palette 0's, as installed) */
	if (!l || !look_palette_of(l, LOOK_SPRITE_FIELD, c)) memcpy(c, s->colours[0], sizeof c);
	if (!memcmp(c, s->colours[1 + k], sizeof c)) return;
	uint8_t b[BN6_LOOK_PAL_SIZE];
	for (int i = 0; i < 16; ++i) put16(b + 2 * i, c[i]);
	emu_write(s->block + (uint32_t)(1 + k) * BN6_LOOK_PAL_SIZE, b, sizeof b);
	memcpy(s->colours[1 + k], c, sizeof c);
}

uint32_t look_sprite_object(uint32_t script) {
	for (unsigned i = 0; script && i < BN6_NPC_COUNT; ++i) {
		uint32_t o = BN6_NPC_OBJECTS + i * BN6_NPC_SIZE;
		if (!(emu_read8(o + BN6_NPC_IN_USE) & 1u)) continue;
		uint32_t at = emu_read32(o + BN6_NPC_SCRIPT_AT);
		if (at >= script && at < script + PUPPET_BYTES) return o;
	}
	return 0;
}

bool look_sprite_stale(uint32_t o) {
	uint32_t sprite = o + (uint32_t)(emu_read8(o + BN6_OBJ_SPRITE_AT) >> 4) * 16;
	return emu_read8(sprite + BN6_SPRITE_PAL_STALE_AT) & BN6_SPRITE_PAL_STALE;
}

void look_sprite_paint(uint32_t object, int k) {
	if (object && emu_read8(object + BN6_NPC_PALETTE) != 1 + k) emu_write8(object + BN6_NPC_PALETTE, (uint8_t)(1 + k));
}

Look look_sprite_wire(const uint8_t bytes[2 * LOOK_CHANNELS]) {
	Look l;
	memcpy(&l, bytes, sizeof l);
	return look_valid(&l) ? l : look_plain();
}
