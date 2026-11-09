/* Oran Isle's three people (docs/world/bn5-town.md, What is there): the
 * guardian's word as hearsay, a stall of three of the older Net's chips at
 * a Net Dealer's price, one to buy, and a job for the act. Their talk is
 * built here from the act the stop leads into; what they say is in
 * oran_words.c. */
#include "oran.h"

#include <stdio.h>
#include <string.h>

#include "chip_pool.h"
#include "data.h"
#include "game.h"
#include "guardians.h"
#include "jobs.h"
#include "loot.h"
#include "net.h"
#include "run.h"
#include "script_kit.h"
#include "text.h"

#define STALL_TIER_FIRST 1   /* the act's tier: uncommon from act 2 on, rare from act 4 */

static struct {
	int biome, navi, depth;
	OranStall stall;
} O;

static void roll_stall(uint32_t seed) {
	uint32_t kept = rng_state();
	rng_seed(seed ^ 0x0A7A15u);
	int act = jobs_act(O.depth), tier = STALL_TIER_FIRST + (act >= 3);
	memset(&O.stall, 0, sizeof O.stall);
	for (int k = 0; k < 3; ++k) {
		int id = -1;
		for (int tries = 0; tries < 16; ++tries) {
			id = chip_pool_pick_net(tier, CHIP_NET_BN5);
			bool again = id <= 0;
			for (int j = 0; j < k; ++j) again |= O.stall.chip[j] == id;
			if (!again) break;
		}
		if (id <= 0) id = chip_pool_pick(tier);
		ChipInfo ci;
		chip_info(id, &ci);
		char code = loot_fit_code(id, ci.ncodes ? ci.codes[0] : '*', true);
		O.stall.chip[k] = id;
		O.stall.code[k] = code == '*' ? 26 : code - 'A';
		O.stall.price[k] = chip_price(id);
		snprintf(O.stall.name[k], sizeof O.stall.name[k], "%s", ci.name);
	}
	rng_seed(kept);
}

void oran_prepare(int biome, int navi, int depth, uint32_t seed) {
	O.biome = biome;
	O.navi = navi;
	O.depth = depth;
	roll_stall(seed ^ (uint32_t)depth * 0x9E3779B9u);
}

const OranStall *oran_stall(void) { return &O.stall; }

/* the word: who guards the act, and his manner, as the net tells it */
static int word(TextArchive *t, int face) {
	const char *rumor = O.navi > 0 ? guardian_rumor(O.navi) : NULL;
	if (!rumor) return -1;
	return ta_say(t, face, oran_word_words(guardian(O.navi)->name, guardian_area_in_text(O.biome, LAYER_NORMAL), rumor));
}

/* the stall: its three in a column, each asked and bought, one only */
static int stall(TextArchive *t, int face) {
	const OranStall *s = &O.stall;
	int sold = ta_say(t, face, oran_stall_sold());
	int leave = ta_say(t, face, oran_stall_leave());
	int broke = ta_say(t, face, oran_stall_broke());
	int buy[3], menu = t->n + 6;   /* (the three's asks and buys first) */
	for (int k = 0; k < 3; ++k) {
		buy[k] = ta_script(t);
		ta_ask(t, face, oran_stall_ask(s->name[k], s->code[k] == 26 ? '*' : (char)('A' + s->code[k]), s->price[k]), menu);
		ta_take_zenny(t, s->price[k], t->n, broke);
		ta_end(t);
		ta_script(t);
		bool first = false;
		ta_give_chip(t, s->chip[k], s->code[k], 1);
		ta_got_chip(t, s->name[k], s->code[k], &first);
		ta_flag_set(t, ORAN_STALL_FLAG);
		ta_end(t);
	}
	ta_script(t);
	ta_mugshot(t, face);
	ta_clear(t);
	static const uint8_t opt[3][4] = { { 0xEB, 0x00, 0x00, 0x21 }, { 0xEB, 0x00, 0x11, 0x02 }, { 0xEB, 0x00, 0x22, 0x10 } };
	static const uint8_t space[] = { 0xEC, 0x00, 0x01 };
	for (int k = 0; k < 3; ++k) {
		char line[48];
		ta_bytes(t, opt[k], 4);
		ta_bytes(t, space, sizeof space);
		snprintf(line, sizeof line, "%s %c %dz%s", s->name[k], s->code[k] == 26 ? '*' : 'A' + s->code[k], s->price[k], k < 2 ? "\n" : "");
		ta_text(t, line);
	}
	ta_choose(t, buy, 3, leave);
	int open = ta_script(t);
	uint8_t done[] = { 0xEF, 0x00, (uint8_t)ORAN_STALL_FLAG, (uint8_t)(ORAN_STALL_FLAG >> 8), (uint8_t)sold, 0xFF };   /* ts_check_flag */
	ta_bytes(t, done, sizeof done);
	bool first = true;
	ta_pages(t, oran_stall_lead(), face, &first);
	ta_jump(t, menu);
	return open;
}

/* the job: one request for the act, taken or not (one at a time) */
static int job(TextArchive *t, int face) {
	int no = ta_say(t, face, oran_job_no());
	int taken = ta_say(t, face, oran_job_taken());
	if (run.job.kind != JOB_NONE && run.job.state == JOB_TAKEN) return ta_say(t, face, oran_job_held());
	int open = ta_script(t);
	uint8_t done[] = { 0xEF, 0x00, (uint8_t)ORAN_JOB_FLAG, (uint8_t)(ORAN_JOB_FLAG >> 8), (uint8_t)taken, 0xFF };   /* ts_check_flag */
	ta_bytes(t, done, sizeof done);
	bool first = true;
	ta_pages(t, oran_job_offer(ORAN_JOB_BATTLES), face, &first);
	ta_ask_in(t, face, oran_job_question(), no, true, false);
	ta_flag_set(t, ORAN_JOB_FLAG);
	ta_jump(t, taken);
	return open;
}

int oran_script(void *text, int i, int face) {
	TextArchive *t = text;
	switch (i) {
	case 0: return word(t, face);
	case 1: return stall(t, face);
	case 2: return job(t, face);
	default: return -1;
	}
}
