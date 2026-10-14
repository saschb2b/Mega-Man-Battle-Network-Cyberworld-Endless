#include "online_link_ipc_wire.h"

#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>
#include <stddef.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define LINK_IPC_MAGIC UINT32_C(0x43574950)
#define LINK_IPC_VERSION 1

uint64_t link_ipc_now(void) {
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	return (uint64_t)time.tv_sec * 1000 + (uint64_t)time.tv_nsec / 1000000;
}

static bool transfer(int fd, void *data, size_t size, unsigned timeout, bool sending) {
	uint8_t *bytes = data;
	uint64_t deadline = link_ipc_now() + timeout;
	while (size) {
		uint64_t now = link_ipc_now();
		if (now >= deadline) { errno = ETIMEDOUT; return false; }
		struct pollfd poller = { .fd = fd, .events = sending ? POLLOUT : POLLIN };
		int ready = poll(&poller, 1, (int)(deadline - now));
		if (ready < 0 && errno == EINTR) continue;
		if (ready <= 0) { if (!ready) errno = ETIMEDOUT; return false; }
		ssize_t count = sending ? send(fd, bytes, size, MSG_NOSIGNAL) : recv(fd, bytes, size, 0);
		if (count < 0 && errno == EINTR) continue;
		if (count <= 0) { if (!count) errno = ECONNRESET; return false; }
		bytes += (size_t)count;
		size -= (size_t)count;
	}
	return true;
}

bool link_ipc_send(int fd, const struct LinkIpcPacket *packet, unsigned timeout) {
	uint32_t words[5 + LINK_IPC_WORDS] = {
		htonl(LINK_IPC_MAGIC), htonl(LINK_IPC_VERSION), htonl(packet->kind),
		htonl(packet->sequence), htonl(LINK_IPC_WORDS * 4)
	};
	for (unsigned i = 0; i < LINK_IPC_WORDS; ++i) words[i + 5] = htonl(packet->words[i]);
	return transfer(fd, words, sizeof words, timeout, true);
}

bool link_ipc_receive(int fd, struct LinkIpcPacket *packet, unsigned timeout) {
	uint32_t header[5], words[LINK_IPC_WORDS];
	if (!transfer(fd, header, sizeof header, timeout, false)) return false;
	if (ntohl(header[0]) != LINK_IPC_MAGIC || ntohl(header[1]) != LINK_IPC_VERSION ||
		ntohl(header[4]) != sizeof words || ntohl(header[2]) < LINK_IPC_LOCK ||
		ntohl(header[2]) > LINK_IPC_STOP) { errno = EPROTO; return false; }
	if (!transfer(fd, words, sizeof words, timeout, false)) return false;
	packet->kind = ntohl(header[2]);
	packet->sequence = ntohl(header[3]);
	for (unsigned i = 0; i < LINK_IPC_WORDS; ++i) packet->words[i] = ntohl(words[i]);
	return true;
}
