/* update_swap.h. */
#include "update_swap.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define SD_ROOT "sdmc:/"

/* `s` ends in `ext`, in any case */
static bool ends_in(const char *s, const char *ext) {
	size_t n = strlen(s), e = strlen(ext);
	if (n <= e) return false;
	for (size_t i = 0; i < e; ++i)
		if (tolower((unsigned char)s[n - e + i]) != tolower((unsigned char)ext[i])) return false;
	return true;
}

bool update_swap_self(const char *arg0, const char *ext, char *self, size_t n) {
	self[0] = 0;
	if (!arg0 || (strncmp(arg0, SD_ROOT, strlen(SD_ROOT)) && arg0[0] != '/') || !ends_in(arg0, ext)) return false;
	/* (room for the names beside it: ".update", ".old") */
	if (strlen(arg0) + 8 > n) return false;
	struct stat st;
	if (stat(arg0, &st) != 0 || !S_ISREG(st.st_mode)) return false;
	snprintf(self, n, "%s", arg0);
	return true;
}

void update_swap_staged(const char *self, char *staged, size_t n) { snprintf(staged, n, "%s.update", self); }

/* the old one's name while the new one takes its own */
static void aside_name(const char *self, char *aside, size_t n) { snprintf(aside, n, "%s.old", self); }

void update_swap_tidy(const char *self) {
	char staged[1100], aside[1100];
	struct stat st;
	update_swap_staged(self, staged, sizeof staged);
	aside_name(self, aside, sizeof aside);
	remove(staged);
	/* (a swap cut short between its renames leaves the old one aside and
	 * none in its name: then nothing started this game, and the old one
	 * stays for a player to name back) */
	if (stat(self, &st) == 0) remove(aside);
}

bool update_swap(const char *self) {
	char staged[1100], aside[1100];
	struct stat st;
	update_swap_staged(self, staged, sizeof staged);
	aside_name(self, aside, sizeof aside);
	if (stat(staged, &st) != 0) return false;
	remove(aside);
	if (rename(self, aside) != 0) return false;
	if (rename(staged, self) != 0) {
		rename(aside, self);
		return false;
	}
	remove(aside);
	return true;
}
