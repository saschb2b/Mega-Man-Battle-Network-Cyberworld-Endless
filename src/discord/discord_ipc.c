/* discord_ipc.h: each desktop's pipe; the other builds have none. */
#include "discord_ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PIPES 10   /* discord-ipc-0 to -9: Discord takes the first free, a second client the next */

#if defined(DISCORD_IPC) && defined(_WIN32)

/* ---- Windows: a named pipe ---- */

#include <windows.h>

static HANDLE handle = INVALID_HANDLE_VALUE;

bool discord_ipc_open(void) {
	for (int i = 0; i < PIPES && handle == INVALID_HANDLE_VALUE; ++i) {
		char name[32];
		snprintf(name, sizeof name, "\\\\.\\pipe\\discord-ipc-%d", i);
		handle = CreateFileA(name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	}
	return handle != INVALID_HANDLE_VALUE;
}

bool discord_ipc_send(const void *data, size_t n) {
	DWORD wrote = 0;
	if (handle == INVALID_HANDLE_VALUE) return false;
	if (WriteFile(handle, data, (DWORD)n, &wrote, NULL) && wrote == (DWORD)n) return true;
	discord_ipc_close();
	return false;
}

long discord_ipc_recv(void *buf, size_t n) {
	DWORD there = 0, got = 0;
	if (handle == INVALID_HANDLE_VALUE) return -1;
	/* (a peek first: a ReadFile on an empty pipe would wait) */
	if (!PeekNamedPipe(handle, NULL, 0, NULL, &there, NULL)) {
		discord_ipc_close();
		return -1;
	}
	if (!there) return 0;
	if (!ReadFile(handle, buf, there < (DWORD)n ? there : (DWORD)n, &got, NULL)) {
		discord_ipc_close();
		return -1;
	}
	return (long)got;
}

void discord_ipc_close(void) {
	if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
	handle = INVALID_HANDLE_VALUE;
}

long discord_ipc_pid(void) { return (long)GetCurrentProcessId(); }

#elif defined(DISCORD_IPC)

/* ---- Linux and macOS: a Unix socket ---- */

#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0   /* (macOS: SO_NOSIGPIPE on the socket instead) */
#endif

static int sock = -1;

/* A socket connected to `path`, or -1 */
static int connect_to(const char *path) {
	struct sockaddr_un a;
	if (strlen(path) >= sizeof a.sun_path) return -1;
	int s = socket(AF_UNIX, SOCK_STREAM, 0);
	if (s < 0) return -1;
	memset(&a, 0, sizeof a);
	a.sun_family = AF_UNIX;
	memcpy(a.sun_path, path, strlen(path) + 1);
	fcntl(s, F_SETFD, FD_CLOEXEC);
#ifdef SO_NOSIGPIPE
	int one = 1;
	setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof one);
#endif
	if (connect(s, (struct sockaddr *)&a, sizeof a) < 0) {
		close(s);
		return -1;
	}
	/* (connected, it never waits: a read finds what is there, or nothing) */
	fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK);
	return s;
}

/* The folders Discord's sockets stand in: the runtime folder ($XDG_RUNTIME_DIR,
 * Linux's), the temporary one ($TMPDIR, macOS's per user), /tmp; and in
 * each, the Flatpak's and the Snap's Discord's own */
bool discord_ipc_open(void) {
	static const char *const vars[] = { "XDG_RUNTIME_DIR", "TMPDIR", "TMP", "TEMP" };
	static const char *const subs[] = { "", "app/com.discordapp.Discord/", "snap.discord/", ".flatpak/dev.vencord.Vesktop/xdg-run/" };
	const char *bases[5];
	int nb = 0;
	for (size_t v = 0; v < sizeof vars / sizeof *vars; ++v) {
		const char *b = getenv(vars[v]);
		if (b && b[0]) bases[nb++] = b;
	}
	bases[nb++] = "/tmp";
	for (int b = 0; b < nb && sock < 0; ++b)
		for (size_t s = 0; s < sizeof subs / sizeof *subs && sock < 0; ++s)
			for (int i = 0; i < PIPES && sock < 0; ++i) {
				char path[512];
				size_t len = strlen(bases[b]);
				snprintf(path, sizeof path, "%s%s%sdiscord-ipc-%d", bases[b], len && bases[b][len - 1] == '/' ? "" : "/", subs[s], i);
				sock = connect_to(path);
			}
	return sock >= 0;
}

bool discord_ipc_send(const void *data, size_t n) {
	const char *p = data;
	if (sock < 0) return false;
	while (n) {
		ssize_t k = send(sock, p, n, MSG_NOSIGNAL);
		if (k <= 0 && errno == EINTR) continue;
		/* (a full socket: Discord is not reading, and the frame cannot
		 * wait half sent) */
		if (k <= 0) {
			discord_ipc_close();
			return false;
		}
		p += k;
		n -= (size_t)k;
	}
	return true;
}

long discord_ipc_recv(void *buf, size_t n) {
	if (sock < 0) return -1;
	ssize_t k = recv(sock, buf, n, 0);
	if (k > 0) return (long)k;
	if (k < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) return 0;
	discord_ipc_close();
	return -1;
}

void discord_ipc_close(void) {
	if (sock >= 0) close(sock);
	sock = -1;
}

long discord_ipc_pid(void) { return (long)getpid(); }

#else

/* ---- the builds without one ---- */

bool discord_ipc_open(void) { return false; }

bool discord_ipc_send(const void *data, size_t n) {
	(void)data;
	(void)n;
	return false;
}

long discord_ipc_recv(void *buf, size_t n) {
	(void)buf;
	(void)n;
	return -1;
}

void discord_ipc_close(void) {}

long discord_ipc_pid(void) { return 0; }

#endif
