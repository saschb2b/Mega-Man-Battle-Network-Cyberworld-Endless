/* RollSoul's heal (soulroll.h). */
#include "soulroll.h"

#include <stdint.h>

#include "bn6.h"
#include "bn6_chips.h"
#include "bytes.h"
#include "chips.h"
#include "claims.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"
#include "soul_room.h"
#include "soulbn4.h"
#include "soulkinds.h"
#include "soullife.h"
#include "soulpassive.h"
#include "thumb_reloc.h"
#include "xchips_code.h"
#include "xsong.h"

#define BUS 0x08000000u
#define OP_MOVS_R3_A0 0x23A0u    /* BN6_CHIP_USE_HEAL's movs r3,#0xa0 */
#define OP_ADDS_R3_R6 0x199Bu    /* ... adds r3,r3,r6 */

static SoulBn4Heal heals[SOULS_BN4];   /* each BN4 Soul's heal, by k */
static bool heal_of[SOULS_BN4];

/* (BN6_CHIP_USE_HEAL, r5 him, r4 the chip's record) where he stands in a BN4 Soul that heals and the chip is of another kind,
 * his max HP over its share healed, then on to the stub: the effect, the sound, the two instructions the hook stands on */
static HookAct heal(HookRegs *r, void *user) {
	uint32_t me = r->r[5];
	int k = soulbn4_k(soul_id_of(me));
	if (k < 0 || !heal_of[k]) return HOOK_CONTINUE;
	uint32_t rec = r->r[4], table = chips_table_bus();
	if (rec < table || (rec - table) % BN6_CHIP_RECORD_SIZE) return HOOK_CONTINUE;
	int chip = (int)((rec - table) / BN6_CHIP_RECORD_SIZE);
	if (chip <= 0 || chip >= CHIPS_IDS || soulkinds_bn4_of(chip) == heals[k].kind) return HOOK_CONTINUE;
	uint16_t max = hook_read16(me + BN6_T1_MAX_HP), hp = hook_read16(me + BN6_T1_HP);
	uint16_t more = (uint16_t)(max / heals[k].share);
	if (!more) return HOOK_CONTINUE;
	hook_write16(me + BN6_T1_HP, (uint16_t)(hp + more > max ? max : hp + more));   /* (BN6_OBJECT_HEAL's work, BN4's 0x0800ABBA) */
	r->r[12] = (uintptr_t)user | 1;
	return HOOK_JUMP;
}

bool soulroll_install(int breaks) {
	static bool done;
	if (done) return true;
	int n = 0, effect = -1, sound = -1, song = 0;
	for (int k = 0; k < SOULS_BN4; ++k) {
		heal_of[k] = soulbn4_heal(k, &heals[k]);
		if (!heal_of[k]) continue;
		++n;
		/* (the effect as Repair's unit moved it, the sound in its slot of BN4's sounds: one of each, the Souls' one heal) */
		effect = xchips_bn4_effect(heals[k].effect);
		song = heals[k].sound;
		sound = xsong_sound_slot(XROM_BN4_RED_SUN_US, song);
		if (effect < 0 || sound < 0) return false;
	}
	if (!n || (breaks & SOULPASSIVE_BREAK_READY)) return done = true;
	claims_begin("RollSoul's heal (soulroll.c)");
	bool copied = xsong_install_at(XROM_BN4_RED_SUN_US, song, sound) == sound;
	claims_end();
	if (!copied) return false;
	if (get16(R.data + (BN6_CHIP_USE_HEAL - BUS)) != OP_MOVS_R3_A0 || get16(R.data + (BN6_CHIP_USE_HEAL + 2 - BUS)) != OP_ADDS_R3_R6)
		return false;
	/* (r1-r3 his place, as BN4's ldmia from his +0x34; each call through a bx, past a bl's reach) */
	static const uint16_t code[] = {
		0xB410,   /* 00 push {r4} */
		0x2034,   /* 02 movs r0,#0x34 */
		0x1940,   /* 04 adds r0,r0,r5 */
		0xC80E,   /* 06 ldmia r0!,{r1,r2,r3}: his place */
		0x2400,   /* 08 movs r4,#effect */
		0x4807,   /* 0A ldr r0,[pc,#0x1c]: BN6_SPAWN_EFFECT */
		0, 0,     /* 0C bl 0x22 */
		0x4B06,   /* 10 ldr r3,[pc,#0x18]: BN6_PLAY_SOUND */
		0x4807,   /* 12 ldr r0,[pc,#0x1c]: the sound */
		0, 0,     /* 14 bl 0x24 */
		0xBC10,   /* 18 pop {r4}: the chip's record again */
		OP_MOVS_R3_A0,   /* 1A the two the hook stands on */
		OP_ADDS_R3_R6,   /* 1C */
		0x4805,   /* 1E ldr r0,[pc,#0x14]: back (BN6 loads r0 anew there) */
		0x4700,   /* 20 bx r0 */
		0x4700,   /* 22 bx r0 */
		0x4718,   /* 24 bx r3 */
		0x46C0,   /* 26 nop */
		0, 0,     /* 28 BN6_SPAWN_EFFECT */
		0, 0,     /* 2C BN6_PLAY_SOUND */
		0, 0,     /* 30 the sound */
		0, 0,     /* 34 BN6_CHIP_USE_HEAL_PAST */
	};
	_Static_assert(sizeof code <= SOULS4_HEAL_END - SOULS4_HEAL, "the heal's room");
	uint8_t out[sizeof code];
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i) put16(out + 2 * i, code[i]);
	put16(out + 0x08, (uint16_t)(0x2400 | (effect & 0xFF)));
	put32(out + 0x28, BN6_SPAWN_EFFECT | 1);
	put32(out + 0x2C, BN6_PLAY_SOUND | 1);
	put32(out + 0x30, (uint32_t)sound);
	put32(out + 0x34, BN6_CHIP_USE_HEAL_PAST | 1);
	if (!thumb_bl_write(out, 0x0C, SOULS4_HEAL, SOULS4_HEAL + 0x22) || !thumb_bl_write(out, 0x14, SOULS4_HEAL, SOULS4_HEAL + 0x24))
		return false;
	claims_begin("RollSoul's heal (soulroll.c)");
	emu_write(SOULS4_HEAL, out, sizeof out);
	bool ok = emu_hook(BN6_CHIP_USE_HEAL, heal, (void *)(uintptr_t)SOULS4_HEAL);
	claims_end();
	return done = ok;
}
