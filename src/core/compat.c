/* compat.h */
#include "compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <limits.h>
#include <mach-o/dyld.h>
#include <stdlib.h>
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
#elif defined(__APPLE__)
	char path[PATH_MAX];
	uint32_t size = sizeof path;
	if (_NSGetExecutablePath(path, &size) != 0) return false;
	char *real = realpath(path, NULL);
	if (!real) return false;
	snprintf(out, n, "%s", real);
	free(real);
	return true;
#else
	ssize_t len = readlink("/proc/self/exe", out, n - 1);
	if (len <= 0) return false;
	out[len] = 0;
	return true;
#endif
}

bool cw_rename(const char *from, const char *to) {
#ifdef _WIN32
	/* (UTF-8 paths: the .exe's manifest makes them the ANSI calls' own) */
	return MoveFileExA(from, to, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
#ifdef __3DS__
	remove(to);
#endif
	return rename(from, to) == 0;
#endif
}
