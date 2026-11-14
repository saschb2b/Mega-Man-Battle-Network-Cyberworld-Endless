/* update_install.h. */
#include "update_install.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

/* the project's download page, which leads with the system it is opened on */
#define PAGE_URL "https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/download/"

bool update_install_page(void) {
#if SDL_VERSION_ATLEAST(2, 0, 14)
	return SDL_OpenURL(PAGE_URL) == 0;
#else
	return false;
#endif
}

#if defined(__linux__) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)

#include <sys/stat.h>
#include <unistd.h>

/* (the download beside the AppImage, so that the rename is one file
 * system's: the AppImage's name and ".update"; or in the PortMaster
 * launcher's update folder, update.zip and ready.txt beside it once
 * checked, which port/update.py puts in place as the game ends) */
static char staged[1100], port_zip[1100], port_ready[1100];

/* the AppImage's own path, "" where this is no AppImage */
static const char *appimage(void) {
	const char *a = getenv("APPIMAGE");
	return a && a[0] == '/' ? a : "";
}

#ifdef CW_DESKTOP
/* `file`'s folder holds the itch app's receipt (.itch/): its install */
static bool itch_beside(const char *file) {
	char dir[1100], probe[1200];
	snprintf(dir, sizeof dir, "%s", file);
	char *slash = strrchr(dir, '/');
	if (!slash) return false;
	*slash = 0;
	snprintf(probe, sizeof probe, "%s/.itch", dir);
	struct stat st;
	return stat(probe, &st) == 0 && S_ISDIR(st.st_mode);
}

/* `file`'s folder can be written to */
static bool folder_writable(const char *file) {
	char dir[1100];
	snprintf(dir, sizeof dir, "%s", file);
	char *slash = strrchr(dir, '/');
	if (!slash) return false;
	*slash = 0;
	return access(dir[0] ? dir : "/", W_OK) == 0;
}
#endif

UpdateWay update_install_way(const char **key) {
	*key = NULL;
#ifndef CW_DESKTOP
	/* (a PortMaster handheld: its launcher's update folder, where the
	 * launcher can put the update in place; an older launcher's, or one
	 * without python3: the releases page, there being no browser) */
	const char *dir = getenv("CYBERWORLD_UPDATE_DIR");
	if (!dir || dir[0] != '/' || access(dir, W_OK) != 0) return UPDATE_WAY_RELEASES;
	snprintf(staged, sizeof staged, "%s/update.zip.part", dir);
	snprintf(port_zip, sizeof port_zip, "%s/update.zip", dir);
	snprintf(port_ready, sizeof port_ready, "%s/ready.txt", dir);
	*key = "portmaster";
	return UPDATE_WAY_INSTALL;
#else
	char self[1100];
	ssize_t n = readlink("/proc/self/exe", self, sizeof self - 1);
	self[n > 0 ? n : 0] = 0;
	const char *a = appimage();
	if (itch_beside(a[0] ? a : self)) return UPDATE_WAY_ITCH;
	/* (the Flatpak's sandbox, the .deb's /usr and a folder from the
	 * tar.gz: the download page) */
	if (!a[0] || access("/.flatpak-info", F_OK) == 0 || !folder_writable(a)) return UPDATE_WAY_PAGE;
	snprintf(staged, sizeof staged, "%s.update", a);
	*key = "appimage";
	return UPDATE_WAY_INSTALL;
#endif
}

/* (and on a handheld an update the launcher did not put in place as the
 * last game ended: stale) */
void update_install_tidy(void) {
	if (staged[0]) remove(staged);
	if (port_zip[0]) {
		remove(port_ready);
		remove(port_zip);
	}
}

FILE *update_install_open(void) { return staged[0] ? fopen(staged, "wb") : NULL; }

/* (the running AppImage keeps the file it was started from: the new one
 * replaces its name, never its bytes; a handheld's zip waits for its
 * launcher, ready.txt last) */
bool update_install_place(const char *version) {
	if (!staged[0]) return false;
	if (!port_zip[0]) return chmod(staged, 0755) == 0 && rename(staged, appimage()) == 0;
	FILE *f = rename(staged, port_zip) == 0 ? fopen(port_ready, "w") : NULL;
	if (!f) return false;
	bool ok = fprintf(f, "%s\n", version) > 0;
	return fclose(f) == 0 && ok;
}

void update_install_drop(void) { update_install_tidy(); }

/* (the same process, the AppImage's runtime started anew in it: a
 * launcher that waits for the game, Steam's among them, goes on waiting;
 * the game's arguments as the system has them, the first the AppImage) */
void update_install_start(void) {
	static char line[8192];
	static char *args[128];
	const char *a = appimage();
	/* (a handheld's launcher puts it in place and starts it) */
	FILE *f = a[0] && !port_zip[0] ? fopen("/proc/self/cmdline", "rb") : NULL;
	if (!f) return;
	size_t n = fread(line, 1, sizeof line - 1, f);
	fclose(f);
	line[n] = 0;
	int argc = 0;
	args[argc++] = (char *)(uintptr_t)a;
	for (size_t at = strlen(line) + 1; at < n && argc < 127; at += strlen(line + at) + 1) args[argc++] = line + at;
	args[argc] = NULL;
	fflush(NULL);
	execv(a, args);
}

#elif defined(_WIN32)

#include <windows.h>

#define APP_KEY L"Software\\io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless"

static wchar_t staged[MAX_PATH + 64];

/* the .exe's folder, without its last backslash */
static bool exe_folder(wchar_t *dir, DWORD size) {
	DWORD n = GetModuleFileNameW(NULL, dir, size);
	if (!n || n >= size) return false;
	wchar_t *slash = wcsrchr(dir, L'\\');
	if (!slash) return false;
	*slash = 0;
	return true;
}

/* the installer's folder, as it wrote it (windows/installer.nsi) */
static bool installed_folder(wchar_t *dir, DWORD size) {
	DWORD bytes = size * sizeof *dir;
	if (RegGetValueW(HKEY_CURRENT_USER, APP_KEY, L"InstallDir", RRF_RT_REG_SZ, NULL, dir, &bytes) != ERROR_SUCCESS) return false;
	size_t n = wcslen(dir);
	if (n && dir[n - 1] == L'\\') dir[n - 1] = 0;
	return true;
}

UpdateWay update_install_way(const char **key) {
	wchar_t here[MAX_PATH], there[MAX_PATH], probe[MAX_PATH + 8];
	*key = NULL;
	if (!exe_folder(here, MAX_PATH)) return UPDATE_WAY_PAGE;
	_snwprintf(probe, MAX_PATH + 8, L"%ls\\.itch", here);
	probe[MAX_PATH + 7] = 0;
	DWORD at = GetFileAttributesW(probe);
	if (at != INVALID_FILE_ATTRIBUTES && (at & FILE_ATTRIBUTE_DIRECTORY)) return UPDATE_WAY_ITCH;
	/* (the zip's folder: the download page) */
	if (!installed_folder(there, MAX_PATH) || _wcsicmp(here, there)) return UPDATE_WAY_PAGE;
	wchar_t temp[MAX_PATH];
	DWORD n = GetTempPathW(MAX_PATH, temp);
	if (!n || n >= MAX_PATH) return UPDATE_WAY_PAGE;
	_snwprintf(staged, MAX_PATH + 64, L"%lscyberworld-endless-update-setup.exe", temp);
	staged[MAX_PATH + 63] = 0;
	*key = "windows-setup";
	return UPDATE_WAY_INSTALL;
}

void update_install_tidy(void) {
	if (staged[0]) DeleteFileW(staged);
}

FILE *update_install_open(void) { return staged[0] ? _wfopen(staged, L"wb") : NULL; }

/* (the installer stays where it was downloaded, and runs after the game's end) */
bool update_install_place(const char *version) {
	(void)version;
	return staged[0] != 0;
}

void update_install_drop(void) { update_install_tidy(); }

/* (silently, for this user alone as the first install was: it waits for
 * the game's .exe to be free, puts the new one over it and, asked by
 * /RUN, starts it) */
void update_install_start(void) {
	wchar_t line[MAX_PATH + 96];
	_snwprintf(line, MAX_PATH + 96, L"\"%ls\" /S /RUN", staged);
	line[MAX_PATH + 95] = 0;
	STARTUPINFOW si = { .cb = sizeof si };
	PROCESS_INFORMATION pi;
	if (!CreateProcessW(staged, line, NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &si, &pi)) {
		printf("update: the installer did not start (%lu)\n", (unsigned long)GetLastError());
		return;
	}
	printf("update: the installer runs\n");
	fflush(stdout);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

#else

#ifdef __ANDROID__
#include "update_android.h"

/* (Android: GameActivity's Java downloads the APK, checks it and hands
 * it to Android's installer, update_android.h; nothing is written here) */
UpdateWay update_install_way(const char **key) {
	*key = "android";
	return UPDATE_WAY_INSTALL;
}

void update_install_tidy(void) { update_android_tidy(); }
#else
/* (the systems whose updates come another way: none looks yet) */
UpdateWay update_install_way(const char **key) {
	*key = NULL;
	return UPDATE_WAY_NONE;
}

void update_install_tidy(void) {}
#endif

FILE *update_install_open(void) { return NULL; }

bool update_install_place(const char *version) {
	(void)version;
	return false;
}

void update_install_drop(void) {}

void update_install_start(void) {}

#endif
