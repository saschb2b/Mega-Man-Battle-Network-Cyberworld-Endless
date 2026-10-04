/* Another game's chips as BN6's, by name (xchips.h). */
#include "xchips.h"

#include <stdbool.h>
#include <string.h>

static bool same(const char *a, const char *b) { return a[0] && !strncmp(a, b, XCHIP_NAME); }

int xchips_pair(const char (*names6)[XCHIP_NAME], int n6, const char (*namesx)[XCHIP_NAME], int nx, uint16_t *to_x, uint16_t *from_x) {
	int paired = 0;
	for (int i = 0; i < n6; ++i) {
		to_x[i] = 0;
		for (int j = 1; i > 0 && j < nx && !to_x[i]; ++j)
			if (same(names6[i], namesx[j])) to_x[i] = (uint16_t)j;
		paired += to_x[i] != 0;
	}
	/* (and back: each of the other game's chips of a name BN6 has) */
	for (int j = 0; j < nx; ++j) {
		from_x[j] = 0;
		for (int i = 1; j > 0 && i < n6 && !from_x[j]; ++i)
			if (same(namesx[j], names6[i])) from_x[j] = (uint16_t)i;
	}
	return paired;
}

int xchips_code(const uint8_t codes[4], int code) {
	for (int i = 0; i < 4; ++i)
		if (codes[i] == code) return code;
	for (int i = 0; i < 4; ++i)
		if (codes[i] == 26) return 26;
	return codes[0] <= 26 ? codes[0] : code;
}

int xchips_out(const uint16_t *folder, int n, const uint16_t *to_x, int n6, uint16_t *out, int max) {
	int k = 0;
	for (int i = 0; i < n; ++i) {
		int id = folder[i] & 0x1FF;
		if (folder[i] == 0xFFFF || !id || (id < n6 && to_x[id])) continue;
		bool again = false;
		for (int e = 0; e < i && !again; ++e) again = folder[e] != 0xFFFF && (folder[e] & 0x1FF) == id;
		if (again) continue;
		if (k < max) out[k] = (uint16_t)id;
		++k;
	}
	return k;
}

static int has(const uint8_t codes[4], int code) {
	for (int i = 0; i < 4; ++i)
		if (codes[i] == code) return 1;
	return 0;
}

int xchips_fit(const uint8_t other[4], const uint8_t bn6[4], const uint8_t folder[3], int code, int row) {
	if (!(row & 1)) return code;
	for (int k = 0; k < 3 && folder[k]; ++k)
		if (has(other, folder[k] - 1) && has(bn6, folder[k] - 1)) return folder[k] - 1;
	return code;
}
