#include "online_frame_wire.h"

#include <errno.h>
#include <poll.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

uint64_t online_frame_now_ms(void) {
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return (uint64_t)now.tv_sec * 1000 + (uint64_t)now.tv_nsec / 1000000;
}

void online_frame_packet(OnlineFramePacket *p, unsigned kind, unsigned frame, unsigned side) {
	memset(p, 0, sizeof *p);
	p->words[FRAME_MAGIC] = ONLINE_FRAME_MAGIC;
	p->words[FRAME_KIND] = kind;
	p->words[FRAME_SEQUENCE] = frame;
	p->words[FRAME_SIDE] = side;
}

static bool transfer(int fd, uint8_t *bytes, bool sending, unsigned timeout_ms) {
	uint64_t deadline = online_frame_now_ms() + timeout_ms;
	size_t done = 0;
	while (done < ONLINE_FRAME_BYTES) {
		uint64_t now = online_frame_now_ms();
		if (now >= deadline) { errno = ETIMEDOUT; return false; }
		struct pollfd p = { .fd = fd, .events = sending ? POLLOUT : POLLIN };
		int ready = poll(&p, 1, (int)(deadline - now));
		if (ready < 0 && errno == EINTR) continue;
		if (ready <= 0) { if (!ready) errno = ETIMEDOUT; return false; }
		ssize_t count = sending ? send(fd, bytes + done, ONLINE_FRAME_BYTES - done, MSG_NOSIGNAL | MSG_DONTWAIT) :
			recv(fd, bytes + done, ONLINE_FRAME_BYTES - done, MSG_DONTWAIT);
		if (count < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) continue;
		if (count <= 0) { if (!count) errno = ECONNRESET; return false; }
		done += (size_t)count;
	}
	return true;
}

bool online_frame_send(int fd, const OnlineFramePacket *p, unsigned timeout_ms) {
	uint8_t bytes[ONLINE_FRAME_BYTES];
	for (unsigned i = 0; i < ONLINE_FRAME_WORDS; ++i)
		for (unsigned byte = 0; byte < 4; ++byte) bytes[i * 4 + byte] = (uint8_t)(p->words[i] >> (byte * 8));
	return transfer(fd, bytes, true, timeout_ms);
}

bool online_frame_receive(int fd, OnlineFramePacket *p, unsigned timeout_ms) {
	uint8_t bytes[ONLINE_FRAME_BYTES];
	if (!transfer(fd, bytes, false, timeout_ms)) return false;
	for (unsigned i = 0; i < ONLINE_FRAME_WORDS; ++i) {
		p->words[i] = 0;
		for (unsigned byte = 0; byte < 4; ++byte) p->words[i] |= (uint32_t)bytes[i * 4 + byte] << (byte * 8);
	}
	if (p->words[FRAME_MAGIC] != ONLINE_FRAME_MAGIC || p->words[FRAME_KIND] < ONLINE_FRAME_HELLO ||
		p->words[FRAME_KIND] > ONLINE_FRAME_STOP || p->words[FRAME_SIDE] > 1) {
		errno = EPROTO;
		return false;
	}
	return true;
}

bool online_frame_queue_push(OnlineFrameQueue *q, unsigned side, unsigned frame, unsigned keys) {
	if (side > 1 || frame != q->next[side] || frame < q->consumed || frame - q->consumed >= ONLINE_FRAME_QUEUE || keys > 0x3FFu)
		return false;
	unsigned slot = frame % ONLINE_FRAME_QUEUE;
	q->keys[slot][side] = (uint16_t)keys;
	q->ready[slot] |= (uint8_t)(1u << side);
	++q->next[side];
	return true;
}

bool online_frame_queue_pop(OnlineFrameQueue *q, uint16_t keys[2]) {
	unsigned slot = q->consumed % ONLINE_FRAME_QUEUE;
	if (q->ready[slot] != 3) return false;
	memcpy(keys, q->keys[slot], sizeof q->keys[slot]);
	q->ready[slot] = 0;
	++q->consumed;
	return true;
}
