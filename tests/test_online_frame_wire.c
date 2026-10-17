#include "online_frame_wire.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

static void queues(void) {
	OnlineFrameQueue queue = {0};
	uint16_t keys[2];
	assert(!online_frame_queue_pop(&queue, keys));
	assert(!online_frame_queue_push(&queue, 2, 0, 1));
	assert(!online_frame_queue_push(&queue, 0, 1, 1));
	assert(!online_frame_queue_push(&queue, 0, 0, 1024));
	for (unsigned frame = 0; frame < ONLINE_FRAME_QUEUE; ++frame)
		assert(online_frame_queue_push(&queue, 0, frame, frame));
	assert(!online_frame_queue_push(&queue, 0, ONLINE_FRAME_QUEUE, 1));
	assert(!online_frame_queue_push(&queue, 0, 0, 1));
	for (unsigned frame = 0; frame < ONLINE_FRAME_QUEUE; ++frame) {
		assert(online_frame_queue_push(&queue, 1, frame, frame + 1));
		assert(online_frame_queue_pop(&queue, keys));
		assert(keys[0] == frame && keys[1] == frame + 1);
	}
	assert(online_frame_queue_push(&queue, 0, ONLINE_FRAME_QUEUE, 7));
	assert(online_frame_queue_push(&queue, 1, ONLINE_FRAME_QUEUE, 8));
	assert(online_frame_queue_pop(&queue, keys) && keys[0] == 7 && keys[1] == 8);
}

static void framing(void) {
	int fd[2];
	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == 0);
	OnlineFramePacket sent, received;
	online_frame_packet(&sent, ONLINE_FRAME_INPUT, 12, 1);
	sent.words[FRAME_VALUE] = 0x3FF;
	assert(online_frame_send(fd[0], &sent, 100));
	assert(online_frame_receive(fd[1], &received, 100));
	assert(received.words[FRAME_SEQUENCE] == 12 && received.words[FRAME_VALUE] == 0x3FF);
	sent.words[FRAME_MAGIC] ^= 1;
	assert(online_frame_send(fd[0], &sent, 100));
	assert(!online_frame_receive(fd[1], &received, 100) && errno == EPROTO);
	online_frame_packet(&sent, 99, 0, 0);
	assert(online_frame_send(fd[0], &sent, 100));
	assert(!online_frame_receive(fd[1], &received, 100) && errno == EPROTO);
	uint8_t byte = 0;
	assert(send(fd[0], &byte, 1, 0) == 1);
	assert(!online_frame_receive(fd[1], &received, 5) && errno == ETIMEDOUT);
	close(fd[0]);
	assert(!online_frame_receive(fd[1], &received, 100) && errno == ECONNRESET);
	close(fd[1]);
}

int main(void) {
	queues();
	framing();
	puts("online frame wire: bounded queues, framing, timeout and EOF passed");
	return 0;
}
