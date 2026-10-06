/* job_words.h. The askers' requests in their own voices (docs/VOICE.md):
 * the NetBattler at AsterLand's board (a grown-up who calls Lan "kid"),
 * the NetBattle club's member in class 6-1 (a classmate), the man from
 * Dad's lab in town. Each offers, waits, and settles at the next visit,
 * paying in BN6's own way ("MegaMan got: ..."); BN6's Request BBS's rule
 * holds: one request at a time. */
#include "job_words.h"

#include <stdio.h>

#include "data.h"
#include "flags.h"
#include "script_kit.h"

JobTalk job_talk[JOB_ASKERS];

static const char *element_name(int e) {
	switch (e) {
	case ELEM_FIRE: return "Fire";
	case ELEM_AQUA: return "Aqua";
	case ELEM_ELEC: return "Elec";
	default: return "Wood";
	}
}

/* "an Elec", "a Fire" */
static const char *an_element(int e) {
	static char w[16];
	snprintf(w, sizeof w, "%s %s", e == ELEM_AQUA || e == ELEM_ELEC ? "an" : "a", element_name(e));
	return w;
}

/* the pay as a thing: "600 Zenny", "7 BugFrags", "Cannon A", "HPMemory" */
static const char *pay_name(const Job *j) {
	static char w[32];
	ChipInfo ci;
	switch (j->pay_kind) {
	case PAY_ZENNY: snprintf(w, sizeof w, "%u Zenny", j->pay); break;
	case PAY_BUGFRAGS: snprintf(w, sizeof w, "%u BugFrags", j->pay); break;
	case PAY_CHIP:
		chip_info(j->pay, &ci);
		snprintf(w, sizeof w, "%s %c", ci.name, j->code == 26 ? '*' : 'A' + j->code);
		break;
	default: snprintf(w, sizeof w, "HPMemory"); break;
	}
	return w;
}

/* ---- what each says ---- */

static const char *offer_words(const Job *j) {
	static char w[256];
	switch (j->kind) {
	case JOB_BRING:
		snprintf(w, sizeof w, "Hey,kid! Got a sec?|I need %s chip. Big NetBattle!|Bring me one from your Pack?|I'll trade you my %s!",
			an_element(j->need), pay_name(j));
		break;
	case JOB_VOW:
		snprintf(w, sizeof w, "Heard about the guardians down there?|Bet you can't beat the next one|without a Mr.Prog's patch-up!|"
			"Do it,and my HPMemory's yours!");
		break;
	case JOB_QUICK:
		snprintf(w, sizeof w, "Lan! Club challenge!|Win %u battles,ten seconds each!|Do it,and %s are yours!", j->need, pay_name(j));
		break;
	case JOB_EXPLORE:
		snprintf(w, sizeof w, "Oh,Lan! Perfect timing.|The lab needs Mystery Data readings.|Open every one on a layer!|"
			"Three or more. I'll pay %s.", pay_name(j));
		break;
	default:   /* (JOB_CLEAN: the club's, or the lab's) */
		if (j->asker == JOB_CLUB) snprintf(w, sizeof w, "Lan! Club challenge!|Win %u battles without a scratch!|The prize is %s!", j->need, pay_name(j));
		else snprintf(w, sizeof w, "Oh,Lan! The lab's studying clean wins.|Win %u battles without taking damage.|I'll pay %s for the data.",
			j->need, pay_name(j));
		break;
	}
	return w;
}

/* the asker's lines but the offer and the settling: taken just now, asked
 * again, waiting on it, too busy with another, turned down, thanked again */
enum { SAY_TAKEN, SAY_AGAIN, SAY_WAIT, SAY_BUSY, SAY_NO, SAY_AFTER, SAYS };
static const char *const says[JOB_ASKERS][SAYS] = {
	[JOB_BOARD] = { "Ha! Deal!|Don't let me down,kid!", "Don't let me down,kid!", "Still on it,kid?|Don't let me down!",
		"Got a request going already?|One at a time,kid. Next time!", "Too busy,huh? No problem.", "Thanks again,kid!" },
	[JOB_CLUB] = { "Yay! Good luck,Lan!", "Good luck,Lan!", "How's the challenge going,Lan?|The club's rooting for you!",
		"Oh,you've got a request already?|One at a time! Next time,then.", "Aww... Maybe next time!", "That was amazing,Lan!" },
	[JOB_LAB] = { "Wonderful! Good luck,Lan.", "Good luck down there,Lan.", "The readings? No rush,Lan.",
		"You've got a request going?|One at a time. Next time,then!", "I see. Maybe next time.", "Thanks again,Lan!" },
};

static const char *done_words(const Job *j) {
	switch (j->asker) {
	case JOB_BOARD: return j->kind == JOB_VOW ? "You actually did it!?|Not one patch-up... Respect!|Here. You earned it." : "That's the one! Thanks,kid!|A deal's a deal. Here!";
	case JOB_CLUB: return "You did it,Lan!|The whole club saw it!|Here's your prize!";
	default: return "The readings came through!|Wonderful work,Lan. Here!";
	}
}

static const char *failed_words(const Job *j) {
	switch (j->asker) {
	case JOB_BOARD: return j->kind == JOB_VOW ? "Heh. Had a patch-up,huh?|No shame in that,kid." : "No worries. Next time,kid!";
	case JOB_CLUB: return "Aww,not quite...|Next time,Lan!";
	default: return "No readings this time?|Don't worry about it,Lan.";
	}
}

const char *job_done_words(int asker) {
	switch (asker) {
	case JOB_BOARD: return "@M We did it without a patch-up!|@M That HPMemory's ours,Lan!";
	case JOB_CLUB: return "@M Lan! That's the club's challenge done!|@M Let's tell them next time we're home!";
	default: return "@M That's the lab's readings,Lan!|@M Let's tell him next time we're home!";
	}
}

const char *job_due_words(const Job *job) {
	static const char *const where[JOB_ASKERS] = { "@M He's at AsterLand's board!", "@M Let's stop by the Academy!", "@M He's out in town somewhere!" };
	static const char *const who[JOB_ASKERS] = { "the NetBattler", "the club", "the man from the lab" };
	static char w[160];
	int k = job->asker < JOB_ASKERS ? job->asker : JOB_LAB;
	if (job->kind == JOB_BRING)
		snprintf(w, sizeof w, "@M Lan,the NetBattler wants %s chip!|@M One from our Pack. %s", an_element(job->need), where[k] + 3);
	else if (job->state == JOB_DONE) snprintf(w, sizeof w, "@M Lan,%s's reward is waiting!|%s", who[k], where[k]);
	else snprintf(w, sizeof w, "@M We didn't finish %s's request...|@M Let's tell them. Then we can take another!", who[k]);
	return w;
}

const char *job_broken_words(void) { return "@M Uh-oh... That patch-up broke our vow.|@M Oh well. Safety first,Lan!"; }

const char *job_vow_words(void) { return "@M Remember our vow,Lan!|@M No Mr.Prog patch-ups this time!"; }

const char *job_board_words(void) {
	static char w[200];
	const JobTalk *jt = &job_talk[JOB_BOARD];
	if (jt->mode != JOB_TALK_OFFER || flag_get(JOB_HELD_FLAG)) return "The request board.|Nothing new posted.";
	if (jt->job.kind == JOB_VOW)
		snprintf(w, sizeof w, "A request on the board!|\"A challenge! Beat the next guardian|without a Mr.Prog's patch-up.\"|\"Reward: HPMemory.\"");
	else snprintf(w, sizeof w, "A request on the board!|\"Wanted: %s chip.\"|\"Reward: %s.\"", an_element(jt->job.need), pay_name(&jt->job));
	return w;
}

/* ---- the scripts ---- */

/* ts_check_flag: on to `yes` where `flag` is set */
static void if_flag(TextArchive *t, int flag, int yes) {
	uint8_t b[] = { 0xEF, 0x00, (uint8_t)flag, (uint8_t)(flag >> 8), (uint8_t)yes, 0xFF };
	ta_bytes(t, b, sizeof b);
}

/* the held request settled: its flags as the director keeps them */
static void settled(TextArchive *t) {
	ta_flag_set(t, JOB_SETTLED_FLAG);
	ta_flag_clear(t, JOB_HELD_FLAG);
}

/* the pay given, as BN6 gives it, and told */
static void give_pay(TextArchive *t, const Job *j, bool *first) {
	uint32_t n = j->pay;
	switch (j->pay_kind) {
	case PAY_ZENNY: case PAY_BUGFRAGS: {
		/* (ts_check_give_zenny, ts_check_give_bug_frags) */
		uint8_t b[] = { 0xEF, (uint8_t)(j->pay_kind == PAY_ZENNY ? 0x0E : 0x12), (uint8_t)n, (uint8_t)(n >> 8), (uint8_t)(n >> 16), (uint8_t)(n >> 24),
			0xFF, 0xFF, 0xFF };
		ta_bytes(t, b, sizeof b);
		ta_got(t, pay_name(j), first);
		break;
	}
	case PAY_CHIP: {
		ChipInfo ci;
		chip_info(j->pay, &ci);
		ta_give_chip(t, j->pay, j->code, 1);
		ta_got_chip(t, ci.name, j->code, first);
		break;
	}
	default:
		ta_give_hp_memory(t, (int)n);
		ta_got_hp(t, (int)n, first);
		break;
	}
}

static int offer_script(TextArchive *t, int face, const Job *j) {
	int k = j->asker;
	int again = ta_say(t, face, says[k][SAY_AGAIN]);
	int busy = ta_say(t, face, says[k][SAY_BUSY]);
	int no = ta_say(t, face, says[k][SAY_NO]);
	int i = ta_script(t);
	if_flag(t, JOB_TAKE_FLAG + k, again);
	if_flag(t, JOB_HELD_FLAG, busy);
	bool first = true;
	ta_pages(t, offer_words(j), face, &first);
	/* (a vow costs a run its heals: it starts on No) */
	ta_ask_in(t, face, "Will you take it?\n", no, true, j->kind == JOB_VOW);
	ta_flag_set(t, JOB_TAKE_FLAG + k);
	ta_flag_set(t, JOB_HELD_FLAG);
	ta_pages(t, says[k][SAY_TAKEN], face, &first);
	ta_end(t);
	return i;
}

/* a chip handed over: chip c of the talk's, taken from the Pack, then the pay */
static int hand_over(TextArchive *t, int face, const JobTalk *jt, int c) {
	ChipInfo ci;
	chip_info(jt->bring_chip[c], &ci);
	int i = ta_script(t);
	char line[48];
	snprintf(line, sizeof line, "Lan handed over:\n\"%s %c\".", ci.name, jt->bring_code[c] == 26 ? '*' : 'A' + jt->bring_code[c]);
	ta_page(t, FACE_NONE, line, true);
	uint8_t take[] = { 0xF4, 0x11, (uint8_t)jt->bring_chip[c], (uint8_t)(jt->bring_chip[c] >> 8), (uint8_t)jt->bring_code[c], 1 };   /* ts_item_take_chip */
	ta_bytes(t, take, sizeof take);
	bool first = false;
	ta_pages(t, done_words(&jt->job), face, &first);
	give_pay(t, &jt->job, &first);
	settled(t);
	ta_end(t);
	return i;
}

/* a chip asked for: the first of the element's MegaMan holds that is in
 * the Pack as Lan talks; else where it has to be, and whether to let the
 * request go */
static int bring_script(TextArchive *t, int face, const JobTalk *jt, int after) {
	int hand[JOB_BRING_MAX];
	for (int c = 0; c < jt->nbring; ++c) hand[c] = hand_over(t, face, jt, c);
	int keep = ta_say(t, face, "I'll wait,then!");
	int none = ta_script(t);
	char w[128];
	snprintf(w, sizeof w, "No %s chip in your Pack?|From your Pack,kid. Not your Folder!", element_name(jt->job.need));
	bool first = true;
	ta_pages(t, w, face, &first);
	ta_ask_in(t, face, "Give up on it?\n", keep, true, true);
	ta_pages(t, failed_words(&jt->job), face, &first);
	settled(t);
	ta_end(t);
	int i = ta_script(t);
	if_flag(t, JOB_SETTLED_FLAG, after);
	for (int c = 0; c < jt->nbring; ++c) {
		/* (ts_check_pack_chip_code: one or more in the Pack) */
		uint8_t has[] = { 0xEF, 0x09, (uint8_t)jt->bring_chip[c], (uint8_t)(jt->bring_chip[c] >> 8), (uint8_t)jt->bring_code[c], 1,
			(uint8_t)hand[c], (uint8_t)hand[c], 0xFF };
		ta_bytes(t, has, sizeof has);
	}
	ta_jump(t, none);
	return i;
}

static int settle_script(TextArchive *t, int face, const JobTalk *jt) {
	int after = ta_say(t, face, says[jt->job.asker][SAY_AFTER]);
	if (jt->job.kind == JOB_BRING) return bring_script(t, face, jt, after);
	int i = ta_script(t);
	if_flag(t, JOB_SETTLED_FLAG, after);
	bool first = true;
	if (jt->job.state == JOB_DONE) {
		ta_pages(t, done_words(&jt->job), face, &first);
		give_pay(t, &jt->job, &first);
	} else ta_pages(t, failed_words(&jt->job), face, &first);
	settled(t);
	ta_end(t);
	return i;
}

int job_script(TextArchive *t, int asker, int face, const char *plain) {
	const JobTalk *jt = asker >= 0 && asker < JOB_ASKERS ? &job_talk[asker] : NULL;
	switch (jt ? jt->mode : JOB_TALK_NONE) {
	case JOB_TALK_OFFER: return offer_script(t, face, &jt->job);
	case JOB_TALK_WAIT: return ta_say(t, face, says[asker][SAY_WAIT]);
	case JOB_TALK_SETTLE: return settle_script(t, face, jt);
	default: return ta_talk(t, plain, face);
	}
}
