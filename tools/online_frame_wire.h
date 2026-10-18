/* Developer-only frame-input experiment. No game memory crosses this wire. */
#ifndef CW_ONLINE_FRAME_WIRE_H
#define CW_ONLINE_FRAME_WIRE_H

#include <stdbool.h>
#include <stdint.h>

#define ONLINE_FRAME_MAGIC UINT32_C(0x31465743)
#define ONLINE_FRAME_WORDS 24
#define ONLINE_FRAME_BYTES (ONLINE_FRAME_WORDS * 4)
#define ONLINE_FRAME_QUEUE 64

enum OnlineFrameKind { ONLINE_FRAME_HELLO = 1, ONLINE_FRAME_TICK, ONLINE_FRAME_INPUT,
	ONLINE_FRAME_STATE, ONLINE_FRAME_STOP };
enum OnlineFrameField { FRAME_MAGIC, FRAME_KIND, FRAME_SEQUENCE, FRAME_SIDE,
	FRAME_VALUE, FRAME_HASH_LOW, FRAME_HASH_HIGH, FRAME_SAMPLE = 7, FRAME_AGREES = 23 };

typedef struct { uint32_t words[ONLINE_FRAME_WORDS]; } OnlineFramePacket;
typedef struct {
	unsigned consumed, next[2];
	uint16_t keys[ONLINE_FRAME_QUEUE][2];
	uint8_t ready[ONLINE_FRAME_QUEUE];
} OnlineFrameQueue;

void online_frame_packet(OnlineFramePacket *packet, unsigned kind, unsigned frame, unsigned side);
bool online_frame_send(int fd, const OnlineFramePacket *packet, unsigned timeout_ms);
bool online_frame_receive(int fd, OnlineFramePacket *packet, unsigned timeout_ms);
bool online_frame_queue_push(OnlineFrameQueue *queue, unsigned side, unsigned frame, unsigned keys);
bool online_frame_queue_pop(OnlineFrameQueue *queue, uint16_t keys[2]);
uint64_t online_frame_now_ms(void);

#endif
