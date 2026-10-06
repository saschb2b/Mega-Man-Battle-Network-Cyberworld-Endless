/* home_words.h. Coming home after an act: MegaMan's relief and Lan's
 * answer, one beat a box (docs/VOICE.md), by how far the run has come;
 * Lan's HP's portals and what MegaMan says of them. */
#include "home_words.h"

#include <stdio.h>
#include <string.h>

#include "guardians.h"
#include "jobs.h"
#include "net.h"
#include "run.h"

const char *home_words(const char *beaten, int ways, const char *const ports[3], const char *sealed) {
	static char words[400];
	const char *who = beaten ? beaten : "The guardian";
	int act = (run.depth - 1) % CYCLE_LAYERS / 3;   /* (acts cleared in this cycle, the Nest's 0) */
	if (act == 0)
		snprintf(words, sizeof words, "@M Lan... The Net's rebuilding.|@L Then we dive again! Ready,MegaMan?");
	else if (run_short_nest(run.depth))
		snprintf(words, sizeof words, "@M Phew... %s's deleted,Lan.|@M The Nest is next.|@L Then we get ready. Together!", who);
	else if (act == 1)
		snprintf(words, sizeof words, "@M Phew! We're home,Lan!|@M %s's deleted!|@L Nice one,MegaMan! Let's catch our breath.", who);
	else if (act == 2)
		snprintf(words, sizeof words, "@M Whew... %s was tough.|@L But we won! Home sweet home!", who);
	else
		snprintf(words, sizeof words, "@M Home again,Lan!|@L Alright! Let's get set for the next dive!");
	/* (the portals and where they are: docs/HOME.md) */
	size_t k = strlen(words);
	if (ways == 2)
		snprintf(words + k, sizeof words - k, "|@M Two ways are open this time!|@M The %s,and the %s!", ports[0], ports[1]);
	else if (ways == 3)
		snprintf(words + k, sizeof words - k, "|@M Three ways are open!|@M The %s,the %s...|@M And something dark at the %s.", ports[0],
			ports[1], ports[2]);
	k = strlen(words);
	if (sealed) snprintf(words + k, sizeof words - k, "|@M Something dark waits at the %s too...", sealed);
	return words;
}

/* What MegaMan reads of an area through a link he has not taken: its data,
 * never its name (docs/META.md, what MegaMan knows) */
static const char *area_feel(int biome) {
	static const struct { int biome; const char *line; } feels[] = {
		{ BIOME_CENTRAL, "@M Busy data over there,Lan." },
		{ BIOME_SEASIDE, "@M Whoa,salty data! Like the sea!" },
		{ BIOME_SKY, "@M The data's so light... Like wind!" },
		{ BIOME_GREEN, "@M Wild data,growing everywhere!" },
		{ BIOME_GRAVEYARD, "@M Brr... Cold,deleted data." },
		{ BIOME_UNDERNET, "@M Lan... Dark,rough data. Careful." },
		{ BIOME_SECRET, "@M Something really strong is over there..." },
		{ BIOME_NEST, "@M Lan... This one goes way down.|@M It has to be the Nest!" },
		{ BIOME_COMP, "@M Circuit data. A comp,maybe?" },
		{ BIOME_HOMEPAGE, "@M Friendly data... Someone's homepage?" },
		{ BIOME_COMP_B, "@M Hmm... Humming machines over there." },
		{ BIOME_ROBOT_COMP, "@M Machine data... Robots,maybe?" },
		{ BIOME_AQUARIUM_COMP, "@M Water data... And fish!?" },
		{ BIOME_JUDGE_COMP, "@M Hmm... Old data,deep as roots." },
		{ BIOME_WEATHER_COMP, "@M Brr! Snowy,cloudy data..." },
		{ BIOME_COPYBOT_COMP, "@M Huh? The same data,over and over..." },
	};
	if (biome >= 0 && biome < BIOME_COUNT && guardian_area_older(biome)) return "@M Whoa... The data over there is old!";
	for (size_t i = 0; i < sizeof feels / sizeof *feels; ++i)
		if (feels[i].biome == biome) return feels[i].line;
	/* (the towns' homepages, and what no line reads) */
	return biome >= BIOME_ACDC_HP && biome <= BIOME_SKY_HP ? "@M Cozy data... A town's homepage?" : "@M Hmm... I can't read much from here.";
}

const char *home_port_words(int biome, int navi, bool dark, bool sealed) {
	static char words[240];
	/* (the Cybeast below the endless Nest: a growl, never a Navi's signal;
	 * docs/BOSSES.md, Super bosses) */
	bool beast = navi == SUPER_CYBEAST, known = navi && guardian_known(navi);
	if (!known) navi = 0;
	char signal[64];
	if (beast) snprintf(signal, sizeof signal, "%s", known ? "@M The Cybeast's down there,Lan. Waiting." : "@M Something's growling down there...|@M Like the beast in me.");
	else if (navi) snprintf(signal, sizeof signal, "@M That signal... It's %s!", guardian(navi)->name);
	else snprintf(signal, sizeof signal, "@M And a strong Navi's signal...");
	if (sealed)
		snprintf(words, sizeof words, "@M A dark way leads down here...|@M It's sealed. Clearing the Secret Area would open it.");
	else if (dark)
		snprintf(words, sizeof words, "@M Lan... This one's dark. Really dark.|@M It feels like the Undernet...|%s", signal);
	else
		snprintf(words, sizeof words, "%s|%s", area_feel(biome), signal);
	return words;
}

const char *home_portal_name(int k) {
	static const char *const names[] = { "pink pad", "link up top", "link on the right", "link on the left", "link down front" };
	return k >= 0 && k < (int)(sizeof names / sizeof *names) ? names[k] : "link";
}

/* how far the Net's clock has run, in MegaMan's words */
static const char *clock_feel(int clock) {
	return clock >= 4 ? "|@M Lan... The Net's copying fast now." : clock >= 2 ? "|@M The Net's been copying a while..." :
		clock == 1 ? "|@M The Net's had a little time to copy." : "";
}

const char *home_hp_status(const char *way, bool first, int clock) {
	static char words[200];
	if (first) snprintf(words, sizeof words, "@M The pink pad's %s,Lan!|@M Or press R,and I'll jack out.%s", way, clock_feel(clock));
	else snprintf(words, sizeof words, "@M The pink pad's %s,Lan!%s", way, clock_feel(clock));
	return words;
}

const char *home_back_portal_words(int biome, bool taught, int clock) {
	static char words[240];
	const char *area = guardian_area_in_text(biome, LAYER_NORMAL);
	if (!taught)
		snprintf(words, sizeof words, "@M This link leads back to %s!|@M We could stock up there,Lan.|@M But the Net's clock keeps running...|"
			"@M Each trip back,the guardians ahead get tougher!", area);
	else if (clock)
		snprintf(words, sizeof words, "@M This one goes back to %s.|@M Another notch on the Net's clock,though...", area);
	else
		snprintf(words, sizeof words, "@M This one goes back to %s.|@M It'd cost a notch on the Net's clock.", area);
	return words;
}

const char *home_back_words(int clock) {
	if (clock >= 4) return "@M Home again,Lan...|@M The Net's copying fast now.|@L Then no more stalling! Let's dive!";
	return "@M Home again,Lan!|@M The Net kept copying while we were away...|@L Then we'd better get moving!";
}

const char *home_errands_words(const char *words, bool requests, bool order, const char *aster) {
	static char buf[560];
	/* (a playtester heard only the way home in town, and found the
	 * request board by chance, session 69; then walked round AsterLand
	 * twice to its door, session 70) */
	int k = snprintf(buf, sizeof buf, "%s%s%s", words, requests ? "|@M Folks posted requests,Lan!|@M AsterLand's board,the club at school...|"
		"@M And the man from Dad's lab,out here!" : "", order ? "|@M AsterLand can order us a chip,too!" : "");
	if ((requests || order) && aster && k > 0 && (size_t)k < sizeof buf) snprintf(buf + k, sizeof buf - (size_t)k, "|@M AsterLand's door is %s!", aster);
	return buf;
}

const char *home_aster_words(const char *words) {
	static char buf[320];
	/* (a playtester never found the Order Service's clerk on two visits,
	 * and L named only the way out, session 70) */
	snprintf(buf, sizeof buf, "@M Chips to order at the counter's right,Lan!|@M SubChips at its left!|%s", words);
	return buf;
}

const char *home_door_words(const char *words, const char *door) {
	static char buf[320];
	/* (seven calls looking for it, session 69) */
	snprintf(buf, sizeof buf, "%s|@M The front door's %s,if we head out!", words, door);
	return buf;
}

const char *home_hp_words(bool taught) {
	if (taught) return "@M Our HP,Lan! The pink pad's waiting!";
	return "@M Our HP,Lan! Home sweet home!|@M The Endless Net's linked in here now...|@M The pink pad leads into it!";
}

/* "A,B AND C" of `n` names into `w` from `k`; the new length */
static size_t name_list(char *w, size_t size, size_t k, const char *const *names, int n) {
	for (int i = 0; i < n && k < size; ++i)
		k += (size_t)snprintf(w + k, size - k, "%s%s", !i ? "" : i == n - 1 ? " AND " : ",", names[i]);
	return k < size ? k : size - 1;
}

const char *home_courier_words(int reward, unsigned posted, int unlockers, bool rush_food, bool www_id) {
	/* (a homepage's helper relaying the town's posts and AsterLand's stock:
	 * where each waits, never what the Net ahead holds) */
	static const char *const paid_by[JOB_ASKERS] = { "THE NETBATTLER,\nASTERLAND!", "THE NETBATTLE CLUB,\nTHE ACADEMY!",
		"THE MAN FROM THE LAB\nOUT IN TOWN!" };
	static const char *const places[JOB_ASKERS] = { "ASTERLAND", "THE ACADEMY", "TOWN" };
	static char w[320];
	const char *names[JOB_ASKERS];
	size_t k = 0;
	int n = 0;
	w[0] = 0;
	if (reward >= 0 && reward < JOB_ASKERS) k = (size_t)snprintf(w, sizeof w, "REWARD WAITING!\n%s", paid_by[reward]);
	else if (posted) {
		for (int a = 0; a < JOB_ASKERS; ++a) if (posted >> a & 1) names[n++] = places[a];
		k = (size_t)snprintf(w, sizeof w, "%s", n > 1 ? "NEW REQUESTS POSTED!\n" : "NEW REQUEST POSTED!\nAT ");
		k = name_list(w, sizeof w, k, names, n);
		k += (size_t)snprintf(w + k, sizeof w - k, "!");
	}
	n = 0;
	if (unlockers) names[n++] = unlockers > 1 ? "UNLOCKERS" : "AN UNLOCKER";
	if (rush_food) names[n++] = "RUSHFOOD";
	if (www_id) names[n++] = "A WWW-ID";
	if (!n || k + 2 >= sizeof w) return w;
	k += (size_t)snprintf(w + k, sizeof w - k, "%sASTERLAND HAS ", k ? "|" : "");
	k = name_list(w, sizeof w, k, names, n);
	snprintf(w + k, sizeof w - k, "!");
	return w;
}
