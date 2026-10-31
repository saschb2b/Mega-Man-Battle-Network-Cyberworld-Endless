/* What the PS5's native-app runtime (its system libraries and the
 * template's clean-room libc.prx: docker/Dockerfile.ps5) lacks of the C
 * library that PacBrew's OpenSSL, libcurl, libzstd and libiconv call: built on
 * what the system has where the game's paths reach them (time, pipes,
 * setjmp), and empty answers for those it never takes there (a terminal's
 * password, QUIC's batched sends, a user's home, an interface's name). */
#ifdef CW_PS5
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <langinfo.h>
#include <net/if.h>
#include <pwd.h>
#include <setjmp.h>
#include <stdlib.h>
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
