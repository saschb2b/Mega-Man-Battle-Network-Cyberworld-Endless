#include "desktop.h"

#ifdef CW_DESKTOP
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

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

/* Runs argv and waits; its first line of output goes to out when given. */
static bool run(char *const argv[], char *out, size_t outlen) {
	int fd[2];
	if (out && pipe(fd) != 0) return false;
	pid_t pid = fork();
	if (pid < 0) return false;
	if (pid == 0) {
		if (out) { dup2(fd[1], 1); close(fd[0]); close(fd[1]); }
		int null = open("/dev/null", O_WRONLY);
		if (null >= 0) { dup2(null, 2); if (!out) dup2(null, 1); }
		execvp(argv[0], argv);
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

/* ---- the ROM ---- */

static bool choose_file(char *path, size_t n) {
	if (on_path("zenity")) {
		char *argv[] = { "zenity", "--file-selection", "--title=Choose your Mega Man Battle Network 6 ROM",
			"--file-filter=GBA ROM | *.gba *.GBA", "--file-filter=All files | *", NULL };
		return run(argv, path, n) && path[0];
	}
	if (on_path("kdialog")) {
		const char *home = getenv("HOME");
		char *argv[] = { "kdialog", "--title", "Choose your Mega Man Battle Network 6 ROM", "--getopenfilename",
			(char *)(home ? home : "/"), "*.gba *.GBA|GBA ROM", NULL };
		return run(argv, path, n) && path[0];
	}
	return false;
}

bool desktop_rom_dialog(const char *rom_dir, bool (*scan)(char *msg, size_t msglen), char *msg, size_t msglen) {
	bool picker = on_path("zenity") || on_path("kdialog");
	enum { QUIT, CHOOSE, FOLDER, AGAIN };
	for (;;) {
		/* the reason, unless it is only that the folder holds no ROM */
		char text[1800], why[700] = "";
		if (strncmp(msg, "Put your", 8)) snprintf(why, sizeof why, "%s\n\n", msg);
		snprintf(text, sizeof text,
			"%sCyberworld Endless runs on your own copy of Mega Man Battle Network 6: Cybeast Gregar (USA), "
			"an unmodified .gba file.\n\n%s it, or copy it into\n%s\nand choose Look again.",
			why, picker ? "Choose" : "Open the folder and put", rom_dir);
		wrap(text, sizeof text, 60);
		SDL_MessageBoxButtonData buttons[4];
		int nb = 0;
		if (picker) buttons[nb++] = (SDL_MessageBoxButtonData){ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, CHOOSE, "Choose ROM…" };
		buttons[nb++] = (SDL_MessageBoxButtonData){ picker ? 0 : SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, FOLDER, "Open folder" };
		buttons[nb++] = (SDL_MessageBoxButtonData){ 0, AGAIN, "Look again" };
		buttons[nb++] = (SDL_MessageBoxButtonData){ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, QUIT, "Quit" };
		SDL_MessageBoxData box = { SDL_MESSAGEBOX_INFORMATION | SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT, P.window,
			"Cyberworld Endless", text, nb, buttons, NULL };
		int hit = QUIT;
		if (SDL_ShowMessageBox(&box, &hit) != 0) return false;
		if (hit == QUIT || hit < 0) return false;
		if (hit == CHOOSE) {
			char path[1024] = "";
			if (!choose_file(path, sizeof path)) continue;
			if (!rom_load_file(path, msg, msglen)) continue;
			/* keep a copy where the next start looks first */
			const char *base = strrchr(path, '/');
			char to[1400];
			snprintf(to, sizeof to, "%s/%s", rom_dir, base ? base + 1 : path);
			if (strcmp(to, path) && access(to, F_OK) != 0 && !copy_file(path, to))
				fprintf(stderr, "could not copy the ROM to %s; it is used from %s\n", to, path);
			return true;
		}
		if (hit == FOLDER) {
			char *argv[] = { "xdg-open", (char *)rom_dir, NULL };
			run(argv, NULL, 0);
		}
		if (scan(msg, msglen)) return true;
	}
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

	enum { LATER, ADD };
	SDL_MessageBoxButtonData buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, ADD, "Add to menu" },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, LATER, "Not now" },
	};
	char text[1400];
	snprintf(text, sizeof text,
		"Add Cyberworld Endless to your application menu?\n\n"
		"It will start this AppImage:\n%s\n\nIf you move the file, start it once from its new place.", appimage);
	wrap(text, sizeof text, 60);
	SDL_MessageBoxData box = { SDL_MESSAGEBOX_INFORMATION | SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT, P.window,
		"Cyberworld Endless", text, 2, buttons, NULL };
	int hit = LATER;
	if (SDL_ShowMessageBox(&box, &hit) != 0) return;
	if (hit != ADD) {
		FILE *d = fopen(declined, "w");
		if (d) fclose(d);
		return;
	}
	make_dirs(apps);
	copy_icons(appdir, share);
	if (!write_entry(file, appimage)) { fprintf(stderr, "could not write %s\n", file); return; }
	if (on_path("update-desktop-database")) {
		char *argv[] = { "update-desktop-database", apps, NULL };
		run(argv, NULL, 0);
	}
	printf("added %s\n", file);
}

#else
typedef int desktop_unused;
#endif
