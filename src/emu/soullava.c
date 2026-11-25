/* FireSoul on lava (soullava.h). */
#include "soullava.h"

#include <stdint.h>

#include "bn4.h"
#include "bn4_souls.h"
#include "bn6.h"
#include "bn6_forms.h"
#include "bn6_panels.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "soul_room.h"
#include "soulbn4.h"
#include "soullife.h"
#include "xpanels.h"

#define BUS 0x08000000u
#define FORM_FIRE 5
#define HEAL 0x32                /* BN4_LAVA_FIRE_HEAL's movs r0,#0x32 */
#define EFFECT_HEAL 0xCE         /* BN4's effect 7 as the BN4 chips' units moved it (Repair's: src/emu/xchips_bn4.c) */
#define SOUND_HEAL 0x39          /* BN4's sound 0x9A as Walla's family copied it (src/audio/xsong.c XSOUNDS) */
#define LITS 0x44                /* the Thumb's literals: BN6's heal, panel set, effect spawn and sound */
#define OP_CMP_R0 0x2800u
#define OP_MOVS_R0 0x2000u
#define OP_MOVS_R4 0x2400u

/* BN4_LAVA_STEP's FireSoul as BN6's twins run it, his collision data's Fire tally BN6's (r5 him, r3 the hooked instruction with
 * its Thumb bit, back there with r3, r4, r7 and lr as they were; assembled and read back with tools/xrecipe_thumb.py) */
static const uint16_t code[] = {
	0xB598,                  /* 00 push {r3,r4,r7,lr} */
	0x6D6F,                  /* 02 ldr r7,[r5,#0x54]: his collision data */
	0x2000 | HEAL,           /* 04 movs r0,#50 */
	0x4B0F, 0xF000, 0xF819,  /* 06 ldr r3,=BN6_OBJECT_HEAL; bl the bx r3 */
	0x2196,                  /* 0C movs r1,#0x96 (BN6_COLL_FIRE_DAMAGE) */
	0x5A78,                  /* 0E ldrh r0,[r7,r1] */
	0x3000 | HEAL,           /* 10 adds r0,#50 */
	0x5278,                  /* 12 strh r0,[r7,r1] */
	0x7AB8,                  /* 14 ldrb r0,[r7,#0xa]: his column (BN6_COLL_PANEL_X) */
	0x7AF9,                  /* 16 ldrb r1,[r7,#0xb]: his row */
	0x2202,                  /* 18 movs r2,#2: plain */
	0x4B0B, 0xF000, 0xF80F,  /* 1A ldr r3,=BN6_SET_PANEL_TYPE; bl the bx r3 */
	0x2034,                  /* 20 movs r0,#0x34 */
	0x1940,                  /* 22 adds r0,r0,r5 */
	0xC80E,                  /* 24 ldmia r0!,{r1,r2,r3}: his place */
	0x2400 | EFFECT_HEAL,    /* 26 movs r4,#effect */
	0x4F08, 0xF000, 0xF809,  /* 28 ldr r7,=BN6_SPAWN_EFFECT; bl the bx r7 */
	0x2000 | SOUND_HEAL,     /* 2E movs r0,#sound */
	0x4B07, 0xF000, 0xF804,  /* 30 ldr r3,=BN6_PLAY_SOUND; bl the bx r3 */
	0xBC98,                  /* 36 pop {r3,r4,r7} */
	0xBC04,                  /* 38 pop {r2} */
	0x4696,                  /* 3A mov lr,r2 */
	0x4718,                  /* 3C bx r3: back */
	0x4718,                  /* 3E bx r3 */
	0x4738,                  /* 40 bx r7 */
	0x46C0,                  /* 42 nop */
};
_Static_assert(sizeof code == LITS && LITS + 16 <= SOULS_BN4_LAVA_END - SOULS_BN4_LAVA, "FireSoul's lava past its room (soul_room.h)");
_Static_assert(BN6_COLL_FIRE_DAMAGE == 0x96 && BN6_COLL_PANEL_X == 0x0A && BN6_COLL_PANEL_Y == 0x0B && BN6_T1_COLLISION == 0x54,
	"the lava's Thumb's offsets are bn6.h's");

static int fire_k = -1;   /* FireSoul's k, -1 where he is not moved */

static bool his(uint32_t obj) { return fire_k >= 0 && soulbn4_k(soul_id_of(obj)) == fire_k; }

/* BN4's code the Thumb copies: the form tested, the heal, the effect and the sound as read */
static bool expected(const uint8_t *b4) {
	return get16(b4 + (BN4_LAVA_FIRE_FORM - BUS)) == (OP_CMP_R0 | FORM_FIRE) && get16(b4 + (BN4_LAVA_FIRE_HEAL - BUS)) == (OP_MOVS_R0 | HEAL) &&
		get16(b4 + (0x080131D6u - BUS)) == (OP_MOVS_R4 | 7) && get16(b4 + (0x080131DCu - BUS)) == (OP_MOVS_R0 | 0x9A);
}

bool soullava_install(void) {
	static bool done;
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	if (done) return true;
	for (int k = 0; k < SOULS_BN4; ++k)
		if (soulbn4_form(k) == FORM_FIRE) fire_k = k;
	if (fire_k < 0 || !b4 || !expected(b4) || !xpanels_ready()) return true;
	uint8_t out[LITS + 16];
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i) put16(out + 2 * i, code[i]);
	put32(out + LITS, BN6_OBJECT_HEAL | 1);
	put32(out + LITS + 4, BN6_SET_PANEL_TYPE | 1);
	put32(out + LITS + 8, BN6_SPAWN_EFFECT | 1);
	put32(out + LITS + 12, BN6_PLAY_SOUND | 1);
	claims_begin("FireSoul on lava (soullava.c)");
	emu_write(SOULS_BN4_LAVA, out, sizeof out);
	claims_end();
	xpanels_lava_own(his, SOULS_BN4_LAVA);
	return done = true;
}
