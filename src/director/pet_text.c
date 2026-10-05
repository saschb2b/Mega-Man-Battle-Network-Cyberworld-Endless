/* The PET's KeyItem and E-Mail screens with the run's own words
 * (docs/PET.md, docs/ROM_DATA.md). */
#include "pet_text.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "boss.h"
#include "director.h"
#include "emu.h"
#include "flags.h"
#include "guardians.h"
#include "layer_objs.h"
#include "meta.h"
#include "navicust.h"
#include "net.h"
#include "powers.h"
#include "rivals.h"
#include "rom.h"
#include "run.h"
#include "save.h"
#include "text.h"

#define NAMES_AT   (EMU_FREE + 0x154000)   /* the key items' names */
#define DESCS_AT   (EMU_FREE + 0x155000)   /* ... their descriptions */
#define MAILS_AT   (EMU_FREE + 0x158000)   /* the mails' senders and subjects */
#define BODIES_AT  (EMU_FREE + 0x15A000)   /* ... their bodies */
#define NAMES_MAX  0x1000
#define DESCS_MAX  0x3000
#define MAILS_MAX  0x2000
#define BODIES_MAX 0xC000

/* the run's own key items, in ids BN6 leaves nameless (0x47-0x4F) */
enum { ITEM_SCRT = 0x31, ITEM_CODES = 0x47, ITEM_PASS = 0x48, ITEM_LIBRARY = 0x49 };

typedef struct { const uint8_t *b; int len; } Script;

static int u16at(const uint8_t *p, int i) { return p[2 * i] | p[2 * i + 1] << 8; }

/* An archive of `n` scripts (u16 offsets from its start, then the scripts)
 * made from `src`, script i replaced where rep[i].len, the rest as they
 * were (those that shared an offset share one again: BN6's empty entries
 * share the next's). Its size, 0 where it would not fit. */
static int rebuild(const uint8_t *src, int src_len, int n, const Script *rep, uint8_t *out, int max) {
	int len = 2 * n;
	if (len > max) return 0;
	for (int i = 0; i < n; ++i) {
		int o = u16at(src, i), at = -1;
		const uint8_t *b = rep[i].b;
		int bl = rep[i].len;
		if (!bl) {
			for (int j = 0; j < i && at < 0; ++j)
				if (!rep[j].len && u16at(src, j) == o) at = u16at(out, j);
			int end = src_len;
			for (int j = 0; j < n; ++j) {
				int oj = u16at(src, j);
				if (oj > o && oj < end) end = oj;
			}
			b = src + o;
			bl = o < src_len ? end - o : 0;
		}
		if (at < 0) {
			if (len + bl > max || len > 0xFFFF) return 0;
			memcpy(out + len, b, (size_t)bl);
			at = len;
			len += bl;
		}
		out[2 * i] = (uint8_t)at;
		out[2 * i + 1] = (uint8_t)(at >> 8);
	}
	return len;
}

/* A compressed archive of the ROM's, unpacked: its scripts, past the four
 * bytes BN6's unpacking skips (decompAndCopyData). NULL where it is not. */
static uint8_t *unpack(uint32_t at, int *len) {
	uint32_t off = at - 0x08000000u;
	if (off + 4 > ROM_SIZE || R.data[off] != 0x10) return NULL;
	size_t n = 0;
	uint8_t *d = lz77_decompress(R.data + off, ROM_SIZE - off, &n);
	if (!d || n < 8) { free(d); return NULL; }
	memmove(d, d + 4, n - 4);
	*len = (int)n - 4;
	return d;
}

/* words in the game's charmap, ending with `tail` */
static int words(const char *s, const uint8_t *tail, int ntail, uint8_t *out, int max) {
	int k = ta_encode(s, out, max - ntail);
	memcpy(out + k, tail, (size_t)ntail);
	return k + ntail;
}

/* ---- KeyItem ---- */

/* A description: BN6's own opening (its box and font, from ScrtData's),
 * `s` in lines of twenty letters at most, three at most, the end. */
static int description(const uint8_t *vanilla_31, const char *s, uint8_t *out, int max) {
	int k = 16;
	if (max < 64) return 0;
	memcpy(out, vanilla_31, 16);
	char line[32];
	int lines = 0;
	const char *p = s;
	while (*p && lines < 3) {
		int n = (int)strlen(p) > 20 ? 20 : (int)strlen(p);
		if (p[n]) { int c = n; while (c > 0 && p[c] != ' ') --c; if (c > 0) n = c; }
		snprintf(line, sizeof line, "%.*s", n, p);
		if (lines) out[k++] = 0xE9;
		k += ta_encode(line, out + k, max - k - 4);
		p += n;
		while (*p == ' ') ++p;
		++lines;
	}
	out[k++] = 0xEE;
	out[k++] = 0xFF;
	return k;
}

/* The guardians whose code MegaMan has (deleted GATE_CODE times, in any
 * runs): how many, and into `text` (NULL: none) what the NaviCode says of
 * them, two by name at most (three lines of twenty letters: four names
 * pushed the end off the box). */
static int codes_known(char *text, int size) {
	static const uint8_t navis[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18 };
	int n = 0, first = 0, second = 0;
	for (unsigned i = 0; i < sizeof navis; ++i) {
		if (rival(navis[i])->megaman_won < GATE_CODE) continue;
		if (!n) first = navis[i];
		else if (n == 1) second = navis[i];
		++n;
	}
	if (!text) return n;
	if (!n) snprintf(text, (size_t)size, "No codes yet. Delete a guardian twice for his.");
	else if (n == 1) snprintf(text, (size_t)size, "%s's code. It opens the gate sealed with it.", guardian(first)->name);
	else if (n == 2) snprintf(text, (size_t)size, "%s's and %s's codes. Each opens its gate.", guardian(first)->name, guardian(second)->name);
	/* (all of them in Dad's Records mail: a playtester asked whose the
	 * other two of four were) */
	else snprintf(text, (size_t)size, "%d codes. Dad's Records mail lists them.", n);
	return n;
}

static void install_descriptions(void) {
	int len = 0;
	uint8_t *src = unpack(BN6_KEY_DESCS, &len);
	if (!src) return;
	int n = u16at(src, 0) / 2;
	static uint8_t out[DESCS_MAX], scrt[128], codes[160], pass[128], library[128];
	Script rep[256] = { 0 };
	if (n > 256 || ITEM_LIBRARY >= n) { free(src); return; }
	const uint8_t *v31 = src + u16at(src, ITEM_SCRT);
	char text[160];
	rep[ITEM_SCRT] = (Script){ scrt, description(v31, "Three of these open the golden gate to the Secret Area.", scrt, sizeof scrt) };
	codes_known(text, sizeof text);
	rep[ITEM_CODES] = (Script){ codes, description(v31, text, codes, sizeof codes) };
	rep[ITEM_PASS] = (Script){ pass, description(v31, "Clearing the Secret Area opened the dark way to the Undernet.", pass, sizeof pass) };
	/* (a collector's vault's need, on this layer: "vaults here" read as a
	 * place) */
	/* (what a vault holds: a playtester saw its count move and knew
	 * nothing else of it) */
	snprintf(text, sizeof text, "Library: %d chips. Vaults open at %d. Rare chips inside!", meta_library_count(-1), meta_vault_need(run.depth));
	rep[ITEM_LIBRARY] = (Script){ library, description(v31, text, library, sizeof library) };
	int size = rebuild(src, len, n, rep, out, sizeof out);
	free(src);
	if (!size) return;
	emu_write(DESCS_AT, out, (size_t)size);
	emu_write32(BN6_KEY_DESC_PTR, DESCS_AT);
}

static void install_names(void) {
	uint32_t src = BN6_KEY_NAMES - 0x08000000u;
	if (src + 0x400 > ROM_SIZE) return;
	const uint8_t *a = R.data + src;
	int n = u16at(a, 0) / 2, end = 0;
	if (n < ITEM_LIBRARY + 1 || n > 256) return;
	for (int i = 0; i < n; ++i) {
		int o = u16at(a, i), j = o;
		while (src + (uint32_t)j < ROM_SIZE && a[j] != 0xE6 && j < 0x2000) ++j;
		if (j + 1 > end) end = j + 1;
	}
	static const uint8_t e6[] = { 0xE6 };
	static uint8_t out[NAMES_MAX], codes[16], pass[16], library[16];
	Script rep[256] = { 0 };
	rep[ITEM_CODES] = (Script){ codes, words("NaviCode", e6, 1, codes, sizeof codes) };
	rep[ITEM_PASS] = (Script){ pass, words("DarkPass", e6, 1, pass, sizeof pass) };
	rep[ITEM_LIBRARY] = (Script){ library, words("LibCard", e6, 1, library, sizeof library) };
	int size = rebuild(a, end, n, rep, out, sizeof out);
	if (!size) return;
	emu_write(NAMES_AT, out, (size_t)size);
	/* (the KeyItem screen's, the text scripts', the shop's and SubChip's) */
	static const uint32_t ptrs[] = BN6_KEY_NAMES_PTRS;
	for (unsigned i = 0; i < sizeof ptrs / sizeof *ptrs; ++i)
		if (emu_read32(ptrs[i]) == BN6_KEY_NAMES) emu_write32(ptrs[i], NAMES_AT);
}

/* Counts for the run's own items, from the profile, with the check the
 * game's giving writes (a count without it reads as none: CheckKeyItem). */
static void item_set(int id, int count) {
	uint32_t items = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS), check = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_CHECK);
	if (items < BN6_EWRAM || items >= BN6_EWRAM_END || check < BN6_EWRAM || check >= BN6_EWRAM_END) return;
	emu_write8(items + (uint32_t)id, (uint8_t)count);
	if (count) emu_write8(check + (uint32_t)id, (uint8_t)(emu_read8(BN6_KEY_ITEM_SEEDS + (uint32_t)id) ^ 0x55));
}

/* ---- E-Mail: Dad's, one a guardian, his battle data as MegaMan logged it ---- */

/* a guardian's mail: its id is the navi's (BN6's own mails 1-18 are the
 * story's, which a run never delivers); then the lab's two */
static bool mail_of(int navi) { return navi >= 1 && navi <= 18 && navi != 17 && guardian_tip(navi); }
enum { MAIL_REPORT = 19, MAIL_RECORDS = 20, MAIL_CODES = 21, MAIL_BBS = 22 };

/* `s` into a mail's pages: lines of twenty letters at most, three a page,
 * a page's end waiting and clearing (E7 00 F2) as BN6's mails do, '\f'
 * turning the page early; the bytes written, or -1 where they would not
 * fit. */
static int mail_pages(const char *s, uint8_t *out, int max) {
	int k = 0, lines = 0;
	char line[32];
	const char *p = s;
	while (*p) {
		if (*p == '\f') { if (lines) lines = 3; ++p; continue; }
		int n = (int)strcspn(p, "\n\f");
		if (n > 20) n = 20;
		if (p[n] && p[n] != '\n' && p[n] != '\f') { int c = n; while (c > 0 && p[c] != ' ') --c; if (c > 0) n = c; }
		if (k + 40 > max) return -1;
		if (lines == 3) { out[k++] = 0xE7; out[k++] = 0x00; out[k++] = 0xF2; lines = 0; }
		else if (lines) out[k++] = 0xE9;
		snprintf(line, sizeof line, "%.*s", n, p);
		k += ta_encode(line, out + k, max - k - 8);
		p += n;
		while (*p == ' ' || *p == '\n') ++p;
		++lines;
	}
	return k;
}

/* Dad's words, a page or more, then the mail's end: the dive's report and
 * the records, made again on each layer */
static int dad_mail(const char *text, uint8_t *out, int max) {
	static const uint8_t dad[] = { 0xFC, 0x06, 0xF5, 0x00, FACE_DAD, 0xF5, 0x02, 0x01, 0xF5, 0x03, 0x00, 0xE8, 0x10 },
		end[] = { 0xE7, 0x00, 0xEE, 0xFF, 0x00, 0x00 };
	if (max < (int)(sizeof dad + sizeof end) + 8) return 0;
	memcpy(out, dad, sizeof dad);
	int k = (int)sizeof dad, n = mail_pages(text, out + k, max - k - (int)sizeof end);
	if (n < 0) return 0;
	k += n;
	memcpy(out + k, end, sizeof end);
	return k + (int)sizeof end;
}

/* The Cross the run brought, after its folder (" and HeatCross (fire
 * chips +50...; Aqua attacks do 2x to it).", or "."), with what it does
 * to a Navi where that differs */
static const char *cross_words(void) {
	static char buf[200];
	const char *weak = run.cross ? powers_cross_weakness(run.cross) : NULL;
	const char *strong = run.cross ? powers_cross_strength(run.cross) : NULL;
	const char *navis = run.cross ? powers_cross_on_navis(run.cross) : NULL;
	if (!run.cross) return ".";
	if (weak && strong)
		snprintf(buf, sizeof buf, " and %s.\fIn it,%c%s.\f%s%s%s attacks do double to it,and break it.", powers_cross_name(run.cross), strong[0] - 'A' + 'a', strong + 1,
			navis ? navis : "", navis ? ".\f" : "", weak);
	else if (weak) snprintf(buf, sizeof buf, " and %s.\f%s attacks do double to it,and break it.", powers_cross_name(run.cross), weak);
	else snprintf(buf, sizeof buf, " and %s.", powers_cross_name(run.cross));
	return buf;
}

/* The report's where: the layer and the act's area, or a trip back
 * (docs/HOME.md); the new length */
static int where_text(char *s, int k, int size, const char *area) {
	#define ADD(...) (k += snprintf(s + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	if (run.side_kind == LAYER_BACK) return ADD("You went back to %s.", area);
	if (run.mode == RUN_SHORT) ADD("You're on layer %d of %d.", run.depth, SHORT_LAYERS + (run.threat >= 10));
	else ADD("You're on layer %d.", run.depth);
	if (run.side_kind == LAYER_NORMAL && run.biome != BIOME_NEST) ADD(" Act %d is %s.", ((run.depth - 1) % CYCLE_LAYERS) / 3 + 1, area);
	else ADD(" You're in %s.", area);
	#undef ADD
	return k;
}

/* The dive as the lab sees it: where, what waits, what was brought (the
 * SciLab link that Comm had opened, as Dad's mail: BN6's own screen, its
 * own music) */
static void report_text(char *s, int size) {
	int k = 0;
	#define ADD(...) (k += snprintf(s + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	ADD("Lan,here's your dive as the lab sees it.\f");
	const char *area = guardian_area_name(run.side_kind == LAYER_UNDERNET ? BIOME_UNDERNET : run.side_kind == LAYER_SECRET ? BIOME_SECRET : run.biome);
	k = where_text(s, k, size, area);
	int navi = run.side_kind == LAYER_NORMAL ? run_guardian(run.biome) : 0;
	/* (made again as his battle ends: a playtester read "guards this
	 * act's end" after deleting the Nest's last guardian) */
	bool down = navi && is_boss_depth(run.depth) && boss_done();
	if (down && run.mode == RUN_SHORT && run_short_last(run.depth))
		ADD("\f%s is deleted! The whole Net has gone quiet.\fJack out and come home,Lan!", guardian(navi)->name);
	else if (down) ADD("\f%s is deleted. The way on is open!", guardian(navi)->name);
	else if (navi && guardian_known(navi)) ADD("\f%s guards this act's end. MegaMan knows him.", guardian(navi)->name);
	else if (navi && director_guardian_heard()) ADD("\fThe Net says %s guards this act.", guardian(navi)->name);
	else if (navi) ADD("\fA strong Navi guards this act's end. We don't know who yet.");
	ADD("\fYou've found %d of 3 ScrtData%s.", run.fragments > 3 ? 3 : run.fragments, run.secret_cleared ? ",and the gate's open" : "");
	if (run.clock) ADD("\fTrips back: %d. The Net kept copying: the guardians ahead are %d%% tougher.", run.clock, run.clock * RUN_CLOCK_PERCENT);
	ADD("\fYou brought the %s folder%s", meta_folder(run.folder)->name, cross_words());
	ADD("\fThe threat is %d.", run.threat);
	if (run.helpers & ((1 << HELPERS) - 1)) {
		ADD(" Your helpers are");
		for (int h = 0; h < HELPERS; ++h) if (run.helpers >> h & 1) ADD(" %s", meta_helper(h)->name);
		ADD(".");
	}
	#undef ADD
}

/* Every guardian met, in any run, with how their battles went, a line
 * each, and whose code MegaMan holds */
static void records_text(char *s, int size) {
	static const uint8_t navis[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18 };
	int k = 0, met = 0;
	#define ADD(...) (k += snprintf(s + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	ADD("Lan,MegaMan's battle records! Wins,then losses.\f");
	for (unsigned i = 0; i < sizeof navis; ++i) {
		const Rival *r = rival(navis[i]);
		if (!r->met && !r->megaman_won && !r->navi_won) continue;
		++met;
		ADD("%s %d-%d%s\n", guardian(navis[i])->name, r->megaman_won, r->navi_won, r->megaman_won >= GATE_CODE ? ",code" : "");
	}
	if (!met) ADD("No guardians met yet.\n");
	/* (what ", code" means, once one is held) */
	for (unsigned i = 0; i < sizeof navis; ++i)
		if (rival(navis[i])->megaman_won >= GATE_CODE) { ADD("\"code\" means we hold his NaviCode.\n"); break; }
	/* (the rival's duels: docs/RIVAL.md) */
	if (profile.duel_won + profile.duel_lost) ADD("ProtoMan duels %d-%d\n", profile.duel_won, profile.duel_lost);
	ADD("\f%d of %d guardians met. Your best dive reached layer %d", met, (int)sizeof navis, profile.best_depth);
	if (profile.nest_clears) ADD(",and you beat the Nest %d time%s", profile.nest_clears, profile.nest_clears == 1 ? "" : "s");
	ADD(".");
	#undef ADD
}

/* The compression codes entered in any run (issue #50), after how to enter
 * one, three a page: each program as the NaviCust lists it (SuprArmr, as
 * eight letters allow), its code five and five, a line of twenty */
static void codes_text(char *s, int size) {
	int k = 0;
	#define ADD(...) (k += snprintf(s + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	ADD("Lan,your compression codes,kept for every dive.\f"
		"In the NaviCust,hold RIGHT on a program. Then press its code.\f");
	for (int p = 1; p < NAVICUST_PROGRAMS; ++p) {
		const char *about = navicust_about(p);
		char code[12];
		if (!profile_code_entered(p) || !about || !navicust_code(p, code)) continue;
		int n = (int)strcspn(about, ":");
		if (n > 8) ADD("%-8s %s\n", p == 1 ? "SuprArmr" : "", code);
		else ADD("%-8.*s %s\n", n, about, code);
	}
	#undef ADD
}

/* The Endless Net BBS (docs/META.md, Rumors): netizens' threads, a post
 * a page or two, each with its poster's face (the townsfolk's for the
 * kids: a person's face is its sprite less 0x20, town.c), more as the
 * profile goes deeper; each true, or a joke that says so. */
typedef struct { int face; const char *who, *what; } Post;

static int bbs_posts(Post *out) {
	int n = 0;
	#define POST(f, w, t) (out[n++] = (Post){ (f), (w), (t) })
	POST(0x14, "DiveKid", "Jacked in at the statue again. The paths were all different. AGAIN!");
	POST(FACE_NAVI, "NetSurfer", "Every dive,kid. And it only goes down.");
	POST(0x0F, "GigaHunter", "The kids say there's a Giga chip under the bus in Central Town!");
	POST(FACE_HEEL, "BusStopper", "I checked. Gum wrapper.");
	POST(0x0F, "GigaHunter", "...They said it only shows at midnight.");
	POST(FACE_TECH, "PAfan", "Sword,WideSwrd,then LongSwrd. One code,in that order. Trust me!");
	POST(FACE_PROG, "Skeptic", "No way... LifeSrd! It's real!");
	POST(0x0B, "FamousFan", "Mr.Famous says he finished the Endless Net blindfolded! Wow!");
	POST(FACE_NAVI, "Realist", "It's endless. Nobody finishes it.");
	if (profile.best_depth >= 3) POST(FACE_HEEL, "GuardianWatch", "The guardians down there fight like the real ones. Learn their moves!");
	if (profile.best_depth >= 5) {
		POST(0x0E, "Wanderer", "I swear I walked on nothing past a walkway. Some floor can't be seen!");
		POST(0x16, "SeasideKid", "Look for a lonely pad out in the void.");
	}
	if (profile.spins) POST(FACE_TECH, "SpinCollector", "Found a Spin in a blue Mystery Data. Now my programs turn!");
	if (profile.duel_won) POST(0x19, "ChaudFan", "Beat ProtoMan's time,and Chaud shows you TagChips. Worth it!");
	if (profile.best_depth >= 10 || profile.nest_clears) POST(FACE_NAVI, "DeepDiver", "I reached the Nest. Something down there copies everything... Spooky.");
	#undef POST
	return n;
}

/* The BBS's mail: each post in its poster's face, the box cleared between
 * them (as a guardian's mail turns to MegaMan's) */
static int bbs_body(uint8_t *out, int max) {
	static Post post[24];
	static const uint8_t end[] = { 0xE7, 0x00, 0xEE, 0xFF, 0x00, 0x00 };
	int n = bbs_posts(post), k = 0;
	char s[200];
	for (int i = 0; i < n; ++i) {
		const uint8_t open[] = { 0xFC, 0x06, 0xF5, 0x00, (uint8_t)post[i].face, 0xF5, 0x02, 0x01, 0xF5, 0x03, 0x00, 0xE8, 0x10 };
		const uint8_t turn[] = { 0xE7, 0x00, 0xF2, 0xF5, 0x00, (uint8_t)post[i].face };
		const uint8_t *head = i ? turn : open;
		int hn = i ? (int)sizeof turn : (int)sizeof open;
		if (k + hn + (int)sizeof end + 8 > max) return 0;
		memcpy(out + k, head, (size_t)hn);
		k += hn;
		snprintf(s, sizeof s, "%s:\n%s", post[i].who, post[i].what);
		int w = mail_pages(s, out + k, max - k - (int)sizeof end);
		if (w < 0) return 0;
		k += w;
	}
	memcpy(out + k, end, sizeof end);
	return k + (int)sizeof end;
}

/* A guardian's mail: Dad's face and words, then MegaMan's, the warning as
 * he logged it (the briefing's: the tip's boxes, "|@M " apart), in BN6's
 * own mail form (a face set up with FC 06 and F5 02/03, the box opened
 * with E8 10; the map chat's F5/E8 00 drew the mail as a chat box, its
 * banner garbled). */
static int mail_body(int navi, uint8_t *out, int max) {
	static const uint8_t dad[] = { 0xFC, 0x06, 0xF5, 0x00, FACE_DAD, 0xF5, 0x02, 0x01, 0xF5, 0x03, 0x00, 0xE8, 0x10 };
	static const uint8_t turn[] = { 0xE7, 0x00, 0xF2, 0xF5, 0x00, FACE_MEGAMAN }, next[] = { 0xE7, 0x00, 0xF2 },
		end[] = { 0xE7, 0x00, 0xEE, 0xFF, 0x00, 0x00 };
	char s[400];
	int k = 0, n;
	#define PUT(b) do { if (k + (int)sizeof b > max) return 0; memcpy(out + k, b, sizeof b); k += (int)sizeof b; } while (0)
	PUT(dad);
	snprintf(s, sizeof s, "Lan,I sorted out MegaMan's data on %s's copy.\fHere it is,just as he logged it.", guardian(navi)->name);
	if ((n = mail_pages(s, out + k, max - k)) < 0) return 0;
	k += n;
	PUT(turn);
	const char *tip = guardian_tip(navi);
	for (bool first = true; tip && *tip; first = false) {
		const char *bar = strstr(tip, "|@M ");
		int len = bar ? (int)(bar - tip) : (int)strlen(tip);
		snprintf(s, sizeof s, "%.*s", len, tip);
		if (!first) PUT(next);
		if ((n = mail_pages(s, out + k, max - k)) < 0) return 0;
		k += n;
		tip = bar ? bar + 4 : NULL;
	}
	PUT(end);
	#undef PUT
	return k;
}

static void install_mails(void) {
	int slen = 0, blen = 0;
	uint8_t *senders = unpack(BN6_MAIL_TEXT, &slen), *bodies = unpack(BN6_MAIL_BODIES, &blen);
	if (!senders || !bodies) { free(senders); free(bodies); return; }
	int ns = u16at(senders, 0) / 2, nb = u16at(bodies, 0) / 2;
	static uint8_t sout[MAILS_MAX], bout[BODIES_MAX], from[MAIL_BBS + 1][16], about[MAIL_BBS + 1][24], body[MAIL_BBS + 1][2000];
	Script srep[512] = { 0 }, brep[256] = { 0 };
	if (ns > 512 || nb > 256 || ns < 2 * (MAIL_BBS + 1) || nb < MAIL_BBS + 1) { free(senders); free(bodies); return; }
	static const uint8_t tail[] = { 0xE7, 0x00, 0xE6 };
	for (int m = 1; m <= MAIL_BBS; ++m) {
		if (m >= MAIL_REPORT) {
			static char text[2000];
			static const char *const subjects[] = { "Dive report", "Records", "Compression", "Endless Net" };
			if (m == MAIL_REPORT) report_text(text, sizeof text);
			else if (m == MAIL_RECORDS) records_text(text, sizeof text);
			else codes_text(text, sizeof text);
			srep[2 * m] = (Script){ from[m], words(m == MAIL_BBS ? "NetBBS" : "Dad", tail, 3, from[m], sizeof from[m]) };
			srep[2 * m + 1] = (Script){ about[m], words(subjects[m - MAIL_REPORT], tail, 3, about[m], sizeof about[m]) };
			brep[m] = (Script){ body[m], m == MAIL_BBS ? bbs_body(body[m], sizeof body[m]) : dad_mail(text, body[m], sizeof body[m]) };
			uint8_t row[4] = { 0x04, emu_read8(BN6_MAIL_TABLE + 4u * (uint32_t)m + 1), 0x08, (uint8_t)(0x20 + m) };
			emu_write(BN6_MAIL_TABLE + 4u * (uint32_t)m, row, sizeof row);
			continue;
		}
		if (!mail_of(m)) continue;
		srep[2 * m] = (Script){ from[m], words("Dad", tail, 3, from[m], sizeof from[m]) };
		srep[2 * m + 1] = (Script){ about[m], words(guardian(m)->name, tail, 3, about[m], sizeof about[m]) };
		brep[m] = (Script){ body[m], mail_body(m, body[m], sizeof body[m]) };
		/* its row: a plain mail (bit 7 would file it in Lan's HP), the
		 * sender's and the subject's places in the sorts */
		uint8_t row[4] = { 0x04, emu_read8(BN6_MAIL_TABLE + 4u * (uint32_t)m + 1), 0x08, (uint8_t)(0x40 + m) };
		emu_write(BN6_MAIL_TABLE + 4u * (uint32_t)m, row, sizeof row);
	}
	int ssize = rebuild(senders, slen, ns, srep, sout, sizeof sout), bsize = rebuild(bodies, blen, nb, brep, bout, sizeof bout);
	free(senders);
	free(bodies);
	if (!ssize || !bsize) return;
	emu_write(MAILS_AT, sout, (size_t)ssize);
	emu_write(BODIES_AT, bout, (size_t)bsize);
	if (emu_read32(BN6_MAIL_TEXT_PTR) == BN6_MAIL_TEXT_BUF) emu_write32(BN6_MAIL_TEXT_PTR, MAILS_AT);
	if (emu_read32(BN6_MAIL_BODY_PTR) == BN6_MAIL_BODY_BUF) emu_write32(BN6_MAIL_BODY_PTR, BODIES_AT);
}

/* The game's own giving of mail `m` (bn6f addMail_802f238), done here:
 * received and unread, first in the list of 128, the count one more. */
static bool mail_deliver(int m) {
	if (flag_get(BN6_FLAG_MAIL_GOT + m)) return false;
	flag_set(BN6_FLAG_MAIL_GOT + m);
	flag_set(BN6_FLAG_MAIL_NEW + m);
	flag_clear(BN6_FLAG_MAIL_READ + m);
	uint8_t list[128];
	for (int i = 0; i < 128; ++i) list[i] = emu_read8(BN6_MAIL_LIST + (uint32_t)i);
	memmove(list + 1, list, sizeof list - 1);
	list[0] = (uint8_t)m;
	emu_write(BN6_MAIL_LIST, list, sizeof list);
	emu_write32(BN6_MAIL_COUNT, emu_read32(BN6_MAIL_COUNT) + 1);
	return true;
}

/* Mail `m` first in the list (the lab's, as each layer makes them again):
 * taken out where it stands and put back at the head, the count as it was */
static void mail_first(int m) {
	uint8_t list[128];
	int n = 0;
	for (int i = 0; i < 128; ++i) {
		int id = emu_read8(BN6_MAIL_LIST + (uint32_t)i);
		if (id != m) list[n++] = (uint8_t)id;
	}
	if (n == 128) return;
	memmove(list + 1, list, (size_t)n);
	list[0] = (uint8_t)m;
	for (int i = n + 1; i < 128; ++i) list[i] = 0;
	emu_write(BN6_MAIL_LIST, list, sizeof list);
}

void pet_text_install(void) {
	install_names();
	install_descriptions();
	install_mails();
}

int pet_text_refresh(void) {
	install_descriptions();
	install_mails();
	int codes = codes_known(NULL, 0);
	item_set(ITEM_CODES, codes > 99 ? 99 : codes);
	item_set(ITEM_PASS, meta_dark_way_open() ? 1 : 0);
	item_set(ITEM_LIBRARY, 1);
	int got = 0;
	for (int m = 1; m <= 18; ++m)
		if (mail_of(m) && guardian_known(m) && mail_deliver(m)) got = m;
	/* (the lab's two at the head of the list, NEW only with news: the
	 * report on an act's first layer, a new guardian ahead; the records
	 * once a battle has changed them. A playtester never saw either NEW,
	 * so never knew when to read them) */
	static const uint8_t navis[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18 };
	uint32_t mark = 1;
	for (unsigned i = 0; i < sizeof navis; ++i) {
		const Rival *r = rival(navis[i]);
		mark = mark * 31u + (uint32_t)(r->met * 7 + r->megaman_won * 101 + r->navi_won * 1009);
	}
	bool records_news = mark != profile.records_mark,
		report_news = run.side_kind == LAYER_NORMAL && layer_in_act(run.depth) == 0;
	if (records_news) { profile.records_mark = mark; profile_save(); }
	/* (and the codes', third, from the first code entered: NEW with each
	 * new one, issue #50) */
	int entered = profile_codes_entered();
	bool codes_news = entered > profile.codes_mailed;
	if (codes_news) { profile.codes_mailed = (uint8_t)entered; profile_save(); }
	/* (and the BBS's, last, NEW with each new post) */
	static Post post[24];
	int posts = bbs_posts(post);
	bool bbs_news = posts > profile.bbs_seen;
	if (bbs_news) { profile.bbs_seen = (uint8_t)posts; profile_save(); }
	for (int m = MAIL_BBS; m >= MAIL_REPORT; --m) {
		if (m == MAIL_CODES && !entered) continue;
		bool news = m == MAIL_REPORT ? report_news : m == MAIL_RECORDS ? records_news : m == MAIL_CODES ? codes_news : bbs_news;
		mail_deliver(m);
		if (news) { flag_set(BN6_FLAG_MAIL_NEW + m); flag_clear(BN6_FLAG_MAIL_READ + m); }
		mail_first(m);
	}
	return got;
}
