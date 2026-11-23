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
#include "xnavi_room.h"
#include "xvirus_tx.h"

#define BUS 0x08000000u
#define LIST_SIZE 4           /* a sprite list's byte offset per list (BN4_FORM_COMPANIONS' low byte, sprite_load's r1) */
#define OP_MOVS_R0 0x2000u    /* movs r0,#imm8 */
#define OP_STR_R0_R7_48 0x64B8u   /* str r0,[r7,#0x48]: BN4_SOUL_COMPANION's keeping at his AI +0x48 */

static bool done;

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

uint32_t soulcompanion_leave(int k) { return done && word_of(k) ? BN6_FORM_OVERLAY_TAKE : 0; }
