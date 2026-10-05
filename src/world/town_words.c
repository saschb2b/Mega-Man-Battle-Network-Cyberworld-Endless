/* town_words.h. A town that remembers (docs/HOME.md; docs/VOICE.md): the
 * plaza's Mr.Prog calls the Net's news, Lan's classmate his progress, the
 * neighbor and the man from the lab the run and the runs before it, the
 * gossip by the statue a guardian beaten often. One line a person, each
 * visit its own; the rest say what is said where they stand. */
#include "town_words.h"

#include <ctype.h>
#include <stdio.h>

#include "guardians.h"
#include "net.h"
#include "rivals.h"
#include "run.h"
#include "save.h"

/* who has lines of their own: the Mr.Prog of sprite list 7, and people of
 * list 5 by their sprites */
enum { NEWS_PROG = 0x0F, CLASSMATE = 0x34, NEIGHBOR = 0x38, LAB = 0x2D, GOSSIP = 0x36 };

/* the guardian deleted at the act just done, and his area; 0 at the run's
 * start */
static int last_guardian(int *biome) {
	int depth = run_reached(), p = (depth - 1) % CYCLE_LAYERS;
	if (depth <= 1) return 0;
	/* (past the endless Nest, the next cycle's start) */
	if (p == 0) { *biome = BIOME_NEST; return run.boss_order[BIOME_NEST]; }
	*biome = run.biome_order[p / 3 - 1];
	return run_guardian(*biome);
}

/* the guardian MegaMan has beaten most often in any run, and how often */
static int beaten_most(int *times) {
	int most = 0;
	*times = 0;
	for (int n = 1; n < RIVAL_NAVIS; ++n)
		if (rival(n)->megaman_won > *times && guardian(n)->name[0] != '?') { *times = rival(n)->megaman_won; most = n; }
	return most;
}

/* a name as a Mr.Prog says it, in capitals (two at once) */
static const char *loud(const char *s) {
	static char buf[2][40];
	static int k;
	k ^= 1;
	snprintf(buf[k], sizeof buf[k], "%s", s);
	for (char *p = buf[k]; *p; ++p) *p = (char)toupper((unsigned char)*p);
	return buf[k];
}

static const char *news_words(char *w, size_t n, int navi, int biome) {
	if (navi)
		snprintf(w, n, "NET NEWS! NET NEWS!|%s'S COPY IS DELETED!|IN %s! WHAT A BATTLE!", loud(guardian(navi)->name),
			loud(guardian_area_in_text(biome, LAYER_NORMAL)));
	else if (!profile.runs) snprintf(w, n, "NET NEWS! NET NEWS!|A NEW NET OPENED UNDER TOWN!|THEY CALL IT THE ENDLESS NET!");
	else if (profile.last_won) snprintf(w, n, "NET NEWS! NET NEWS!|LAN HIKARI BROUGHT DOWN THE NEST!|THE WHOLE TOWN'S TALKING!");
	else if (profile.last_lost_to)
		snprintf(w, n, "NET NEWS! NET NEWS!|%s'S COPY DELETED MEGAMAN!|ON LAYER %d! OH NO,OH NO!", loud(guardian(profile.last_lost_to)->name),
			profile.last_depth);
	else snprintf(w, n, "NET NEWS! NET NEWS!|MEGAMAN DOVE TO LAYER %d LAST TIME!", profile.last_depth);
	return w;
}

static const char *classmate_words(char *w, size_t n, int navi, const char *place) {
	if (navi && run.bosses_beaten >= 3) snprintf(w, n, "%d guardians!? Lan,you're unreal!|The whole class is rooting for you!", run.bosses_beaten);
	else if (navi && run.bosses_beaten == 2) snprintf(w, n, "Two guardians already!?|Lan,you're a legend!");
	else if (navi) snprintf(w, n, "Lan! You beat %s's copy?|No way! Tell me everything at school!", guardian(navi)->name);
	else if (profile.runs) snprintf(w, n, "Hey,Lan! Diving again?|My Navi only got to layer 2...|How do you do it?");
	else return place;
	return w;
}

static const char *neighbor_words(char *w, size_t n, int navi, const char *place) {
	if (navi) snprintf(w, n, "Oh,Lan! Back home already?|Did MegaMan win again? My,my!");
	else if (run.threat) snprintf(w, n, "Heading out,Lan?|They say the Net's rougher these days...|Be careful,OK?");
	else return place;
	return w;
}

static const char *lab_words(char *w, size_t n, int navi, const char *place) {
	if (navi) snprintf(w, n, "Phew! Long day at the lab...|Your readings keep coming in!|Your dad's so proud,Lan!");
	else if (profile.best_depth >= 3)
		snprintf(w, n, "Phew! Long day at the lab...|Your dad keeps looking at your best dive!|Layer %d! He can't stop smiling!", profile.best_depth);
	else return place;
	return w;
}

static const char *gossip_words(char *w, size_t n) {
	int times, often = beaten_most(&times);
	if (often && times >= 3) snprintf(w, n, "Did you hear?|Lan's MegaMan beat %s %d times!|Down in the Endless Net!", guardian(often)->name, times);
	else snprintf(w, n, "Did you hear?|The Net under town copies every battle!|Even the strongest Navis!");
	return w;
}

const char *town_folk_words(int cat, int sprite, const char *place_line) {
	static char w[240];
	int biome = 0, navi = last_guardian(&biome);
	if (cat == 7 && sprite == NEWS_PROG) return news_words(w, sizeof w, navi, biome);
	if (cat != 5) return place_line;
	switch (sprite) {
	case CLASSMATE: return classmate_words(w, sizeof w, navi, place_line);
	case NEIGHBOR: return neighbor_words(w, sizeof w, navi, place_line);
	case LAB: return lab_words(w, sizeof w, navi, place_line);
	case GOSSIP: return gossip_words(w, sizeof w);
	default: return place_line;
	}
}
