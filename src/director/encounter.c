/* The game decides when a random battle happens (its own step counter); the
 * roll that picks which battle returns a BattleSettings record the engine
 * writes into the free ROM space. Layout per the bn6f disassembly's
 * battle_settings_struct: field, unk, music, type, background, number,
 * side modifier, unk, options (u32), then a pointer to the entity list of
 * 4-byte entries (kind, panel y<<4|x, id) ending with 0xF0. */
#include "encounter.h"

#include <stdbool.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "darkbn6.h"
#include "data.h"
#include "emu.h"
#include "events.h"
#include "loot.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"

int encounter_song[2] = { ENCOUNTER_SONG_BATTLE, ENCOUNTER_SONG_BOSS };
int encounter_backdrop = -1;
/* Two records in turn, each BattleSettings and 0x20 on its entity list: the
 * next battle is written to the one not handed out, and the hooks turned to
 * it. A battle the game has rolled but not yet set up reads the record it
 * was handed (rewritten in place, a re-roll in that gap gave the battle
 * another's MegaMan panel and field). */
#define RECORDS     (EMU_FREE + 0x200)
#define RECORD_SIZE 0x80
static int slot;
static uint32_t settings_of(int s) { return RECORDS + (uint32_t)s * RECORD_SIZE; }

/* What the hooks hand out, set between frames (the hooks only read it):
 * the record, and whether a battle starts now. */
static uint32_t record;
static bool forcing;
bool encounter_guest;
/* Whether each record is a guardian's battle, and the battle on is one
 * (emu_encounter_started): his Guardian Data gives his chip, and his
 * battle's own reward gave a second, which no folder can hold (one Mega
 * chip of a kind), so zenny comes in its place. */
static bool boss_record[2], boss_on;

/* The overworld's check (bn6f checkThenStartBattle) calls the roll once a
 * frame, then branches on the flags it leaves: on that branch, a battle the
 * roll gave (its own step counter and chance) becomes the engine's record. */
static HookAct rolled(HookRegs *r, void *user) {
	(void)user;
	if (!r->r[0]) return HOOK_CONTINUE;
	/* (a layer whose battles are the guest's: none begun here, the
	 * director starts the guest's, guest.c) */
	if (encounter_guest) {
		hook_post(r, EV_GUEST_BATTLE);
		r->r[12] = BN6_ENCOUNTER_SKIP;
		return HOOK_JUMP;
	}
	r->r[0] = record;
	return HOOK_CONTINUE;
}

/* A battle now: from the check's first test (MegaMan on the map) straight
 * to StartBattle with the record, past the flags, fades and chat that
 * would hold a random one, and the roll. */
static HookAct forced(HookRegs *r, void *user) {
	(void)user;
	if (!forcing) return HOOK_CONTINUE;
	r->r[0] = record;
	r->r[12] = BN6_ENCOUNTER_START;
	return HOOK_JUMP;
}

static HookAct picking(HookRegs *r, void *user);

void emu_encounters_install(void) {
	static bool done;
	if (done) return;
	done = true;
	emu_hook(BN6_ENCOUNTER_ROLLED, rolled, NULL);
	emu_hook(BN6_ENCOUNTER_CHECK, forced, NULL);
	emu_hook_event(BN6_START_BATTLE, EV_BATTLE_START);
	emu_hook(BN6_REWARD_PICK, picking, NULL);
	darkbn6_install();   /* (BN6's own DarkChips' uses: docs/META.md) */
}

void emu_battle_force(const Encounter *e) {
	emu_encounter_set(e);
	forcing = true;
}

void emu_battle_release(void) { forcing = false; }

bool emu_battle_forcing(void) { return forcing; }

void emu_encounter_set(const Encounter *e) {
	uint8_t list[4 * (MAX_FOES + 1 + MAX_FIELD_OBJS) + 1], *p = list;
	int next = slot ^ 1;
	/* MegaMan where the battle's field has him: column 2 row 2, or beside
	 * it where that panel is a hole (he started in one) */
	*p++ = 0x00; *p++ = (uint8_t)(e->player ? e->player : 0x22); *p++ = 0; *p++ = 0;
	for (int i = 0; i < e->nfoes && i < MAX_FOES; ++i) {
		const Foe *f = &e->foes[i];
		int id = f->id;
		if (id < 0) id = enemy_id(f->kind == FOE_NAVI ? 1 : 0, f->family, f->version);
		if (id < 0) id = enemy_id(f->kind == FOE_NAVI ? 1 : 0, f->family, 0);
		if (id < 0) continue;
		*p++ = 0x11;
		*p++ = (uint8_t)((f->row + 1) << 4 | (f->col + 1));
		*p++ = (uint8_t)id;
		*p++ = (uint8_t)(id >> 8);
	}
	/* the field's objects as the area's battle has them (rocks, cubes,
	 * statues); a Mystery Data takes this record's reward row (its byte
	 * the row, the tier, over 0x0F: the spawn keeps it where that byte
	 * beats RNG % 15, so always), its rewards rolled into the row in the
	 * core's copy: one of them is the battle's second reward where it is
	 * still there at the end (bn6f sub_80DA9FE, sub_80AA8E0) */
	bool gem = false;
	for (int i = 0; i < e->nobj && i < MAX_FIELD_OBJS; ++i) {
		int arg = e->obj[i].arg;
		if (e->obj[i].kind >> 4 == FIELD_GEM) {
			if (gem) continue;
			arg = next << 4 | 0x0F;
			gem = true;
		}
		*p++ = (uint8_t)e->obj[i].kind;
		*p++ = (uint8_t)e->obj[i].panel;
		*p++ = (uint8_t)arg;
		*p++ = (uint8_t)(arg >> 8);
	}
	if (gem && R.layout->battle_gem_rewards) {
		uint16_t r[4];
		loot_gem_rewards(run.depth, r);
		uint8_t row[16];
		for (int k = 0; k < 8; ++k) { row[2 * k] = (uint8_t)r[k / 2]; row[2 * k + 1] = (uint8_t)(r[k / 2] >> 8); }
		emu_write(0x08000000u + R.layout->battle_gem_rewards + (uint32_t)next * 16, row, sizeof row);
	}
	*p++ = 0xF0;
	boss_record[next] = e->boss;
	slot = next;
	uint32_t settings = settings_of(slot);
	emu_write(settings + 0x20, list, (size_t)(p - list));
	/* the values of a Central Area random battle, with the formation's
	 * battlefield, the area's background and the virus or boss theme */
	int bg = biome_bg(e->biome);   /* (rolled either way) */
	if (encounter_backdrop >= 0) bg = encounter_backdrop;
	uint8_t s[16] = { (uint8_t)e->field, 0x36, (uint8_t)encounter_song[e->boss], 0x00, (uint8_t)bg, 0x00, 0x38, 0x00 };
	/* (bit 0x20 of the options lets MegaMan run, bn6f 0x08026EC8: the
	 * story's bosses clear it, the random battles and BN6's roaming SP
	 * Navis set it; docs/ROM_DATA.md) */
	put32(s + 8, e->held ? 0x000049C2 : 0x000049E2);
	put32(s + 12, settings + 0x20);
	emu_write(settings, s, sizeof s);
	record = settings;
}

int emu_encounter_slot(void) { return slot; }

int emu_encounter_record(uint32_t settings) {
	return settings == settings_of(0) ? 0 : settings == settings_of(1) ? 1 : -1;
}

void emu_encounter_started(uint32_t settings) {
	int s = emu_encounter_record(settings);
	boss_on = s >= 0 && boss_record[s];
}

/* A guardian's chip entry's zenny: an Unlocker's price in his act (shop.c,
 * add_keys), so busting him well pays the act's lock. */
int encounter_boss_zenny(void) {
	int act = pacing_act(run.depth) + 7 * pacing_loop(run.depth), z = 100 * (6 + 3 * act);
	return z > 0x3FFF ? 0x3FFF : z;
}

static uint16_t boss_zenny(void) { return (uint16_t)(1 << 14 | encounter_boss_zenny()); }

/* One enemy's reward row in the folder's codes, half the time (docs/
 * META.md). BN6 picks the reward as the battle ends: one of the enemies
 * spawned, then one of its row's 20 entries by the busting level, a coin
 * and MegaMan's HP; a chip entry has bits 14-15 clear, the id in 0-8 and
 * the code in 9-13. The coin picks between the two entries of a pair: the
 * second of each, rewritten from the ROM's own in the core's copy, comes in
 * one of the folder's codes where the chip does, so a chip reward leans
 * half the time, as Mystery Data's; a Navi's chip, in either entry, in its
 * * off the folder's codes, and in his own guardian's battle zenny
 * (boss_on); with the All * helper every chip in *. (From a hook: the
 * player's ROM and the run's codes are only read, and loot_fit_code rolls
 * nothing when always.) */
static void lean_row(int id) {
	if (id <= 0 || id >= 0x200) return;
	uint32_t row = BN6_DROP_ROWS - 0x08000000u + (uint32_t)id * 0x28;
	for (uint32_t k = 0; k < 20; ++k) {
		uint16_t v = rom_u16(row + 2 * k);
		if (v == 0xFFFF || v >> 14) continue;
		int chip = v & 0x1FF, code = v >> 9 & 0x1F;
		if (boss_on && chip_family(chip) == CHIP_FAMILY_NAVI) {
			hook_write16(0x08000000u + row + 2 * k, boss_zenny());
			continue;
		}
		char c = code >= 26 || run_all_star() ? '*' : (char)('A' + code);   /* (every chip in * with All *) */
		if (k & 1) c = loot_fit_code(chip, c, true);
		/* (a Navi's chip in its * where the folder holds not its letter,
		 * as his Guardian Data gives it, from both entries of a pair: a
		 * Blade folder's guardian dropped ChrgeMan C, and with the second
		 * entry alone, SpoutMan A) */
		bool held = false;
		for (int h = 0; h < 3 && run.codes[h]; ++h) held |= c == 'A' + run.codes[h] - 1;
		if (!held && c != '*' && chip_family(chip) == CHIP_FAMILY_NAVI) {
			ChipInfo ci;
			chip_info(chip, &ci);
			if (memchr(ci.codes, '*', (size_t)ci.ncodes)) c = '*';
		}
		hook_write16(0x08000000u + row + 2 * k, (uint16_t)((v & ~(0x1F << 9)) | (c == '*' ? 26 : c - 'A') << 9));
	}
}

/* The reward pick, as a battle ends (bn6f sub_80AA910, the rows' one
 * reader): the rows of the enemies it picks from, leaned just before. */
static HookAct picking(HookRegs *r, void *user) {
	(void)user;
	for (uint32_t i = 0; i < r->r[1] && i < 8; ++i) lean_row(hook_read16(r->r[0] + 2 * i));
	return HOOK_CONTINUE;
}

/* A watched battle (the duel's): MegaMan's hits, and the HP the first
 * enemy spawns with at most, which the spawn's hook clears once used; and
 * a guardian's HP raised, from his own to the clock's */
static int cap, hp_from, hp_to;

/* MegaMan's object: the first of the battle's objects in play on his side */
static uint32_t megaman_object(void) {
	for (uint32_t k = 0; k < BN6_T1_COUNT; ++k) {
		uint32_t o = BN6_T1_OBJECTS + k * BN6_T1_SIZE;
		if ((hook_read8(o + BN6_T1_IN_PLAY) & 1) && hook_read8(o + BN6_T1_ALLIANCE) == 0) return o;
	}
	return 0;
}

/* object_subtractHP, run on every object every frame, mostly with nothing
 * to take: an event where it lowers MegaMan's HP */
static HookAct hurt(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] && hook_read16(r->r[5] + BN6_T1_HP) && r->r[5] == megaman_object()) hook_post(r, EV_MEGAMAN_HIT);
	return HOOK_CONTINUE;
}

/* An enemy's spawn, as it is given its HP and MaxHP (r2): the first
 * enemy's at most the cap; the guardian's, his own HP, raised */
static HookAct spawned(HookRegs *r, void *user) {
	(void)user;
	if (hook_read8(r->r[5] + BN6_T1_ALLIANCE) != 1) return HOOK_CONTINUE;
	if (cap) {
		if (r->r[2] > (uint32_t)cap) r->r[2] = (uint32_t)cap;
		cap = 0;
	}
	if (hp_from && r->r[2] == (uint32_t)hp_from) {
		r->r[2] = (uint32_t)hp_to;
		hp_from = 0;
	}
	return HOOK_CONTINUE;
}

void emu_battle_watch(int hp_cap) {
	emu_hook(BN6_SUBTRACT_HP, hurt, NULL);
	if (hp_cap > 0) emu_hook(BN6_SPAWN_HP, spawned, NULL);
	cap = hp_cap > 0 ? hp_cap : 0;
}

void emu_battle_unwatch(void) {
	emu_unhook(BN6_SUBTRACT_HP);
	emu_unhook(BN6_SPAWN_HP);
	cap = hp_from = 0;
}

void emu_battle_clock(int own) {
	int more = pacing_clock_hp(own, run.clock);
	if (own <= 0 || more == own) return;
	emu_hook(BN6_SPAWN_HP, spawned, NULL);
	hp_from = own;
	hp_to = more;
}
