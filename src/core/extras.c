/* extras.h. */
#include "extras.h"

#include <stdio.h>
#include <string.h>

#include "extras_text.h"
#include "rom.h"
#include "run.h"

bool extra_shown(int e) {
	if (e < 0 || e >= EXTRAS) return false;
	if (e != EXTRA_OLDER) return true;
	for (int x = 0; x < XROM_COUNT; ++x)
		if (XR[x].data && xrom_offered(x)) return true;
	return false;
}

int extras_off_names(char *buf, size_t n, unsigned off, const char *sep) {
	int k = 0;
	if (n) buf[0] = 0;
	for (int e = 0; e < EXTRAS; ++e)
		if (off >> e & 1) {
			size_t m = strlen(buf);
			snprintf(buf + m, n - m, "%s%s", k++ ? sep : "", extra_words(e)->name);
		}
	return k;
}
