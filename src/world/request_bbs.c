/* request_bbs.h. The visit's requests on BN6's own Request BBS (bn6_bbs.h;
 * docs/world/home/jobs.md): the three askers' posts in one place, as BN6's
 * requests 0-2 (asker k's request k), their titles and posts the run's
 * (job_words.c) in place of BN6's, which a run never posts. BN6's screen
 * lists them, reads one out, asks "Take this request?" and keeps its own
 * rule, one at a time; a request taken there is its asker's taken
 * (director_jobs.c), and the asker pays where they stand. */
#include "request_bbs.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bn6_bbs.h"
#include "emu.h"
#include "flags.h"
#include "free_space.h"
#include "job_words.h"
#include "run.h"
#include "text.h"

#define TITLE_LETTERS 14   /* a title's room in the list */

/* `n` bytes as BN6's unpack of the board's text reads them, at `at`: LZ77
 * of literals alone (a flag byte of 0 before every eight), unpacking to the
 * archive's size word (its length, shifted up a byte) and the archive;
 * returns the next free place, word aligned */
static uint32_t lz_literals(uint32_t at, const uint8_t *data, int n) {
	static uint8_t out[TEXT_ARCHIVE_MAX + TEXT_ARCHIVE_MAX / 8 + 16];
	uint8_t head[4] = { 0, (uint8_t)(n + 4), (uint8_t)((n + 4) >> 8), (uint8_t)((n + 4) >> 16) };
	int k = 0, total = n + 4;
	out[k++] = 0x10;   /* (LZ77, then the unpacked size) */
	out[k++] = (uint8_t)total;
	out[k++] = (uint8_t)(total >> 8);
	out[k++] = (uint8_t)(total >> 16);
	for (int i = 0; i < total; ++i) {
		if (!(i & 7)) out[k++] = 0;
		out[k++] = i < 4 ? head[i] : data[i - 4];
	}
	emu_write(at, out, (size_t)k);
	return (at + (uint32_t)k + 3u) & ~3u;
}

/* whether asker k's request is up on the board this visit, and whether it
 * is the one held */
static bool held(int k) { return run.job.kind != JOB_NONE && run.job.asker == k; }
static bool posted(int k) { return job_talk[k].mode == JOB_TALK_OFFER || held(k); }

/* the titles' archive: request k's title padded to its room, the rest of
 * BN6's 48 empty (its list's empty rows read the last) */
static int titles(uint8_t *out) {
	static TextArchive t;
	ta_begin(&t);
	for (int r = 0; r < BN6_BBS_REQUESTS; ++r) {
		ta_script(&t);
		if (r < JOB_ASKERS && posted(r)) {
			char w[TITLE_LETTERS + 1];
			int n = snprintf(w, sizeof w, "%s", job_bbs_title(&job_talk[r].job));
			for (; n < TITLE_LETTERS; ++n) w[n] = ' ';
			w[TITLE_LETTERS] = 0;
			ta_text(&t, w);
		}
		ta_end(&t);
	}
	return ta_build(&t, out);
}

/* the posts' archive: request k's as BN6 lays its own out (the text's and
 * the arrow's places, its pages, a wait for A, then held for the board's
 * own question) */
static int posts(uint8_t *out) {
	static const uint8_t place[] = { 0xF7, 0x02, 0x18, 0x44, 0x03, 0xF7, 0x04, 0xD2, 0x6C };   /* ts_position_text, ts_position_arrow */
	static const uint8_t hold[] = { 0xE7, 0x00, 0xEE, 0xFF, 0x00, 0x00 };                      /* ts_key_wait, ts_wait_hold */
	static TextArchive t;
	ta_begin(&t);
	for (int r = 0; r < BN6_BBS_REQUESTS; ++r) {
		ta_script(&t);
		ta_bytes(&t, place, sizeof place);
		if (r < JOB_ASKERS && posted(r)) {
			const char *p = job_bbs_post(&job_talk[r].job);
			char box[200];
			int n = 0;
			/* (its boxes each a page or more, as the asker's own talk) */
			for (bool first = true;; ++p) {
				if (*p && *p != '|') { if (n < (int)sizeof box - 1) box[n++] = *p; continue; }
				box[n] = 0;
				n = 0;
				if (!first) { ta_wait(&t); ta_clear(&t); }
				ta_wrap(&t, box);
				first = false;
				if (!*p) break;
			}
		}
		ta_bytes(&t, hold, sizeof hold);
	}
	return ta_build(&t, out);
}

void request_bbs_post(void) {
	static uint8_t out[TEXT_ARCHIVE_MAX];
	uint32_t at = FREE_BBS;
	emu_write32(BN6_BBS_TITLES_PTR, at);
	at = lz_literals(at, out, titles(out));
	emu_write32(BN6_BBS_POSTS_PTR, at);
	lz_literals(at, out, posts(out));
	/* (the board's list newest first: the posts in reverse, the
	 * NetBattler's on top) */
	uint8_t list[JOB_ASKERS];
	int n = 0;
	for (int k = JOB_ASKERS - 1; k >= 0; --k)
		if (posted(k)) list[n++] = (uint8_t)k;
	emu_write(BN6_BBS_LIST, list, (size_t)n);
	emu_write32(BN6_BBS_COUNT, (uint32_t)n);
	emu_write32(BN6_BBS_SEEN, 0);
	for (int r = 0; r < BN6_BBS_REQUESTS; ++r) {
		bool up = r < JOB_ASKERS && posted(r), mine = r < JOB_ASKERS && held(r);
		int flags[] = { BN6_FLAG_BBS_POSTED, BN6_FLAG_BBS_NEW, BN6_FLAG_BBS_TAKEN, BN6_FLAG_BBS_FINISHED, BN6_FLAG_BBS_DONE };
		bool on[] = { up, up && !mine, mine, false, false };
		for (int f = 0; f < 5; ++f) {
			if (on[f]) flag_set(flags[f] + r);
			else flag_clear(flags[f] + r);
		}
	}
	/* (one at a time, BN6's own rule and words, while one is held) */
	if (run.job.kind != JOB_NONE) flag_set(BN6_FLAG_BBS_HOLDING);
	else flag_clear(BN6_FLAG_BBS_HOLDING);
	emu_write8(BN6_BBS_STATE + BN6_BBS_STATE_TAKEN, run.job.kind != JOB_NONE ? run.job.asker : 0);
}

int request_bbs_taken(int flag) {
	int k = flag - BN6_FLAG_BBS_TAKEN;
	return k >= 0 && k < JOB_ASKERS && job_talk[k].mode == JOB_TALK_OFFER ? k : -1;
}
