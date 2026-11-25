/* BN4's Souls' companions in BN6's engine (soulcompanion.h). */
#include "soulcompanion.h"

#include <stdio.h>
#include <string.h>

#include "bn4.h"
#include "bn4_souls.h"
#include "bn6.h"
#include "bn6_forms.h"
#include "bn6_navis.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"
#include "soul_room.h"
#include "soulbn4.h"
#include "thumb_reloc.h"
#include "xnavi_room.h"
#include "xvirus_tx.h"

#define BUS 0x08000000u
#define LIST_SIZE 4           /* a sprite list's byte offset per list (BN4_FORM_COMPANIONS' low byte, sprite_load's r1) */
#define OP_MOVS_R0 0x2000u    /* movs r0,#imm8 */
#define OP_STR_R0_R7_48 0x64B8u   /* str r0,[r7,#0x48]: BN4_SOUL_COMPANION's keeping at his AI +0x48 */

static bool done;
static int both_k = -1;   /* the BN4 Soul whose stub SOULS_BN4_BOTH is, -1 none */

/* The stub of BN4's k-th Soul, who has both (as BN4_FORM_STATE runs the form's routine, BN4_SOUL_COMPANION spawns at the unison):
 * the overlay of BN4's Souls' kind through BN6's own spawn (BN6_FORM_OVERLAY_SPAWN), then his taking of the form's copy; r5 him,
 * entered from the proxy form's overlay routine's call */
static bool both_write(int k, uint32_t enter) {
	static const uint16_t code[] = {
		0xB500,                       /* 00 push {lr} */
		0x2000 | SOUL_OVERLAY_BN4,    /* 02 movs r0,#kind */
		0x2200,                       /* 04 movs r2,#0 */
		0x4B03,                       /* 06 ldr r3,[pc,#0xC]: BN6's overlay spawn */
		0, 0,                         /* 08 bl the bx r3 */
		0x4B02,                       /* 0C ldr r3,[pc,#8]: his taking of the form's copy */
		0, 0,                         /* 0E bl the bx r3 */
		0xBD00,                       /* 12 pop {pc} */
	};
	uint8_t out[SOULS_BN4_BOTH_END - SOULS_BN4_BOTH] = { 0 };
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i) put16(out + 2 * i, code[i]);
	put32(out + 0x14, BN6_FORM_OVERLAY_SPAWN | 1);
	put32(out + 0x18, enter);
	put16(out + 0x1C, 0x4718);   /* bx r3 */
	put16(out + 0x1E, 0x46C0);   /* nop */
	if (!thumb_bl_write(out, 0x08, SOULS_BN4_BOTH, SOULS_BN4_BOTH + 0x1C) || !thumb_bl_write(out, 0x0E, SOULS_BN4_BOTH, SOULS_BN4_BOTH + 0x1C))
		return false;
	emu_write(SOULS_BN4_BOTH, out, sizeof out);
	both_k = k;
	return true;
}

static uint16_t word_of(int k) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	int form = soulbn4_form(k);
	return b4 && form > 0 ? get16(b4 + (BN4_FORM_COMPANIONS - BUS) + 2u * (uint32_t)form) : 0;
}

bool soulcompanion_sprite(int k, int *list, int *number) {
	uint16_t w = word_of(k);
	*list = (w & 0xFF) / LIST_SIZE;
	*number = w >> 8;
	return w != 0;
}

/* BN4's code the companion stands on: BN4_SOUL_COMPANION spawning BN4_FIRE_COMPANION_SPAWN's type and keeping it at his AI
 * +0x48 (0x0800B80E), that spawn's type BN4_FIRE_COMPANION_TYPE (0x080CC5F8) */
static bool expected(const uint8_t *b4) {
	return get16(b4 + (BN4_FIRE_COMPANION_SPAWN + 4 - BUS)) == (OP_MOVS_R0 | BN4_FIRE_COMPANION_TYPE) &&
		get16(b4 + (BN4_SOUL_COMPANION + 0x26 - BUS)) == OP_STR_R0_R7_48;
}

/* The overlay's kind SOUL_OVERLAY_BN4 in the Navis' grown overlay tables: its masks by his animation (0 a step behind him;
 * none 0, as BN4's companion keeps his place always, 0x080CC47A) and its sprite (list 0's SOUL_LIST0_BN4_COMPANION, which
 * the shared look room fills with the united Soul's companion's), staged and written as BN4's Souls' */
bool soulcompanion_install(void) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	if (done) return true;
	if (!b4 || !expected(b4) || !xn_grow()) return true;
	static const uint8_t sprite[2] = { 0, SOUL_LIST0_BN4_COMPANION };
	xv_tx_begin();
	uint8_t *m = xv_put(SOULS4_COMPANION_MASKS, SOULS4_COMPANION_MASKS_END - SOULS4_COMPANION_MASKS, true);
	if (m) memset(m, 1, SOULS4_COMPANION_MASKS_END - SOULS4_COMPANION_MASKS);
	xn_set_word(XN_T_OVL_MASKS, SOUL_OVERLAY_BN4, SOULS4_COMPANION_MASKS);
	xn_set(XN_T_OVL_SPRITES, SOUL_OVERLAY_BN4, sprite);
	if (xv_failed()) {
		fprintf(stderr, "soulcompanion: BN4's Souls' companions not installed: %s\n", xv_why());
		xv_tx_end();
		return true;
	}
	claims_begin("BN4's Souls' companions (soulcompanion.c)");
	xv_commit();
	/* (one Soul with both, FireSoul: the stub is his) */
	for (int k = 0; k < SOULS_BN4; ++k) {
		uint32_t enter = soulbn4_enter_copy(k);
		if (word_of(k) && enter && (both_k >= 0 || !both_write(k, enter))) fprintf(stderr, "soulcompanion: BN4's Soul %d's companion and object: no room for both\n", soulbn4_form(k));
	}
	claims_end();
	xv_tx_end();
	done = true;
	printf("soulcompanion: BN4's Souls' companions (NumSoul's, FireSoul's) as BN6's overlay of kind %d\n", SOUL_OVERLAY_BN4);
	return true;
}

bool soulcompanion_enter(int k, uint32_t *word) {
	*word = SOUL_OVERLAY_BN4;   /* (its +4 the kind, +5 0: his palette its own, as BN4's companion's init takes it, 0x080CC40C) */
	return done && word_of(k);
}

uint32_t soulcompanion_owner(uint32_t obj) {
	return hook_read8(obj + BN6_T1_TYPE) == BN6_OVERLAY_TYPE && hook_read8(obj + BN6_T1_PARAM) == SOUL_OVERLAY_BN4 ?
		hook_read32(obj + BN6_T1_PARENT) : obj;
}

uint32_t soulcompanion_both(int k) { return done && k >= 0 && k == both_k ? SOULS_BN4_BOTH : 0; }

uint32_t soulcompanion_leave(int k) { return done && word_of(k) ? BN6_FORM_OVERLAY_TAKE : 0; }
