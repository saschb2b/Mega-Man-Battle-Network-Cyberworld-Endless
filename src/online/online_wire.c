#include "online_wire.h"

#include <string.h>

void online_put16(uint8_t *out, uint16_t value) {
	out[0] = (uint8_t)(value >> 8); out[1] = (uint8_t)value;
}

void online_put32(uint8_t *out, uint32_t value) {
	out[0] = (uint8_t)(value >> 24); out[1] = (uint8_t)(value >> 16);
	out[2] = (uint8_t)(value >> 8); out[3] = (uint8_t)value;
}

static uint16_t get16(const uint8_t *in) { return (uint16_t)((unsigned)in[0] << 8 | in[1]); }
static uint32_t get32(const uint8_t *in) {
	return (uint32_t)in[0] << 24 | (uint32_t)in[1] << 16 | (uint32_t)in[2] << 8 | in[3];
}

static int32_t signed32(const uint8_t *in) {
	uint32_t value = get32(in);
	return (int32_t)((int64_t)(value & INT32_MAX) - (int64_t)(value >> 31) * 2147483648LL);
}

bool online_wire_name(const char *name) {
	size_t size = strlen(name);
	if (!size || size > ONLINE_NAME_BYTES) return false;
	for (size_t i = 0; i < size; ++i) {
		char c = name[i];
		bool alnum = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
		if (!alnum && (!i || (c != ' ' && c != '_' && c != '-'))) return false;
	}
	return true;
}

static bool identity_valid(const uint8_t *id) {
	for (unsigned i = 0; i < ONLINE_IDENTITY_BYTES; ++i) if (id[i]) return true;
	return false;
}

static bool read_peers(const uint8_t *data, size_t size, uint32_t now, OnlineSnapshot *snapshot) {
	if (size < 2 || data[1] >= ONLINE_MAX_PLAYERS || size != 2 + (size_t)data[1] * ONLINE_PEER_BYTES) return false;
	snapshot->peer_count = data[1];
	for (size_t i = 0; i < snapshot->peer_count; ++i) {
		const uint8_t *in = data + 2 + i * ONLINE_PEER_BYTES;
		OnlinePeer *peer = &snapshot->peers[i];
		peer->id = get32(in);
		memcpy(peer->identity, in + 4, ONLINE_IDENTITY_BYTES);
		peer->x = signed32(in + 20); peer->y = signed32(in + 24); peer->z = signed32(in + 28);
		peer->facing = in[32];
		memcpy(peer->name, in + 33, ONLINE_NAME_BYTES);
		peer->name[ONLINE_NAME_BYTES] = '\0';
		peer->roms = in[49];
		if ((peer->roms & ~ONLINE_ROMS_BN5) || !peer->id || peer->id == snapshot->player_id || peer->facing > 7 ||
		    !online_wire_name(peer->name) || !identity_valid(peer->identity)) return false;
		if (peer->x < -ONLINE_COORDINATE_LIMIT || peer->x > ONLINE_COORDINATE_LIMIT ||
		    peer->y < -ONLINE_COORDINATE_LIMIT || peer->y > ONLINE_COORDINATE_LIMIT ||
		    peer->z < -ONLINE_COORDINATE_LIMIT || peer->z > ONLINE_COORDINATE_LIMIT) return false;
		for (size_t j = 0; j < i; ++j)
			if (snapshot->peers[j].id == peer->id || !memcmp(snapshot->peers[j].identity, peer->identity, ONLINE_IDENTITY_BYTES)) return false;
	}
	++snapshot->position_serial;
	snapshot->position_ticks = now;
	return true;
}

static bool profile_valid(const OnlineProfile *p) {
	if (!p->max_hp || p->hp > p->max_hp || p->max_hp > 9999 || p->attack > 5 || p->speed > 5 || p->charge > 5 ||
	    p->souls > 63 || p->crosses > 31 || p->marks > 65535) return false;
	for (unsigned i = 0; i < ONLINE_BOARD_CELLS; ++i) if (p->board[i] > 6) return false;
	return true;
}

bool online_wire_profile(uint8_t out[ONLINE_PROFILE_BYTES + 1], const OnlineProfile *p) {
	if (!profile_valid(p)) return false;
	out[0] = ONLINE_PROFILE;
	online_put16(out + 1, p->hp); online_put16(out + 3, p->max_hp);
	out[5] = p->attack; out[6] = p->speed; out[7] = p->charge;
	online_put16(out + 8, p->souls); online_put16(out + 10, p->crosses); online_put16(out + 12, p->depth);
	out[14] = p->act; online_put16(out + 15, p->best_depth); online_put32(out + 17, p->marks);
	memcpy(out + 21, p->board, ONLINE_BOARD_CELLS);
	return true;
}

static bool read_card(const uint8_t *data, size_t size, OnlineSnapshot *snapshot) {
	if (size != 21 + ONLINE_PROFILE_BYTES || !get32(data + 1) || !identity_valid(data + 5)) return false;
	OnlineProfile p = { .hp = get16(data + 21), .max_hp = get16(data + 23),
		.attack = data[25], .speed = data[26], .charge = data[27],
		.souls = get16(data + 28), .crosses = get16(data + 30), .depth = get16(data + 32),
		.act = data[34], .best_depth = get16(data + 35), .marks = get32(data + 37) };
	memcpy(p.board, data + 41, ONLINE_BOARD_CELLS);
	if (!profile_valid(&p)) return false;
	snapshot->card = p;
	snapshot->card_player = get32(data + 1);
	memcpy(snapshot->card_identity, data + 5, ONLINE_IDENTITY_BYTES);
	++snapshot->card_serial;
	return true;
}

static bool read_welcome(const uint8_t *data, size_t size, OnlineSnapshot *snapshot) {
	if (size != ONLINE_WELCOME_BYTES) return false;
	uint32_t id = get32(data + 5);
	uint16_t channel = get16(data + 12), channels = get16(data + 30);
	if (!id || !get16(data + 9) || get16(data + 9) > 30 || !data[11] || data[11] > ONLINE_MAX_PLAYERS ||
	    !channel || channel > channels || channels > 256 || !identity_valid(data + 14)) return false;
	if (snapshot->connected && (id != snapshot->player_id || memcmp(snapshot->identity, data + 14, ONLINE_IDENTITY_BYTES))) return false;
	snapshot->seed = get32(data + 1); snapshot->player_id = id;
	snapshot->channel = channel; snapshot->channels = channels;
	memcpy(snapshot->identity, data + 14, ONLINE_IDENTITY_BYTES);
	snapshot->peer_count = 0;
	snapshot->connected = true;
	return true;
}

static bool read_status(const uint8_t *data, size_t size, OnlineSnapshot *snapshot) {
	if (size < 3 || size > 99) return false;
	char *out = data[0] == ONLINE_ERROR ? snapshot->error : snapshot->notice;
	for (size_t i = 3; i < size; ++i) {
		if (data[i] < 32 || data[i] >= 127) return false;
		out[i - 3] = (char)data[i];
	}
	out[size - 3] = '\0';
	if (data[0] == ONLINE_ERROR) {
		snapshot->error_code = get16(data + 1);
		return false;
	}
	++snapshot->notice_serial;
	return true;
}

bool online_wire_message(const uint8_t *data, size_t size, uint32_t now, OnlineSnapshot *snapshot) {
	if (!size || size > ONLINE_MAX_MESSAGE) return false;
	if (data[0] == ONLINE_WELCOME) return read_welcome(data, size, snapshot);
	if (data[0] == ONLINE_ERROR || data[0] == ONLINE_NOTICE) return read_status(data, size, snapshot);
	if (!snapshot->connected) return false;
	if (data[0] == ONLINE_SNAPSHOT) return read_peers(data, size, now, snapshot);
	if (data[0] == ONLINE_CARD) return read_card(data, size, snapshot);
	if (data[0] == ONLINE_PONG && size == 5) {
		snapshot->latency_ms = now - get32(data + 1);
		return snapshot->latency_ms < 15000;
	}
	if (data[0] == ONLINE_EVENT && size == 7 && get32(data + 1) && data[5] <= 1 &&
	    data[6] < (data[5] ? ONLINE_EMOTE_COUNT : ONLINE_PHRASE_COUNT)) {
		++snapshot->event_serial;
		snapshot->events[snapshot->event_serial % ONLINE_EVENT_HISTORY] =
			(OnlineEvent){ snapshot->event_serial, get32(data + 1), now, data[5], data[6] };
		return true;
	}
	return false;
}
