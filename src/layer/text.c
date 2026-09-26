/* Text archive builder. Characters follow the game's charmap: space 0x00,
 * digits 0x01-0x0A, A-Z 0x0B-0x24, a-z 0x26-0x3F, punctuation as below;
 * 0xE9 breaks the line. */
#include "text.h"

#include <stdbool.h>
#include <string.h>

#include "mapslot.h"

static int code_of(char c) {
	if (c == ' ') return 0x00;
	if (c >= '0' && c <= '9') return 0x01 + (c - '0');
	if (c >= 'A' && c <= 'Z') return 0x0B + (c - 'A');
	if (c >= 'a' && c <= 'z') return 0x26 + (c - 'a');
	switch (c) {
	case '*': return 0x25; case '-': return 0x98; case '=': return 0x9A; case ':': return 0x9B;
	case '%': return 0x9C; case '?': return 0x9D; case '+': return 0x9E; case '!': return 0xA2;
	case '&': return 0xA3; case ',': return 0xA4; case '.': return 0xA6; case ';': return 0xA8;
	case '\'': return 0xA9; case '"': return 0xAA; case '~': return 0xAB; case '/': return 0xAC;
	case '(': return 0xAD; case ')': return 0xAE; case '>': return 0xB1; case '_': return 0xB2;
	case '\n': return 0xE9;
	default: return 0x00;
	}
}

int ta_encode(const char *s, uint8_t *out, int max) {
	int n = 0;
	for (; *s && n < max; ++s) out[n++] = (uint8_t)code_of(*s);
	return n;
}

void ta_begin(TextArchive *t) { t->len = 0; t->n = 0; t->full = false; }

int ta_script(TextArchive *t) {
	if (t->n >= TEXT_MAX_SCRIPTS) { t->full = true; return t->n - 1; }
	t->off[t->n] = (uint16_t)t->len;
	return t->n++;
}

void ta_bytes(TextArchive *t, const uint8_t *b, int n) {
	if (t->len + n > TEXT_MAX_BYTES) { t->full = true; return; }
	memcpy(t->buf + t->len, b, (size_t)n);
	t->len += n;
}

void ta_text(TextArchive *t, const char *s) {
	for (; *s; ++s) {
		uint8_t c = (uint8_t)code_of(*s);
		ta_bytes(t, &c, 1);
	}
}

void ta_open(TextArchive *t) { static const uint8_t b[] = { 0xE8, 0x00 }; ta_bytes(t, b, 2); }
void ta_wait(TextArchive *t) { static const uint8_t b[] = { 0xE7, 0x00 }; ta_bytes(t, b, 2); }
void ta_clear(TextArchive *t) { static const uint8_t b[] = { 0xF2 }; ta_bytes(t, b, 1); }
void ta_end(TextArchive *t) { static const uint8_t b[] = { 0xE6 }; ta_bytes(t, b, 1); }
void ta_mugshot(TextArchive *t, int m) { uint8_t b[] = { 0xF5, 0x00, (uint8_t)m }; ta_bytes(t, b, 3); }

/* Word wrap to the chat box: its text fits 22 characters a line (8 pixels
 * each, beside the face's place, which the box keeps with or without a
 * face); 20 leaves the page's arrow its room. Three lines a page. */
#define LINE_CHARS 20

#define MAX_LINES 24

/* Whether a line ends a sentence (a good place to turn the page). */
static bool sentence_end(const char *line) {
	size_t n = strlen(line);
	return n && (line[n - 1] == '.' || line[n - 1] == '!' || line[n - 1] == '?');
}

/* `s` word-wrapped into the open box. Pages hold three lines; a box that
 * does not fill its pages evenly spreads its lines over them (four lines
 * are two pages of two, never three and one), turning where a sentence
 * ends if it can. */
static void wrap(TextArchive *t, const char *s) {
	static char lines[MAX_LINES][LINE_CHARS + 1];
	int nl = 0, n = 0;
	const char *p = s;
	for (;;) {
		/* the next word */
		while (*p == ' ') ++p;
		const char *w = p;
		while (*p && *p != ' ' && *p != '\n') ++p;
		int wl = (int)(p - w);
		bool brk = *p == '\n' || !*p;
		if (wl && n + (n ? 1 : 0) + wl > LINE_CHARS && n && nl < MAX_LINES) { lines[nl++][n] = 0; n = 0; }
		if (wl && nl < MAX_LINES) {
			if (n) lines[nl][n++] = ' ';
			for (int k = 0; k < wl && n < LINE_CHARS; ++k) lines[nl][n++] = w[k];
		}
		if (brk && n && nl < MAX_LINES) { lines[nl++][n] = 0; n = 0; }
		if (!*p) break;
		if (*p == '\n') ++p;
	}
	/* the fewest pages, then the page breaks that cost least: a page of one
	 * line, a page turned mid-sentence */
	int pages = (nl + 2) / 3;
	enum { BIG = 1 << 20 };
	static int cost[MAX_LINES + 1][MAX_LINES / 3 + 2], from[MAX_LINES + 1][MAX_LINES / 3 + 2];
	for (int i = 0; i <= nl; ++i)
		for (int k = 0; k <= pages; ++k) cost[i][k] = BIG;
	cost[0][0] = 0;
	for (int k = 1; k <= pages; ++k)
		for (int i = 1; i <= nl; ++i)
			for (int take = 1; take <= 3 && take <= i; ++take) {
				int prev = cost[i - take][k - 1];
				if (prev >= BIG) continue;
				int c = prev + (take == 1 && nl > 1 ? 3 : 0) + (i < nl && !sentence_end(lines[i - 1]) ? 2 : 0);
				if (c < cost[i][k]) { cost[i][k] = c; from[i][k] = i - take; }
			}
	int ends[MAX_LINES / 3 + 2];
	for (int k = pages, i = nl; k > 0; i = from[i][k], --k) ends[k - 1] = i;
	for (int k = 0, i = 0; k < pages; ++k) {
		if (k) { ta_wait(t); ta_clear(t); }
		for (int first = i; i < ends[k]; ++i) {
			if (i > first) ta_text(t, "\n");
			ta_text(t, lines[i]);
		}
	}
}

void ta_page(TextArchive *t, int face, const char *s, bool first) {
	static const uint8_t hide[] = { 0xF5, 0x01 };
	if (face >= 0) ta_mugshot(t, face);
	else ta_bytes(t, hide, sizeof hide);
	if (first) ta_open(t); else ta_clear(t);
	wrap(t, s);
	ta_wait(t);
}

/* The face a box's speaker mark names, or `face`. */
static int speaker(const char **s, int face) {
	static const struct { char mark; int face; } marks[] = {
		{ 'L', FACE_LAN }, { 'M', FACE_MEGAMAN }, { 'D', FACE_DAD }, { 'P', FACE_PROG }, { 'B', FACE_BEAST },
		{ 'C', FACE_CHAUD }, { 'Y', FACE_MAYL }, { 'H', FACE_HEEL }, { 'N', FACE_NONE },
	};
	const char *p = *s;
	if (p[0] != '@') return face;
	for (unsigned i = 0; i < sizeof marks / sizeof *marks; ++i)
		if (p[1] == marks[i].mark) {
			p += 2;
			while (*p == ' ') ++p;
			*s = p;
			return marks[i].face;
		}
	return face;
}

void ta_pages(TextArchive *t, const char *boxes, int face, bool *first) {
	char box[200];
	int k = 0;
	for (const char *p = boxes;; ++p) {
		if (*p && *p != '|') { if (k < (int)sizeof box - 1) box[k++] = *p; continue; }
		box[k] = 0;
		k = 0;
		const char *b = box;
		int f = speaker(&b, face);
		ta_page(t, f, b, *first);
		*first = false;
		if (!*p) break;
	}
}

int ta_talk(TextArchive *t, const char *boxes, int face) {
	int i = ta_script(t);
	bool first = true;
	ta_pages(t, boxes, face, &first);
	ta_end(t);
	return i;
}

int ta_say(TextArchive *t, int face, const char *s) { return ta_talk(t, s, face); }

int ta_build(const TextArchive *t, uint8_t *out) {
	int head = t->n * 2;
	for (int i = 0; i < t->n; ++i) {
		uint16_t o = (uint16_t)(t->off[i] + head);
		out[i * 2] = (uint8_t)o;
		out[i * 2 + 1] = (uint8_t)(o >> 8);
	}
	memcpy(out + head, t->buf, (size_t)t->len);
	return head + t->len;
}

uint32_t ta_commit(TextArchive *t) {
	static uint8_t out[TEXT_ARCHIVE_MAX];
	return mapslot_alloc(out, ta_build(t, out));
}
