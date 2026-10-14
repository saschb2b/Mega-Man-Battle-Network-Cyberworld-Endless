#include "online_link_ipc.h"
#include "online_link_ipc_wire.h"

#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include <mgba/core/core.h>
#include <mgba/internal/gba/gba.h>
#include <mgba/internal/gba/io.h>
#include <mgba/internal/gba/sio/lockstep.h>

/* Only these numeric cable fields cross the socket. Core memory and pointers
 * are never transmitted. Inherited peer nodes are inert driver shadows. */
enum { ATTACHED, PHASE, CYCLES, MULTI, NORMAL, RECV = 5, NRECV = 9,
	MODE = 13, SIOCNT = 15, CREDIT = 17, WAITING = 19, FRAME, TRANSFERS,
	HASH_LOW, HASH_HIGH, SAMPLE = 24, RECEIVED = 32, FINISHED = 36, SLEEP, WAKE, POSTED };

typedef struct {
	struct GBASIOLockstep *cable;
	struct mCore *core;
	const struct OnlineLinkIpcConfig *config;
	int fd;
	unsigned side, sequence, start_frame, frame, transfers, waiting;
	int32_t credit[2];
	uint64_t hash;
	bool finished, sleeping, posted;
	struct LinkIpcPacket packet;
} Child;

typedef struct {
	const struct OnlineLinkIpcConfig *config;
	struct OnlineLinkIpcResult *result;
	struct LinkIpcPacket state, request[2];
	int fd[2], owner;
	unsigned sequence[2];
	uint16_t pending_mask[2], pending_bits[2];
	bool pending[2], acknowledge[2], sleeping[2], wake_slave, injected, delay_enabled;
	uint64_t started, blocked, blocked_start, last_progress;
	FILE *trace;
} Parent;

static int32_t signed_word(uint32_t value) {
	int32_t result;
	memcpy(&result, &value, sizeof result);
	return result;
}

static void cable_pack(struct GBASIOLockstep *cable, uint32_t *w) {
	w[ATTACHED] = (unsigned)cable->d.attached;
	w[PHASE] = (unsigned)cable->d.transferActive;
	w[CYCLES] = (uint32_t)cable->d.transferCycles;
	w[MULTI] = (unsigned)cable->attachedMulti;
	w[NORMAL] = (unsigned)cable->attachedNormal;
	for (unsigned i = 0; i < 4; ++i) {
		w[RECV + i] = cable->multiRecv[i];
		w[NRECV + i] = cable->normalRecv[i];
	}
	for (unsigned i = 0; i < 2; ++i) {
		w[MODE + i] = (unsigned)cable->players[i]->mode;
		w[SIOCNT + i] = cable->players[i]->d.p->siocnt;
	}
}

static void cable_unpack(struct GBASIOLockstep *cable, const uint32_t *w) {
	cable->d.attached = (int)w[ATTACHED];
	cable->d.transferActive = (enum mLockstepPhase)w[PHASE];
	cable->d.transferCycles = signed_word(w[CYCLES]);
	cable->attachedMulti = (int)w[MULTI];
	cable->attachedNormal = (int)w[NORMAL];
	for (unsigned i = 0; i < 4; ++i) {
		cable->multiRecv[i] = (uint16_t)w[RECV + i];
		cable->normalRecv[i] = w[NRECV + i];
	}
	for (unsigned i = 0; i < 2; ++i) {
		cable->players[i]->mode = (enum GBASIOMode)w[MODE + i];
		cable->players[i]->d.p->siocnt = (uint16_t)w[SIOCNT + i];
	}
}

static void child_pack(Child *child) {
	uint32_t *w = child->packet.words;
	cable_pack(child->cable, w);
	for (unsigned i = 0; i < 2; ++i) w[CREDIT + i] = (uint32_t)child->credit[i];
	w[WAITING] = child->waiting;
	w[FRAME] = child->frame;
	w[TRANSFERS] = child->transfers;
	w[HASH_LOW] = (uint32_t)child->hash;
	w[HASH_HIGH] = (uint32_t)(child->hash >> 32);
	if (child->config->sample) child->config->sample(child->core, child->side, w + SAMPLE, child->config->context);
}

static void child_exchange(Child *child, unsigned kind, unsigned expected) {
	child->packet.kind = kind;
	child->packet.sequence = child->sequence;
	if (!link_ipc_send(child->fd, &child->packet, child->config->timeout_ms) ||
		!link_ipc_receive(child->fd, &child->packet, child->config->timeout_ms)) _exit(10);
	if (child->packet.kind == LINK_IPC_STOP) _exit(0);
	if (child->packet.kind != expected || child->packet.sequence != child->sequence) _exit(11);
}

static void child_lock(struct mLockstep *lock) {
	Child *child = lock->context;
	++child->sequence;
	child_pack(child);
	child_exchange(child, LINK_IPC_LOCK, LINK_IPC_GRANT);
	cable_unpack(child->cable, child->packet.words);
	for (unsigned i = 0; i < 2; ++i) child->credit[i] = signed_word(child->packet.words[CREDIT + i]);
	child->waiting = child->packet.words[WAITING];
	child->finished = child->cable->players[child->side]->transferFinished;
	child->sleeping = false;
	child->posted = false;
}

static void child_unlock(struct mLockstep *lock) {
	Child *child = lock->context;
	struct GBASIOLockstepNode *node = child->cable->players[child->side];
	bool finished = !child->finished && node->transferFinished && node->mode == SIO_MULTI;
	uint32_t words[4] = {0};
	if (finished) {
		for (unsigned i = 0; i < 4; ++i) {
			words[i] = ((struct GBA *)child->core->board)->memory.io[(REG_SIOMULTI0 / 2) + i];
			child->hash ^= words[i];
			child->hash *= UINT64_C(1099511628211);
		}
		++child->transfers;
	}
	child_pack(child);
	child->packet.words[FINISHED] = finished;
	child->packet.words[SLEEP] = child->sleeping;
	child->packet.words[POSTED] = child->posted;
	for (unsigned i = 0; i < 4; ++i) child->packet.words[RECEIVED + i] = words[i];
	child_exchange(child, LINK_IPC_COMMIT, LINK_IPC_ACK);
	/* Match pinned mGBA's threaded scheduler: a sleeping slave receives its
	 * posted cycle budget in nextEvent when the master wakes it. */
	if (child->packet.words[WAKE]) node->nextEvent += signed_word(child->packet.words[CREDIT + child->side]);
}

static bool child_signal(struct mLockstep *lock, unsigned mask) {
	Child *child = lock->context;
	if (child->side != 0) child->waiting &= ~mask;
	return true;
}
static bool child_wait(struct mLockstep *lock, unsigned mask) {
	((Child *)lock->context)->waiting = mask;
	return true;
}
static void child_add(struct mLockstep *lock, int id, int32_t cycles) {
	Child *child = lock->context;
	child->credit[id == 0 ? 1 : id] += cycles;
	if (!id) child->posted = true;
}
static int32_t child_use(struct mLockstep *lock, int id, int32_t cycles) {
	Child *child = lock->context;
	child->credit[id] -= cycles;
	if (child->credit[id] <= 0) child->sleeping = true;
	return child->credit[id];
}
static int32_t child_unused(struct mLockstep *lock, int id) {
	return ((Child *)lock->context)->credit[id];
}
static void child_unload(struct mLockstep *lock, int id) {
	((Child *)lock->context)->waiting &= ~(1u << (unsigned)id);
}

static void child_run(struct GBASIOLockstep *cable, struct mCore *core,
	const struct OnlineLinkIpcConfig *config, unsigned side, int fd) {
	Child child = { .cable = cable, .core = core, .config = config, .fd = fd,
		.side = side, .waiting = config->initial_waiting, .hash = UINT64_C(14695981039346656037) };
	memcpy(child.credit, config->initial_credit, sizeof child.credit);
	child.start_frame = core->frameCounter(core);
	cable->d.context = &child;
	cable->d.lock = child_lock;
	cable->d.unlock = child_unlock;
	cable->d.signal = child_signal;
	cable->d.wait = child_wait;
	cable->d.addCycles = child_add;
	cable->d.useCycles = child_use;
	cable->d.unusedCycles = child_unused;
	cable->d.unload = child_unload;
	if (config->frame) config->frame(core, side, 0, config->context);
	for (;;) {
		core->step(core);
		unsigned frame = core->frameCounter(core) - child.start_frame;
		if (frame == child.frame) continue;
		child.frame = frame;
		if (config->frame) config->frame(core, side, frame, config->context);
		/* Ensure deadline/progress checks also work after the link unloads. */
		child_lock(&cable->d);
		child_unlock(&cable->d);
	}
}

static void record_progress(Parent *parent, unsigned side, const uint32_t *w) {
	struct OnlineLinkIpcResult *r = parent->result;
	if (parent->trace && w[FRAME] != r->frames[side]) fprintf(parent->trace,
		"%u,%u,%u,%u,%u,%u,%u,%u,%u\n", side, w[SAMPLE + 5], w[SAMPLE + 2], w[SAMPLE],
		w[SAMPLE + 3] & 65535, w[SAMPLE + 3] >> 16, w[SAMPLE + 4], w[SAMPLE + 6], w[SAMPLE + 7]);
	if (w[FRAME] != r->frames[side] || w[TRANSFERS] != r->transfers[side]) parent->last_progress = link_ipc_now();
	r->frames[side] = w[FRAME];
	r->transfers[side] = w[TRANSFERS];
	r->transcript[side] = ((uint64_t)w[HASH_HIGH] << 32) | w[HASH_LOW];
	memcpy(r->sample[side], w + SAMPLE, sizeof r->sample[side]);
}

static bool accept_lock(Parent *p, unsigned side, const struct LinkIpcPacket *packet) {
	if (p->pending[side] || p->owner == (int)side || packet->sequence != p->sequence[side] + 1) return false;
	p->sequence[side] = packet->sequence;
	p->request[side] = *packet;
	p->pending[side] = true;
	record_progress(p, side, packet->words);
	return true;
}

static bool valid_state(const uint32_t *w) {
	return w[ATTACHED] == 2 && w[PHASE] <= TRANSFER_FINISHED && w[MULTI] <= 2 &&
		w[NORMAL] <= 2 && w[WAITING] <= 3 && w[FINISHED] <= 1 && w[SLEEP] <= 1 && w[POSTED] <= 1;
}

static void io_failure(Parent *parent) {
	parent->result->status = errno == ETIMEDOUT ? ONLINE_LINK_IPC_TIMEOUT :
		errno == EPROTO ? ONLINE_LINK_IPC_PROTOCOL : ONLINE_LINK_IPC_DISCONNECTED;
}

static bool accept_commit(Parent *p, unsigned side, const struct LinkIpcPacket *packet) {
	const uint32_t *w = packet->words;
	if (p->owner != (int)side || packet->sequence != p->sequence[side] || !valid_state(w)) return false;
	for (unsigned i = 0; i < 4; ++i) {
		bool write = (!side && p->state.words[PHASE] == TRANSFER_STARTING) ||
			(side && i == side && p->state.words[PHASE] == TRANSFER_STARTED);
		if (write || w[RECV + i] == p->state.words[RECV + i]) continue;
		p->result->status = ONLINE_LINK_IPC_DESYNC;
		return false;
	}
	if (w[FINISHED]) for (unsigned i = 0; i < 4; ++i) {
		if (w[RECEIVED + i] == p->state.words[RECV + i]) continue;
		p->result->status = ONLINE_LINK_IPC_DESYNC;
		return false;
	}
	unsigned other = side ^ 1;
	uint16_t changed = (uint16_t)(w[SIOCNT + other] ^ p->state.words[SIOCNT + other]);
	p->pending_mask[other] |= changed;
	p->pending_bits[other] = (uint16_t)((p->pending_bits[other] & ~changed) | (w[SIOCNT + other] & changed));
	bool transfer = p->state.words[PHASE] != TRANSFER_STARTED && w[PHASE] == TRANSFER_STARTED;
	memcpy(p->state.words, w, (WAITING + 1) * sizeof *w);
	record_progress(p, side, w);
	if (!p->delay_enabled && transfer && p->config->delay_ms &&
		p->result->frames[0] >= p->config->delay_after_frames && p->result->frames[1] >= p->config->delay_after_frames &&
		(!p->config->delay_ready || p->config->delay_ready(p->result->sample[0], p->result->sample[1]))) {
		p->delay_enabled = true;
		memcpy(p->result->delay_start_frames, p->result->frames, sizeof p->result->frames);
		memcpy(p->result->delay_start_transfers, p->result->transfers, sizeof p->result->transfers);
		p->result->delay_start_elapsed_ms = link_ipc_now() - p->started;
	}
	if (transfer && p->delay_enabled) {
		p->blocked_start = link_ipc_now();
		p->blocked = p->blocked_start + p->config->delay_ms;
		++p->result->delayed_transfers;
	}
	p->acknowledge[side] = true;
	p->sleeping[side] = side ? w[SLEEP] != 0 : w[WAITING] != 0;
	/* A posted budget is also a phase notification: upstream wakes a sleeping
	 * slave even when zero cycles were posted at a transfer boundary. */
	if (!side && w[POSTED] && p->acknowledge[1] && p->sleeping[1]) p->wake_slave = true;
	p->owner = -1;
	return true;
}

static bool acknowledge(Parent *p) {
	for (unsigned side = 0; side < 2; ++side) {
		if (!p->acknowledge[side]) continue;
		if (p->sleeping[side] && (side ? !p->wake_slave : p->state.words[WAITING] != 0)) continue;
		struct LinkIpcPacket ack = { .kind = LINK_IPC_ACK, .sequence = p->sequence[side] };
		ack.words[WAKE] = side && p->sleeping[side];
		ack.words[CREDIT + side] = p->state.words[CREDIT + side];
		if (!link_ipc_send(p->fd[side], &ack, p->config->timeout_ms)) { io_failure(p); return false; }
		p->acknowledge[side] = false;
		p->sleeping[side] = false;
		if (side) p->wake_slave = false;
	}
	return true;
}

static bool grant_lock(Parent *p, unsigned side) {
	uint32_t own_sio = p->request[side].words[SIOCNT + side];
	p->state.words[SIOCNT + side] = (own_sio & (uint16_t)~p->pending_mask[side]) | p->pending_bits[side];
	p->pending_mask[side] = p->pending_bits[side] = 0;
	struct LinkIpcPacket grant = p->state;
	grant.kind = LINK_IPC_GRANT;
	grant.sequence = p->sequence[side];
	if (!p->injected && p->config->corrupt_after && side == 1 &&
		p->result->transfers[0] >= p->config->corrupt_after && grant.words[PHASE] == TRANSFER_FINISHED) {
		grant.words[RECV] ^= 1;
		p->injected = true;
	}
	if (!link_ipc_send(p->fd[side], &grant, p->config->timeout_ms)) { io_failure(p); return false; }
	p->pending[side] = false;
	p->owner = (int)side;
	return true;
}

static bool finish_or_timeout(Parent *p, uint64_t now) {
	struct OnlineLinkIpcResult *r = p->result;
	if (now - p->started >= p->config->wall_limit_ms) { r->status = ONLINE_LINK_IPC_DEADLINE; return true; }
	if (now - p->last_progress >= p->config->timeout_ms) { r->status = ONLINE_LINK_IPC_TIMEOUT; return true; }
	if (r->frames[0] < p->config->frames || r->frames[1] < p->config->frames || p->state.words[PHASE] != TRANSFER_IDLE) return false;
	r->status = r->transfers[0] == r->transfers[1] && r->transcript[0] == r->transcript[1]
		? ONLINE_LINK_IPC_COMPLETE : ONLINE_LINK_IPC_DESYNC;
	return true;
}

static void coordinate(Parent *p) {
	unsigned next = 0;
	for (;;) {
		uint64_t now = link_ipc_now();
		if (finish_or_timeout(p, now)) break;
		if (p->blocked && now >= p->blocked) {
			p->result->applied_delay_ms += now - p->blocked_start;
			p->blocked = 0;
		}
		if (!p->blocked && !acknowledge(p)) return;
		if (p->owner < 0 && !p->blocked) {
			for (unsigned k = 0; k < 2; ++k) {
				unsigned side = (next + k) % 2;
				if (!p->pending[side] || (!side && p->state.words[WAITING])) continue;
				if (!grant_lock(p, side)) return;
				next = side ^ 1;
				break;
			}
		}
		if (!p->injected && p->config->disconnect_after && p->result->transfers[0] >= p->config->disconnect_after) {
			kill(p->result->pids[1], SIGKILL);
			p->injected = true;
		}
		struct pollfd fds[2] = { { .fd = p->fd[0], .events = POLLIN }, { .fd = p->fd[1], .events = POLLIN } };
		int ready = poll(fds, 2, 5);
		if (ready < 0 && errno == EINTR) continue;
		if (ready < 0) return;
		for (unsigned i = 0; i < 2; ++i) if (fds[i].revents) {
			struct LinkIpcPacket packet;
			if (!link_ipc_receive(p->fd[i], &packet, p->config->timeout_ms)) {
				io_failure(p);
				return;
			}
			bool ok = packet.kind == LINK_IPC_LOCK ? accept_lock(p, i, &packet) :
				packet.kind == LINK_IPC_COMMIT && accept_commit(p, i, &packet);
			if (!ok) return;
		}
	}
}

static void reap_children(Parent *p) {
	for (unsigned i = 0; i < 2; ++i) close(p->fd[i]);
	for (unsigned i = 0; i < 2; ++i) {
		int status;
		pid_t child = p->result->pids[i];
		if (waitpid(child, &status, WNOHANG) == 0) {
			kill(child, SIGTERM);
			while (waitpid(child, &status, 0) < 0 && errno == EINTR) {}
		}
	}
}

bool online_link_ipc_run(struct GBASIOLockstep *cable, struct mCore *cores[2],
	const struct OnlineLinkIpcConfig *config, struct OnlineLinkIpcResult *result) {
	memset(result, 0, sizeof *result);
	result->status = ONLINE_LINK_IPC_PROTOCOL;
	if (!cable || !cores[0] || !cores[1] || !config->frames || !config->timeout_ms ||
		!config->wall_limit_ms || config->delay_ms >= config->timeout_ms || cable->d.attached != 2) return false;
	int sockets[2][2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets[0])) return false;
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets[1])) { close(sockets[0][0]); close(sockets[0][1]); return false; }
	fflush(NULL);
	for (unsigned side = 0; side < 2; ++side) {
		pid_t pid = fork();
		if (pid < 0) {
			for (unsigned i = 0; i < 2; ++i) { close(sockets[i][0]); close(sockets[i][1]); }
			if (side) { kill(result->pids[0], SIGTERM); waitpid(result->pids[0], NULL, 0); }
			return false;
		}
		if (!pid) {
			close(sockets[0][0]); close(sockets[1][0]); close(sockets[side ^ 1][1]);
			child_run(cable, cores[side], config, side, sockets[side][1]);
			_exit(12);
		}
		result->pids[side] = pid;
	}
	close(sockets[0][1]); close(sockets[1][1]);
	Parent parent = { .config = config, .result = result, .fd = {sockets[0][0], sockets[1][0]}, .owner = -1 };
	if (config->trace_path) {
		parent.trace = fopen(config->trace_path, "w");
		if (!parent.trace) { reap_children(&parent); return false; }
		fputs("core,frame,battle_timer,result,hp0,hp1,positions,menu,battle_state\n", parent.trace);
	}
	cable_pack(cable, parent.state.words);
	for (unsigned i = 0; i < 2; ++i) parent.state.words[CREDIT + i] = (uint32_t)config->initial_credit[i];
	parent.state.words[WAITING] = config->initial_waiting;
	parent.started = parent.last_progress = link_ipc_now();
	coordinate(&parent);
	if (result->status != ONLINE_LINK_IPC_COMPLETE) fprintf(stderr,
		"IPC stop: phase=%u waiting=%u credit=%d,%d owner=%d pending=%u,%u ack=%u,%u sleep=%u,%u sequence=%u,%u\n",
		parent.state.words[PHASE], parent.state.words[WAITING], signed_word(parent.state.words[CREDIT]),
		signed_word(parent.state.words[CREDIT + 1]), parent.owner, parent.pending[0], parent.pending[1],
		parent.acknowledge[0], parent.acknowledge[1], parent.sleeping[0], parent.sleeping[1], parent.sequence[0], parent.sequence[1]);
	result->elapsed_ms = link_ipc_now() - parent.started;
	if (parent.blocked) result->applied_delay_ms += link_ipc_now() - parent.blocked_start;
	if (parent.trace) fclose(parent.trace);
	reap_children(&parent);
	return result->status == ONLINE_LINK_IPC_COMPLETE;
}

const char *online_link_ipc_status(enum OnlineLinkIpcStatus status) {
	static const char *const names[] = { "complete", "deadline", "disconnected", "desync", "protocol", "timeout" };
	return status <= ONLINE_LINK_IPC_TIMEOUT ? names[status] : "unknown";
}
