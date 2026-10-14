/* ROM-free checks for the bounded serial research channel. */
#include "online_link_ipc_wire.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int fail(const char *name) { fprintf(stderr, "IPC wire failed: %s (errno=%d)\n", name, errno); return 1; }

int main(void) {
	int pair[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair)) return fail("socketpair");
	struct LinkIpcPacket sent = { .kind = LINK_IPC_LOCK, .sequence = UINT32_C(0xfedcba98) }, received;
	for (unsigned i = 0; i < LINK_IPC_WORDS; ++i) sent.words[i] = UINT32_C(0xf0000000) + i;
	if (!link_ipc_send(pair[0], &sent, 100) || !link_ipc_receive(pair[1], &received, 100) ||
		received.kind != sent.kind || received.sequence != sent.sequence ||
		memcmp(received.words, sent.words, sizeof sent.words)) return fail("roundtrip");
	uint32_t oversized[] = {htonl(UINT32_C(0x43574950)), htonl(1), htonl(LINK_IPC_LOCK), htonl(1), htonl(1048576)};
	if (write(pair[0], oversized, sizeof oversized) != sizeof oversized ||
		link_ipc_receive(pair[1], &received, 100) || errno != EPROTO) return fail("oversize rejected before payload");
	oversized[1] = htonl(2);
	oversized[4] = htonl(LINK_IPC_WORDS * 4);
	if (write(pair[0], oversized, sizeof oversized) != sizeof oversized ||
		link_ipc_receive(pair[1], &received, 100) || errno != EPROTO) return fail("unknown version");
	oversized[1] = htonl(1);
	oversized[2] = htonl(999);
	if (write(pair[0], oversized, sizeof oversized) != sizeof oversized ||
		link_ipc_receive(pair[1], &received, 100) || errno != EPROTO) return fail("unknown opcode");
	uint64_t before = link_ipc_now();
	if (link_ipc_receive(pair[1], &received, 20) || errno != ETIMEDOUT || link_ipc_now() - before > 500) return fail("bounded idle timeout");
	if (write(pair[0], oversized, 3) != 3) return fail("partial header");
	close(pair[0]);
	if (link_ipc_receive(pair[1], &received, 100) || errno != ECONNRESET) return fail("EOF");
	close(pair[1]);
	puts("IPC wire: roundtrip, oversized framing, unknown version/opcode, timeout and truncated EOF passed");
	return 0;
}
