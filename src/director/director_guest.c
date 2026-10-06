/* The older net's battles on the guest core (docs/MULTIROM.md, Guest
 * battles): picked to the act's band, MegaMan taken in as the run has him,
 * and their results brought back into the run. */
#include "director_guest.h"

#include <stdio.h>
#include <string.h>

#include "boss.h"
#include "dark_words.h"
#include "darkchips.h"
#include "data.h"
#include "debug.h"
#include "director.h"
#include "director_dark.h"
#include "director_folder.h"
#include "director_jobs.h"
#include "director_state.h"
#include "emu.h"
#include "encounter.h"
#include "gamecall.h"
#include "guardians.h"
#include "guest_words.h"
#include "pacing.h"
#include "save.h"
#include "souls.h"
#include "xguardian.h"

/* The older net reads our data as it knew it (the world's own reason for
 * the guest's translation, docs/MULTIROM.md): a chip whose code its chip
 * of that name lacked fought with one it had, and a chip won there came
 * back with a code ours has. MegaMan names each the first time, with the
 * chip (a quirk said in the world's terms, as the owner asked). */
static void guest_recode_note(const GuestResult *r) {
	/* (a chip its results screen gave, the busting level's or a Mystery
	 * Data's find, that came back in another code) */
	const GuestReward *w = r->reward.chip && r->reward.from >= 0 ? &r->reward : r->find.chip && r->find.from >= 0 ? &r->find : NULL;
	if (r->recoded && !(profile.recode_taught & 1) && !D.recode_due) {
		D.recode_due = 1;
		D.recode_chip = r->recode_chip; D.recode_from = r->recode_from; D.recode_to = r->recode_to;
	} else if (w && !(profile.recode_taught & 2) && !D.recode_due) {
		D.recode_due = 2;
		D.recode_chip = w->chip; D.recode_from = w->code; D.recode_to = w->from;
	}
}

char code_letter(int code) { return code == 26 ? '*' : (char)('A' + (code >= 0 && code < 26 ? code : 0)); }

/* What a guest battle's results screen gave, as the run got it ("Cannon
 * A", "200 zenny", "HP+50", "1 BugFrag", "none"; its second, a green
 * Mystery Data's, after "find") */
static int reward_words(char *s, size_t n, const GuestReward *w) {
	ChipInfo ci;
	if (w->chip) {
		chip_info(w->chip, &ci);
		return snprintf(s, n, "%s %c", ci.name, code_letter(w->code));
	}
	if (w->zenny) return snprintf(s, n, "%d zenny", w->zenny);
	if (w->bugfrags) return snprintf(s, n, "%d BugFrag%s", w->bugfrags, w->bugfrags == 1 ? "" : "s");
	if (w->heal) return snprintf(s, n, "HP+%d", w->heal);
	return snprintf(s, n, "none");
}

static const char *guest_reward_words(const GuestResult *r) {
	static char s[80];
	int k = reward_words(s, sizeof s, &r->reward);
	bool found = r->find.chip || r->find.zenny || r->find.bugfrags || r->find.heal;
	if (found && k > 0 && k + 6 < (int)sizeof s) {
		memcpy(s + k, " find ", 6);
		reward_words(s + k + 6, sizeof s - (size_t)k - 6, &r->find);
	}
	return s;
}

/* A reward of its results screen into BN6, through BN6's own routines:
 * BN6's chip of the same name to the Pack, zenny, BugFrags (HP+N is in
 * the HP already) */
static void reward_give(const GuestReward *w) {
	uint32_t out[2];
	if (w->chip) game_call_ret(BN6_GIVE_CHIPS, (uint32_t)w->chip, (uint32_t)w->code, 1, out);
	if (w->zenny) game_call(BN6_GIVE_ZENNY, (uint32_t)w->zenny, 0);
	if (w->bugfrags) game_call(BN6_GIVE_BUGFRAGS, (uint32_t)w->bugfrags, 0);
}

/* BN6's encounter walk cleared, as BN6 clears it entering the map a
 * battle returns to: an older net's battle never left BN6's map, so its
 * chance stayed at the walk's top, and a playtester met five battles on
 * one layer, one every 600 frames of walking (session 66). */
static void steps_cleared(void) {
	uint32_t steps = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_STEPS);
	if (steps < BN6_EWRAM || steps >= BN6_EWRAM_END) return;
	if (emu_debug_on()) fprintf(stderr, "guest: BN6's encounter walk %u cleared\n", (unsigned)emu_read16(steps + BN6_STEPS_WALKED));
	uint16_t z = 0;
	emu_write(steps + BN6_STEPS_WALKED, &z, sizeof z);
	emu_write(steps + BN6_STEPS_CHECKED, &z, sizeof z);
}

void director_guest_done(const GuestResult *r) {
	if (!D.active) return;
	/* (a guardian's of the older net: his end goes to his staging, boss.c,
	 * as BN6's guardians' battles' do) */
	int guardian_navi = D.guest_guardian;
	D.guest_guardian = 0;
	if (!guardian_navi) ++D.battles;
	if (emu_debug_on()) fprintf(stderr, "guest: %s in %d frames, HP %d\n", r->outcome == GUEST_LOST ? "lost" : r->outcome == GUEST_ESCAPED ? "escaped" : "won",
		r->frames, r->hp);
	/* (deleted in the guest's battle, its deletion shown there: the run ends
	 * as BN6's would, by him where a guardian deleted him) */
	if (r->outcome == GUEST_LOST) {
		D.gameover = true;
		D.lost_to = guardian_navi;
		if (guardian_navi) boss_lost();
		end_run();
		return;
	}
	/* (MegaMan's HP as the battle left it) */
	int max = emu_read16(BN6_NAVI_MAX_HP);
	uint16_t hp = (uint16_t)(r->hp < 1 ? 1 : r->hp > max ? max : r->hp);
	emu_write(BN6_NAVI_HP, &hp, sizeof hp);
	if (r->outcome == GUEST_WON && !guardian_navi) run.viruses_deleted += D.guest_foes;
	/* (a request's battles: MegaMan's HP before its results screen's HP+) */
	if (!guardian_navi) home_jobs_battle_end(r->outcome == GUEST_WON, r->hp - r->reward.heal - r->find.heal, r->frames);
	steps_cleared();
	if (!profile.guest_taught) D.guest_due = 1 + out_names(D.guest_out, sizeof D.guest_out);
	guest_recode_note(r);
	dark_set_counts(r->dark);
	if (r->dark_used) dark_price(false);
	if (r->dark_rose) dark_rose_said();
	/* (what its results screen gave: the busting level's reward, and the
	 * find of a green Mystery Data left on the field) */
	reward_give(&r->reward);
	reward_give(&r->find);
	/* (and in the run log, its reward as the run got it) */
	runlog_guest_end(r->outcome == GUEST_WON, guest_reward_words(r), r->frames);
	if (emu_debug_on())
		fprintf(stderr, "guest: MegaMan back at %d/%d HP, reward %s (folder codes %c%c%c); %d viruses deleted in the run\n", hp, max,
			r->outcome == GUEST_WON ? guest_reward_words(r) : "none", run.codes[0] ? 'A' + run.codes[0] - 1 : '-', run.codes[1] ? 'A' + run.codes[1] - 1 : '-',
			run.codes[2] ? 'A' + run.codes[2] - 1 : '-', run.viruses_deleted);
	if (guardian_navi) {
		boss_battle_over(r->outcome == GUEST_WON);
		D.pet_refreshed = false;   /* (Dad's mails made again: the Records, the report) */
	}
}

/* The older net's battles held to the act's band, floor and cap, as BN6's
 * are (pacing.c, docs/PROGRESSION.md): a record's viruses taken up to the
 * act's version, as BN6's formations are, inside the band from the area's
 * own records, else its other maps', else its game's other areas'; only
 * then under the band's floor, the same way; else the area's own weakest.
 * End Area dressing an opening act outclassed a run's first battle, a
 * playtester deleted there (session 65), and from act 2 ACDC Area's and
 * Oran Area's own fell far under the band (guest_pool, which build.py
 * pacing reads too). Its game's story battles (its roaming Navis at 2000
 * HP) never. */
static GuestBand guest_band(void) {
	PacingBand b = pacing_band(run.depth, false, D.battles == 0);
	int vlo, vhi;
	pacing_virus_versions(run.depth, false, &vlo, &vhi);
	return (GuestBand){ b.lo, b.hi, b.cap, vhi };
}

static uint32_t guest_pick(uint32_t h, GuestScale *sc) {
	static uint32_t pool[192];
	static uint8_t ups[192];
	GuestBand band = guest_band();
	int n = guest_pool(D.guest_xrom, D.guest_area, D.guest_group, D.guest_number, &band, pool, ups, 192, NULL);
	if (!n) return 0;
	uint32_t k = (h >> 16) % (uint32_t)n;
	*sc = (GuestScale){ ups[k], band.vcap };
	return pool[k];
}

/* MegaMan as the run has him, into a guest battle: his HP, the run's
 * folder (into `folder`) and his buster as the run's NaviCust makes it
 * (parity, no power of BN5's own), and the run's DarkChips, which BN5
 * brings into its Custom screen when he worries (docs/META.md) */
static GuestMegaMan guest_megaman(uint16_t folder[BN6_FOLDER_ENTRIES]) {
	folder_now(folder);
	GuestMegaMan mm = { emu_read16(BN6_NAVI_HP), emu_read16(BN6_NAVI_MAX_HP), folder, { 0 },
		{ emu_read8(BN6_NAVI_ATTACK), emu_read8(BN6_NAVI_SPEED), emu_read8(BN6_NAVI_CHARGE) }, run_all_star(), { 0 } };
	for (int k = 0; k < GUEST_DARK_KINDS; ++k) mm.dark[k] = (uint8_t)dark_count(k);
	memcpy(mm.codes, run.codes, sizeof mm.codes);
	mm.souls = (uint8_t)souls_held();   /* (the run's Souls, offered by its own rule: docs/META.md) */
	return mm;
}

bool director_guest_guardian(int navi, int version) {
	if (!guardian_older(navi) || !D.active) return false;
	uint16_t folder[BN6_FOLDER_ENTRIES];
	GuestMegaMan mm = guest_megaman(folder);
	/* (his HP held to the act's band as he spawns, a tenth more each notch
	 * of the Net's clock, docs/HOME.md, and his results screen paying an
	 * Unlocker's price, as BN6's guardians' battles do) */
	GuestBoss boss = { guardian_older_ai(navi), version, pacing_clock_hp(xguardian_hp_fought(navi, version, run.depth), run.clock),
		encounter_boss_zenny() };
	D.map_shown = false;
	if (!guest_boss_battle(&boss, &mm)) {
		if (emu_debug_on()) fprintf(stderr, "guest: guardian %s could not be fought in its engine\n", guardian(navi)->name);
		return false;
	}
	D.guest_guardian = navi;
	/* (in the run log as a guest battle, "guardian", his id by his game's) */
	int ids[1] = { 0 };
	uint32_t record = guest_navi_record(XROM_BN5_COLONEL_US, boss.ai, version);
	guest_record_foes_scaled(XROM_BN5_COLONEL_US, record, (GuestScale){ 0, 0 }, ids, 1);
	runlog_guest_guardian_start(record, navi, ids[0], boss.hp_cap);
	return true;
}

/* A battle the roll gave on a layer whose battles are the guest's: one of
 * its own game's records for the map, picked from the layer's seed and
 * its battles so far; the roll found the moment free (no chat, fade or
 * cutscene), so it begins at once (guest.c) */
void guest_begin(void) {
	uint32_t h = (run.layer_seed ^ (uint32_t)(D.battles + 1) * 2654435761u) * 2246822519u;
	uint16_t folder[BN6_FOLDER_ENTRIES];
	GuestMegaMan mm = guest_megaman(folder);
	GuestScale sc = { 0, 0 };
	uint32_t record = guest_pick(h, &sc);
	int hp = 0, dmg = 0;
	D.guest_foes = guest_record_scaled(D.guest_xrom, record, sc, &hp, &dmg);
	D.map_shown = false;   /* (SELECT's map, held as it began, drawn under its opening's white) */
	if (emu_debug_on()) {
		PacingBand b = pacing_band(run.depth, false, D.battles == 0);
		fprintf(stderr, "guest: record %08X, %d versions up, its viruses %d HP, %d a hit at most (the act's band %d-%d HP, %d a hit)\n", record,
			sc.up, hp, dmg, b.lo, b.hi, b.cap);
	}
	/* (in the run log as BN6's battles are, its viruses by BN5's ids, as
	 * scaled) */
	int ids[16] = { 0 }, n = guest_record_foes_scaled(D.guest_xrom, record, sc, ids, 16);
	if (record && guest_battle(record, sc, &mm)) { runlog_guest_start(record, ids, n, hp); home_jobs_battle_start(); }
}
