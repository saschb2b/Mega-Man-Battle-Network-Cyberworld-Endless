/* A carried MegaMan in an Arena core (arena_loadout.h). The addresses
 * are the ones the carried record is read from (online_snapshot.c), each
 * a bn6.h name (docs/ROM_DATA.md). */
#include "arena_loadout.h"

#include <stdio.h>

#include "arena_format.h"
#include "bn6.h"
#include "protocol.h"

#define CHIP_ID(e)   ((unsigned)(e) & 0x1FFu)   /* a folder entry: chip | code << 9 */
#define CHIP_CODE(e) ((unsigned)(e) >> 9 & 0x1Fu)
#define CODE_STAR    26                          /* '*', the last code */
#define HP_MOST      9999

/* false, the reason said in `why` */
#define REFUSE(...) ((why && n ? (void)snprintf(why, n, __VA_ARGS__) : (void)0), false)

/* a chip id `format`'s cores hold: under BN6's rules BN6's own (1 to
 * ARENA_BN6_CHIP_LAST, its blanks left out); under full ones every id of
 * the chip registry (BN5's chips in BN6's tables, its blanks and the
 * pack's extension among them: the same in every game with BN5's ROM,
 * docs/MULTIROM.md, The chip registry) */
static bool chip_held(unsigned id, unsigned format) {
	if (format == ONLINE_FORMAT_FULL) return id < ONLINE_PACK_IDS;
	return id <= ARENA_BN6_CHIP_LAST && !BN6_CHIP_BLANK(id);
}

bool arena_loadout_check(const OnlineLoadout *l, unsigned format, char *why, size_t n) {
	bool full = format == ONLINE_FORMAT_FULL;
	if (l->souls && !(full && ARENA_FULL_SOULS)) return REFUSE("MegaMan holds Souls (mask %#x): BN5's, which %s cannot run", l->souls,
		full ? "full rules do not carry yet" : "plain BN6");
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
		uint16_t e = l->folder[i];
		if (e == 0xFFFF || !CHIP_ID(e)) continue;
		if (!chip_held(CHIP_ID(e), format)) return REFUSE("the folder holds chip %u: another game's, which plain BN6 has not", CHIP_ID(e));
		if (CHIP_CODE(e) > CODE_STAR) return REFUSE("the folder holds a chip of code %u, which BN6 has not", CHIP_CODE(e));
	}
	for (unsigned id = 1; id < ONLINE_PACK_IDS; ++id)
		for (unsigned k = 0; k < BN6_PACK_ENTRY && !chip_held(id, format); ++k)
			if (l->pack[id][k]) return REFUSE("the pack holds chip %u: another game's, which plain BN6 has not", id);
	if (!l->max_hp || l->max_hp > HP_MOST) return REFUSE("MegaMan's max HP (%u) is not one BN6 holds", l->max_hp);
	return true;
}

static void write16(const ArenaMemory *m, uint32_t at, uint16_t v) {
	m->write8(m->context, at, (uint8_t)v);
	m->write8(m->context, at + 1, (uint8_t)(v >> 8));
}

static void write_bytes(const ArenaMemory *m, uint32_t at, const uint8_t *data, unsigned count) {
	for (unsigned i = 0; i < count; ++i) m->write8(m->context, at + i, data[i]);
}

/* an event flag set or cleared (BN6_EVENT_FLAGS: a bit a flag, its byte's highest first) */
static void write_flag(const ArenaMemory *m, unsigned flag, bool on) {
	uint32_t at = BN6_EVENT_FLAGS + flag / 8;
	uint8_t byte = (uint8_t)(m->read32(m->context, at & ~3u) >> (8 * (at & 3u)));
	uint8_t bit = (uint8_t)(0x80u >> (flag & 7));
	m->write8(m->context, at, on ? (uint8_t)(byte | bit) : (uint8_t)(byte & ~bit));
}

bool arena_loadout_install(const ArenaMemory *m, const OnlineLoadout *l, unsigned format, char *why, size_t n) {
	if (!arena_loadout_check(l, format, why, n)) return false;
	uint32_t folder = m->read32(m->context, BN6_TOOLKIT + BN6_TOOLKIT_CHIPS);
	uint32_t pack = m->read32(m->context, BN6_TOOLKIT + BN6_TOOLKIT_PACK);
	if ((folder & 1u) || folder < BN6_EWRAM || folder > BN6_EWRAM_END - 2 * BN6_FOLDER_ENTRIES ||
		pack < BN6_EWRAM || pack > BN6_EWRAM_END - BN6_PACK_IDS * BN6_PACK_ENTRY)
		return REFUSE("the core's folder is not where BN6 keeps it (%#x)", folder);
	/* the first folder, and the pack's own ids (its extension, BN5's chips' counts, is the pack's clear's at NEW GAME: a
	 * carried pack is the folder's alone, arena_wire.h) */
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) write16(m, folder + 2 * i, l->folder[i]);
	for (unsigned id = 0; id < BN6_PACK_IDS; ++id) write_bytes(m, pack + id * BN6_PACK_ENTRY, l->pack[id], BN6_PACK_ENTRY);
	write_bytes(m, BN6_NAVICUST_PLACED, l->board, ONLINE_BOARD_BYTES);
	write_bytes(m, BN6_NAVICUST_GRID, l->grid, ONLINE_BOARD_CELLS);
	/* (a NetBattle begins whole: HP at its max) */
	write16(m, BN6_NAVI_HP, l->max_hp);
	write16(m, BN6_NAVI_MAX_HP, l->max_hp);
	write16(m, BN6_NAVI_BASE_MAX_HP, l->base_hp);
	const struct { uint32_t at; uint8_t v; } bytes[] = {
		{ BN6_NAVI_ATTACK, l->attack }, { BN6_NAVI_SPEED, l->speed }, { BN6_NAVI_CHARGE, l->charge },
		{ BN6_NAVI_REG, l->regular_mb }, { BN6_NAVI_MEGA_LEVEL, l->mega_limit }, { BN6_NAVI_GIGA_LEVEL, l->giga_limit },
		{ BN6_NAVI_FOLDER1_REG, l->regular }, { BN6_NAVI_FOLDER1_TAG, l->tag[0] }, { BN6_NAVI_FOLDER1_TAG2, l->tag[1] },
	};
	for (size_t i = 0; i < sizeof bytes / sizeof *bytes; ++i) m->write8(m->context, bytes[i].at, bytes[i].v);
	for (unsigned i = 0; i < 5; ++i) write_flag(m, BN6_FLAG_HEAT_CROSS + i, l->crosses >> i & 1u);
	write_flag(m, BN6_FLAG_BEAST_OUT, l->beast != 0);
	return true;
}
