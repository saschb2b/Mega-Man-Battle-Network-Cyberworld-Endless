/* Jobs at home (docs/HOME.md, piece 5; issue #88), the director's part:
 * what each asker says at a visit (job_words.c), the job taken and settled
 * as their scripts set its flags, and its course in the Net (jobs.c): the
 * battles won, a layer's Mystery Data, a Mr.Prog's patch, the guardian's
 * fall, with MegaMan's word on each that matters. */
#include "director_jobs.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "boss.h"
#include "data.h"
#include "debug.h"
#include "director.h"
#include "director_folder.h"
#include "director_state.h"
#include "emu.h"
#include "flags.h"
#include "job_words.h"
#include "layer_objs.h"
#include "mapslot.h"
#include "run.h"
#include "talk.h"

static struct {
	int hp_before;        /* MegaMan's HP as the battle began */
	int frames;           /* how long it has run */
	bool healed, beaten;  /* the layer's Mr.Prog's patch, its guardian, as last seen */
	const char *words;    /* MegaMan's word to say, NULL none */
	bool vow_told;        /* the vow's reminder said, this act */
} J;

/* the chips of `element` MegaMan holds, the Pack's first, then the
 * Folder's (the script checks the Pack as Lan talks) */
static void bring_chips(JobTalk *jt, int element) {
	jt->nbring = 0;
	uint32_t pack = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK);
	bool pack_ok = pack >= BN6_EWRAM && pack + 12u * PACK_CHIPS < BN6_EWRAM_END;
	for (int id = 1; pack_ok && id < PACK_CHIPS && jt->nbring < JOB_BRING_MAX; ++id) {
		if (chip_hits_with(id) != element) continue;
		ChipInfo ci;
		chip_info(id, &ci);
		for (int k = 0; k < ci.ncodes && k < 4 && jt->nbring < JOB_BRING_MAX; ++k)
			if (emu_read8(pack + 12u * (uint32_t)id + (uint32_t)k)) {
				jt->bring_chip[jt->nbring] = id;
				jt->bring_code[jt->nbring++] = ci.codes[k] == '*' ? 26 : ci.codes[k] - 'A';
			}
	}
	uint16_t folder[BN6_FOLDER_ENTRIES];
	folder_now(folder);
	for (int i = 0; i < BN6_FOLDER_ENTRIES && jt->nbring < JOB_BRING_MAX; ++i) {
		int id = folder[i] & 0x1FF, code = folder[i] >> 9;
		bool known = false;
		for (int c = 0; c < jt->nbring; ++c) known |= jt->bring_chip[c] == id && jt->bring_code[c] == code;
		if (id && !known && chip_hits_with(id) == element) {
			jt->bring_chip[jt->nbring] = id;
			jt->bring_code[jt->nbring++] = code;
		}
	}
}

void home_jobs_visit(void) {
	Job offers[JOB_ASKERS];
	jobs_offers(run.seed, run.depth, offers);
	bool due = jobs_due(&run.job, run.depth);
	for (int k = 0; k < JOB_ASKERS; ++k) {
		JobTalk *jt = &job_talk[k];
		memset(jt, 0, sizeof *jt);
		bool theirs = run.job.kind != JOB_NONE && run.job.asker == k;
		jt->mode = theirs ? (due ? JOB_TALK_SETTLE : JOB_TALK_WAIT) : JOB_TALK_OFFER;
		jt->job = theirs ? run.job : offers[k];
		if (theirs && due && run.job.kind == JOB_BRING) bring_chips(jt, run.job.need);
	}
	J.vow_told = false;
	for (int c = 0; emu_debug_on() && c < job_talk[JOB_BOARD].nbring; ++c) {
		uint32_t pack = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK);
		int id = job_talk[JOB_BOARD].bring_chip[c];
		fprintf(stderr, "jobs: bring candidate %d code %d, pack bytes %d %d %d %d\n", id, job_talk[JOB_BOARD].bring_code[c], emu_read8(pack + 12u * (uint32_t)id),
			emu_read8(pack + 12u * (uint32_t)id + 1), emu_read8(pack + 12u * (uint32_t)id + 2), emu_read8(pack + 12u * (uint32_t)id + 3));
	}
	if (emu_debug_on())
		for (int k = 0; k < JOB_ASKERS; ++k)
			fprintf(stderr, "jobs: asker %d says %d, kind %d need %d pay %d:%u (state %d), %d chips of its element held\n", k, job_talk[k].mode,
				job_talk[k].job.kind, job_talk[k].job.need, job_talk[k].job.pay_kind, job_talk[k].job.pay, job_talk[k].job.state, job_talk[k].nbring);
}

void home_jobs_flags(void) {
	for (int k = 0; k < JOB_ASKERS; ++k) flag_clear(JOB_TAKE_FLAG + k);
	flag_clear(JOB_SETTLED_FLAG);
	if (run.job.kind != JOB_NONE) flag_set(JOB_HELD_FLAG);
	else flag_clear(JOB_HELD_FLAG);
}

void home_jobs_flag(int flag) {
	if (!D.town) return;
	if (flag >= JOB_TAKE_FLAG && flag < JOB_TAKE_FLAG + JOB_ASKERS && run.job.kind == JOB_NONE) {
		run.job = job_talk[flag - JOB_TAKE_FLAG].job;
		run.job.state = JOB_TAKEN;
		run.job.got = 0;
	} else if (flag == JOB_SETTLED_FLAG) memset(&run.job, 0, sizeof run.job);
	if (emu_debug_on()) fprintf(stderr, "jobs: flag %04x, the run's job now kind %d from %d\n", flag, run.job.kind, run.job.asker);
}

/* ---- in the Net ---- */

/* the run's job under way: taken, and its act not over (one left
 * unsettled at home counts on no further) */
static bool live(void) { return run.job.state == JOB_TAKEN && !jobs_due(&run.job, run_reached()); }

void home_jobs_battle_start(void) {
	J.hp_before = emu_read16(BN6_NAVI_HP);
	J.frames = 0;
}

void home_jobs_battle_frame(void) {
	int t = (int)emu_read32(BN6_BATTLE_TIMER);
	if (t > J.frames) J.frames = t;
}

void home_jobs_battle_end(bool won, int hp, int frames) {
	if (frames < 0) frames = J.frames;
	if (live() && jobs_battle(&run.job, won, J.hp_before - hp, frames)) J.words = job_done_words(run.job.asker);
	if (emu_debug_on() && run.job.state)
		fprintf(stderr, "jobs: battle %s, %d HP lost, %d frames: %d of %d\n", won ? "won" : "left", J.hp_before - hp, frames, run.job.got, run.job.need);
}

void home_jobs_layer_left(void) {
	int opened = 0;
	for (int k = 0; k < D.objs.nmd; ++k) opened += flag_get(MAPSLOT_MD_FLAG + k);
	if (live() && jobs_layer_left(&run.job, D.objs.nmd, opened)) J.words = job_done_words(run.job.asker);
	J.healed = J.beaten = false;
}

void home_jobs_watch(void) {
	if (D.town || !live()) return;
	bool healed = flag_get(LAYER_HEAL_TOLD_FLAG), beaten = boss_beaten();
	if (healed && !J.healed && jobs_heal(&run.job)) J.words = job_broken_words();
	if (beaten && !J.beaten && jobs_guardian(&run.job)) J.words = job_done_words(run.job.asker);
	J.healed = healed;
	J.beaten = beaten;
	/* (a vow's reminder, once an act, as its first layer begins) */
	if (run.job.kind == JOB_VOW && run.job.state == JOB_TAKEN && !J.vow_told && !J.words) {
		J.words = job_vow_words();
		J.vow_told = true;
	}
}

void home_jobs_words(void) {
	if (J.words && !D.town && !talk_busy() && !emu_read8(BN6_CHATBOX) && on_map() && !boss_fighting() && talk_start(J.words, FACE_MEGAMAN))
		J.words = NULL;
}

const char *home_jobs_status(const char *words) {
	static char buf[512];
	if (!run.job.kind || !jobs_due(&run.job, run.depth) || flag_get(JOB_SETTLED_FLAG)) return words;
	snprintf(buf, sizeof buf, "%s|%s", words, job_due_words(&run.job));
	return buf;
}
