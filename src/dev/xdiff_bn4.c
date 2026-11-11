/* xdiff_bn4.h: BN4 Red Sun's own battle on the guest core, fought to its end. */
#include "xdiff_bn4.h"

#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>

#include "bn4.h"
#include "bytes.h"
#include "devtools.h"
#include "emu.h"
#include "guest.h"
#include "rom.h"
#include "xnavi_bn4.h"
#include "xnavi_bn4_sounds.h"
#include "xvirus_bn4.h"
#include "xvirus_shim.h"

#define BUDGET 9000   /* the guest's frames its battle may take, from its asking to its map or GAME OVER */

static int mode(void) { return guest_read8(guest_read32(BN4_TOOLKIT)); }
static int battle_phase(void) { return guest_read8(BN4_BATTLE_STATE + 1); }

/* A random battle's record as the net's are (weight 0x20, its song, the map's background, every progress), its entity
 * pointer guest_dev_battle's, then MegaMan and the foes (id, column, row, kind), 0xFF ending them: its length */
uint32_t xd4_record(const Spec *s, uint8_t *rec) {
	const uint8_t head[8] = { 0x20, 0x00, 0x14, BN4_SONG_BATTLE, 0x00, 0xFF, 0x1F, 0x00 };
	uint32_t len = 0;
	for (int i = 0; i < 8; ++i) rec[len++] = head[i];
	len += 4;
	const uint8_t mm[4] = { 0x00, (uint8_t)(s->mm & 0xF), (uint8_t)(s->mm >> 4), 0x00 };
	for (int i = 0; i < 4; ++i) rec[len++] = mm[i];
	for (int k = 0; k < s->nfoes; ++k) {
		const uint8_t f[4] = { (uint8_t)s->foes[k].id, (uint8_t)(s->foes[k].panel & 0xF), (uint8_t)(s->foes[k].panel >> 4), 0x01 };
		for (int i = 0; i < 4; ++i) rec[len++] = f[i];
	}
	rec[len++] = 0xFF;
	return len;
}

/* the keys of guest frame f: each Custom screen passed by START, then A; the buster tapped while it fights; A through
 * its results */
static uint32_t keys_of(int f) {
	if (mode() != BN4_MODE_BATTLE) return 0;
	int p = battle_phase();
	if (p == 0x08) return f % 40 < 2 ? KEY_START : f % 40 >= 20 && f % 40 < 22 ? KEY_A : 0;
	if (p == 0x0C && !guest_read8(BN4_BATTLE_RESULT)) return f % 16 < 2 ? KEY_B : 0;
	return f % 30 < 2 ? KEY_A : 0;
}

/* (--dev god and onehit in its battle: MegaMan's object at hp=, the rest of the first kind at 1 HP at most) */
static void dev_hold(const Spec *s) {
	if (mode() != BN4_MODE_BATTLE || battle_phase() < 0x08) return;
	for (uint32_t k = 0; k < 8; ++k) {
		uint32_t at = BN4_T1_OBJECTS + k * BN4_T1_SIZE + BN4_T1_HP;
		if (k == 0 && dev.god) guest_write16(at, (uint16_t)s->hp);
		else if (k && dev.onehit && guest_read16(at) > 1) guest_write16(at, 1);
	}
}

static void film_take(const Spec *s, int f) {
	const Film *m = &s->film;
	if (!m->step || f < m->from || f > m->to || (f - m->from) % m->step) return;
	static uint32_t px[EMU_W * EMU_H];
	const uint32_t *v = guest_video();
	for (int k = 0; k < EMU_W * EMU_H; ++k) px[k] = v[k] | 0xFF000000u;
	char path[200];
	snprintf(path, sizeof path, "%s_bn4_f%05d.bmp", m->prefix, f);
	SDL_Surface *b = SDL_CreateRGBSurfaceWithFormatFrom(px, EMU_W, EMU_H, 32, EMU_W * 4, SDL_PIXELFORMAT_ABGR8888);   /* (the core's pixels' order: emu_picture.c) */
	if (!b || SDL_SaveBMP(b, path) != 0) fprintf(stderr, "xdiff: %s not saved\n", path);
	SDL_FreeSurface(b);
}

/* the foes as BN4's enemy records have them */
static void foes_say(const Spec *s) {
	const uint8_t *rom = XR[XROM_BN4_RED_SUN_US].data;
	for (int k = 0; k < s->nfoes; ++k) {
		uint32_t id = (uint32_t)s->foes[k].id, stat = BN4_ENEMY_STATS - 0x08000000u + id * BN4_ENEMY_STAT_SIZE;
		uint16_t hp = id < BN4_ENEMY_COUNT ? get16(rom + stat + BN4_ENEMY_HP) : 0;
		printf("xdiff: BN4's enemy %02x at %d,%d: HP %d, element %d\n", (unsigned)id, s->foes[k].panel & 0xF, s->foes[k].panel >> 4,
			hp & 0xFFF, hp >> 12);
	}
}

/* BN4's k-th moved recipe whose ids a foe may be: its families' (k < xvirus_bn4_count), then its Navis' code (xnavi_bn4.h),
 * and how many versions it has */
static const XVirusFamily *moved_bn4(int k, int *versions) {
	if (k < xvirus_bn4_count) {
		*versions = XV_VERSIONS;
		return xvirus_bn4[k];
	}
	*versions = xnavi_bn4[k - xvirus_bn4_count]->versions;
	return &xnavi_bn4[k - xvirus_bn4_count]->code;
}

/* (fams with f, where it is not there yet and there is room: its count) */
static int with(const XVirusFamily **fams, int n, const XVirusFamily *f) {
	for (int j = 0; j < n; ++j)
		if (fams[j] == f) return n;
	if (n < MAX_FOES) fams[n++] = f;
	return n;
}

/* (a Navi's summons' families too, her fight's: Roll's Mettaurs, docs/multirom/bn4-navis.md, Roll built) */
static int summoned(const XNavi *x, const XVirusFamily **fams, int n) {
	for (int j = 0; x->summon && j < x->summon->nids; ++j)
		for (int k = 0; k < xvirus_bn4_count; ++k) {
			const XVirusFamily *f = xvirus_bn4[k];
			if (x->summon->ids[j].to >= f->first_id && x->summon->ids[j].to < f->first_id + XV_VERSIONS) n = with(fams, n, f);
		}
	return n;
}

int xd4_families(const Spec *s, const XVirusFamily **fams) {
	int n = 0;
	for (int i = 0; i < s->nfoes; ++i)
		for (int k = 0; k < xvirus_bn4_count + xnavi_bn4_count; ++k) {
			int versions;
			const XVirusFamily *f = moved_bn4(k, &versions);
			bool mine = false;
			for (int v = 0; v < versions; ++v) mine = mine || f->names[v] == s->foes[i].id;
			if (!mine) continue;
			n = with(fams, n, f);
			if (k >= xvirus_bn4_count) n = summoned(xnavi_bn4[k - xvirus_bn4_count], fams, n);
		}
	return n;
}

int xd4_summon_draws(const XVirusFamily *const *fams, int n, XvMoved *out, int max) {
	int k = 0;
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < xnavi_bn4_count && k < max; ++j)
			if (fams[i] == &xnavi_bn4[j]->code && xnavi_bn4[j]->summon)
				out[k++] = (XvMoved){ BN4_SPAWN_ENEMY_DRAW, xnavi_bn4[j]->summon->at + XV_SHIM_SUMMON_DRAW, 2 };
	return k;
}

int xd4_family_count(void) { return dev.bn4families ? xvirus_bn4_count : 0; }

const XVirusFamily *xd4_family(int k) { return xvirus_bn4[k]; }

int xd4_bn6_id(int id) {
	for (int k = 0; k < xvirus_bn4_count + xnavi_bn4_count; ++k) {
		int versions;
		const XVirusFamily *f = moved_bn4(k, &versions);
		for (int v = 0; v < versions; ++v)
			if (f->names[v] == id) return f->first_id + v;
	}
	return id;
}

void xd4_pool_point(const Spec *s) {
	for (int i = 0; i < s->nfoes; ++i) xnavi_bn4_sounds_for((uint32_t)xd4_bn6_id(s->foes[i].id));
}

/* (a byte into the guest's RAM, by its halfword) */
static void guest_wr8(uint32_t a, uint8_t v) {
	uint16_t w = guest_read16(a & ~1u);
	guest_write16(a & ~1u, a & 1 ? (uint16_t)((w & 0x00FF) | v << 8) : (uint16_t)((w & 0xFF00) | v));
}

void xd4_folder_put(const Spec *s) {
	static const FolderPlaces places = { BN4_TOOLKIT, BN4_TOOLKIT_FOLDERS, BN4_TOOLKIT_CHIP_COPY, BN4_CHIP_KEYS, BN4_CHIP_KEY_XOR, 0, 0, CHIP9_MASK,
		guest_read8, guest_read32, guest_wr8, guest_write16 };
	if (s->nfolder[0]) xd_folder_put(&places, s->folder[0]);
	if (s->karma4 >= 0) guest_write16(guest_read32(BN4_TOOLKIT + BN4_TOOLKIT_NAVI_STATS) + BN4_NAVI_KARMA, (uint16_t)s->karma4);
}

/* (a flag of BN4's event flags set, eToolkit +0x48's: flag f in byte f >> 3, bit 0x80 >> (f & 7)) */
static void flag_set4(int f) {
	uint32_t at = guest_read32(BN4_TOOLKIT + BN4_TOOLKIT_FLAGS) + (uint32_t)(f >> 3);
	guest_wr8(at, (uint8_t)(guest_read8(at) | 0x80 >> (f & 7)));
}

static bool souls_given;    /* (a Soul's spec: its form watched, xd4_form_watch) */
static int form_was, frames_run;

void xd4_form_watch(void) {
	++frames_run;
	int now = souls_given ? guest_read8(BN4_BATTLE_NAVI + BN4_NAVI_FORM) : 0;
	if (now != form_was) printf("xdiff: BN4's MegaMan in form %d at its frame %d\n", now, frames_run);
	form_was = now;
}

void xd4_souls_put(const Spec *s) {
	souls_given = s->souls4 != 0;
	form_was = frames_run = 0;
	for (int n = 1; n <= 12; ++n) {
		if (!(s->souls4 >> (n - 1) & 1)) continue;
		/* (its flag from BN4's own table: Blue Moon's six refused, 0xFF, docs/multirom/bn4-souls.md) */
		int flag = guest_read8(BN4_SOUL_FLAGS + (uint32_t)(n - 1));
		if (flag == 0xFF) { fprintf(stderr, "xdiff: BN4's Soul %d has no flag in Red Sun\n", n); continue; }
		flag_set4(flag);
		/* (its pseudo-chip's byte as the game writes it when it gives the Soul, else BN4_CHIP_GATE drops it at OK) */
		uint32_t id = BN4_SOUL_CHIPS + (uint32_t)n;
		guest_wr8(guest_read32(BN4_TOOLKIT + BN4_TOOLKIT_CHIP_COPY) + id, (uint8_t)(guest_read8(BN4_CHIP_KEYS + id) ^ BN4_CHIP_KEY_XOR));
	}
	if (s->souls4) flag_set4(BN4_FLAG_DOUBLE_SOUL);
}

int xdiff_bn4_run(const Spec *s) {
	if (!guest_start(XROM_BN4_RED_SUN_US)) {
		fprintf(stderr, "xdiff: BN4 Red Sun's ROM is not beside BN6's, or its guest did not reach Lan's room\n");
		return 2;
	}
	for (int k = 0; k < s->nfoes; ++k)
		if (s->foes[k].id < 1 || s->foes[k].id >= BN4_ENEMY_COUNT) { fprintf(stderr, "xdiff: %x is no BN4 enemy id\n", s->foes[k].id); return 1; }
	uint8_t rec[0x80];
	uint32_t len = xd4_record(s, rec);
	if (!guest_dev_battle(rec, len, s->hp, s->seed[0])) { fprintf(stderr, "xdiff: BN4's battle could not be asked\n"); return 1; }
	xd4_folder_put(s);
	xd4_souls_put(s);
	foes_say(s);
	int f = 0, result = 0, hp_end = -1, begun = -1;
	for (; guest_battling() && f < BUDGET; ++f) {
		guest_frame_quiet(keys_of(f));
		xd4_form_watch();
		dev_hold(s);
		film_take(s, f);
		if (begun < 0 && mode() == BN4_MODE_BATTLE) begun = f;
		if (!result && begun >= 0 && (guest_read8(BN4_BATTLE_RESULT) == 1 || guest_read8(BN4_BATTLE_RESULT) == 2)) {
			result = guest_read8(BN4_BATTLE_RESULT);
			hp_end = guest_read16(BN4_T1_OBJECTS + BN4_T1_HP);
			printf("xdiff: BN4's result at frame %d (its battle from %d): %d, MegaMan's HP %d\n", f, begun, result, hp_end);
		}
	}
	int over = mode();
	if (guest_battling() || !result) {
		printf("xdiff: BN4's battle did not end: main mode %02x, phase %02x after %d frames\n", over, battle_phase(), f);
		return 1;
	}
	/* (back on the map, the HP the battle left in its NaviStats, moved again) */
	int left = guest_read16(guest_read32(BN4_TOOLKIT + BN4_TOOLKIT_NAVI_STATS) + BN4_NAVI_HP);
	printf("xdiff: BN4's battle %s after %d frames, MegaMan's HP %d of %d left%s; ", result == 1 ? "won" : "lost", f, hp_end, s->hp,
		dev.god && dev.onehit ? " (--dev god,onehit)" : dev.god ? " (--dev god)" : dev.onehit ? " (--dev onehit)" : "");
	if (over == BN4_MODE_MAP) printf("back on the map, its NaviStats' HP %d\n", left);
	else printf("%s\n", over == BN4_MODE_GAME_OVER ? "GAME OVER" : "its main mode another");
	return 0;
}
