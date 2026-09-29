/* What differs between the POSIX builds and Windows (MinGW-w64): making a
 * folder, and where the running program is. Paths are UTF-8 with '/' on
 * both (the Windows build's manifest makes the C library's paths UTF-8, and
 * Windows takes '/'). */
#ifndef COMPAT_H
#define COMPAT_H

#include <stdbool.h>
#include <stddef.h>

#ifdef _WIN32
#include <direct.h>
#define cw_mkdir(path) _mkdir(path)
#else
#include <sys/stat.h>
#define cw_mkdir(path) mkdir(path, 0755)
#endif

/* The running program's path; false when it cannot be told. */
bool cw_exe_path(char *out, size_t n);

/* rename(), replacing `to` where it exists, as POSIX's does: Windows' C
 * library and the 3DS's SD card refuse a name that is taken (a save's
 * .tmp was left beside the old save, which stayed) */
bool cw_rename(const char *from, const char *to);

#endif
