/* compat.h */
#include "compat.h"

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

bool cw_exe_path(char *out, size_t n) {
#ifdef _WIN32
	wchar_t w[MAX_PATH * 2];
	DWORD len = GetModuleFileNameW(NULL, w, (DWORD)(sizeof w / sizeof *w));
	if (len == 0 || len >= sizeof w / sizeof *w) return false;
	if (WideCharToMultiByte(CP_UTF8, 0, w, -1, out, (int)n, NULL, NULL) <= 0) return false;
	for (char *c = out; *c; ++c)
		if (*c == '\\') *c = '/';
	return true;
#else
	ssize_t len = readlink("/proc/self/exe", out, n - 1);
	if (len <= 0) return false;
	out[len] = 0;
	return true;
#endif
}
