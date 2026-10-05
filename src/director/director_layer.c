/* Layers made and entered: the area a layer is drawn in and its host map,
 * its song, its flags, the run's start in the town and the layer's start,
 * the label the map shows and the next random battle's roll. */
#include "director_layer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "boss.h"
#include "cinema.h"
#include "darkbn6.h"
#include "darkchips.h"
#include "debug.h"
#include "devtools.h"
#include "director.h"
#include "director_board.h"
#include "director_dark.h"
#include "director_duel.h"
#include "director_folder.h"
#include "director_state.h"
#include "emu.h"
#include "encounter.h"
#include "game.h"
#include "gamecall.h"
#include "gfx.h"
#include "guardians.h"
#include "loot.h"
#include "meta.h"
#include "netmap.h"
#include "platform.h"
#include "powers.h"
#include "rivals.h"
#include "save.h"
#include "souls.h"
#include "story_words.h"
#include "talk.h"
#include "town.h"
#include "xbackdrop.h"
#include "xnavi.h"
#include "xsong.h"

/* The director: the run's structure around the game.
 *
 * A layer is generated (net_gen), built into its area's map (netmap), given
 * its exit pad and Mystery Data (mapslot, npc), and entered through the
 * game's warp. The game then runs everything MegaMan does; the director only
 * watches his position to take him to the next layer, and keeps the next
 * random battle's enemies in step with the depth. */

int director_debug_biome = -1;
int director_debug_area = -1;

/* An act begins (or a side layer): its title card, as Hades names each
 * region on entering it. */
void begin_area(bool new_act) {
	D.area_card = true;
	D.arrived = 0;
	if (!new_act) return;
	D.act_viruses = run.viruses_deleted;
	D.act_frames = 0;
	D.act_resumed = false;
}

/* The label the game keeps at the map's bottom right: its own name for the
 * original map ("AquarumComp3") gave way to where the run is. The game's
 * label routine is pointed at an archive of ours whose every name is it. */
#define LABEL_AT    (EMU_FREE + 0x152000)
#define LABEL_SIZE  0x1000
#define PET_LABEL_AT (EMU_FREE + 0x153000)   /* the PET's PLACE: the area and the layer (docs/EMULATION.md) */

/* The archive holds more than names: the PET prints its HP, zenny and
 * BugFrags by scripts 0xF0-0xF2, and others are placeholders. A copy of the
 * player's own archive is written with only its twelve-character names
 * pointed at the label. */
static int label_archive(const char *label, uint8_t *out, int max) {
	uint32_t src = rom_u32(BN6_MAP_NAMES_PTR - 0x08000000u) - 0x08000000u;
	if (src + 2 > ROM_SIZE) return 0;
	int n = rom_u16(src) / 2, end = 0;
	if (n <= 0 || n > 512) return 0;
	for (int k = 0; k < n; ++k) {
		int o = rom_u16(src + 2 * (uint32_t)k), j = o;
		while (src + (uint32_t)j < ROM_SIZE && R.data[src + (uint32_t)j] != 0xE6) ++j;
		if (j + 1 > end) end = j + 1;
	}
	if (end + 13 > max) return 0;
	memcpy(out, R.data + src, (size_t)end);
	char padded[20];
	snprintf(padded, sizeof padded, "%12.12s", label);
	ta_encode(padded, out + end, 12);
	out[end + 12] = 0xE6;
	for (int k = 0; k < n; ++k) {
		int o = rom_u16(src + 2 * (uint32_t)k), len = 0;
		bool name = true;
		while (R.data[src + (uint32_t)(o + len)] != 0xE6 && len < 16) { name &= R.data[src + (uint32_t)(o + len)] < 0xE7; ++len; }
		if (!name || len != 12) continue;
		out[2 * k] = (uint8_t)end;
		out[2 * k + 1] = (uint8_t)(end >> 8);
	}
	return end + 13;
}

void map_label(void) {
	static char last[16];
	char name[16];
	if (D.town) snprintf(name, sizeof name, "%s", town_info()->name ? town_info()->name : "Town");
	else if (run.side_kind == LAYER_UNDERNET) snprintf(name, sizeof name, "Undernet");
	else if (run.side_kind == LAYER_SECRET) snprintf(name, sizeof name, "Secret Area");
	else if (run.biome == BIOME_NEST) snprintf(name, sizeof name, "Cybeast Nest");
	else snprintf(name, sizeof name, "Layer %d", run.depth);
	/* the PET's PLACE says where, beside the layer ("ACDC HP 8"): a
	 * label of its own, the map's entry keeping "Layer 8" */
	char place[16];
	if (D.town || run.side_kind != LAYER_NORMAL || run.biome == BIOME_NEST) snprintf(place, sizeof place, "%s", name);
	else snprintf(place, sizeof place, "%s %d", guardian_area_short(run.biome), run.depth);
	static char last_place[16];
	if (!strcmp(name, last) && !strcmp(place, last_place) && emu_read32(BN6_MAP_NAMES_PTR) == LABEL_AT &&
		emu_read32(BN6_PET_MAP_NAMES_PTR) == PET_LABEL_AT) return;
	snprintf(last, sizeof last, "%s", name);
	snprintf(last_place, sizeof last_place, "%s", place);
	/* (right-aligned with spaces: the game pads its own with underscores,
	 * which show) */
	static uint8_t a[LABEL_SIZE];
	int len = label_archive(name, a, sizeof a);
	if (!len) return;
	emu_write(LABEL_AT, a, (size_t)len);
	emu_write32(BN6_MAP_NAMES_PTR, LABEL_AT);
	int plen = label_archive(place, a, sizeof a);
	if (plen) {
		emu_write(PET_LABEL_AT, a, (size_t)plen);
		emu_write32(BN6_PET_MAP_NAMES_PTR, PET_LABEL_AT);
	} else emu_write32(BN6_PET_MAP_NAMES_PTR, LABEL_AT);
}

/* A Navi on the net has named the act's guardian this session: the first
 * bystander on its first layer, or the Net Dealer's word from its second
 * (docs/META.md, what MegaMan knows). */
bool guardian_heard(void) {
	return D.layer_act && (D.heard_act == D.layer_act || D.dealer_act == D.layer_act ||
		(flag_get(LAYER_DEALER_TOLD_FLAG) && layer_objs_dealer_named));
}

bool director_guardian_heard(void) { return guardian_heard(); }

/* The next battle's enemies, for the game's encounter roll. */
void set_encounter(const Encounter *e, bool force) {
	D.foes = e->nfoes;
	D.next = *e;
	if (emu_debug_on()) {
		fprintf(stderr, "encounter field %02x player %02x:", e->field, e->player);
		for (int i = 0; i < e->nfoes; ++i) fprintf(stderr, " %d/%d/%d@%d,%d", e->foes[i].kind, e->foes[i].family, e->foes[i].version, e->foes[i].col, e->foes[i].row);
		fprintf(stderr, "\n");
	}
	if (force) emu_battle_force(e);
	else emu_encounter_set(e);
	D.rolled[emu_encounter_slot()] = *e;
}

static const __typeof__(R.layout->net_area[0]) *area(int biome) {
	return &R.layout->net_area[biome < 0 || biome >= NET_AREAS ? 0 : biome];
}

static int layer_biome(void) {
	if (director_debug_biome >= 0 && director_debug_biome < BIOME_COUNT) return director_debug_biome;
	if (run.side_kind == LAYER_UNDERNET) return BIOME_UNDERNET;
	if (run.side_kind == LAYER_SECRET) return BIOME_SECRET;
	return biome_for_depth(run.depth);
}

/* MegaMan stays in the run: no jacking out, and no PET save to the game's
 * own flash (the run keeps its checkpoints). The NaviCust is there from the
 * start, for the programs a run finds and buys. */
void lock_run(void) {
	flag_set(BN6_FLAG_NO_JACK);
	flag_set(BN6_FLAG_NO_PET_SAVE);
	flag_set(BN6_FLAG_NAVICUST);
}

/* The next random battle: the run's first two and every one on the first
 * layer after a guardian from the lower half of the act's band
 * (docs/PROGRESSION.md). */
void roll_encounter(void) {
	bool opening = run.side_kind == LAYER_NORMAL && layer_in_act(run.depth) == 0 &&
		(run.depth == 1 ? D.battles < 2 : true);
	bool first = run.depth == 1 && run.side_kind == LAYER_NORMAL && D.battles == 0;
	Encounter e = make_encounter(run.depth, run.biome, first ? ENC_FIRST : opening ? ENC_EASY : ENC_NORMAL);
	if (dev.gem) loot_add_gem(&e);
	set_encounter(&e, false);
}

/* The layer's Server's battle, rolled as the layer is built, aside from
 * its own rolls as the duel's is, so its words can name what it holds: a
 * Navi's signal said as one, his name where MegaMan has battled him (a
 * playtester's "strong virus signal" held ElementMan SP, session 65), and
 * the same battle after a CONTINUE. */
static void server_roll(void) {
	static char words[96];
	layer_objs_server_navi = "";
	D.server_rolled = false;
	uint32_t saved = rng_state();
	LootMemory fought, none = { -1, -1, 0, 0, 0 };
	loot_memory(&fought);
	loot_memory_set(&none);
	rng_seed(run.layer_seed ^ 0x5E4FE4C5u);
	D.server_enc = make_encounter(run.depth, run.biome, ENC_CHALLENGE);
	D.server_rolled = true;
	loot_memory_set(&fought);
	rng_restore(saved);
	if (emu_debug_on()) {
		fprintf(stderr, "server battle:");
		for (int k = 0; k < D.server_enc.nfoes; ++k) fprintf(stderr, " %d/%d/%d", D.server_enc.foes[k].kind, D.server_enc.foes[k].family, D.server_enc.foes[k].version);
		fprintf(stderr, "\n");
	}
	for (int k = 0; k < D.server_enc.nfoes; ++k) {
		const Foe *f = &D.server_enc.foes[k];
		if (f->kind != FOE_NAVI) continue;
		static const char *const suffix[4] = { "", " EX", " SP", " DS" };
		bool known = f->family >= 1 && f->family < RIVAL_NAVIS && (guardian_known(f->family) || rival(f->family)->met);
		if (known) snprintf(words, sizeof words, "It's %s%s!", guardian(f->family)->name, suffix[f->version & 3]);
		else snprintf(words, sizeof words, "One we've never battled.");
		layer_objs_server_navi = words;
		break;
	}
}

/* A layer takes a second to make on a New 3DS, seconds more where a new
 * area's tiles are learned: where the last took long, the next one's
 * making is named over the still picture as MegaMan leaves a layer, or the
 * pause reads as a hang (a player on a 3DS: "felt like it froze"). A PC's
 * 60 ms show nothing. */
#define SLOW_BUILD_MS 100
static double build_ms;

static void building_word(void) {
	if (build_ms < SLOW_BUILD_MS) return;
	static const char *word = "Building the next layer...";
	int w = text_width(word) + 12, y = P.core_y + EMU_H - 38;
	platform_draw_over();
	fill_rect(P.core_x + (EMU_W - w) / 2, y, w, 16, rgba(0, 16, 40, 200));
	text_draw(P.core_x + EMU_W / 2, y + 4, word, WHITE, TEXT_CENTER);
	platform_present_now();
}

/* The net area a layer is drawn in: the biome's own, another game's that
 * dresses it in this run (run_dress), or the test hook's area of another
 * game (docs/MULTIROM.md), laid out as the BN6 area it is like, which then
 * is the layer's biome. */
void director_net_biome_arg(const char *v) {
	if (v[0] == 'x') director_debug_area = run_debug_area = NET_AREAS + atoi(v + 1);
	else director_debug_biome = atoi(v);
}

static int layer_area(int *biome) {
	const NetAreaDef *x = director_debug_area >= 0 ? net_area_def(director_debug_area) : NULL;
	if (!x) return run.side_kind == LAYER_NORMAL ? run_dress(*biome) : *biome;
	*biome = x->like;
	return director_debug_area;
}

/* The song an area's layers play: another game's area its own theme,
 * copied into BN6 (docs/MULTIROM.md), where it can be; else `song`, the
 * BN6 area's. Its battles' themes (encounter_song) the same way: that
 * game's virus battle and boss music, else BN6's. */
static int layer_song(int tiles, int song) {
	const NetAreaDef *x = net_area_def(tiles);
	static const int bn6[2] = { ENCOUNTER_SONG_BATTLE, ENCOUNTER_SONG_BOSS };
	for (int b = 0; b < 2; ++b) {
		int theirs = x && x->xrom > 0 ? XR[x->xrom - 1].layout->battle_songs[b] : 0;
		encounter_song[b] = theirs ? xsong_install(x->xrom - 1, theirs, bn6[b]) : bn6[b];
	}
	return x && x->xrom > 0 && x->xsong ? xsong_install(x->xrom - 1, x->xsong, song) : song;
}

/* The layer's own flags, cleared as a layer begins: what has been told on
 * it (L, the dealer, the vendor, a heal, Chaud's call, Rush's gap) and
 * what its gates gave. */
static void layer_flags_clear(void) {
	static const int flags[] = { LAYER_TOLD_FLAG, LAYER_DEALER_TOLD_FLAG, LAYER_VENDOR_TOLD_FLAG, LAYER_HEAL_TOLD_FLAG, LAYER_VAULT_FLAG,
		LAYER_OFFICIAL_FLAG, LAYER_DUEL_CALLED_FLAG, LAYER_RUSH_TOLD_FLAG, LAYER_PCODE_FLAG, LAYER_NUMBER_SEALED_FLAG, LAYER_DARK_TAKEN_FLAG };
	for (unsigned k = 0; k < sizeof flags / sizeof *flags; ++k) flag_clear(flags[k]);
}

/* The map an area's layers take over, the Navi their bystanders are
 * (layer_objs_bystander), the map objects it lends them (layer_objs_xlooks),
 * their battles' background (encounter_backdrop) and the map's backdrop and
 * animations: another game's area its own, copied into BN6
 * (docs/MULTIROM.md), where they can be. */
static void layer_host(int tiles, int *group, int *number) {
	const NetAreaDef *a = net_area_def(tiles);
	if (!a) a = net_area_def(0);
	layer_objs_bystander = a->xrom > 0 && a->xnavi ? xnavi_slot(a->xrom - 1, a->xnavi, LAYER_BYSTANDER) : LAYER_BYSTANDER;
	layer_objs_bystander2 = a->xrom > 0 && a->xnavi2 ? xnavi_slot(a->xrom - 1, a->xnavi2, layer_objs_bystander) : layer_objs_bystander;
	layer_objs_xlooks = a->xrom > 0 ? a->xlooks : 0;
	encounter_backdrop = a->xrom > 0 && a->xbg ? xbackdrop_install(a->xrom - 1, a->xbg, -1) : -1;
	guest_backdrop = a->xrom > 0 && a->xbg ? a->xbg : -1;   /* (its own number, in its own game's tables) */
	xbackdrop_map(tiles);
	/* (and its battles, where its own engine can fight them on the guest
	 * core: its game's records for the map, guest.c) */
	const uint8_t *xb = a->xbattles[layer_in_act(run.depth)];
	D.guest_xrom = a->xrom - 1;
	D.guest_area = a;
	D.guest_group = xb[0];
	D.guest_number = xb[1];
	encounter_guest = a->xrom > 0 && xb[0] && guest_start(a->xrom - 1) && guest_records(D.guest_xrom, D.guest_group, D.guest_number) > 0;
	dark_flame_setup();
	if (a->xrom) { *group = a->over[0]; *number = a->over[1]; return; }
	*group = a->group;
	*number = a->host ? a->host - 1 : a->number;
}

static bool build_layer(void) {
	int biome = layer_biome(), tiles = layer_area(&biome);
	run.biome = biome;
	D.layer_tiles = tiles;
	run.layer_seed = run.seed ^ (uint32_t)(run.depth * 2654435761u) ^ (uint32_t)(run.side_kind * 40503u);
	LayerKit kit;
	netmap_kit(tiles, &kit);
	layer_generate(run.layer_seed, run.depth, biome, run.side_kind, &kit);
	if (emu_debug_on()) fprintf(stderr, "layer depth %d biome %d layout %d stairs %d rise %d\n", run.depth, biome, layer.layout, layer.nstairs, layer.rise);
	if (!netmap_build_layer(tiles, run.layer_seed)) return false;

	const __typeof__(R.layout->net_area[0]) *a = area(biome);
	layer_host(tiles, &D.group, &D.number);   /* (the map its layers take over, their bystanders) */
	/* (the layer just left had its Net Dealer speak: this act's next say
	 * a line, not the greeting and the pick's reasons again, 5 to 8 boxes
	 * on every layer for a playtester) */
	if (D.layer_act && flag_get(LAYER_DEALER_TOLD_FLAG) && layer_objs_dealer_named) D.dealer_act = D.layer_act;
	D.layer_act = run.side_kind == LAYER_NORMAL ? (run.depth - 1) / 3 + 1 : 0;
	layer_objs_dealer_again = D.layer_act && D.dealer_act == D.layer_act;
	navicust_set_spins(meta_spins());   /* (the draft fits what turns) */
	D.duel_call_due = false;
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_DUEL) duel_roll();
	layer_objs_server_navi = "";
	D.server_rolled = false;
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_CHALLENGE) { server_roll(); break; }
	if (!layer_objs_install(D.group, D.number, &D.objs)) return false;
	mapslot_music(D.group, D.number, layer_song(tiles, a->song));
	D.chosen = D.choices_due = 0;
	boss_begin_layer(D.objs.archive, &D.objs.guardian);

	D.battles = 0;
	roll_encounter();
	D.start_x = D.objs.start_x;
	D.start_y = D.objs.start_y;
	D.free_x = D.start_x;
	D.free_y = D.start_y;
	D.wedged = 0;
	/* until MegaMan takes it, the exit pad leads back to the layer's start */
	mapslot_exit_to(D.group, D.number, D.start_x, D.start_y, 4);
	D.active = true;
	D.frame = 0;
	D.gameover = false;
	D.act_guardian = D.objs.guardian.navi ? guardian(D.objs.guardian.navi)->name : NULL;
	bool first_of_act = run.side_kind == LAYER_NORMAL && layer_in_act(run.depth) == 0;
	if (first_of_act || run.side_kind != LAYER_NORMAL || biome == BIOME_NEST) begin_area(first_of_act);
	arrival_words();
	memset(D.seen, 0, sizeof D.seen);
	D.layer_told = D.more_told = false;
	D.fragment_due = false;
	D.spin_due = false;
	D.duel = false;
	D.duel_verdict_due = false;
	spins_sync();
	D.bugs_known = false;
	D.off_told = false;
	D.last_stop_told = false;
	D.final_told = false;
	D.checkpoint_data = false;
	D.pet_refreshed = false;
	layer_flags_clear();
	official_sync(false);
	D.arrow_pending = false;
	cinema_arrow(0, 0);
	D.secret_call = run.side_kind == LAYER_SECRET;
	talk_reset();
	return true;
}

/* build_layer timed (build_ms), named first where MegaMan leaves a layer */
bool new_layer(bool leaving) {
	if (leaving) building_word();
	/* (no battle watched: a duel's, left by a battle never finished) */
	emu_battle_unwatch();
	/* (the chip records as the run's helpers have them, in the ROM copy,
	 * which a state does not hold; BN6's own DarkChips' made whole there,
	 * and looked for in the Pack once on the map) */
	star_records(run_all_star());
	darkbn6_records();
	D.dark_pack_due = true;
	uint64_t t0 = SDL_GetPerformanceCounter();
	bool ok = build_layer();
	build_ms = (double)(SDL_GetPerformanceCounter() - t0) * 1000.0 / (double)SDL_GetPerformanceFrequency();
	return ok;
}

/* What this session has heard, forgotten by a run begun or continued (a
 * last run's act 1 is not this one's). */
void forget_heard(void) { D.heard_act = D.dealer_act = 0; D.mail_quiet = false; D.mail_due = 0; }

bool director_start_run(void) {
	drop_events();
	dark_new_run(run.seed);
	souls_new_run(run.seed);
	/* a new run leaves the last one behind: CONTINUE is for runs that
	 * have reached the net (one left so is no deletion to speak of) */
	no_room_told = -1;
	dark_price_told = false;
	off_board_forget();
	forget_heard();
	if (emu_debug_on()) {
		fprintf(stderr, "run guardians:");
		for (int a = 0; a < 4; ++a) {
			int g = run_guardian(run.biome_order[a]);
			fprintf(stderr, " %s%s", guardian(g)->name, guardian_known(g) || rival(g)->met ? "" : "(new)");
		}
		fprintf(stderr, " nest %s\n", guardian(run.boss_order[BIOME_NEST])->name);
	}
	set_start_folder();
	library_to_game();
	powers_bring(run.cross);
	dev_folder();
	star_folder_pack();
	note_folder_codes();
	town_after_abandon = save_exists();
	save_delete();
	/* the first layer, entered through the town's port; the town itself
	 * (its seed apart from the layers') */
	if (!new_layer(false)) return false;
	if (!town_plan(town_seed(run.seed)) || !town_install(D.group, D.number, D.start_x, D.start_y)) {
		fprintf(stderr, "town: not built; starting in the net\n");
		lock_run();
		emu_warp(D.group, D.number, D.start_x, D.start_y, 4);
		D.checkpoint = true;
		return true;
	}
	/* R jacks in there; the PET's own Save stays off */
	flag_clear(BN6_FLAG_NO_JACK);
	flag_set(BN6_FLAG_NO_PET_SAVE);
	flag_set(BN6_FLAG_NAVICUST);
	const TownInfo *ti = town_info();
	emu_warp(ti->group, ti->number, ti->start_x, ti->start_y, ti->start_face);
	D.town = true;
	D.town_seen = false;
	D.town_frames = 0;
	D.intro_said = false;
	D.port_told = false;
	D.free_x = town_info()->start_x;
	D.free_y = town_info()->start_y;
	return true;
}

bool director_start_layer(void) {
	drop_events();
	no_room_told = -1;
	dark_price_told = false;
	off_board_forget();
	forget_heard();
	D.town = false;
	/* (a headless run starting in the net: its folder as the town would
	 * have set it, and no DarkChips but the dev flag's) */
	if (run.depth == 1 && run.side_kind == LAYER_NORMAL) { set_start_folder(); library_to_game(); dark_new_run(run.seed); souls_new_run(run.seed); }
	else { dark_begin(run.seed); souls_begin(run.seed); }
	/* (and the Cross it brought at any depth, as a run has it there, and
	 * its chips in * with the All * helper) */
	powers_bring(run.cross);
	dev_folder();
	star_folder_pack();
	note_folder_codes();
	if (!new_layer(false)) return false;
	lock_run();
	emu_warp(D.group, D.number, D.start_x, D.start_y, 4);
	D.checkpoint = true;
	return true;
}
