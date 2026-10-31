/* What the PS5's native-app runtime (its system libraries and the
 * template's clean-room libc.prx: docker/Dockerfile.ps5) lacks of the C
 * library that PacBrew's OpenSSL, libcurl, libzstd and libiconv call: built on
 * what the system has where the game's paths reach them (time, pipes,
 * setjmp), and empty answers for those it never takes there (a terminal's
 * password, QUIC's batched sends, a user's home, an interface's name).
 * And what the SDK's stubs give only from modules the system does not load
 * into a game's process, where a call into one is a call to nothing (the
 * Makefile's PS5_LOADED): the name lookups and two string functions, which
 * only libScePosixForWebKit exports, made here on libSceNet's resolver (as
 * the PS5's own games resolve); readlink, libkernel_sys's alone; and the
 * keyboard, the text dialog and the random bytes SDL and the game ask for.
 * And the folders' reading and two of fcntl's settings on a socket, which
 * a game's process is refused. All of them the game's own, never exported (the Makefile builds the file
 * hidden): the template's converter publishes nothing of an app's. */
#ifdef CW_PS5
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <langinfo.h>
#include <fnmatch.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

/* (BSD's setjmp without the signal mask, the system's own reached by a
 * jump, which leaves the caller's frame for it to keep) */
__asm__(".globl _setjmp\n.type _setjmp,@function\n_setjmp:\n\tjmp setjmp@PLT\n"
	".globl _longjmp\n.type _longjmp,@function\n_longjmp:\n\tjmp longjmp@PLT\n");

/* (the system's gmtime, copied out: the certificates' dates and curl's) */
struct tm *gmtime_r(const time_t *when, struct tm *out) {
	struct tm *shared = gmtime(when);
	if (!shared) return NULL;
	*out = *shared;
	return out;
}

/* (curl's wake-up pipe) */
int pipe2(int *fds, int flags) {
	if (pipe(fds) < 0) return -1;
	for (int i = 0; i < 2; ++i) {
		if (flags & O_NONBLOCK) fcntl(fds[i], F_SETFL, fcntl(fds[i], F_GETFL) | O_NONBLOCK);
		if (flags & O_CLOEXEC) fcntl(fds[i], F_SETFD, FD_CLOEXEC);
	}
	return 0;
}

int getpwuid_r(uid_t uid, struct passwd *entry, char *buf, size_t size, struct passwd **result) {
	(void)uid;
	(void)entry;
	(void)buf;
	(void)size;
	*result = NULL;
	return ENOENT;
}

int dladdr(const void *restrict address, Dl_info *restrict info) {
	(void)address;
	(void)info;
	return 0;
}

ssize_t sendmmsg(int s, struct mmsghdr *restrict messages, size_t count, int flags) {
	(void)s;
	(void)messages;
	(void)count;
	(void)flags;
	errno = ENOSYS;
	return -1;
}

ssize_t recvmmsg(int s, struct mmsghdr *restrict messages, size_t count, int flags, const struct timespec *restrict wait) {
	(void)s;
	(void)messages;
	(void)count;
	(void)flags;
	(void)wait;
	errno = ENOSYS;
	return -1;
}

unsigned int if_nametoindex(const char *name) {
	(void)name;
	return 0;
}

/* (iconv's locale: the C locale's, whose characters are one byte each) */
char *nl_langinfo(nl_item item) {
	static char utf8[] = "UTF-8", none[] = "";
	return item == CODESET ? utf8 : none;
}

int ___mb_cur_max(void) { return 1; }

/* ---- the name lookups, on libSceNet's resolver (IPv4: the PS5's own) ---- */

int sceNetPoolCreate(const char *name, int size, int flags);
int sceNetPoolDestroy(int pool);
int sceNetResolverCreate(const char *name, int pool, int flags);
int sceNetResolverStartNtoa(int resolver, const char *host, struct in_addr *out, int timeout, int retry, int flags);
int sceNetResolverDestroy(int resolver);
int sceNetInetPton(int family, const char *text, void *out);
const char *sceNetInetNtop(int family, const void *address, char *out, unsigned int size);

/* (a pool and a resolver each lookup, as curl's, the Net Square's and the
 * statistics' threads may each ask at once) */
static int resolve(const char *host, struct in_addr *out) {
	if (sceNetInetPton(AF_INET, host, out) == 1) return 0;
	int pool = sceNetPoolCreate("cw-resolve", 16 * 1024, 0);
	if (pool < 0) return -1;
	int resolver = sceNetResolverCreate("cw-resolve", pool, 0);
	int got = resolver < 0 ? -1 : sceNetResolverStartNtoa(resolver, host, out, 0, 0, 0);
	if (resolver >= 0) sceNetResolverDestroy(resolver);
	sceNetPoolDestroy(pool);
	return got < 0 ? -1 : 0;
}

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **result) {
	if (hints && hints->ai_family != AF_UNSPEC && hints->ai_family != AF_INET) return EAI_FAMILY;
	struct in_addr address;
	if (!node || resolve(node, &address) < 0) return EAI_NONAME;
	/* (the entry and its address in one block, which freeaddrinfo frees) */
	struct one { struct addrinfo info; struct sockaddr_in in; } *r = calloc(1, sizeof *r);
	if (!r) return EAI_MEMORY;
	r->in.sin_len = sizeof r->in;
	r->in.sin_family = AF_INET;
	r->in.sin_port = htons((uint16_t)(service ? atoi(service) : 0));
	r->in.sin_addr = address;
	r->info.ai_family = AF_INET;
	r->info.ai_socktype = hints && hints->ai_socktype ? hints->ai_socktype : SOCK_STREAM;
	r->info.ai_protocol = hints ? hints->ai_protocol : 0;
	r->info.ai_addrlen = sizeof r->in;
	r->info.ai_addr = (struct sockaddr *)&r->in;
	*result = &r->info;
	return 0;
}

void freeaddrinfo(struct addrinfo *info) {
	while (info) {
		struct addrinfo *next = info->ai_next;
		free(info);
		info = next;
	}
}

const char *gai_strerror(int error) {
	(void)error;
	return "the name could not be looked up";
}

struct hostent *gethostbyname(const char *name) {
	(void)name;
	return NULL;
}

int getnameinfo(const struct sockaddr *address, socklen_t size, char *host, size_t host_size, char *service,
	size_t service_size, int flags) {
	(void)size;
	(void)flags;
	if (address->sa_family != AF_INET) return EAI_FAMILY;
	const struct sockaddr_in *in = (const struct sockaddr_in *)(const void *)address;
	if (host && host_size && !sceNetInetNtop(AF_INET, &in->sin_addr, host, (unsigned int)host_size)) return EAI_FAIL;
	if (service && service_size) snprintf(service, service_size, "%u", (unsigned)ntohs(in->sin_port));
	return 0;
}

/* ---- two string functions and readlink ---- */

/* (stars, question marks and plain characters: what OpenSSL's and curl's
 * paths here ask of it) */
int fnmatch(const char *pattern, const char *string, int flags) {
	if (*pattern == '*') {
		while (*pattern == '*') ++pattern;
		if (!*pattern) return 0;
		for (; *string; ++string)
			if (!fnmatch(pattern, string, flags)) return 0;
		return FNM_NOMATCH;
	}
	if (!*pattern) return *string ? FNM_NOMATCH : 0;
	if (!*string || (*pattern != '?' && *pattern != *string)) return FNM_NOMATCH;
	return fnmatch(pattern + 1, string + 1, flags);
}

char *strcasestr(const char *haystack, const char *needle) {
	size_t n = strlen(needle);
	for (; *haystack; ++haystack)
		if (!strncasecmp(haystack, needle, n)) return (char *)(uintptr_t)haystack;
	return n ? NULL : (char *)(uintptr_t)haystack;
}

/* (the game's own path, cw_exe_path's, which goes without it) */
ssize_t readlink(const char *restrict path, char *restrict out, size_t size) {
	(void)path;
	(void)out;
	(void)size;
	errno = ENOSYS;
	return -1;
}

/* ---- the keyboard, the system's text dialog and random bytes ---- */

/* (their modules are not loaded into a game's process either, which leaves
 * SDL's calls into them calls to nothing: the game fell at SDL's video
 * start. No keyboard and no dialog here, as SDL takes their errors; the
 * game's own key grid takes text, src/online/online_field.c) */
int sceKeyboardInit(void);
int sceKeyboardOpen(int user, int type, int index, void *param);
int sceKeyboardClose(int handle);
int sceKeyboardReadState(int handle, void *state);
int sceImeDialogInit(const void *param, void *extended);
int sceImeDialogGetStatus(void);
int sceImeDialogGetResult(void *result);
int sceImeDialogTerm(void);
int sceRandomGetRandomNumber(void *out, size_t size);

int sceKeyboardInit(void) { return -1; }
int sceKeyboardOpen(int user, int type, int index, void *param) {
	(void)user;
	(void)type;
	(void)index;
	(void)param;
	return -1;
}
int sceKeyboardClose(int handle) {
	(void)handle;
	return 0;
}
int sceKeyboardReadState(int handle, void *state) {
	(void)handle;
	(void)state;
	return -1;
}
int sceImeDialogInit(const void *param, void *extended) {
	(void)param;
	(void)extended;
	return -1;
}
int sceImeDialogGetStatus(void) { return 0; }
int sceImeDialogGetResult(void *result) {
	(void)result;
	return -1;
}
int sceImeDialogTerm(void) { return 0; }

/* (the kernel's, as OpenSSL takes its own) */
int sceRandomGetRandomNumber(void *out, size_t size) {
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) return -1;
	ssize_t got = read(fd, out, size);
	close(fd);
	return got == (ssize_t)size ? 0 : -1;
}

/* ---- the folders, read with the kernel's getdents ---- */

/* (libc's opendir is refused in a game's process, EPERM, where open reads
 * the same folder: the ROMs' places, the saves, the backups. The internal
 * drive's folders come in blocks of 64 KB, which getdents fills whole or
 * refuses, EINVAL) */
struct _dirdesc {
	int fd;
	int at, end;
	_Alignas(8) char buf[64 * 1024];
};

DIR *opendir(const char *path) {
	int fd = open(path, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
	if (fd < 0) return NULL;
	DIR *dir = calloc(1, sizeof *dir);
	if (!dir) {
		close(fd);
		errno = ENOMEM;
		return NULL;
	}
	dir->fd = fd;
	return dir;
}

struct dirent *readdir(DIR *dir) {
	for (;;) {
		if (dir->at >= dir->end) {
			int got = getdents(dir->fd, dir->buf, (int)sizeof dir->buf);
			if (got <= 0) return NULL;
			dir->at = 0;
			dir->end = got;
		}
		struct dirent *entry = (struct dirent *)(void *)(dir->buf + dir->at);
		if (!entry->d_reclen) {
			dir->end = 0;
			return NULL;
		}
		dir->at += entry->d_reclen;
		if (entry->d_fileno) return entry;
	}
}

void rewinddir(DIR *dir) {
	lseek(dir->fd, 0, SEEK_SET);
	dir->at = dir->end = 0;
}

int closedir(DIR *dir) {
	int result = close(dir->fd);
	free(dir);
	return result;
}

/* ---- fcntl on a socket (the Makefile's --wrap) ---- */

/* (refused a game's process on a socket: close-on-exec, nothing to a game
 * that never execs, and O_NONBLOCK, which the socket's own option sets;
 * curl asks for both on each connection, and gave up at the first) */
#define SO_NBIO 0x1200

int __real_fcntl(int fd, int cmd, ...);
int __wrap_fcntl(int fd, int cmd, ...);

int __wrap_fcntl(int fd, int cmd, ...) {
	va_list ap;
	va_start(ap, cmd);
	intptr_t arg = va_arg(ap, intptr_t);   /* (an int or a pointer, as the command takes) */
	va_end(ap);
	int result = __real_fcntl(fd, cmd, arg);
	if (result != -1) return result;
	int saved = errno;
	if (cmd == F_SETFD) return 0;
	int on = ((int)arg & O_NONBLOCK) != 0;
	if (cmd == F_SETFL && setsockopt(fd, SOL_SOCKET, SO_NBIO, &on, sizeof on) == 0) return 0;
	errno = saved;
	return -1;
}

/* (zstd's tracing hooks, weak in PacBrew's libzstd, which curl's decoding
 * brings: none here, so its tracing stays off; the template's converter
 * takes no symbol the system's stubs do not give) */
unsigned long long ZSTD_trace_decompress_begin(const void *context);
void ZSTD_trace_decompress_end(unsigned long long trace, const void *what);
unsigned long long ZSTD_trace_decompress_begin(const void *context) {
	(void)context;
	return 0;
}
void ZSTD_trace_decompress_end(unsigned long long trace, const void *what) {
	(void)trace;
	(void)what;
}
#endif
