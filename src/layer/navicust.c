/* The NaviCust as a run's second build axis (navicust.h, docs/NAVICUST.md). */
#include "navicust.h"

#include <stdio.h>
#include <string.h>

#include "game.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"

/* The pool: BN6's programs a run can be offered, the act (0-5 of the first
 * cycle, 7 the second cycle) from which a guardian may offer each, its
 * build, and what it does. Left out: Humor and Poem (they answer the L
 * button, which the engine takes), Rush, Beat and Tango (VS battles only),
 * OilBody, Fish, Battery and Jungle (they steer the game's encounter roll,
 * which the engine replaces), Millions (its Mystery Data rule loops
 * forever on the engine's single-entry Mystery Data), SneakRun (it judges
 * the game's rolled battle, not the engine's) and NumbrOpn (its condition
 * is not yet known). The tiers follow the board: 4x4 until the act 2
 * guardian's ExpMemry, 5x4 until the act 4 guardian's, then 5x5. */
static const struct {
	uint8_t program, tier, build;
	const char *about;
} POOL[] = {
	{ 1, 0, BUILD_GUARD, "SuperArmor: I won't flinch when I'm hit." },
	{ 2, 0, BUILD_HAND, "Custom1: one more chip each turn." },
	{ 3, 4, BUILD_HAND, "Custom2: two more chips each turn." },
	{ 4, 0, BUILD_HAND, "MegFldr1: room for one more Mega chip in the folder." },
	{ 5, 2, BUILD_HAND, "MegFldr2: room for two more Mega chips in the folder." },
	{ 6, 4, BUILD_HAND, "GigFldr1: room for a second Giga chip in the folder." },
	{ 7, 0, BUILD_GUARD, "FstBarr: every battle starts with a Barrier on me." },
	{ 8, 0, BUILD_GUARD, "Shield: B and Left raises a shield." },
	{ 9, 2, BUILD_GUARD, "Reflect: B and Left raises a shield that shoots back." },
	{ 10, 2, BUILD_GUARD, "AntiDmg: B and Left, and a hit becomes my counterattack." },
	{ 11, 2, BUILD_FIELD, "FlotShoe: panels can't hurt me or hold me." },
	{ 12, 2, BUILD_FIELD, "AirShoes: I can stand over holes." },
	{ 13, 0, BUILD_GUARD, "UnderSht: a hit that would delete me leaves me 1 HP instead." },
	{ 14, 4, BUILD_HAND, "ChpShufl: one reshuffle in the Custom screen." },
	{ 21, 2, BUILD_FIELD, "Collect: viruses drop their chips more often." },
	{ 25, 0, BUILD_FIELD, "SlipRunr: hold B to slide along the net." },
	{ 26, 2, BUILD_FIELD, "AutoHeal: a little HP back after every battle." },
	{ 27, 2, BUILD_BUSTER, "BustPack: buster attack, speed and charge up three each." },
	{ 28, 4, BUILD_GUARD, "BodyPack: SuperArmor, FlotShoe, AirShoes and UnderSht in one." },
	{ 29, 2, BUILD_HAND, "FldrPak1: Custom1 and MegFldr1 in one." },
	{ 30, 4, BUILD_HAND, "FldrPak2: Custom2 and MegFldr2 in one." },
	{ 31, 4, BUILD_FIELD, "BugStop: no bug can touch our NaviCust." },
	{ 35, 0, BUILD_BUSTER, "Attack+1: a stronger buster." },
	{ 36, 0, BUILD_BUSTER, "Speed+1: a faster buster." },
	{ 37, 0, BUILD_BUSTER, "Charge+1: a quicker charge shot." },
	{ 38, 4, BUILD_BUSTER, "AttckMAX: my buster's attack at its highest." },
	{ 39, 2, BUILD_BUSTER, "SpeedMAX: my buster's speed at its highest." },
	{ 40, 2, BUILD_BUSTER, "ChargMAX: my charge shot at its quickest." },
	{ 41, 0, BUILD_HP, "HP+50: fifty more max HP." },
	{ 42, 0, BUILD_HP, "HP+100: a hundred more max HP." },
	{ 43, 2, BUILD_HP, "HP+200: two hundred more max HP." },
	{ 44, 7, BUILD_HP, "HP+300: three hundred more max HP." },
	{ 45, 7, BUILD_HP, "HP+400: four hundred more max HP." },
	{ 46, 7, BUILD_HP, "HP+500: five hundred more max HP." },
};
#define POOL_N ((int)(sizeof POOL / sizeof *POOL))

static int find(int program) {
	for (int i = 0; i < POOL_N; ++i)
		if (POOL[i].program == program) return i;
	return -1;
}

bool navicust_in_pool(int program) { return find(program) >= 0; }
int navicust_build(int program) { int i = find(program); return i < 0 ? -1 : POOL[i].build; }
const char *navicust_about(int program) { int i = find(program); return i < 0 ? NULL : POOL[i].about; }

/* the act a guardian at `depth` closes, as the pool's tiers count it */
static int reached(int depth) { return pacing_loop(depth) > 0 ? 7 : pacing_act(depth); }

int navicust_draft(int depth, NaviProgram out[NAVICUST_DRAFT]) {
	/* three builds of the five, in a random order, one program of each:
	 * the choice is a direction, not three of a kind */
	int builds[BUILD_COUNT] = { BUILD_BUSTER, BUILD_HAND, BUILD_GUARD, BUILD_FIELD, BUILD_HP };
	for (int i = BUILD_COUNT - 1; i > 0; --i) {
		int j = rng_range(0, i), t = builds[i];
		builds[i] = builds[j];
		builds[j] = t;
	}
	int n = 0, top = reached(depth);
	for (int b = 0; b < BUILD_COUNT && n < NAVICUST_DRAFT; ++b) {
		int fit[POOL_N], nfit = 0;
		for (int i = 0; i < POOL_N; ++i)
			if (POOL[i].build == builds[b] && POOL[i].tier <= top) fit[nfit++] = i;
		if (!nfit) continue;
		out[n].program = POOL[fit[rng_range(0, nfit - 1)]].program;
		out[n++].color = 0;
	}
	return n;
}

int navicust_color(int program) {
	/* the program records: 16 bytes per colour variant (program * 4 + v),
	 * the colour at +3, 0 where the variant is absent */
	if (!R.data || !R.layout || !R.layout->navicust_programs) return 0;
	int colors[4], n = 0;
	for (int v = 0; v < 4; ++v) {
		int c = R.data[R.layout->navicust_programs + (uint32_t)(program * 4 + v) * 16 + 3];
		if (c >= 1 && c <= 6) colors[n++] = c;
	}
	return n ? colors[rng_range(0, n - 1)] : 0;
}

int navicust_skip_frags(int depth) { return 10 + 5 * (pacing_act(depth) + 7 * pacing_loop(depth)); }

bool navicust_expmemry(int depth) { return depth == 6 || depth == 12; }

const char *navicust_bug_words(const uint8_t counts[NAVICUST_BUGS]) {
	/* the game's bug types (its compile counts one per violation; the
	 * level is the count, up to 3) and what each does, in MegaMan's words */
	static const char *const name[NAVICUST_BUGS] = {
		[1] = "moving", [2] = "emotion", [3] = "panel", [4] = "Custom", [5] = "encounter", [6] = "reward",
		[7] = "buster", [9] = "HP",
	};
	static const char *const effect[NAVICUST_BUGS] = {
		[1] = "every step slides me as far as I can go",
		[2] = "my mood will swing in battle",
		[3] = "panels may crack under me as I move",
		[4] = "fewer chips each turn as a battle goes on",
		[5] = "more viruses will find us",
		[6] = "battles will pay zenny instead of chips",
		[7] = "my buster may misfire",
		[9] = "I'll lose HP in battle, faster with every hit",
		[11] = "five colors: something odd happens at the start of every battle",
		[12] = "six colors: something odd happens at the start of every battle, for longer",
	};
	static char buf[800];
	int k = 0, n = 0;
	for (int t = 1; t < NAVICUST_BUGS; ++t) n += counts[t] && effect[t];
	if (!n) return "";
	k += snprintf(buf + k, sizeof buf - (size_t)k, n == 1 ? "@M Lan, our NaviCust has a bug!" : "@M Lan, our NaviCust has bugs!");
	for (int t = 1; t < NAVICUST_BUGS && k < (int)sizeof buf - 160; ++t) {
		if (!counts[t] || !effect[t]) continue;
		if (!name[t]) {
			k += snprintf(buf + k, sizeof buf - (size_t)k, "|@M %c%s!", effect[t][0] - 'a' + 'A', effect[t] + 1);
			continue;
		}
		int level = counts[t] > 3 ? 3 : counts[t];
		bool vowel = strchr("aeiouAEIOU", name[t][0]) || name[t][0] == 'H';   /* ("an HP bug") */
		const char *article = level == 1 ? "A light" : level == 3 ? "A bad" : vowel ? "An" : "A";
		k += snprintf(buf + k, sizeof buf - (size_t)k, "|@M %s %s bug: %s.", article, name[t], effect[t]);
	}
	if (k < (int)sizeof buf - 80) snprintf(buf + k, sizeof buf - (size_t)k, "|@M We can rearrange it in the PET, or live with it.");
	return buf;
}
