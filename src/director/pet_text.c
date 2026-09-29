/* The PET's KeyItem and E-Mail screens with the run's own words
 * (docs/PET.md, docs/ROM_DATA.md). */
#include "pet_text.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "emu.h"
#include "flags.h"
#include "guardians.h"
#include "layer_objs.h"
#include "meta.h"
#include "rivals.h"
#include "rom.h"
#include "run.h"
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
	if (!n) snprintf(text, (size_t)size, "No guardian's code yet: delete one twice for his.");
	else if (n == 1) snprintf(text, (size_t)size, "%s's code. It opens the gate sealed with it.", guardian(first)->name);
	else if (n == 2) snprintf(text, (size_t)size, "%s's and %s's codes: each opens its gate.", guardian(first)->name, guardian(second)->name);
	else snprintf(text, (size_t)size, "%d codes: %s, %s and more.", n, guardian(first)->name, guardian(second)->name);
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
	snprintf(text, sizeof text, "The Library holds %d chips. Vaults here open at %d.", meta_library_count(-1), meta_vault_need(run.depth));
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
	if (items < 0x02000000u || items >= 0x02040000u || check < 0x02000000u || check >= 0x02040000u) return;
	emu_write8(items + (uint32_t)id, (uint8_t)count);
	if (count) emu_write8(check + (uint32_t)id, (uint8_t)(emu_read8(BN6_KEY_ITEM_SEEDS + (uint32_t)id) ^ 0x55));
}

/* ---- E-Mail: Dad's, one a guardian, his battle data as MegaMan logged it ---- */

/* a guardian's mail: its id is the navi's (BN6's own mails 1-18 are the
 * story's, which a run never delivers) */
static bool mail_of(int navi) { return navi >= 1 && navi <= 18 && navi != 17 && guardian_tip(navi); }

static int mail_body(int navi, uint8_t *out, int max) {
	static TextArchive t;
	char boxes[900];
	/* (the tip in MegaMan's words, as his warning says it: Dad forwards it) */
	snprintf(boxes, sizeof boxes, "@D Lan, I sorted out MegaMan's battle data on %s's copy. Here it is, as he logged it.|@M %s",
		guardian(navi)->name, guardian_tip(navi));
	ta_begin(&t);
	ta_script(&t);
	bool first = true;
	ta_pages(&t, boxes, FACE_DAD, &first);
	static const uint8_t end[] = { 0xEE, 0xFF, 0x00, 0x00 };
	ta_bytes(&t, end, sizeof end);
	if (t.full || t.len > max) return 0;
	memcpy(out, t.buf, (size_t)t.len);
	return t.len;
}

static void install_mails(void) {
	int slen = 0, blen = 0;
	uint8_t *senders = unpack(BN6_MAIL_TEXT, &slen), *bodies = unpack(BN6_MAIL_BODIES, &blen);
	if (!senders || !bodies) { free(senders); free(bodies); return; }
	int ns = u16at(senders, 0) / 2, nb = u16at(bodies, 0) / 2;
	static uint8_t sout[MAILS_MAX], bout[BODIES_MAX], from[19][16], about[19][24], body[19][1100];
	Script srep[512] = { 0 }, brep[256] = { 0 };
	if (ns > 512 || nb > 256 || ns < 2 * 19 || nb < 19) { free(senders); free(bodies); return; }
	static const uint8_t tail[] = { 0xE7, 0x00, 0xE6 };
	for (int m = 1; m <= 18; ++m) {
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

void pet_text_install(void) {
	install_names();
	install_descriptions();
	install_mails();
}

int pet_text_refresh(void) {
	install_descriptions();
	int codes = codes_known(NULL, 0);
	item_set(ITEM_CODES, codes > 99 ? 99 : codes);
	item_set(ITEM_PASS, meta_dark_way_open() ? 1 : 0);
	item_set(ITEM_LIBRARY, 1);
	int got = 0;
	for (int m = 1; m <= 18; ++m)
		if (mail_of(m) && guardian_known(m) && mail_deliver(m)) got = m;
	return got;
}
