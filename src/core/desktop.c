#include "desktop.h"

#if defined(CW_DESKTOP) && !defined(_WIN32)
#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#ifndef GLOB_ONLYDIR
#define GLOB_ONLYDIR 0   /* (a GNU flag: elsewhere a file matches too, and rom_find passes over it) */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "compat.h"
#include "launcher_roms.h"
#include "platform.h"
#include "rom.h"

/* ---- small helpers ---- */

static bool on_path(const char *prog) {
	const char *path = getenv("PATH");
	if (!path) return false;
	char dir[1024];
	for (const char *p = path; *p;) {
		size_t n = strcspn(p, ":");
		snprintf(dir, sizeof dir, "%.*s/%s", (int)n, p, prog);
		if (access(dir, X_OK) == 0) return true;
		p += n + (p[n] == ':');
	}
	return false;
}

/* execvp for a command line of constant strings (execvp takes them as
 * char *const and changes none) */
static void exec_line(const char *const argv[]) {
	union { const char *const *c; char *const *v; } a = { argv };
	execvp(argv[0], a.v);
}

/* Runs argv and waits; its first line of output goes to out when given. */
static bool run(const char *const argv[], char *out, size_t outlen) {
	int fd[2];
	if (out && pipe(fd) != 0) return false;
	pid_t pid = fork();
	if (pid < 0) {
		/* (the pipe too: the analyzer found it left open, issue #19) */
		if (out) { close(fd[0]); close(fd[1]); }
		return false;
	}
	if (pid == 0) {
		if (out) { dup2(fd[1], 1); close(fd[0]); close(fd[1]); }
		int null = open("/dev/null", O_WRONLY);
		if (null >= 0) { dup2(null, 2); if (!out) dup2(null, 1); }
		exec_line(argv);
		_exit(127);
	}
	if (out) {
		close(fd[1]);
		size_t got = 0;
		ssize_t r;
		while (got + 1 < outlen && (r = read(fd[0], out + got, outlen - 1 - got)) > 0) got += (size_t)r;
		out[got] = 0;
		out[strcspn(out, "\n")] = 0;
		close(fd[0]);
	}
	int status;
	while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {}
	return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/* Runs argv and waits; its exit status, or -1. */
static int run_status(const char *const argv[]) {
	pid_t pid = fork();
	if (pid < 0) return -1;
	if (pid == 0) {
		int null = open("/dev/null", O_WRONLY);
		if (null >= 0) { dup2(null, 1); dup2(null, 2); }
		exec_line(argv);
		_exit(127);
	}
	int status;
	while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {}
	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

static bool copy_file(const char *from, const char *to) {
	FILE *in = fopen(from, "rb");
	if (!in) return false;
	FILE *out = fopen(to, "wb");
	if (!out) { fclose(in); return false; }
	char buf[65536];
	size_t n;
	bool ok = true;
	while ((n = fread(buf, 1, sizeof buf, in)) > 0) ok = ok && fwrite(buf, 1, n, out) == n;
	fclose(in);
	ok = fclose(out) == 0 && ok;
	if (!ok) remove(to);
	return ok;
}

static void make_dirs(const char *path) {
	char p[1024];
	snprintf(p, sizeof p, "%s", path);
	for (char *c = p + 1; *c; ++c)
		if (*c == '/') { *c = 0; mkdir(p, 0755); *c = '/'; }
	mkdir(p, 0755);
}

/* Wraps text in place at about width characters (SDL's X11 message box
 * does not), breaking at spaces or, in a long path, after a slash. */
static void wrap(char *text, size_t cap, int width) {
	int col = 0;
	char *brk = NULL;
	for (char *c = text; *c; ++c) {
		if (*c == '\n') { col = 0; brk = NULL; continue; }
		if (*c == ' ') brk = c;
		if (++col > width && brk) {
			if (*brk == ' ') { *brk = '\n'; col = (int)(c - brk); }
			else if (strlen(text) + 2 <= cap) {
				/* after a slash: shift the rest to make room for the break */
				size_t n = strlen(brk + 1);
				memmove(brk + 2, brk + 1, n + 1);
				brk[1] = '\n';
				col = (int)(c + 1 - (brk + 1));
				++c;
			}
			brk = NULL;
		}
		if (*c == '/' && col > width / 2) brk = c;
	}
}

/* ---- questions: the desktop's own dialog (zenity on GNOME and most
 * others, kdialog on KDE), else SDL's. They come before the game's window
 * exists: a window that stops answering while a dialog waits is marked
 * "not responding". ---- */

/* Whether the desktop can find the icon by name: installed, or inside the
 * AppImage (whose share folder the dialogs then get to search). */
static bool icon_known(void) {
	char path[1200];
	const char *appdir = getenv("APPDIR"), *home = getenv("HOME");
	if (appdir) {
		snprintf(path, sizeof path, "%s/usr/share/icons/hicolor/256x256/apps/" APP_ID ".png", appdir);
		if (access(path, R_OK) == 0) {
			const char *old = getenv("XDG_DATA_DIRS");
			char dirs_env[2400];
			snprintf(dirs_env, sizeof dirs_env, "%s/usr/share:%s", appdir, old && *old ? old : "/usr/local/share:/usr/share");
			setenv("XDG_DATA_DIRS", dirs_env, 1);
			return true;
		}
	}
	if (home) {
		snprintf(path, sizeof path, "%s/.local/share/icons/hicolor/256x256/apps/" APP_ID ".png", home);
		if (access(path, R_OK) == 0) return true;
	}
	return access("/usr/share/icons/hicolor/256x256/apps/" APP_ID ".png", R_OK) == 0;
}

/* zenity's text is Pango markup */
static void markup_escape(char *out, size_t n, const char *in) {
	size_t o = 0;
	for (; *in && o + 6 < n; ++in) {
		const char *rep = *in == '&' ? "&amp;" : *in == '<' ? "&lt;" : *in == '>' ? "&gt;" : NULL;
		if (rep) { memcpy(out + o, rep, strlen(rep)); o += strlen(rep); }
		else out[o++] = *in;
	}
	out[o] = 0;
}

/* Asks with two or three buttons: labels[0] is the default, the last is
 * what closing the dialog means. Returns the chosen label's index, or -1
 * when no dialog could be shown (a Flatpak has no zenity or kdialog, and
 * SDL's box on Wayland is zenity's). */
static int ask(const char *text, const char *const *labels, int n) {
	if (on_path("zenity")) {
		char body[2400], ok[80], cancel[80], extra[80];
		markup_escape(body, sizeof body, text);
		char textarg[2500];
		snprintf(textarg, sizeof textarg, "--text=%s", body);
		snprintf(ok, sizeof ok, "--ok-label=%s", labels[0]);
		snprintf(cancel, sizeof cancel, "--cancel-label=%s", labels[n - 1]);
		snprintf(extra, sizeof extra, "--extra-button=%s", n > 2 ? labels[1] : "");
		const char *argv[] = { "zenity", "--question", "--title=Cyberworld Endless", "--width=440", textarg, ok, cancel,
			icon_known() ? "--icon=" APP_ID : "--icon=dialog-question", n > 2 ? extra : NULL, NULL };
		char out[128] = "";
		if (run(argv, out, sizeof out)) return 0;
		for (int i = 1; i < n - 1; ++i) if (!strcmp(out, labels[i])) return i;
		return n - 1;
	}
	if (on_path("kdialog")) {
		const char *argv[] = { "kdialog", "--title", "Cyberworld Endless", n > 2 ? "--yesnocancel" : "--yesno", text,
			"--yes-label", labels[0], "--no-label", labels[1],
			n > 2 ? "--cancel-label" : NULL, labels[n - 1], NULL };
		int code = run_status(argv);
		return code >= 0 && code < n ? code : n - 1;
	}
	char body[2400];
	snprintf(body, sizeof body, "%s", text);
	wrap(body, sizeof body, 60);
	SDL_MessageBoxButtonData buttons[3];
	for (int i = 0; i < n; ++i)
		buttons[i] = (SDL_MessageBoxButtonData){ i == 0 ? SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT
			: i == n - 1 ? SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT : 0, i, labels[i] };
	SDL_MessageBoxData box = { SDL_MESSAGEBOX_INFORMATION | SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT, NULL,
		"Cyberworld Endless", body, n, buttons, NULL };
	int hit = n - 1;
	if (SDL_ShowMessageBox(&box, &hit) != 0) return -1;
	return hit < 0 ? n - 1 : hit;
}

/* ---- the ROM: the launcher's file chooser (pick_desktop.c, on a thread
 * of its own) ---- */

bool desktop_can_choose(void) {
#ifdef __APPLE__
	return true;
#else
	return on_path("zenity") || on_path("kdialog");
#endif
}

bool desktop_choose_rom(int slot, char *path, size_t n) {
	const char *title = slot == 1 ? "Choose your Mega Man Battle Network 5 ROM" : "Choose your Mega Man Battle Network 6 ROM";
	path[0] = 0;
#ifdef __APPLE__
	/* (macOS: its own open panel, through AppleScript) */
	char script[200];
	snprintf(script, sizeof script, "POSIX path of (choose file with prompt \"%s\")", title);
	const char *argv[] = { "osascript", "-e", script, NULL };
	return run(argv, path, n) && path[0];
#else
	char titlearg[120];
	snprintf(titlearg, sizeof titlearg, "--title=%s", title);
	if (on_path("zenity")) {
		const char *argv[] = { "zenity", "--file-selection", titlearg, "--file-filter=GBA ROM | *.gba *.GBA",
			"--file-filter=All files | *", NULL };
		return run(argv, path, n) && path[0];
	}
	if (on_path("kdialog")) {
		const char *home = getenv("HOME");
		const char *argv[] = { "kdialog", "--title", title, "--getopenfilename", home ? home : "/", "*.gba *.GBA|GBA ROM", NULL };
		return run(argv, path, n) && path[0];
	}
	return false;
#endif
}

bool desktop_choose_saves(int kind, const char *start, char *path, size_t n) {
	path[0] = 0;
#ifdef __APPLE__
	const char *script = kind == 2 ? "on run argv\nPOSIX path of (choose folder with prompt \"Choose the auto-export folder\" default location (POSIX file (item 1 of argv)))\nend run"
		: kind == 1 ? "on run argv\nPOSIX path of (choose file name with prompt \"Export saves\" default name \"cyberworld-endless.cwsave\" default location (POSIX file (item 1 of argv)))\nend run"
		: "on run argv\nPOSIX path of (choose file with prompt \"Import saves\" default location (POSIX file (item 1 of argv)))\nend run";
	const char *argv[] = { "osascript", "-e", script, start, NULL };
	return run(argv, path, n) && path[0];
#else
	const char *title = kind == 2 ? "Choose the auto-export folder" : kind == 1 ? "Export saves" : "Import saves";
	if (on_path("zenity")) {
		char titlearg[100], initial[1200];
		snprintf(titlearg, sizeof titlearg, "--title=%s", title);
		snprintf(initial, sizeof initial, "--filename=%s%s", start, kind == 1 ? "/cyberworld-endless.cwsave" : "/");
		const char *argv[] = { "zenity", "--file-selection", titlearg, initial,
			kind == 2 ? "--directory" : kind == 1 ? "--save" : "--file-filter=Saves | *.cwsave",
			kind == 1 ? "--confirm-overwrite" : NULL, NULL };
		return run(argv, path, n) && path[0];
	}
	if (on_path("kdialog")) {
		char initial[1200];
		snprintf(initial, sizeof initial, "%s%s", start, kind == 1 ? "/cyberworld-endless.cwsave" : "/");
		const char *argv[] = { "kdialog", "--title", title,
			kind == 2 ? "--getexistingdirectory" : kind == 1 ? "--getsavefilename" : "--getopenfilename", initial,
			kind == 2 ? NULL : "*.cwsave|Saves", NULL };
		return run(argv, path, n) && path[0];
	}
	return false;
#endif
}

/* ---- the ROM where front ends keep it ---- */

/* The Downloads folder as the desktop names it (user-dirs.dirs), else ~/Downloads. */
static void downloads_dir(const char *home, char *out, size_t n) {
	snprintf(out, n, "%s/Downloads", home);
	const char *config = getenv("XDG_CONFIG_HOME");
	char file[1024], line[1024];
	if (config && *config == '/') snprintf(file, sizeof file, "%s/user-dirs.dirs", config);
	else snprintf(file, sizeof file, "%s/.config/user-dirs.dirs", home);
	FILE *f = fopen(file, "r");
	if (!f) return;
	while (fgets(line, sizeof line, f)) {
		if (strncmp(line, "XDG_DOWNLOAD_DIR=\"", 18)) continue;
		char *v = line + 18, *end = strchr(v, '"');
		if (!end) break;
		*end = 0;
		if (!strncmp(v, "$HOME", 5)) snprintf(out, n, "%s%s", home, v + 5);
		else if (*v == '/') snprintf(out, n, "%s", v);
		break;
	}
	fclose(f);
}

bool desktop_saves_default(char *path, size_t n) {
	const char *home = getenv("HOME");
	if (!home || !*home) return false;
	downloads_dir(home, path, n);
	make_dirs(path);
	return true;
}

void desktop_saves_reveal(const char *path) {
#ifdef __APPLE__
	const char *argv[] = { "open", "-R", path, NULL };
	run(argv, NULL, 0);
#else
	char dir[1100];
	snprintf(dir, sizeof dir, "%s", path);
	char *slash = strrchr(dir, '/');
	if (slash) *slash = 0;
	const char *argv[] = { "xdg-open", dir, NULL };
	run(argv, NULL, 0);
#endif
}

bool desktop_rom_elsewhere(const char *rom_dir, char *msg, size_t msglen) {
	const char *home = getenv("HOME");
	if (!home || !*home) return false;
	/* EmuDeck's folder, on the Deck or its SD card (/run/media/mmcblk0p1 or
	 * /run/media/deck/LABEL), RetroDECK's, a ROMs folder, the downloads */
	static const char *const places[] = {
		"~/Emulation/roms/gba", "/run/media/*/Emulation/roms/gba", "/run/media/*/*/Emulation/roms/gba",
		"~/retrodeck/roms/gba", "/run/media/*/retrodeck/roms/gba", "/run/media/*/*/retrodeck/roms/gba",
		"~/ROMs/gba", "~/ROMs/GBA", "~/roms/gba", "~/ROMs", "~/roms", NULL,
	};
	char said[512], pattern[1100], dl[1024], why[512];
	snprintf(said, sizeof said, "%s", msg);
	downloads_dir(home, dl, sizeof dl);
	bool found = false;
	for (int i = 0; !found && i <= (int)(sizeof places / sizeof *places) - 1; ++i) {
		if (places[i]) snprintf(pattern, sizeof pattern, "%s%s", places[i][0] == '~' ? home : "", places[i] + (places[i][0] == '~'));
		else snprintf(pattern, sizeof pattern, "%s", dl);
		glob_t g;
		if (glob(pattern, GLOB_ONLYDIR, NULL, &g) != 0) continue;
		for (size_t k = 0; !found && k < g.gl_pathc; ++k) found = rom_find(g.gl_pathv[k], why, sizeof why);
		globfree(&g);
	}
	/* (another game's .gba in those folders is no news: the message stays the ROM folder's) */
	snprintf(msg, msglen, "%s", said);
	if (!found) return false;
	fprintf(stderr, "found the ROM at %s\n", R.path);
	/* (a copy where the next start looks first: the SD card may be out then) */
	roms_keep_copies(rom_dir);
	return true;
}

/* ---- Steam's big screen ---- */

bool desktop_big_screen(void) {
	const char *deck = getenv("SteamDeck"), *ui = getenv("SteamGamepadUI"), *desktop = getenv("XDG_CURRENT_DESKTOP");
	return (deck && !strcmp(deck, "1")) || (ui && !strcmp(ui, "1")) || getenv("GAMESCOPE_WAYLAND_DISPLAY") ||
		(desktop && strstr(desktop, "gamescope"));
}

/* ---- the AppImage's menu entry ---- */

/* A path as an argument of the desktop entry's Exec key (quoted, with the
 * characters the spec escapes). */
static void exec_quote(char *out, size_t n, const char *path) {
	size_t o = 0;
	if (o + 1 < n) out[o++] = '"';
	for (const char *c = path; *c && o + 3 < n; ++c) {
		if (strchr("\"`$\\", *c)) out[o++] = '\\';
		if (*c == '%') out[o++] = '%';
		out[o++] = *c;
	}
	if (o + 1 < n) out[o++] = '"';
	out[o] = 0;
}

static bool write_entry(const char *file, const char *appimage) {
	char exec[2200];
	exec_quote(exec, sizeof exec, appimage);
	FILE *f = fopen(file, "w");
	if (!f) return false;
	fprintf(f,
		"[Desktop Entry]\n"
		"Type=Application\n"
		"Name=Cyberworld Endless\n"
		"GenericName=Roguelike\n"
		"Comment=A roguelike on Mega Man Battle Network 6 (bring your own ROM)\n"
		"Exec=%s\n"
		"TryExec=%s\n"
		"Icon=" APP_ID "\n"
		"Terminal=false\n"
		"Categories=Game;ActionGame;RolePlaying;\n"
		"Keywords=Mega Man;Battle Network;MMBN;roguelike;GBA;\n"
		"StartupWMClass=" APP_ID "\n"
		"X-Cyberworld-AppImage=%s\n",
		exec, appimage, appimage);
	return fclose(f) == 0;
}

/* the icons the AppImage carries, into the user's icon theme */
static void copy_icons(const char *appdir, const char *share) {
	static const int sizes[] = { 32, 64, 128, 256, 512 };
	for (size_t i = 0; i < sizeof sizes / sizeof *sizes; ++i) {
		char from[1400], dir[1400], to[1500];
		snprintf(from, sizeof from, "%s/usr/share/icons/hicolor/%dx%d/apps/" APP_ID ".png", appdir, sizes[i], sizes[i]);
		snprintf(dir, sizeof dir, "%s/icons/hicolor/%dx%d/apps", share, sizes[i], sizes[i]);
		make_dirs(dir);
		snprintf(to, sizeof to, "%s/" APP_ID ".png", dir);
		copy_file(from, to);
	}
}

/* The application ID before this one: an entry and icons this AppImage made
 * under it move to APP_ID (else the menu would show the game twice). */
#define OLD_APP_ID "io.github.saschb2b.CyberworldEndless"

static bool made_by_us(const char *file) {
	FILE *f = fopen(file, "r");
	if (!f) return false;
	char line[2200];
	bool ours = false;
	while (!ours && fgets(line, sizeof line, f)) ours = !strncmp(line, "X-Cyberworld-AppImage=", 22);
	fclose(f);
	return ours;
}

static void drop_old_entry(const char *apps, const char *share) {
	static const int sizes[] = { 32, 64, 128, 256, 512 };
	char old[1200], icon[1400];
	snprintf(old, sizeof old, "%s/" OLD_APP_ID ".desktop", apps);
	if (!made_by_us(old)) return;
	unlink(old);
	for (size_t i = 0; i < sizeof sizes / sizeof *sizes; ++i) {
		snprintf(icon, sizeof icon, "%s/icons/hicolor/%dx%d/apps/" OLD_APP_ID ".png", share, sizes[i], sizes[i]);
		unlink(icon);
	}
}

void desktop_menu_entry(const char *data_dir) {
	const char *appimage = getenv("APPIMAGE"), *appdir = getenv("APPDIR");
	const char *xdg = getenv("XDG_DATA_HOME"), *home = getenv("HOME");
	if (!appimage || !appdir || *appimage != '/') return;
	char share[1024], apps[1100], file[1200], declined[1024];
	if (xdg && *xdg == '/') snprintf(share, sizeof share, "%s", xdg);
	else if (home && *home) snprintf(share, sizeof share, "%s/.local/share", home);
	else return;
	snprintf(apps, sizeof apps, "%s/applications", share);
	snprintf(file, sizeof file, "%s/" APP_ID ".desktop", apps);
	snprintf(declined, sizeof declined, "%s/menu-entry-declined", data_dir);

	/* an entry made under the old ID: made again under this one */
	char old_file[1200];
	snprintf(old_file, sizeof old_file, "%s/" OLD_APP_ID ".desktop", apps);
	if (access(file, F_OK) != 0 && made_by_us(old_file)) {
		copy_icons(appdir, share);
		if (write_entry(file, appimage)) { drop_old_entry(apps, share); printf("moved %s to %s\n", old_file, file); }
		return;
	}

	FILE *f = fopen(file, "r");
	if (f) {
		/* ours, for an AppImage that has moved or been renamed: follow it */
		char line[2200], old[2200] = "";
		while (fgets(line, sizeof line, f))
			if (!strncmp(line, "X-Cyberworld-AppImage=", 22)) { snprintf(old, sizeof old, "%s", line + 22); old[strcspn(old, "\n")] = 0; }
		fclose(f);
		if (old[0] && strcmp(old, appimage)) { write_entry(file, appimage); copy_icons(appdir, share); }
		return;
	}
	if (access(declined, F_OK) == 0) return;

	char text[1400];
	snprintf(text, sizeof text,
		"Add Cyberworld Endless to your application menu?\n\n"
		"The entry starts this AppImage:\n%s\n\nIf you move the file, start it once from its new place.", appimage);
	const char *labels[] = { "Add to menu", "Don't ask again", "Not now" };
	int hit = ask(text, labels, 3);   /* (-1: none could be shown: not now) */
	if (hit == 1) {
		FILE *d = fopen(declined, "w");
		if (d) fclose(d);
	}
	if (hit != 0) return;
	make_dirs(apps);
	copy_icons(appdir, share);
	if (!write_entry(file, appimage)) { fprintf(stderr, "could not write %s\n", file); return; }
	if (on_path("update-desktop-database")) {
		const char *argv[] = { "update-desktop-database", apps, NULL };
		run(argv, NULL, 0);
	}
	printf("added %s\n", file);
}

/* ---- Steam: the game as a non-Steam shortcut, with its artwork ---- */

/* Says something with an OK button. */
static void tell(const char *text) {
	if (on_path("zenity")) {
		char body[2400], textarg[2500];
		markup_escape(body, sizeof body, text);
		snprintf(textarg, sizeof textarg, "--text=%s", body);
		const char *argv[] = { "zenity", "--info", "--title=Cyberworld Endless", "--width=440", textarg,
			icon_known() ? "--icon=" APP_ID : "--icon=dialog-information", NULL };
		run(argv, NULL, 0);
		return;
	}
	if (on_path("kdialog")) {
		const char *argv[] = { "kdialog", "--title", "Cyberworld Endless", "--msgbox", text, NULL };
		run(argv, NULL, 0);
		return;
	}
	char body[2400];
	snprintf(body, sizeof body, "%s", text);
	wrap(body, sizeof body, 60);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Cyberworld Endless", body, NULL);
}

/* linux/steam/add-to-steam.py where this build carries it (the AppImage's
 * and the .deb's share folder, the tar.gz's steam folder beside the
 * program), and the program Steam should start. */
static bool steam_script(char *script, size_t n, char *exe, size_t en) {
	const char *appimage = getenv("APPIMAGE"), *appdir = getenv("APPDIR");
	if (appimage && appdir && *appimage == '/') {
		snprintf(script, n, "%s/usr/share/cyberworld-endless/steam/add-to-steam.py", appdir);
		snprintf(exe, en, "%s", appimage);
		return access(script, R_OK) == 0;
	}
	char self[1024];
	if (!cw_exe_path(self, sizeof self)) return false;
	char dir[1024];
	snprintf(dir, sizeof dir, "%s", self);
	char *slash = strrchr(dir, '/');
	if (!slash) return false;
	*slash = 0;
	snprintf(exe, en, "%s", self);
	snprintf(script, n, "%s/steam/add-to-steam.py", dir);
	if (access(script, R_OK) == 0) return true;
	snprintf(script, n, "%s/../../share/cyberworld-endless/steam/add-to-steam.py", dir);
	if (access(script, R_OK) != 0) return false;
	if (access("/usr/bin/cyberworld-endless", X_OK) == 0) snprintf(exe, en, "/usr/bin/cyberworld-endless");
	return true;
}

int desktop_steam_command(bool remove) {
	if (getenv("FLATPAK_ID")) {
		/* (the sandbox has no way into Steam's folder: the host runs it) */
		printf("The Flatpak cannot reach Steam's folder from its sandbox. In a terminal, run:\n\n"
			"  python3 \"$(flatpak info --show-location " APP_ID ")/files/share/cyberworld-endless/steam/add-to-steam.py\"%s\n",
			remove ? " --remove" : "");
		return 1;
	}
	char script[1200], exe[1100];
	if (!steam_script(script, sizeof script, exe, sizeof exe)) {
		fprintf(stderr, "this build carries no add-to-steam.py\n");
		return 1;
	}
	const char *argv[] = { "python3", script, "--exe", exe, remove ? "--remove" : NULL, NULL };
	exec_line(argv);
	fprintf(stderr, "add-to-steam.py needs python3\n");
	return 1;
}

void desktop_steam_offer(const char *data_dir) {
	/* not when Steam started the game (closing Steam would end it), nor
	 * in the Flatpak's sandbox, nor once answered */
	if (getenv("SteamGameId") || getenv("SteamAppId") || getenv("SteamClientLaunch") || getenv("FLATPAK_ID")) return;
	char done[1024], script[1200], exe[1100];
	snprintf(done, sizeof done, "%s/steam-asked", data_dir);
	if (access(done, F_OK) == 0 || !on_path("python3") || !steam_script(script, sizeof script, exe, sizeof exe)) return;
	/* 0: there already, 2: no Steam here */
	const char *check[] = { "python3", script, "--exe", exe, "--check", NULL };
	if (run_status(check) != 1) return;
	const char *labels[] = { "Add to Steam", "Don't ask again", "Not now" };
	int hit = ask("Add Cyberworld Endless to Steam, with its library artwork?\n\n"
		"Steam closes for a moment and opens again. On a Steam Deck the game is then in "
		"your library in Gaming Mode, where Steam gives it the Deck's controls.", labels, 3);
	if (hit == 2 || hit < 0) return;
	FILE *f = fopen(done, "w");
	if (f) fclose(f);
	if (hit != 0) return;
	const char *add[] = { "python3", script, "--exe", exe, "--yes", NULL };
	if (run_status(add) == 0)
		tell("Cyberworld Endless is in your Steam library, with its artwork.");
	else
		tell("Steam could not be updated. Run the game with --add-to-steam in a terminal to see why.");
}

#else
typedef int desktop_unused;
#endif
