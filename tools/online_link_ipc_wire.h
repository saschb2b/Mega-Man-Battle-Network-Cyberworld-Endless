#ifndef ONLINE_LINK_IPC_WIRE_H
#define ONLINE_LINK_IPC_WIRE_H

#include <stdbool.h>
#include <stdint.h>

#define LINK_IPC_WORDS 40

enum LinkIpcMessage { LINK_IPC_LOCK = 1, LINK_IPC_GRANT, LINK_IPC_COMMIT, LINK_IPC_ACK, LINK_IPC_STOP };
struct LinkIpcPacket { uint32_t kind, sequence, words[LINK_IPC_WORDS]; };
uint64_t link_ipc_now(void);
bool link_ipc_send(int fd, const struct LinkIpcPacket *packet, unsigned timeout_ms);
bool link_ipc_receive(int fd, struct LinkIpcPacket *packet, unsigned timeout_ms);

#endif
