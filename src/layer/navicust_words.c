/* What MegaMan says of the NaviCust (docs/NAVICUST.md, docs/VOICE.md):
 * what each program does, which programs L and R turn, the board's bugs
 * and why they came. The NaviCust itself, navicust.c. */
#include <stdio.h>
#include <string.h>

#include "navicust.h"
#include "rom.h"

/* What each program of the pool does, in MegaMan's words, its name first
 * (navicust.c's POOL) */
static const char *const abouts[] = {
	[1] = "SuperArmor: I won't flinch when I'm hit.",
	[2] = "Custom1: one more chip each turn.",
	[3] = "Custom2: two more chips each turn.",
	[4] = "MegFldr1: one more Mega chip in the Folder.",
	[5] = "MegFldr2: two more Mega chips in the Folder.",
	[6] = "GigFldr1: a second Giga chip in the Folder.",
	[7] = "FstBarr: I start every battle with a Barrier!",
	[8] = "Shield: B and Left raises a shield.",
	[9] = "Reflect: B and Left raises a shield that shoots back.",
	[10] = "AntiDmg: B and Left,and a hit turns into my counter!",
	[11] = "FlotShoe: panels can't hurt me or hold me.",
	[12] = "AirShoes: I can stand over holes.",
	[13] = "UnderSht: a hit that would delete me leaves me at 1 HP!",
	[14] = "ChpShufl: one reshuffle in the Custom screen.",
	[21] = "Collect: viruses drop their chips more often.",
	[25] = "SlipRunr: with B,I slide! Faster,but I keep going till something stops me.",
	[26] = "AutoHeal: a little HP back after every battle.",
	[27] = "BustPack: Buster attack,speed and charge up three each.",
	[28] = "BodyPack: SuperArmor and FlotShoe,AirShoes and UnderSht,all in one!",
	[29] = "FldrPak1: Custom1 and MegFldr1 in one.",
	[30] = "FldrPak2: Custom2 and MegFldr2 in one.",
	[31] = "BugStop: no bug can touch our NaviCust.",
	[35] = "Attack+1: a stronger Buster.",
	[36] = "Speed+1: a faster Buster.",
	[37] = "Charge+1: a quicker charge shot.",
	[38] = "AttckMAX: my Buster's attack at its highest.",
	[39] = "SpeedMAX: my Buster's speed at its highest.",
	[40] = "ChargMAX: my charge shot at its quickest.",
	[41] = "HP+50: fifty more max HP.",
	[42] = "HP+100: a hundred more max HP.",
	[43] = "HP+200: two hundred more max HP.",
	[44] = "HP+300: three hundred more max HP.",
	[45] = "HP+400: four hundred more max HP.",
	[46] = "HP+500: five hundred more max HP.",
};

const char *navicust_about(int program) {
	return navicust_in_pool(program) && program > 0 && program < (int)(sizeof abouts / sizeof *abouts) ? abouts[program] : NULL;
}

const char *navicust_color_turns(int c) {
	static char buf[120];
	if (c < 1 || c > 6) return "";
	if (navicust_spins() >> (c - 1) & 1) snprintf(buf, sizeof buf, "Our %s Spin lets L and R turn it!", navicust_color_name(c));
	else snprintf(buf, sizeof buf, "No %s Spin,so L and R won't turn it.", navicust_color_name(c));
	return buf;
}

const char *navicust_turn_words(int variant) {
	static char buf[160];
	int c = 0, held = 0;
	if (variant > 0 && variant < 47 * 4 && R.data && R.layout && R.layout->navicust_programs)
		c = R.data[R.layout->navicust_programs + (uint32_t)variant * 16 + 3];
	if (c >= 1 && c <= 6) return navicust_color_turns(c);
	for (int k = 1; k <= 6; ++k) held += navicust_spins() >> (k - 1) & 1;
	if (!held) return "L and R turn programs whose Spin we hold. None yet!";
	if (held == 6) return "L and R turn any program as we place it!";
	int k = snprintf(buf, sizeof buf, "L and R turn "), n = 0;
	for (int i = 1; i <= 6; ++i)
		if (navicust_spins() >> (i - 1) & 1)
			k += snprintf(buf + k, sizeof buf - (size_t)k, "%s%s", n++ == 0 ? "" : n == held ? " and " : ", ", navicust_color_name(i));
	snprintf(buf + k, sizeof buf - (size_t)k, " programs.\n%s we hold!", held == 1 ? "That's the Spin" : "Those are the Spins");
	return buf;
}

/* (BN6's compile, bn6f sub_813BBD4: a program's shape lands at its
 * column and row less 3 on the 7x7 grid; the command line is the grid's
 * row 3, whatever the board's size; colours touch only inside its 5x5) */
#define CMD_ROW 3

/* what the rules find of part `p` (1-based): a bit each, by the order the
 * words take */
enum { CAUSE_PLUS = 1, CAUSE_OFF = 2, CAUSE_EDGE = 4 };
static int part_causes(const uint8_t *grid, const NaviPart *part, int p, int w, int h) {
	bool on_line = false, past = false;
	for (int y = 0; y < NAVICUST_GRID; ++y)
		for (int x = 0; x < NAVICUST_GRID; ++x) {
			if (grid[y * NAVICUST_GRID + x] != p) continue;
			on_line |= y == CMD_ROW;
			past |= x < 1 || x > w || y < 1 || y > h;
		}
	return (part->kind == 1 && on_line ? CAUSE_PLUS : 0) | (part->kind == 0 && !on_line ? CAUSE_OFF : 0) | (past ? CAUSE_EDGE : 0);
}

/* the first two programs of one colour side by side inside the 5x5 (their
 * indexes, 1-based, in *a and *b); false for none */
static bool same_colors(const uint8_t *grid, const NaviPart *parts, int n, int *a, int *b) {
	for (int y = 1; y <= 5; ++y)
		for (int x = 1; x <= 5; ++x) {
			int p = grid[y * NAVICUST_GRID + x];
			if (!p || p > n) continue;
			int q[2] = { x < 5 ? grid[y * NAVICUST_GRID + x + 1] : 0, y < 5 ? grid[(y + 1) * NAVICUST_GRID + x] : 0 };
			for (int k = 0; k < 2; ++k)
				if (q[k] && q[k] <= n && q[k] != p && parts[q[k] - 1].color == parts[p - 1].color) { *a = p; *b = q[k]; return true; }
		}
	return false;
}

const char *navicust_bug_cause(const uint8_t grid[NAVICUST_GRID * NAVICUST_GRID], const NaviPart *parts, int n, int w, int h) {
	static const char *const why[3] = {
		" is a plus part on the command line! Those go anywhere else.",
		" is off the command line! A program needs a block on it.",
		" goes past the board's edge!",
	};
	static char buf[300];
	int k = 0, said = 0;
	buf[0] = 0;
	for (int p = 1; p <= n && said < 2; ++p) {
		int c = part_causes(grid, &parts[p - 1], p, w, h);
		for (int r = 0; r < 3 && said < 2; ++r)
			if (c >> r & 1 && parts[p - 1].name) {
				k += snprintf(buf + k, sizeof buf - (size_t)k, "%s%s%s", k ? " " : "", parts[p - 1].name, why[r]);
				++said;
			}
	}
	int a, b;
	if (said < 2 && same_colors(grid, parts, n, &a, &b) && parts[a - 1].name && parts[b - 1].name) {
		k += snprintf(buf + k, sizeof buf - (size_t)k, "%s%s and %s are both %s,and they touch!", k ? " " : "", parts[a - 1].name, parts[b - 1].name,
			navicust_color_name(parts[a - 1].color));
		++said;
	}
	return said ? buf : NULL;
}

const char *navicust_bug_words(const uint8_t counts[NAVICUST_BUGS], bool after_run, const char *cause) {
	/* the game's bug types (its compile counts one per violation; the
	 * level is the count, up to 3) and what each does, in MegaMan's words */
	static const char *const name[NAVICUST_BUGS] = {
		[1] = "moving", [2] = "emotion", [3] = "panel", [4] = "Custom", [5] = "encounter", [6] = "reward",
		[7] = "Buster", [9] = "HP",
	};
	/* (the named ones a box of their own after the bug's name; the colours'
	 * capitalised as said) */
	static const char *const effect[NAVICUST_BUGS] = {
		[1] = "Every step slides me as far as I can go",
		[2] = "My mood will swing in battle",
		[3] = "Panels may crack under me as I move",
		[4] = "I'll get fewer chips as a battle goes on",
		[5] = "More viruses will find us",
		[6] = "Battles will pay Zenny,not chips",
		[7] = "My Buster may misfire",
		[9] = "I'll lose HP in battle,faster with every hit",
		[11] = "five colors! Each battle will start with something odd",
		[12] = "six colors! Each battle will start with something odd,for longer",
	};
	static char buf[800];
	int k = 0, n = 0;
	for (int t = 1; t < NAVICUST_BUGS; ++t) n += counts[t] && effect[t];
	if (!n) return "";
	/* (after the RUN: its "OK! RUN complete!" and "Good job, Lan!" are the
	 * game's whatever the board, and a playtester read them as clean) */
	k += snprintf(buf + k, sizeof buf - (size_t)k, "@M Lan,%s has %s!", after_run ? "the RUN says OK...|@M But our NaviCust" : "our NaviCust",
		n == 1 ? "a bug" : "bugs");
	for (int t = 1; t < NAVICUST_BUGS && k < (int)sizeof buf - 160; ++t) {
		if (!counts[t] || !effect[t]) continue;
		if (!name[t]) {
			k += snprintf(buf + k, sizeof buf - (size_t)k, "|@M %c%s!", effect[t][0] - 'a' + 'A', effect[t] + 1);
			continue;
		}
		int level = counts[t] > 3 ? 3 : counts[t];
		bool vowel = strchr("aeiouAEIOU", name[t][0]) || name[t][0] == 'H';   /* ("an HP bug") */
		const char *article = level == 1 ? "A light" : level == 3 ? "A bad" : vowel ? "An" : "A";
		k += snprintf(buf + k, sizeof buf - (size_t)k, "|@M %s %s bug!|@M %s.", article, name[t], effect[t]);
	}
	/* (where to look, from the game's rules: a playtester told only what a
	 * bug did, of a program he had pushed over the edge, found the board
	 * looking clean again; the colours' count names itself) */
	bool placed = false;
	for (int t = 1; t < 11; ++t) placed |= counts[t] && effect[t];
	/* (and where it can be read from the board, what: a playtester told
	 * the four rules looked for the one he had broken) */
	if (placed && cause && k < (int)sizeof buf - 320) k += snprintf(buf + k, sizeof buf - (size_t)k, "|@M %s", cause);
	else if (placed && k < (int)sizeof buf - 220)
		k += snprintf(buf + k, sizeof buf - (size_t)k,
			"|@M Bugs come from going off the board,or off the command line.|"
			"@M Or a plus part on it,or same colors side by side.");
	if (k < (int)sizeof buf - 200)
		snprintf(buf + k, sizeof buf - (size_t)k, "|@M We can fix it in the PET,or live with it.|@M %s", navicust_turn_words(0));
	return buf;
}
