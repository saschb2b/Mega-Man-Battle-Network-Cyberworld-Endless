/* The Windows build's desktop (desktop.h): the ROM chosen in the system's
 * file dialog or found in Downloads and the usual ROM folders, questions in
 * message boxes. No menu entry (the installer makes the Start menu's) and
 * no Steam script (Steam's own "Add a Non-Steam Game" takes the .exe). */
#include "desktop.h"

#if defined(CW_DESKTOP) && defined(_WIN32)
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "platform.h"
#include "rom.h"

/* UTF-16 to UTF-8 with '/' */
static bool utf8(const wchar_t *w, char *out, size_t n) {
	if (WideCharToMultiByte(CP_UTF8, 0, w, -1, out, (int)n, NULL, NULL) <= 0) return false;
	for (char *c = out; *c; ++c)
		if (*c == '\\') *c = '/';
	return true;
}

/* A path shown to a Windows player, with '\' */
static void shown(char *out, size_t n, const char *path) {
	snprintf(out, n, "%s", path);
	for (char *c = out; *c; ++c)
		if (*c == '/') *c = '\\';
}

static bool exists(const char *path) {
	FILE *f = fopen(path, "rb");
	if (!f) return false;
	fclose(f);
	return true;
}

static bool copy_file(const char *from, const char *to) {
	FILE *in = fopen(from, "rb");
	if (!in) return false;
	FILE *out = fopen(to, "wb");
	if (!out) { fclose(in); return false; }
	char buf[65536];
	size_t n;
	bool ok = true;
	while ((n = fread(buf, 1, sizeof buf, in)) > 0)
		if (fwrite(buf, 1, n, out) != n) { ok = false; break; }
	fclose(in);
	if (fclose(out) != 0) ok = false;
	return ok;
}

/* Asks with two or three buttons: labels[0] is the default, the last is
 * what closing the box means. The chosen label's index, or -1 when no box
 * could be shown. */
static int ask(const char *text, const char *const *labels, int n) {
	SDL_MessageBoxButtonData buttons[3];
	for (int i = 0; i < n; ++i)
		buttons[i] = (SDL_MessageBoxButtonData){ i == 0 ? SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT
			: i == n - 1 ? SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT : 0, i, labels[i] };
	SDL_MessageBoxData box = { SDL_MESSAGEBOX_INFORMATION | SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT, NULL,
		"Cyberworld Endless", text, n, buttons, NULL };
	int hit = n - 1;
	if (SDL_ShowMessageBox(&box, &hit) != 0) return -1;
	return hit < 0 ? n - 1 : hit;
}

/* ---- the ROM ---- */

static bool choose_file(char *path, size_t n) {
	wchar_t file[MAX_PATH * 2] = L"";
	OPENFILENAMEW ofn;
	memset(&ofn, 0, sizeof ofn);
	ofn.lStructSize = sizeof ofn;
	ofn.lpstrFilter = L"GBA ROM (*.gba)\0*.gba\0All files\0*.*\0";
	ofn.lpstrFile = file;
	ofn.nMaxFile = (DWORD)(sizeof file / sizeof *file);
	ofn.lpstrTitle = L"Choose your Mega Man Battle Network 6 ROM";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
	return GetOpenFileNameW(&ofn) && utf8(file, path, n) && path[0];
}

/* a copy where the next start looks first */
static void keep_copy(const char *path, const char *rom_dir) {
	const char *base = strrchr(path, '/');
	char to[1400];
	snprintf(to, sizeof to, "%s/%s", rom_dir, base ? base + 1 : path);
	if (strcmp(to, path) && !exists(to) && !copy_file(path, to))
		fprintf(stderr, "could not copy the ROM to %s; it is used from %s\n", to, path);
}

int desktop_rom_dialog(const char *rom_dir, bool (*scan)(char *msg, size_t msglen), char *msg, size_t msglen) {
	char where[700];
	shown(where, sizeof where, rom_dir);
	for (;;) {
		/* the reason, unless it is only that the folder holds no ROM */
		char text[1800], why[700] = "";
		if (strncmp(msg, "Put your", 8)) snprintf(why, sizeof why, "%s\n\n", msg);
		snprintf(text, sizeof text,
			"%sCyberworld Endless runs on your own copy of Mega Man Battle Network 6: Cybeast Gregar (USA), "
			"an unmodified .gba file.\n\nChoose the file, or put it into\n%s\nor your Downloads folder, and look again.",
			why, where);
		const char *labels[] = { "Choose ROM...", "Look again", "Quit" };
		int hit = ask(text, labels, 3);
		if (hit < 0) return -1;
		if (hit == 2) return 0;
		if (hit == 0) {
			char path[1024] = "";
			if (!choose_file(path, sizeof path)) continue;
			if (!rom_load_file(path, msg, msglen)) continue;
			keep_copy(path, rom_dir);
			return 1;
		}
		if (scan(msg, msglen)) return 1;
	}
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
	keep_copy(R.path, rom_dir);
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
