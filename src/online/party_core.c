/* party_core.h */
#include "party_core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/core.h>

#include "arena_loadout.h"
#include "arena_pair.h"
#include "arena_team_custom.h"
#include "arena_team_navi.h"
#include "arena_team_navicust.h"
#include "bn6.h"
#include "bytes.h"
#include "core_boot.h"
#include "emu.h"
#include "free_space.h"
#include "hook.h"
#include "soulunite.h"
#include "team_shape.h"
#include "xinstall_log.h"

#define READY_MOST 1200   /* frames from the boot's net to the battle's first Custom screen, at most */
#define RECORD_LIST 0x20  /* the record's list, after its BattleSettings (encounter.c) */

struct PartyCore {
	struct mCore *core;
	uint32_t video[PARTY_W * PARTY_H];
	PartyField field;
	bool forcing;            /* the encounter check not yet forced into the battle */
	uint32_t frames;         /* since the ready */
	uint8_t *scratch;        /* the core's raw state, for the hash */
	size_t scratch_size;
	unsigned hp[PARTY_MOST]; /* each seat's HP as last seen */
};

struct PartyCoreSnap {
	ArenaCoreSnap *core;
	PartyField field;
	uint32_t frames;
	unsigned hp[PARTY_MOST];
};

/* ---- the boot, kept ---- */

static struct {
	struct mCore *core;
	CoreBoot boot;
	uint32_t video[PARTY_W * PARTY_H];
	uint8_t *state;          /* the boot's state on the net, kept */
	size_t size;
	bool failed;
} K;

int party_core_boot(const uint8_t *rom, size_t size, unsigned frames) {
	if (K.state) return 0;
	if (K.failed) return -1;
	if (!K.core) {
		/* (full rules' installs as the Arena's cores take them: xinstall_log.h) */
		if (!xlog_hash() || !(K.core = arena_core_new(rom, size, K.video, true)) || !core_boot_begin(&K.boot, K.core)) {
			fprintf(stderr, "party: the boot core could not be made (BN5's installs %s)\n", xlog_hash() ? "in" : "not in this game's core");
			if (K.core) arena_core_free(K.core);
			K.core = NULL;
			K.failed = true;
			return -1;
		}
	}
	for (unsigned i = 0; i < frames; ++i) {
		int more = core_boot_frame(&K.boot);
		if (more > 0) continue;
		core_boot_end(&K.boot);
		if (more == 0) K.state = arena_core_save(K.core, &K.size);
		hook_detach(K.core);
		arena_core_free(K.core);
		K.core = NULL;
		K.failed = !K.state;
		if (K.failed) fprintf(stderr, "party: the boot did not reach the net\n");
		else fprintf(stderr, "party: the boot kept on the net (%zu bytes)\n", K.size);
		return K.failed ? -1 : 0;
	}
	return 1;
}

/* ---- the battle's core ---- */

static uint32_t mem_read32(void *context, uint32_t address) {
	struct mCore *c = context;
	return c->rawRead32(c, address, -1);
}

static void mem_write8(void *context, uint32_t address, uint8_t value) {
	struct mCore *c = context;
	c->rawWrite8(c, address, -1, value);
}

/* the overworld's check (encounter.c's forced): straight to StartBattle
 * with the record, once */
static HookAct forced(HookRegs *r, void *user) {
	PartyCore *pc = user;
	if (!pc->forcing) return HOOK_CONTINUE;
	pc->forcing = false;
	r->r[0] = FREE_RECORDS;
	r->r[12] = BN6_ENCOUNTER_START;
	return HOOK_JUMP;
}

/* the run's ROM with the battle's record at FREE_RECORDS into the core */
static bool rom_in(PartyCore *pc, const uint8_t *run_rom, const PartyBattle *b) {
	uint8_t *rom = malloc(EMU_ROM_SIZE);
	if (!rom) return false;
	memcpy(rom, run_rom, EMU_ROM_SIZE);
	uint32_t at = FREE_RECORDS - 0x08000000u;
	memcpy(rom + at, b->record, PARTY_RECORD_BYTES);
	put32(rom + at + 12, FREE_RECORDS + RECORD_LIST);
	bool ok = arena_core_rom(pc->core, rom, EMU_ROM_SIZE);
	free(rom);
	return ok;
}

static void field_set(PartyField *f, const PartyBattle *b) {
	memset(f, 0, sizeof *f);
	f->n = (int)b->n;
	f->quiet = b->quiet;
	f->scripted = b->scripted;
	f->spread = b->spread;
	f->own = b->own && b->n > 1;
	f->foe_from = b->foe_from;
	f->foe_to = b->foe_to;
	f->end = b->end;
	for (unsigned k = 0; k < b->n && k < PARTY_MOST; ++k) {
		f->hp[k] = b->seat[k].hp ? b->seat[k].hp : b->seat[k].max_hp;
		f->max_hp[k] = b->seat[k].max_hp;
	}
}

/* each partner's own MegaMan for his record, his deck and his Custom screen
 * (arena_team_navi.h, arena_team_custom.h), the same on every game: his
 * NaviCust compiled by BN6 over the kept boot (his loadout's numbers where
 * it cannot be), his folder */
static void partners_given(const PartyBattle *b) {
	for (unsigned p = 0; p < TEAM_PARTNERS; ++p) {
		const OnlineLoadout *l = p + 1 < b->n ? &b->seat[p + 1] : NULL;
		arena_team_navi_compiled(p, NULL);
		arena_team_navi_loadout(p, 0, 0, NULL);
		arena_team_custom_folder(p, l ? l->folder : NULL, l ? l->regular : 0, l ? l->tag : NULL);
		if (!l) continue;
		uint8_t stats[BN6_BATTLE_NAVI_SIZE];
		char why[96];
		if (team_navicust_compile(K.state, K.size, l, stats, why, sizeof why)) {
			arena_team_navi_compiled(p, stats);
			fprintf(stderr, "party: partner %u's NaviCust compiled by BN6: HP %u, Custom %u, buster %u %u %u\n", p + 1,
				(unsigned)get16(stats + BN6_NS_MAX_HP), stats[BN6_NS_CUSTOM], stats[BN6_NS_ATTACK] + 1u, stats[BN6_NS_SPEED] + 1u,
				stats[BN6_NS_CHARGE] + 1u);
			continue;
		}
		const uint8_t buster[3] = { l->attack, l->speed, l->charge };
		arena_team_navi_loadout(p, l->max_hp, l->base_hp, buster);
		fprintf(stderr, "party: partner %u's NaviCust not compiled (%s): his loadout's HP and buster\n", p + 1, why);
	}
}

static bool fail(PartyCore *pc, char *why, size_t wn, const char *what) {
	snprintf(why, wn, "%s", what);
	party_core_close(pc);
	return false;
}

/* frames with no keys until the battle's first Custom screen */
static bool to_custom(PartyCore *pc) {
	struct mCore *c = pc->core;
	for (unsigned i = 0; i < READY_MOST; ++i) {
		c->setKeys(c, 0);
		c->runFrame(c);
		emu_audio_drop_from(c);
		uint32_t mode = c->rawRead8(c, BN6_GAMESTATE, -1);
		if (!pc->forcing && mode == BN6_SUB_BATTLE && c->rawRead8(c, BN6_BATTLE_PHASE, -1) == BN6_PHASE_CUSTOM && pc->field.standing) {
			fprintf(stderr, "party: the battle's first Custom screen after %u frames, %d MegaMen\n", i + 1, pc->field.standing);
			return true;
		}
	}
	return false;
}

PartyCore *party_core_open(const uint8_t *rom, size_t size, const uint8_t *run_rom, const PartyBattle *b, char *why, size_t wn) {
	PartyCore *pc = calloc(1, sizeof *pc);
	if (!pc) { snprintf(why, wn, "no memory for the party's core"); return NULL; }
	if (!K.state) { fail(pc, why, wn, "the boot is not kept"); return NULL; }
	if (b->n < 1 || b->n > PARTY_MOST) { fail(pc, why, wn, "a party of no size"); return NULL; }
	field_set(&pc->field, b);
	pc->core = arena_core_new(rom, size, pc->video, true);
	if (!pc->core || !rom_in(pc, run_rom, b) || !arena_core_load(pc->core, K.state, K.size)) {
		fail(pc, why, wn, "the party's core could not be made (too little memory?)");
		return NULL;
	}
	ArenaMemory m = { mem_read32, mem_write8, pc->core };
	if (!arena_loadout_install(&m, &b->seat[0], why, wn)) { party_core_close(pc); return NULL; }
	HookCore *hc = hook_core(pc->core);
	pc->forcing = true;
	/* (the partners side 0's slots 1-3 on this core, their own MegaMen given: team_shape.h) */
	if (hc) team_shape_party(hook_slot_of(pc->core), pc->field.own, &pc->field.swapped);
	if (pc->field.own) partners_given(b);
	if (!hc || !xlog_hook_core(pc->core) || !soulunite_held_core(pc->core, b->seat[0].souls) || !party_field_hooks(hc, &pc->field) || !party_field_aim(hc, &pc->field) ||
		!hook_on(hc, BN6_ENCOUNTER_CHECK, forced, pc)) {
		fail(pc, why, wn, "the party's core's hooks could not be set");
		return NULL;
	}
	pc->scratch_size = pc->core->stateSize(pc->core);
	if (!(pc->scratch = malloc(pc->scratch_size))) { fail(pc, why, wn, "no memory for the party's core"); return NULL; }
	if (!to_custom(pc)) { fail(pc, why, wn, "the party's battle did not begin"); return NULL; }
	pc->frames = 0;
	return pc;
}

void party_core_close(PartyCore *pc) {
	if (!pc) return;
	if (pc->core) {
		emu_audio_drop_from(pc->core);
		soulunite_held_core(pc->core, 0);
		team_shape_party(hook_slot_of(pc->core), false, NULL);
		hook_detach(pc->core);
		arena_core_free(pc->core);
	}
	free(pc->scratch);
	free(pc);
}

static void seen_hp(PartyCore *pc) {
	for (unsigned k = 0; k < PARTY_MOST; ++k) {
		if ((int)k >= pc->field.standing) { pc->hp[k] = 0; continue; }
		uint32_t o = pc->core->rawRead32(pc->core, BN6_BATTLE_PLAYERS + k * BN6_SIDE_PARTNER, -1);
		pc->hp[k] = o ? (unsigned)(uint16_t)pc->core->rawRead16(pc->core, o + BN6_T1_HP, -1) : 0;
	}
}

/* (as a hook of the field's core runs: the partners' words of the frame
 * into their turns, the keys their battle takes into the field's) */
static void partners_frame(void *user) {
	PartyCore *pc = user;
	uint16_t words[TEAM_PARTNERS], out[TEAM_PARTNERS];
	for (unsigned p = 0; p < TEAM_PARTNERS; ++p) words[p] = pc->field.keys[p + 1];
	arena_team_custom_party(words, (unsigned)pc->field.n - 1u, out);
	for (unsigned p = 0; p < TEAM_PARTNERS; ++p) pc->field.keys[p + 1] = out[p];
}

void party_core_frame(PartyCore *pc, const uint16_t keys[PARTY_MOST]) {
	struct mCore *c = pc->core;
	memcpy(pc->field.keys, keys, sizeof pc->field.keys);
	if (pc->field.own && pc->field.standing) hook_run_on(c, partners_frame, pc);
	c->setKeys(c, keys[0]);
	c->runFrame(c);
	++pc->frames;
	/* (each MegaMan's HP while the battle is on: its results keep what the last frame of it left) */
	if (c->rawRead8(c, BN6_GAMESTATE, -1) == BN6_SUB_BATTLE && c->rawRead8(c, BN6_BATTLE_PHASE, -1) != BN6_PHASE_RESULTS) seen_hp(pc);
}

uint32_t party_core_frames(const PartyCore *pc) { return pc->frames; }

static void hash_more(uint64_t *hash, const void *data, size_t size) {
	const uint8_t *bytes = data;
	for (size_t i = 0; i < size; ++i) {
		*hash ^= bytes[i];
		*hash *= UINT64_C(1099511628211);
	}
}

uint64_t party_core_hash(PartyCore *pc) {
	uint64_t hash = arena_core_hash(pc->core, pc->scratch, pc->scratch_size);
	const PartyField *f = &pc->field;
	const uint32_t kept[] = { (uint32_t)f->standing, f->tick, pc->frames };
	hash_more(&hash, kept, sizeof kept);
	/* (the partners' folders, which no core holds: their decks are dealt from them) */
	if (f->own) hash = team_custom_hash(hash, (unsigned)f->n - 1u);
	return hash ? hash : 1;
}

PartyCoreSnap *party_core_snap_new(PartyCore *pc) {
	PartyCoreSnap *s = calloc(1, sizeof *s);
	if (s && !(s->core = arena_core_snap_new(pc->core))) { free(s); return NULL; }
	return s;
}

void party_core_snap_free(PartyCoreSnap *s) {
	if (!s) return;
	arena_core_snap_free(s->core);
	free(s);
}

bool party_core_save(PartyCore *pc, PartyCoreSnap *s) {
	s->field = pc->field;
	s->frames = pc->frames;
	memcpy(s->hp, pc->hp, sizeof s->hp);
	return arena_core_snap_save(pc->core, s->core);
}

bool party_core_load(PartyCore *pc, const PartyCoreSnap *s) {
	/* (the field's settings stay this battle's; its state goes back) */
	pc->field.standing = s->field.standing;
	pc->field.tick = s->field.tick;
	memcpy(pc->field.seen_hp, s->field.seen_hp, sizeof pc->field.seen_hp);
	memcpy(pc->field.seen_obj, s->field.seen_obj, sizeof pc->field.seen_obj);
	pc->frames = s->frames;
	memcpy(pc->hp, s->hp, sizeof pc->hp);
	return arena_core_snap_load(pc->core, s->core);
}

bool party_core_over(PartyCore *pc, bool *won) {
	struct mCore *c = pc->core;
	uint8_t mode = (uint8_t)c->rawRead8(c, BN6_GAMESTATE, -1);
	bool over = (mode == BN6_SUB_BATTLE && c->rawRead8(c, BN6_BATTLE_PHASE, -1) == BN6_PHASE_RESULTS) || mode == BN6_SUB_MAP || mode == BN6_SUB_ENTER;
	*won = over && c->rawRead8(c, BN6_BATTLE_RESULT, -1) == BN6_RESULT_WON;
	return over;
}

bool party_core_back(PartyCore *pc) {
	uint8_t mode = (uint8_t)pc->core->rawRead8(pc->core, BN6_GAMESTATE, -1);
	return mode == BN6_SUB_MAP || mode == BN6_SUB_ENTER;
}

void party_core_leave(PartyCore *pc, unsigned seat) {
	if ((int)seat >= pc->field.standing) return;
	uint32_t o = pc->core->rawRead32(pc->core, BN6_BATTLE_PLAYERS + seat * BN6_SIDE_PARTNER, -1);
	if (o) pc->core->rawWrite16(pc->core, o + BN6_T1_HP, -1, 0);
}

unsigned party_core_phase(PartyCore *pc) { return (unsigned)(uint8_t)pc->core->rawRead8(pc->core, BN6_BATTLE_PHASE, -1); }

unsigned party_core_hp(const PartyCore *pc, unsigned seat) { return seat < PARTY_MOST ? pc->hp[seat] : 0; }

unsigned party_core_foe_hp(PartyCore *pc) {
	uint32_t o = pc->core->rawRead32(pc->core, BN6_BATTLE_PLAYERS + BN6_BATTLE_PLAYER_STEP, -1);
	return o ? (unsigned)(uint16_t)pc->core->rawRead16(pc->core, o + BN6_T1_HP, -1) : 0;
}

const uint32_t *party_core_video(const PartyCore *pc) { return pc->video; }

struct mCore *party_core_gba(PartyCore *pc) { return pc->core; }

unsigned party_core_deck(PartyCore *pc, unsigned seat, uint16_t out[BN6_DECK_SIZE]) {
	return seat && seat <= TEAM_PARTNERS && pc->field.own ? arena_team_custom_deck(pc->core, seat - 1, out) : 0;
}

bool party_core_picking(PartyCore *pc, unsigned seat) {
	return seat && seat <= TEAM_PARTNERS && pc->field.own && arena_team_custom_picking(pc->core, seat - 1);
}

unsigned party_core_clock(PartyCore *pc) { return arena_team_custom_clock_of(pc->core); }


void party_core_sound(PartyCore *pc, bool heard) {
	if (heard) {
		emu_audio_open();
		emu_audio_from(pc->core);
	} else emu_audio_drop_from(pc->core);
}
