/* qr.h, the steps of web/assets/qr.js in C: the bits, the blocks with
 * their Reed-Solomon codewords, the fixed patterns, the codewords placed,
 * then the mask with the lowest penalty. */
#include "qr.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum { ECL_L, ECL_M };

/* per version 1-10: error correction codewords per block and blocks, for L and M */
static const uint8_t ECC[2][10] = { { 7, 10, 15, 20, 26, 18, 20, 24, 30, 18 }, { 10, 16, 26, 18, 24, 16, 18, 22, 22, 26 } };
static const uint8_t BLOCKS[2][10] = { { 1, 1, 1, 1, 1, 2, 2, 2, 2, 4 }, { 1, 1, 1, 2, 2, 4, 4, 4, 5, 5 } };
static const int FORMAT[2] = { 1, 0 };

/* the longest a version up to 10 has: codewords, and a block's */
#define MAX_WORDS 346
#define MAX_BLOCK 160

/* GF(256) over x^8 + x^4 + x^3 + x^2 + 1, as the standard's Reed-Solomon code */
static uint8_t EXP[512], LOG[256];

static void gf_init(void) {
	if (EXP[0]) return;
	for (int i = 0, x = 1; i < 255; ++i) {
		EXP[i] = (uint8_t)x;
		LOG[x] = (uint8_t)i;
		x <<= 1;
		if (x & 0x100) x ^= 0x11d;
	}
	for (int i = 255; i < 512; ++i) EXP[i] = EXP[i - 255];
}

static uint8_t mul(uint8_t a, uint8_t b) { return a && b ? EXP[LOG[a] + LOG[b]] : 0; }

/* the generator polynomial of `degree` (its leading 1 left out) */
static void divisor(int degree, uint8_t *d) {
	memset(d, 0, (size_t)degree);
	d[degree - 1] = 1;
	uint8_t root = 1;
	for (int i = 0; i < degree; ++i, root = mul(root, 2))
		for (int j = 0; j < degree; ++j) d[j] = (uint8_t)(mul(d[j], root) ^ (j + 1 < degree ? d[j + 1] : 0));
}

static void remainder_of(const uint8_t *data, int n, const uint8_t *d, int degree, uint8_t *r) {
	memset(r, 0, (size_t)degree);
	for (int k = 0; k < n; ++k) {
		uint8_t f = data[k] ^ r[0];
		memmove(r, r + 1, (size_t)degree - 1);
		r[degree - 1] = 0;
		for (int i = 0; i < degree; ++i) r[i] ^= mul(d[i], f);
	}
}

/* the modules a version leaves for data and error correction */
static int raw_modules(int v) {
	int n = (16 * v + 128) * v + 64;
	if (v >= 2) {
		int align = v / 7 + 2;
		n -= (25 * align - 10) * align - 55;
		if (v >= 7) n -= 36;
	}
	return n;
}

static int data_codewords(int v, int ecl) { return (raw_modules(v) >> 3) - ECC[ecl][v - 1] * BLOCKS[ecl][v - 1]; }

/* the alignment patterns' rows and columns: how many */
static int align_positions(int v, int *at) {
	if (v == 1) return 0;
	int n = v / 7 + 2, size = v * 4 + 17;
	int step = (v * 8 + n * 3 + 5) / (n * 4 - 4) * 2;
	at[0] = 6;
	for (int i = n - 1, p = size - 7; i >= 1; --i, p -= step) at[i] = p;
	return n;
}

static void put(uint8_t *buf, int *n, unsigned value, int bits) {
	for (int i = bits - 1; i >= 0; --i, ++*n)
		if (value >> i & 1) buf[*n >> 3] |= (uint8_t)(0x80 >> (*n & 7));
}

/* The codewords in their order: byte mode, the count, the bytes, a
 * terminator, padding; in blocks, each with its error correction, then
 * interleaved. How many. */
static int codewords(const char *text, int len, int v, int ecl, uint8_t *words) {
	uint8_t data[MAX_WORDS] = { 0 };
	int capacity = data_codewords(v, ecl) * 8, n = 0;
	put(data, &n, 4, 4);
	put(data, &n, (unsigned)len, v < 10 ? 8 : 16);
	for (int i = 0; i < len; ++i) put(data, &n, (uint8_t)text[i], 8);
	put(data, &n, 0, capacity - n < 4 ? capacity - n : 4);
	put(data, &n, 0, (8 - n % 8) % 8);
	for (unsigned pad = 0xec; n < capacity; pad ^= 0xec ^ 0x11) put(data, &n, pad, 8);

	int nblocks = BLOCKS[ecl][v - 1], ec_len = ECC[ecl][v - 1], raw = raw_modules(v) >> 3;
	int nshort = nblocks - raw % nblocks, short_len = raw / nblocks;
	uint8_t d[32], blocks[5][MAX_BLOCK] = { { 0 } };
	divisor(ec_len, d);
	for (int i = 0, k = 0; i < nblocks; ++i) {
		int dl = short_len - ec_len + (i < nshort ? 0 : 1);
		memcpy(blocks[i], data + k, (size_t)dl);
		k += dl;
		uint8_t ecc[32];
		remainder_of(blocks[i], dl, d, ec_len, ecc);
		if (i < nshort) blocks[i][dl++] = 0;
		memcpy(blocks[i] + dl, ecc, (size_t)ec_len);
	}
	int nw = 0;
	for (int i = 0; i < short_len + 1; ++i)
		for (int j = 0; j < nblocks; ++j)
			if (i != short_len - ec_len || j >= nshort) words[nw++] = blocks[j][i];
	return nw;
}

typedef struct {
	int size, ecl;
	uint8_t dark[QR_MAX][QR_MAX], fixed[QR_MAX][QR_MAX];
} Code;

static void set(Code *q, int x, int y, bool on) {
	q->dark[y][x] = on;
	q->fixed[y][x] = 1;
}

static int max_abs(int a, int b) { return abs(a) > abs(b) ? abs(a) : abs(b); }

/* the timing lines, the three finders and the alignment patterns */
static void patterns(Code *q, int v) {
	int size = q->size;
	for (int i = 0; i < size; ++i) {
		set(q, 6, i, i % 2 == 0);
		set(q, i, 6, i % 2 == 0);
	}
	const int centres[3][2] = { { 3, 3 }, { size - 4, 3 }, { 3, size - 4 } };
	for (int c = 0; c < 3; ++c)
		for (int dy = -4; dy <= 4; ++dy)
			for (int dx = -4; dx <= 4; ++dx) {
				int x = centres[c][0] + dx, y = centres[c][1] + dy, r = max_abs(dx, dy);
				if (x >= 0 && x < size && y >= 0 && y < size) set(q, x, y, r != 2 && r != 4);
			}
	int at[8], n = align_positions(v, at);
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j) {
			if ((i == 0 && j == 0) || (i == 0 && j == n - 1) || (i == n - 1 && j == 0)) continue;
			for (int dy = -2; dy <= 2; ++dy)
				for (int dx = -2; dx <= 2; ++dx) set(q, at[i] + dx, at[j] + dy, max_abs(dx, dy) != 1);
		}
}

static void format(Code *q, int mask) {
	int value = FORMAT[q->ecl] << 3 | mask, r = value, size = q->size;
	for (int i = 0; i < 10; ++i) r = (r << 1) ^ ((r >> 9) * 0x537);
	int f = (value << 10 | r) ^ 0x5412;
#define BIT(i) ((f >> (i) & 1) != 0)
	for (int i = 0; i <= 5; ++i) set(q, 8, i, BIT(i));
	set(q, 8, 7, BIT(6));
	set(q, 8, 8, BIT(7));
	set(q, 7, 8, BIT(8));
	for (int i = 9; i < 15; ++i) set(q, 14 - i, 8, BIT(i));
	for (int i = 0; i < 8; ++i) set(q, size - 1 - i, 8, BIT(i));
	for (int i = 8; i < 15; ++i) set(q, 8, size - 15 + i, BIT(i));
#undef BIT
	set(q, 8, size - 8, true);
}

static void version_bits(Code *q, int v) {
	if (v < 7) return;
	int r = v;
	for (int i = 0; i < 12; ++i) r = (r << 1) ^ ((r >> 11) * 0x1f25);
	int f = v << 12 | r;
	for (int i = 0; i < 18; ++i) {
		bool on = (f >> i & 1) != 0;
		int a = q->size - 11 + i % 3, b = i / 3;
		set(q, a, b, on);
		set(q, b, a, on);
	}
}

/* the codewords, up and down in pairs of columns from the right */
static void place(Code *q, const uint8_t *words, int nw) {
	int size = q->size;
	for (int right = size - 1, i = 0; right >= 1; right -= 2) {
		if (right == 6) right = 5;
		for (int vert = 0; vert < size; ++vert)
			for (int j = 0; j < 2; ++j) {
				int x = right - j, y = ((right + 1) & 2) == 0 ? size - 1 - vert : vert;
				if (!q->fixed[y][x] && i < nw * 8) {
					q->dark[y][x] = words[i >> 3] >> (7 - (i & 7)) & 1;
					++i;
				}
			}
	}
}

static bool mask_at(int m, int x, int y) {
	switch (m) {
	case 0: return (x + y) % 2 == 0;
	case 1: return y % 2 == 0;
	case 2: return x % 3 == 0;
	case 3: return (x + y) % 3 == 0;
	case 4: return (x / 3 + y / 2) % 2 == 0;
	case 5: return (x * y) % 2 + (x * y) % 3 == 0;
	case 6: return ((x * y) % 2 + (x * y) % 3) % 2 == 0;
	default: return ((x + y) % 2 + (x * y) % 3) % 2 == 0;
	}
}

static void apply(Code *q, int m) {
	for (int y = 0; y < q->size; ++y)
		for (int x = 0; x < q->size; ++x)
			if (!q->fixed[y][x] && mask_at(m, x, y)) q->dark[y][x] ^= 1;
}

static int module(const Code *q, bool columns, int a, int b) { return columns ? q->dark[b][a] : q->dark[a][b]; }

/* runs of five or more alike, and the finder's 1:1:3:1:1 with four light
 * modules on one side, along the rows or the columns */
static int line_penalty(const Code *q, bool columns) {
	int score = 0, size = q->size;
	for (int a = 0; a < size; ++a) {
		int run = 1;
		for (int b = 1; b <= size; ++b) {
			if (b < size && module(q, columns, a, b) == module(q, columns, a, b - 1)) ++run;
			else {
				if (run >= 5) score += 3 + run - 5;
				run = 1;
			}
		}
		for (int b = 0; b + 11 <= size; ++b) {
			int s = 0;
			for (int k = 0; k < 11; ++k) s = s << 1 | module(q, columns, a, b + k);
			if (s == 0x5D0 || s == 0x05D) score += 40;
		}
	}
	return score;
}

static int penalty(const Code *q) {
	int score = line_penalty(q, false) + line_penalty(q, true), size = q->size, n = 0;
	for (int y = 0; y + 1 < size; ++y)
		for (int x = 0; x + 1 < size; ++x) {
			int c = q->dark[y][x];
			if (c == q->dark[y][x + 1] && c == q->dark[y + 1][x] && c == q->dark[y + 1][x + 1]) score += 3;
		}
	for (int y = 0; y < size; ++y)
		for (int x = 0; x < size; ++x) n += q->dark[y][x];
	return score + 10 * (abs(n * 20 - size * size * 10) / (size * size));
}

int qr_make(const char *text, uint8_t m[QR_MAX * QR_MAX]) {
	gf_init();
	int len = (int)strlen(text);
	/* the smallest version it fits, at L; then M where that version holds it */
	int v = 0, ecl = ECL_L;
	for (int t = 1; t <= 10 && !v; ++t)
		if (4 + (t < 10 ? 8 : 16) + len * 8 <= data_codewords(t, ECL_L) * 8) v = t;
	if (!v) return 0;
	if (4 + (v < 10 ? 8 : 16) + len * 8 <= data_codewords(v, ECL_M) * 8) ecl = ECL_M;
	uint8_t words[MAX_WORDS];
	int nw = codewords(text, len, v, ecl, words);

	Code *q = calloc(1, sizeof *q);
	if (!q) return 0;
	q->size = v * 4 + 17;
	q->ecl = ecl;
	patterns(q, v);
	format(q, 0);
	version_bits(q, v);
	place(q, words, nw);
	/* the mask with the lowest penalty */
	int best = 0, lowest = -1;
	for (int k = 0; k < 8; ++k) {
		apply(q, k);
		format(q, k);
		int p = penalty(q);
		if (lowest < 0 || p < lowest) { lowest = p; best = k; }
		apply(q, k);
	}
	apply(q, best);
	format(q, best);
	int size = q->size;
	for (int y = 0; y < size; ++y)
		for (int x = 0; x < size; ++x) m[y * size + x] = q->dark[y][x];
	free(q);
	return size;
}
