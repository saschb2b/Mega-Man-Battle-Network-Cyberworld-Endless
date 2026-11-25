/* BN4's Souls' shots that are a chip (soulroute.h). */
#include "soulroute.h"

#include <stdbool.h>
#include <string.h>

#include "bn4.h"
#include "bn4_souls.h"
#include "bn6_attacks.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "soul_room.h"
#include "thumb_reloc.h"
#include "xchips.h"

#define BUS 0x08000000u
#define OP_PUSH_LR 0xB500u    /* push {lr} */
#define OP_POP_PC 0xBD00u     /* pop {pc} */
#define OP_MOVS_R0 0x2000u    /* movs r0,#imm8: the chip */
#define OP_MOVS_R1 0x2100u    /* movs r1,#imm8: a store's value */
#define CHIP_AT 2             /* the movs r0's offset, its immediate the low byte */
#define BL_AT 4               /* the bl to the chip route, on to a bx r3 of the stub's: BN6's route lies past a bl's reach */
#define VENEER 0x18           /* ldr r3,[pc,#0]; bx r3 */
#define VENEER_LIT 0x1C       /* ... its word, BN6's route with its Thumb bit */
#define ROUTES ((SOULS4_ROUTES_END - SOULS4_ROUTES) / SOULS4_ROUTE_SIZE)

static uint32_t routes[BN4_POWER_ATTACK_COUNT];   /* each power attack's copy with its Thumb bit, 1 tried and none */
static unsigned used;

/* strh or strb r1 into the attack's variables (r7 +imm) */
static bool is_store(uint16_t h) { return (h & 0xF83Fu) == 0x8039u || (h & 0xF83Fu) == 0x7039u; }

/* the bytes of BN4's routine at `at` up to its pop {pc}, 0 where it is not the chip route's shape */
static size_t shape(const uint8_t *code, uint32_t at) {
	uint32_t to;
	if (get16(code) != OP_PUSH_LR || (get16(code + CHIP_AT) & 0xFF00u) != OP_MOVS_R0 ||
		!thumb_bl_read(code, BL_AT, at, &to) || to != BN4_CHIP_ROUTE)
		return 0;
	size_t n = BL_AT + 4;
	while (n + 6 <= VENEER && (get16(code + n) & 0xFF00u) == OP_MOVS_R1 && is_store(get16(code + n + 2))) n += 4;
	return get16(code + n) == OP_POP_PC ? n + 2 : 0;
}

uint32_t soulroute_at(int power) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	if (!b4 || power < 0 || power >= BN4_POWER_ATTACK_COUNT) return 0;
	if (routes[power]) return routes[power] > 1 ? routes[power] : 0;
	routes[power] = 1;
	uint32_t at = get32(b4 + (BN4_POWER_ATTACKS - BUS) + 4u * (uint32_t)power) & ~1u;
	const uint8_t *code = b4 + (at - BUS);
	size_t n = shape(code, at);
	int chip = n ? xchips_bn4_paid(code[CHIP_AT]) : 0;
	if (!n || chip <= 0 || chip > 0xFF || used >= ROUTES) return 0;
	uint8_t out[SOULS4_ROUTE_SIZE] = { 0 };
	uint32_t place = SOULS4_ROUTES + SOULS4_ROUTE_SIZE * used;
	memcpy(out, code, n);
	out[CHIP_AT] = (uint8_t)chip;   /* (BN6's id of the chip of its name) */
	put16(out + VENEER, 0x4B00);       /* ldr r3,[pc,#0] */
	put16(out + VENEER + 2, 0x4718);   /* bx r3 */
	put32(out + VENEER_LIT, BN6_CHIP_ROUTE | 1);
	if (!thumb_bl_write(out, BL_AT, place, place + VENEER)) return 0;
	claims_begin("BN4's Souls' shots that are a chip (soulroute.c)");
	emu_write(place, out, sizeof out);
	claims_end();
	++used;
	return routes[power] = place | 1;
}
