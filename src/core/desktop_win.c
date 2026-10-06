/* The Windows build's desktop (desktop.h): the ROM chosen in the system's
 * file dialog or found in Downloads and the usual ROM folders. No menu
 * entry (the installer makes the Start menu's) and no Steam script
 * (Steam's own "Add a Non-Steam Game" takes the .exe). */
#include "desktop.h"

#if defined(CW_DESKTOP) && defined(_WIN32)
#include <windows.h>
#include <commdlg.h>
#include <objbase.h>
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL_syswm.h>

#include "compat.h"
#include "launcher_roms.h"
#include "platform.h"
#include "rom.h"

/* UTF-16 to UTF-8 with '/' */
static bool utf8(const wchar_t *w, char *out, size_t n) {
	if (WideCharToMultiByte(CP_UTF8, 0, w, -1, out, (int)n, NULL, NULL) <= 0) return false;
	for (char *c = out; *c; ++c)
		if (*c == '\\') *c = '/';
	return true;
}

/* ---- the ROM: the launcher's file chooser (pick_desktop.c, on a thread
 * of its own: COM made ready there, the dialog owned by the game's window,
 * which it keeps in front of and stills while it is open) ---- */

bool desktop_can_choose(void) { return true; }

bool desktop_choose_rom(int slot, char *path, size_t n) {
	wchar_t file[MAX_PATH * 2] = L"";
	OPENFILENAMEW ofn;
	memset(&ofn, 0, sizeof ofn);
	ofn.lStructSize = sizeof ofn;
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (P.window && SDL_GetWindowWMInfo(P.window, &info) && info.subsystem == SDL_SYSWM_WINDOWS) ofn.hwndOwner = info.info.win.window;
	ofn.lpstrFilter = L"GBA ROM (*.gba)\0*.gba\0All files\0*.*\0";
	ofn.lpstrFile = file;
	ofn.nMaxFile = (DWORD)(sizeof file / sizeof *file);
	ofn.lpstrTitle = slot == 1 ? L"Choose your Mega Man Battle Network 5 ROM" : L"Choose your Mega Man Battle Network 6 ROM";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER;
	HRESULT com = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	bool ok = GetOpenFileNameW(&ofn) && utf8(file, path, n) && path[0];
	if (SUCCEEDED(com)) CoUninitialize();
	return ok;
}

/* ---- the ROM where downloads and front ends keep it ---- */

bool desktop_rom_elsewhere(const char *rom_dir, char *msg, size_t msglen) {
	char places[6][1024];
	int n = 0;
	PWSTR w = NULL;
	if (SHGetKnownFolderPath(&FOLDERID_Downloads, 0, NULL, &w) == S_OK && utf8(w, places[n], sizeof places[n])) ++n;
	CoTaskMemFree(w);
	const char *home = getenv("USERPROFILE");
	if (home && *home) {
		/* EmuDeck's folder, a ROMs folder */
		static const char *const under[] = { "Emulation/roms/gba", "ROMs/gba", "ROMs", "roms" };
		for (size_t i = 0; i < sizeof under / sizeof *under; ++i) {
			snprintf(places[n], sizeof places[n], "%s/%s", home, under[i]);
			for (char *c = places[n]; *c; ++c)
				if (*c == '\\') *c = '/';
			++n;
		}
	}
	char said[512], why[512];
	snprintf(said, sizeof said, "%s", msg);
	bool found = false;
	for (int i = 0; !found && i < n; ++i) found = rom_find(places[i], why, sizeof why);
	/* (another game's .gba in those folders is no news: the message stays the ROM folder's) */
	snprintf(msg, msglen, "%s", said);
	if (!found) return false;
	fprintf(stderr, "found the ROM at %s\n", R.path);
	roms_keep_copies(rom_dir);
	return true;
}

/* ---- the rest of desktop.h ---- */

void desktop_menu_entry(const char *data_dir) { (void)data_dir; }

bool desktop_big_screen(void) {
	const char *deck = getenv("SteamDeck"), *ui = getenv("SteamGamepadUI");
	return (deck && !strcmp(deck, "1")) || (ui && !strcmp(ui, "1"));
}

int desktop_steam_command(bool remove) {
	(void)remove;
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Cyberworld Endless",
		"On Windows, add the game with Steam's own \"Add a Non-Steam Game to My Library\" (the Games menu), "
		"choosing cyberworld-endless.exe.", NULL);
	return 1;
}

void desktop_steam_offer(const char *data_dir) { (void)data_dir; }

#else
typedef int desktop_win_unused;
#endif
