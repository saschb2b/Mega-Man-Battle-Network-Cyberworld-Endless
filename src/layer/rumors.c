/* The net's rumors (docs/META.md, Rumors): one bystander a layer passes on
 * the kind of thing a schoolyard did. BN6's own Program Advances, read from
 * its table and picked for the chips the folder holds (a recipe told by a
 * friend: Sword, WideSwrd and LongSwrd in one code); or a word on a secret
 * the profile has not found yet, which never gives its answer away (that a
 * compression code exists, not the code). Every line is true. */
#include "rumors.h"

#include <stdio.h>
#include <string.h>

#include "chip_pool.h"
#include "data.h"
#include "loot.h"
#include "meta.h"
#include "net.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"
#include "save.h"

/* ---- Program Advances ----
 * BN6's table (docs/ROM_DATA.md): an entry a Program Advance, its chips'
 * count, its kind, its result: kind 4 lists its chips, sent in that order in
 * one code; kind 0 one chip, three of it in codes in a row. */
#define PA_MAX 80
typedef struct { int result, kind, n, chip[4]; } Pa;

static int pa_read(Pa *out) {
	if (!R.data || !R.layout || !R.layout->program_advances) return 0;
	uint32_t at = R.layout->program_advances;
	int k = 0;
	bool rows = false;
	while (k < PA_MAX && at + 12 <= ROM_SIZE) {
		const uint8_t *p = R.data + at;
		int n = p[0], kind = p[1], res = p[2] | p[3] << 8;
		if (!n || n > 4 || (kind != 0 && kind != 4) || res < 256 || res >= 512) break;
		/* (the listed recipes past the codes-in-a-row block are the
		 * BattleChip Gate's, the Japan-only reader's: Cannon, Cannon,
		 * TankCan1 for GigaCannon, where the folder's is Cannon A, B, C;
		 * a whisper told that one, issue #56) */
		if (kind == 4 && rows) break;
		rows |= kind == 0;
		Pa *a = &out[k++];
		a->result = res; a->kind = kind; a->n = n;
		if (kind == 4) {
			for (int i = 0; i < n; ++i) a->chip[i] = p[4 + 2 * i] | p[5 + 2 * i] << 8;
			at += 4u + 2u * (uint32_t)n;
		} else {
			a->chip[0] = p[4] | p[5] << 8;
			at += 6;
		}
	}
	return k;
}

static bool has_code(const ChipInfo *ci, char c) { return memchr(ci->codes, c, (size_t)ci->ncodes) != NULL; }

/* The code to tell it in: kind 4, a letter every chip of it comes in; kind
 * 0, the first of three letters in a row its chip comes in; 0 none. */
static char pa_code(const Pa *a) {
	ChipInfo ci[4];
	for (int i = 0; i < (a->kind == 4 ? a->n : 1); ++i) chip_info(a->chip[i], &ci[i]);
	for (char c = 'A'; c <= 'Z'; ++c) {
		bool all = true;
		if (a->kind == 4)
			for (int i = 0; i < a->n && all; ++i) all = has_code(&ci[i], c);
		else all = c <= 'X' && has_code(&ci[0], c) && has_code(&ci[0], (char)(c + 1)) && has_code(&ci[0], (char)(c + 2));
		if (all) return c;
	}
	return 0;
}

/* Whether a run can hold every chip of it; and how many of them the
 * folder holds, which picks the recipe told */
static int pa_score(const Pa *a) {
	int score = 1;
	for (int i = 0; i < (a->kind == 4 ? a->n : 1); ++i) {
		if (chip_pool_tier(a->chip[i]) < 0) return 0;
		score += loot_folder_copies(a->chip[i]) > 0;
	}
	return score;
}

static const char *pa_line(uint32_t seed) {
	static Pa pa[PA_MAX];
	static char line[240];
	int n = pa_read(pa), best = 0, nbest = 0, pick = -1;
	for (int k = 0; k < n; ++k) {
		int s = pa_code(&pa[k]) ? pa_score(&pa[k]) : 0;
		if (s > best) { best = s; nbest = 0; }
		if (s == best && s) ++nbest;
	}
	if (!nbest) return NULL;
	int want = (int)(seed % (uint32_t)nbest);
	for (int k = 0; k < n && pick < 0; ++k)
		if (pa_code(&pa[k]) && pa_score(&pa[k]) == best && want-- == 0) pick = k;
	const Pa *a = &pa[pick];
	char c = pa_code(a);
	ChipInfo r, ci[4];
	chip_info(a->result, &r);
	for (int i = 0; i < (a->kind == 4 ? a->n : 1); ++i) chip_info(a->chip[i], &ci[i]);
	if (a->kind == 0)
		snprintf(line, sizeof line, "Psst! My operator sent %s %c, %s %c and %s %c, in that order...|And they turned into %s! A Program "
			"Advance!", ci[0].name, c, ci[0].name, c + 1, ci[0].name, c + 2, r.name);
	else if (a->n == 3)
		snprintf(line, sizeof line, "Psst! My operator sent %s %c, %s %c and %s %c, in that order...|And they turned into %s! A Program "
			"Advance!", ci[0].name, c, ci[1].name, c, ci[2].name, c, r.name);
	else
		snprintf(line, sizeof line, "Psst! My operator sent %s %c, %s %c, %s %c and %s %c, in that order...|And they turned into %s! A "
			"Program Advance!", ci[0].name, c, ci[1].name, c, ci[2].name, c, ci[3].name, c, r.name);
	return line;
}

/* ---- Secrets: each said only while the profile has yet to find it ---- */
static const char *secret_line(uint32_t seed) {
	const char *say[5];
	int n = 0;
	int depth = run.depth;
	if (!profile_codes_entered() && depth >= 2)
		say[n++] = "They say a NaviCust program shrinks if you hold RIGHT on it in the NaviCust and press a secret pattern of "
			"buttons.|Nobody I know has a pattern, though!";
	if (meta_spins() != 0x3F && depth >= 2 && depth <= 8)
		say[n++] = "Word is, a blue Mystery Data deeper in this net hides a Spin, a colour nobody's found yet, a new one every dive!";
	if (rival_clearance() == 0 && depth >= 2)
		say[n++] = "Somebody said Chaud teaches a NetBattler's trick to anyone who beats ProtoMan's time. Wonder what it is...";
	if (depth >= 4 && pacing_loop(depth) == 0)
		say[n++] = "I swear I saw a Navi walk off the end of a walkway, right out over nothing!|Some floor down here just can't be seen.";
	if (!profile.reg_taught && pacing_loop(depth) == 0 && !is_boss_depth(depth))
		say[n++] = "Every layer before a guardian hides a RegUp somewhere off the way.|More Reg memory, a bigger Regular Chip!";
	return n ? say[seed % (uint32_t)n] : NULL;
}

const char *rumors_line(void) {
	uint32_t seed = run.layer_seed * 2654435761u;
	const char *secret = secret_line(seed >> 8), *pa = pa_line(seed >> 16);
	/* (a secret one layer in three while one is left, a recipe else) */
	if (secret && (!pa || seed % 3 == 0)) return secret;
	return pa ? pa : secret;
}
