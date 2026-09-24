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

void ta_begin(TextArchive *t) { t->len = 0; t->n = 0; }

int ta_script(TextArchive *t) {
	if (t->n >= TEXT_MAX_SCRIPTS) return t->n - 1;
	t->off[t->n] = (uint16_t)t->len;
	return t->n++;
}

void ta_bytes(TextArchive *t, const uint8_t *b, int n) {
	if (t->len + n > TEXT_MAX_BYTES) return;
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

/* Word wrap to the chat box: 16 characters a line, three lines a page. */
#define LINE_CHARS 16

int ta_say(TextArchive *t, int mugshot, const char *s) {
	int i = ta_script(t);
	if (mugshot >= 0) ta_mugshot(t, mugshot);
	ta_open(t);
	int lines = 0;
	char line[LINE_CHARS + 1];
	int n = 0;
	const char *p = s;
	for (;;) {
		/* the next word */
		while (*p == ' ') ++p;
		const char *w = p;
		while (*p && *p != ' ' && *p != '\n') ++p;
		int wl = (int)(p - w);
		bool brk = *p == '\n' || !*p;
		if (wl && n + (n ? 1 : 0) + wl > LINE_CHARS && n) {
			if (lines == 3) { ta_wait(t); ta_clear(t); lines = 0; }
			if (lines) ta_text(t, "\n");
			line[n] = 0; ta_text(t, line); ++lines; n = 0;
		}
		if (wl) {
			if (n) line[n++] = ' ';
			for (int k = 0; k < wl && n < LINE_CHARS; ++k) line[n++] = w[k];
		}
		if (brk && n) {
			if (lines == 3) { ta_wait(t); ta_clear(t); lines = 0; }
			if (lines) ta_text(t, "\n");
			line[n] = 0; ta_text(t, line); ++lines; n = 0;
		}
		if (!*p) break;
		if (*p == '\n') ++p;
	}
	ta_wait(t);
	ta_end(t);
	return i;
}

uint32_t ta_commit(TextArchive *t) {
	static uint8_t out[TEXT_MAX_SCRIPTS * 2 + TEXT_MAX_BYTES];
	int head = t->n * 2;
	for (int i = 0; i < t->n; ++i) {
		uint16_t o = (uint16_t)(t->off[i] + head);
		out[i * 2] = (uint8_t)o;
		out[i * 2 + 1] = (uint8_t)(o >> 8);
	}
	memcpy(out + head, t->buf, (size_t)t->len);
	return mapslot_alloc(out, head + t->len);
}
