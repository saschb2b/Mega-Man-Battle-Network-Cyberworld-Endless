/* director_courier.h. The courier in Lan's HP (docs/HOME.md, piece 2;
 * issue #108): a Mr.Prog who stands there while the town holds something
 * for this visit, BN6's "!!" over his head, and says where it waits, the
 * most valuable first: a finished request's pay with its asker, else the
 * new requests posted while none is held, then the keys AsterLand's
 * SubChip seller stocked. What the visit holds is fixed as it begins
 * (the run's state); what still waits follows the askers' flags, so a
 * reward taken or a request taken in the town sends him off, and L's word
 * on the requests in the town takes them from his. Heard once, his "!!"
 * goes, as a BN6 person's does. */
#include "director_courier.h"

#include <stdio.h>

#include "bn6.h"
#include "debug.h"
#include "director_state.h"
#include "emu.h"
#include "flags.h"
#include "home_words.h"
#include "job_words.h"
#include "jobs.h"
#include "lanhp.h"
#include "run.h"
#include "script_kit.h"
#include "shop.h"
#include "text.h"

/* what still waits (what_waits): bits of */
enum { WAITS_REWARD = 1, WAITS_REQUESTS = 2, WAITS_KEYS = 4 };

static struct {
	int reward;          /* the asker whose pay waits, -1 none */
	unsigned posted;     /* the askers with a request up this visit (bit k asker k) */
	int unlockers;       /* the SubChip seller's keys this visit */
	bool rush_food, www_id;
	int said;            /* what his words name now (what_waits), -1 unwritten */
} C = { -1, 0, 0, false, false, -1 };

static int what_waits(void) {
	int w = 0;
	if (C.reward >= 0 && !flag_get(JOB_SETTLED_FLAG)) w |= WAITS_REWARD;
	/* (the requests, unless L named them in the town this visit: a
	 * playtester heard them twice, session 73) */
	if (C.posted && !flag_get(JOB_HELD_FLAG) && !D.errands_told) w |= WAITS_REQUESTS;
	if (C.unlockers || C.rush_food || C.www_id) w |= WAITS_KEYS;
	return w;
}

void home_courier_visit(void) {
	C.reward = run.job.kind != JOB_NONE && run.job.state == JOB_DONE && jobs_due(&run.job, run.depth) ? run.job.asker : -1;
	C.posted = 0;
	for (int k = 0; k < JOB_ASKERS; ++k)
		if (job_talk[k].mode == JOB_TALK_OFFER) C.posted |= 1u << k;
	ShopItem stock[SHOP_MAX_ITEMS];
	int n = shop_home_subs(run.depth, stock);
	C.unlockers = 0;
	C.rush_food = C.www_id = false;
	for (int i = 0; i < n; ++i) {
		if (stock[i].kind != 1) continue;
		if (stock[i].id == SUB_UNLOCKER) C.unlockers = stock[i].stock;
		else if (stock[i].id == ITEM_RUSH_FOOD) C.rush_food = true;
		else if (stock[i].id == ITEM_WWW_ID) C.www_id = true;
	}
	C.said = -1;
	if (emu_debug_on())
		fprintf(stderr, "courier: reward from %d, requests 0x%x, keys %d Unlockers%s%s\n", C.reward, C.posted, C.unlockers,
			C.rush_food ? ", RushFood" : "", C.www_id ? ", a WWW-ID" : "");
	home_courier_frame();
}

void home_courier_frame(void) {
	int w = what_waits();
	if (w) flag_clear(LANHP_COURIER_GONE_FLAG);
	else { flag_set(LANHP_COURIER_GONE_FLAG); flag_set(LANHP_COURIER_TOLD_FLAG); }
	/* (his words written again as what waits changes, not while a chat
	 * box reads an archive) */
	if (!w || w == C.said || emu_read8(BN6_CHATBOX)) return;
	static TextArchive t;
	static uint8_t out[TEXT_ARCHIVE_MAX];
	bool keys = w & WAITS_KEYS, first = true;
	ta_begin(&t);
	ta_script(&t);
	ta_pages(&t, home_courier_words(w & WAITS_REWARD ? C.reward : -1, w & WAITS_REQUESTS ? C.posted : 0u, keys ? C.unlockers : 0,
		keys && C.rush_food, keys && C.www_id), FACE_PROG, &first);
	/* (heard, his "!!" goes; new words bring it back as he is met again) */
	ta_flag_set(&t, LANHP_COURIER_TOLD_FLAG);
	ta_end(&t);
	if (lanhp_courier_say(out, ta_build(&t, out))) { C.said = w; flag_clear(LANHP_COURIER_TOLD_FLAG); }
	if (emu_debug_on()) fprintf(stderr, "courier: his words name 0x%x%s\n", w, C.said == w ? "" : ", too long to write");
}
