/* xdiff_spec.h's parser: each NAME=VALUE item into the spec */
#include "xdiff_spec.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5.h"
#include "emu.h"

/* ---- the spec ---- */

static uint32_t buttons(const char *s) {
	static const struct { const char *name; uint32_t key; } names[] = {
		{ "A", KEY_A }, { "B", KEY_B }, { "SELECT", KEY_SELECT }, { "START", KEY_START }, { "RIGHT", KEY_RIGHT },
		{ "LEFT", KEY_LEFT }, { "UP", KEY_UP }, { "DOWN", KEY_DOWN }, { "R", KEY_R }, { "L", KEY_L },
	};
	char buf[64];
	snprintf(buf, sizeof buf, "%s", s);
	uint32_t k = 0;
	char *save = NULL;
	for (char *t = strtok_r(buf, "+", &save); t; t = strtok_r(NULL, "+", &save))
		for (size_t i = 0; i < sizeof names / sizeof *names; ++i)
			if (!strcmp(t, names[i].name)) k |= names[i].key;
	return k;
}

/* "4F@62,01@51": enemy ids at column, row */
static bool foes_parse(Spec *s, char *v) {
	char *save = NULL;
	for (char *t = strtok_r(v, ",", &save); t; t = strtok_r(NULL, ",", &save)) {
		unsigned id;
		int c, r;
		if (sscanf(t, "%x@%1d%1d", &id, &c, &r) != 3 || c < 1 || c > 6 || r < 1 || r > 3 || s->nfoes >= MAX_FOES) return false;
		s->foes[s->nfoes].id = (int)id;
		s->foes[s->nfoes++].panel = r << 4 | c;
	}
	return s->nfoes > 0;
}

/* "150:UP:5,200:B+UP:2" */
static bool keys_parse(Spec *s, char *v) {
	char *save = NULL;
	for (char *t = strtok_r(v, ",", &save); t; t = strtok_r(NULL, ",", &save)) {
		unsigned at, n;
		char k[48];
		if (sscanf(t, "%u:%47[^:]:%u", &at, k, &n) != 3 || s->nkeys >= MAX_KEYS) return false;
		s->keys[s->nkeys++] = (Key){ at, n, buttons(k) };
	}
	return true;
}

/* "0000,000C,0236": hex numbers into out (at most max); how many, -1 where one is not */
static int hex_list(char *v, int *out, int max) {
	int n = 0;
	char *save = NULL;
	for (char *t = strtok_r(v, ",", &save); t; t = strtok_r(NULL, ",", &save)) {
		char *end;
		long x = strtol(t, &end, 16);
		if (*end || x < 0 || x > 0xFFFF || n >= max) return -1;
		out[n++] = (int)x;
	}
	return n;
}

/* "1:A.4/-.8/RIGHT.4": a Custom screen's keys, its screen and its steps */
static bool script_parse(Spec *s, int game, char *v) {
	if (s->nscripts >= MAX_SCRIPTS) return false;
	CustomScript *c = &s->scripts[s->nscripts];
	char *colon = strchr(v, ':');
	if (!colon || (c->screen = atoi(v)) < 1) return false;
	c->game = game;
	c->nsteps = 0;
	char *save = NULL;
	for (char *t = strtok_r(colon + 1, "/", &save); t; t = strtok_r(NULL, "/", &save)) {
		char *dot = strrchr(t, '.');
		if (!dot || c->nsteps >= MAX_SCRIPT_STEPS || atoi(dot + 1) < 1) return false;
		*dot = 0;
		c->steps[c->nsteps++] = (ScriptStep){ strcmp(t, "-") ? buttons(t) : 0, atoi(dot + 1) };
	}
	++s->nscripts;
	return c->nsteps > 0;
}

/* MegaMan's setup in each game: event flags, a folder, his buster, kept from harm, the Custom screens' keys; -1 where
 * `name` is none of them */
static int megaman_parse(Spec *s, const char *name, char *v) {
	int k = name[strlen(name) - 1] == '5' ? 0 : 1, got[BN6_FOLDER_ENTRIES];
	if (!strcmp(name, "flags5") || !strcmp(name, "flags6")) return (s->nflags[k] = hex_list(v, s->flags[k], MAX_FLAGS)) > 0;
	if (!strcmp(name, "folder5") || !strcmp(name, "folder6")) {
		int n = hex_list(v, got, BN6_FOLDER_ENTRIES);
		for (int i = 0; n > 0 && i < BN6_FOLDER_ENTRIES; ++i) s->folder[k][i] = (uint16_t)got[i % n];
		return (s->nfolder[k] = n) > 0;
	}
	if (!strcmp(name, "deck5") || !strcmp(name, "deck6")) {
		int n = hex_list(v, got, BN6_FOLDER_ENTRIES);
		for (int i = 0; n > 0 && i < BN6_FOLDER_ENTRIES; ++i) s->deck[k][i] = (uint16_t)got[i % n];
		return (s->ndeck[k] = n > 0 ? BN6_FOLDER_ENTRIES : 0) > 0;
	}
	if (!strcmp(name, "cust5") || !strcmp(name, "cust6")) return script_parse(s, 5 + k, v);
	if (!strcmp(name, "buster"))
		return sscanf(v, "%d:%d:%d", &s->buster[0], &s->buster[1], &s->buster[2]) == 3 && s->buster[0] >= 0 && s->buster[0] <= BN5_BUSTER_MAX &&
			s->buster[1] >= 0 && s->buster[1] <= BN5_BUSTER_MAX && s->buster[2] >= 0 && s->buster[2] <= BN5_BUSTER_MAX;
	if (!strcmp(name, "safe")) return (s->safe = atoi(v) != 0), 1;
	if (!strcmp(name, "dark")) return (s->dark = atoi(v) != 0), 1;
	return -1;
}

/* film=FROM:TO:STEP:PREFIX: 1 taken, 0 refused, -1 another item */
static int film_parse(Spec *s, const char *name, const char *v) {
	if (strcmp(name, "film")) return -1;
	Film *m = &s->film;
	return sscanf(v, "%d:%d:%d:%159s", &m->from, &m->to, &m->step, m->prefix) == 4 && m->step > 0 && m->to >= m->from;
}

/* the Souls BN6's MegaMan holds, and the rule or parts of them broken (the comparator's own test); -1 where `name` is
 * none of them */
static int souls_parse(Spec *s, const char *name, const char *v) {
	if (!strcmp(name, "souls6")) return (s->souls6 = (unsigned)strtoul(v, NULL, 16)) != 0;
	if (!strcmp(name, "unitebreak")) return (s->unitebreak = atoi(v)) > 0;
	if (!strcmp(name, "soulbreak")) return (s->soulbreak = atoi(v)) > 0;
	return -1;
}

/* the Dark Soul's chip AI's lists: BN6's written from BN5's (dslist), MegaMan's side's in BN5 given (dschips, BN5's ids) */
static int dark_parse(Spec *s, const char *name, char *v) {
	int got[BN6_FOLDER_ENTRIES];
	if (!strcmp(name, "dslist")) return (s->dslist = atoi(v) != 0), 1;
	if (strcmp(name, "dschips")) return -1;
	int n = hex_list(v, got, BN6_FOLDER_ENTRIES);
	for (int i = 0; i < n; ++i) s->dschips[i] = (uint16_t)got[i];
	s->dslist = true;
	return (s->ndschips = n) > 0;
}

/* the spec's numbers: MegaMan's panel, HP and element, the frames, the seed, the foes held; -1 where `name` is none of them */
static int numbers_parse(Spec *s, const char *name, const char *v) {
	unsigned a, b;
	if (!strcmp(name, "hp")) return (s->hp = atoi(v)) > 0 && s->hp <= 9999;
	if (!strcmp(name, "frames")) return (s->frames = atoi(v)) > 0;
	if (!strcmp(name, "hold")) return (s->hold = atoi(v)) > 0 && s->hold <= 0xFFFF;
	if (!strcmp(name, "mm_element")) return (s->mm_element = atoi(v)) >= 0 && s->mm_element <= 4;
	bool mm = !strcmp(name, "mm"), seed = !strcmp(name, "seed");
	if (!mm && !seed) return -1;
	if ((mm ? sscanf(v, "%1u%1u", &a, &b) : sscanf(v, "%x:%x", &a, &b)) != 2) return 0;
	if (seed) {
		s->seed[0] = a;
		s->seed[1] = b;
		return 1;
	}
	if (a < 1 || a > 3 || b < 1 || b > 3) return 0;
	s->mm = (int)(b << 4 | a);
	return 1;
}

/* "080050EC:08004C98:1A[:0]": a BN6 routine whose body is BN5's, a draw within it keyed by its BN5 twin (rng=replay), and
 * the word BN5's draws there take where not BN6's */
static bool twin_parse(Spec *s, const char *v) {
	unsigned a, b, n;
	int word = -1;
	int got = sscanf(v, "%x:%x:%x:%d", &a, &b, &n, &word);
	if (s->ntwins >= MAX_TWINS || got < 3 || !n || word < -1 || word > 1) return false;
	s->twins[s->ntwins++] = (XdTwin){ a, b, n, word };
	return true;
}

/* the comparator's own test's breaks: a poke (poke5=: at an address of
 * BN5's moved code, written at its copy), a call aimed elsewhere; and a
 * spec's change made in both games' copies alike (both=, an address of
 * BN5's: an element given, to put the engine's handling of it under
 * test), or in each game at its own place (each=BN5:BN6:value:size: a
 * table of the engine's both games have); -1 where `name` is none of them */
static int breaks_parse(Spec *s, const char *name, const char *v) {
	unsigned a, b, c;
	int n = 2;
	if (!strcmp(name, "each")) {
		if (s->npokes >= MAX_POKES || sscanf(v, "%x:%x:%x:%d", &a, &b, &c, &n) != 4 || (n != 1 && n != 2 && n != 4)) return 0;
		s->pokes[s->npokes++] = (Poke){ b, c, n, false, false, a };
		return 1;
	}
	bool call = !strcmp(name, "call"), both = !strcmp(name, "both"), bn5 = !strcmp(name, "poke5");
	if (!call && !both && !bn5 && strcmp(name, "poke")) return -1;
	if (s->npokes >= MAX_POKES) return 0;
	if (call ? sscanf(v, "%x:%x", &a, &b) != 2 : sscanf(v, "%x:%x:%d", &a, &b, &n) < 2 || (n != 1 && n != 2 && n != 4) || (both && n == 1)) return 0;
	s->pokes[s->npokes++] = (Poke){ a, b, call ? 0 : n, both, bn5 };
	return 1;
}

static bool item_parse(Spec *s, const char *name, char *v) {
	int r = numbers_parse(s, name, v);
	if (r < 0) r = breaks_parse(s, name, v);
	if (r < 0) r = megaman_parse(s, name, v);
	if (r < 0) r = dark_parse(s, name, v);
	if (r < 0) r = souls_parse(s, name, v);
	if (r < 0) r = film_parse(s, name, v);
	if (r >= 0) return r > 0;
	if (!strcmp(name, "foes")) return foes_parse(s, v);
	if (!strcmp(name, "keys")) return keys_parse(s, v);
	if (!strcmp(name, "out")) return snprintf(s->out, sizeof s->out, "%s", v) > 0;
	if (!strcmp(name, "rng")) return (s->follow = !strcmp(v, "follow")) || (s->replay = !strcmp(v, "replay"));
	if (!strcmp(name, "twin")) return twin_parse(s, v);
	if (!strcmp(name, "shot")) {
		Shot *h = &s->shots[s->nshots];
		return s->nshots < MAX_SHOTS && sscanf(v, "%d:%d:%d:%159s", &h->game, &h->screen, &h->frame, h->path) == 4 && ++s->nshots;
	}
	if (!strcmp(name, "field")) {
		unsigned f, f6 = 0x100;
		int got = sscanf(v, "%x:%x", &f, &f6);
		s->field = got >= 1 && f < 0x100 ? (int)f : -1;
		s->field6 = got == 2 && f6 < 0x100 ? (int)f6 : -1;
		return s->field >= 0 && (got == 1 || s->field6 >= 0);
	}
	if (!strcmp(name, "game")) {
		s->bn5 = !strcmp(v, "bn5");
		s->bn6 = !strcmp(v, "bn6");
		return s->bn5 || s->bn6;
	}
	return false;
}

bool xd_spec_parse(const char *text, Spec *s) {
	*s = (Spec){ .mm = 0x22, .hp = 100, .frames = 1200, .field6 = -1, .mm_element = -1, .seed = { 0x12345678u, 0x9ABCDEF0u }, .bn5 = true, .bn6 = true,
		.buster = { -1, -1, -1 } };
	snprintf(s->out, sizeof s->out, "xdiff");
	char buf[4096];
	snprintf(buf, sizeof buf, "%s", text);
	char *save = NULL;
	for (char *t = strtok_r(buf, ";", &save); t; t = strtok_r(NULL, ";", &save)) {
		char *eq = strchr(t, '=');
		if (!eq) { fprintf(stderr, "xdiff: %s is no item (NAME=VALUE)\n", t); return false; }
		*eq = 0;
		if (!item_parse(s, t, eq + 1)) { fprintf(stderr, "xdiff: %s=%s is not understood\n", t, eq + 1); return false; }
	}
	return s->nfoes > 0;
}
