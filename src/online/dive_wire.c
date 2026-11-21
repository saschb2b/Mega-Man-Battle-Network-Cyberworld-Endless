/* The shared dive's messages on the wire (dive_wire.h). */
#include "dive_wire.h"

#include <string.h>

#include "online_wire.h"

static uint16_t get16(const uint8_t *i) { return (uint16_t)((unsigned)i[0] << 8 | i[1]); }
static uint32_t get32(const uint8_t *i) { return (uint32_t)get16(i) << 16 | get16(i + 2); }
static int16_t gets16(const uint8_t *i) { return (int16_t)((int32_t)get16(i) - (get16(i) & 0x8000u ? 0x10000 : 0)); }
static void puts16(uint8_t *o, int v) { online_put16(o, (uint16_t)(v & 0xFFFF)); }

static bool coordinate(int v) { return v >= -ONLINE_COORDINATE_LIMIT && v <= ONLINE_COORDINATE_LIMIT; }
static bool way_ok(DiveWay w) { return w.depth <= ONLINE_WEEK_LAYERS; }

/* a zero-padded name: one the Square takes, or (where `blank_ok`) none at all */
static bool name_get(char out[ONLINE_NAME_BYTES + 1], const uint8_t *in, bool blank_ok) {
	memcpy(out, in, ONLINE_NAME_BYTES);
	out[ONLINE_NAME_BYTES] = '\0';
	for (size_t k = strlen(out); k < ONLINE_NAME_BYTES; ++k)
		if (in[k]) return false;
	return (blank_ok && !out[0]) || online_wire_name(out);
}

static DiveWay way_get(const uint8_t *in) {
	DiveWay w = { in[0], in[1], in[2] };
	return w;
}

static void way_put(uint8_t *out, DiveWay w) {
	out[0] = w.depth; out[1] = w.side; out[2] = w.way;
}

/* ---- built ---- */

size_t dive_wire_enter(uint8_t out[ONLINE_DIVE_ENTER_BYTES], uint16_t week, uint32_t seed, DiveWay way) {
	if (!way.depth || !way_ok(way)) return 0;
	out[0] = ONLINE_DIVE; out[1] = ONLINE_DIVE_ENTER;
	online_put16(out + 2, week);
	online_put32(out + 4, seed);
	way_put(out + 8, way);
	return ONLINE_DIVE_ENTER_BYTES;
}

size_t dive_wire_leave(uint8_t out[ONLINE_DIVE_LEAVE_BYTES]) {
	out[0] = ONLINE_DIVE; out[1] = ONLINE_DIVE_LEAVE;
	return ONLINE_DIVE_LEAVE_BYTES;
}

size_t dive_wire_end(uint8_t out[ONLINE_DIVE_END_BYTES], unsigned how, int x, int y, int z, unsigned cause) {
	if ((how != ONLINE_DIVE_DELETED && how != ONLINE_DIVE_WON) || !coordinate(x) || !coordinate(y) || !coordinate(z) || cause > 0xFFFFu)
		return 0;
	out[0] = ONLINE_DIVE; out[1] = ONLINE_DIVE_END; out[2] = (uint8_t)how;
	puts16(out + 3, x); puts16(out + 5, y); puts16(out + 7, z);
	online_put16(out + 9, (uint16_t)cause);
	return ONLINE_DIVE_END_BYTES;
}

size_t dive_wire_found(uint8_t out[ONLINE_DIVE_FOUND_BYTES], unsigned index) {
	if (index > 255) return 0;
	out[0] = ONLINE_DIVE; out[1] = ONLINE_DIVE_FOUND; out[2] = (uint8_t)index;
	return ONLINE_DIVE_FOUND_BYTES;
}

size_t dive_wire_vitals(uint8_t out[ONLINE_DIVE_VITALS_BYTES], unsigned hp, unsigned max_hp) {
	if (!max_hp || max_hp > 9999 || hp > max_hp) return 0;
	out[0] = ONLINE_DIVE; out[1] = ONLINE_DIVE_VITALS;
	online_put16(out + 2, (uint16_t)hp);
	online_put16(out + 4, (uint16_t)max_hp);
	return ONLINE_DIVE_VITALS_BYTES;
}

size_t dive_wire_phrase(uint8_t out[ONLINE_DIVE_PHRASE_BYTES], unsigned phrase, int x, int y, int z) {
	if (phrase >= ONLINE_PHRASE_COUNT || !coordinate(x) || !coordinate(y) || !coordinate(z)) return 0;
	out[0] = ONLINE_DIVE; out[1] = ONLINE_DIVE_PHRASE; out[2] = (uint8_t)phrase;
	puts16(out + 3, x); puts16(out + 5, y); puts16(out + 7, z);
	return ONLINE_DIVE_PHRASE_BYTES;
}

size_t dive_wire_wave(uint8_t out[ONLINE_DIVE_WAVE_BYTES], uint32_t id) {
	if (!id) return 0;
	out[0] = ONLINE_DIVE; out[1] = ONLINE_DIVE_WAVE;
	online_put32(out + 2, id);
	return ONLINE_DIVE_WAVE_BYTES;
}

size_t dive_wire_split(uint8_t out[ONLINE_DIVE_SPLIT_BYTES], const DiveSplit *s) {
	if (!s->tenths || !s->level || s->level > ONLINE_BUSTING_MOST || !s->party || s->party > ONLINE_PARTY_MOST) return 0;
	out[0] = ONLINE_DIVE; out[1] = ONLINE_DIVE_SPLIT; out[2] = s->index;
	online_put16(out + 3, s->tenths);
	out[5] = s->level;
	out[6] = s->guardian ? ONLINE_SPLIT_GUARDIAN : 0;
	out[7] = s->party;
	return ONLINE_DIVE_SPLIT_BYTES;
}

size_t dive_wire_crew(uint8_t out[ONLINE_CREW_BYTES], unsigned action, uint32_t ref, unsigned value) {
	bool answer = action == ONLINE_CREW_JOIN || action == ONLINE_CREW_BUSY;
	if ((action != ONLINE_CREW_BATTLE && !answer) || value > 255 || (answer && (!ref || value))) return 0;
	memset(out, 0, ONLINE_CREW_BYTES);
	out[0] = ONLINE_CREW; out[1] = (uint8_t)action;
	online_put32(out + 2, ref);
	out[6] = (uint8_t)value;
	return ONLINE_CREW_BYTES;
}

size_t dive_wire_queue(uint8_t out[ONLINE_QUEUE_BYTES], unsigned action, DiveWay door) {
	if (action < ONLINE_QUEUE_JOIN || action > ONLINE_QUEUE_PASS || !way_ok(door) || (action == ONLINE_QUEUE_JOIN && !door.depth)) return 0;
	out[0] = ONLINE_QUEUE; out[1] = (uint8_t)action;
	way_put(out + 2, door);
	return ONLINE_QUEUE_BYTES;
}

size_t dive_wire_party(uint8_t out[ONLINE_PARTY_BYTES], unsigned kind, uint32_t battle, unsigned first, unsigned second) {
	if ((kind != ONLINE_PARTY_DAMAGE && kind != ONLINE_PARTY_HP) || !battle || first > 0xFFFFu || second > 0xFFFFu ||
	    (kind == ONLINE_PARTY_HP && first > second)) return 0;
	out[0] = (uint8_t)kind;
	online_put32(out + 1, battle);
	online_put16(out + 5, (uint16_t)first);
	online_put16(out + 7, (uint16_t)second);
	return ONLINE_PARTY_BYTES;
}

/* ---- read ---- */

bool dive_wire_room(const uint8_t *data, size_t size, DiveRoom *out) {
	if (size != ONLINE_DIVE_ROOM_BYTES || data[0] != ONLINE_DIVE_ROOM) return false;
	DiveRoom r = { .week = get16(data + 1), .way = way_get(data + 3), .instance = get16(data + 6), .divers = data[8] };
	if (!r.way.depth || !way_ok(r.way) || !r.instance || !r.divers || r.divers > ONLINE_MAX_PLAYERS) return false;
	*out = r;
	return true;
}

static bool mark_get(const uint8_t *e, DiveMark *m) {
	if (!name_get(m->name, e, false)) return false;
	m->x = gets16(e + 16); m->y = gets16(e + 18); m->z = gets16(e + 20);
	m->cause = get16(e + 22); m->how = e[24];
	return coordinate(m->x) && coordinate(m->y) && coordinate(m->z) && (m->how == ONLINE_DIVE_DELETED || m->how == ONLINE_DIVE_WON);
}

static bool phrase_get(const uint8_t *e, DivePhrase *p) {
	p->id = get32(e);
	if (!p->id || !name_get(p->name, e + 4, false)) return false;
	p->phrase = e[20];
	p->x = gets16(e + 21); p->y = gets16(e + 23); p->z = gets16(e + 25);
	p->waves = get16(e + 27);
	return p->phrase < ONLINE_PHRASE_COUNT && coordinate(p->x) && coordinate(p->y) && coordinate(p->z);
}

static bool split_get(const uint8_t *e, DiveSplit *s) {
	s->index = e[0]; s->party = e[1]; s->tenths = get16(e + 2); s->level = e[4];
	s->guardian = e[5] & ONLINE_SPLIT_GUARDIAN;
	return name_get(s->name, e + 6, false) && s->party >= 1 && s->party <= ONLINE_PARTY_MOST && s->tenths && s->level >= 1 &&
		s->level <= ONLINE_BUSTING_MOST && !(e[5] & ~ONLINE_SPLIT_GUARDIAN);
}

/* one entry of a DIVE_TRACES kind, its size; 0 a kind there is none of */
static size_t trace_size(unsigned kind) {
	switch (kind) {
	case ONLINE_TRACE_MARKS: return ONLINE_TRACE_MARK_BYTES;
	case ONLINE_TRACE_PHRASES: return ONLINE_TRACE_PHRASE_BYTES;
	case ONLINE_TRACE_FINDS: return ONLINE_TRACE_FIND_BYTES;
	case ONLINE_TRACE_OPENED: return 1;
	case ONLINE_TRACE_SPLITS: return ONLINE_TRACE_SPLIT_BYTES;
	case ONLINE_TRACE_GONE: return 4;
	default: return 0;
	}
}

static bool trace_get(DiveTraces *t, unsigned k, const uint8_t *e) {
	switch (t->kind) {
	case ONLINE_TRACE_MARKS: return mark_get(e, &t->u.marks[k]);
	case ONLINE_TRACE_PHRASES: return phrase_get(e, &t->u.phrases[k]);
	case ONLINE_TRACE_FINDS: t->u.finds[k].index = e[0]; return name_get(t->u.finds[k].name, e + 1, false);
	case ONLINE_TRACE_OPENED: t->u.opened[k] = e[0]; return true;
	case ONLINE_TRACE_SPLITS: return split_get(e, &t->u.splits[k]);
	default: t->u.gone[k] = get32(e); return t->u.gone[k] != 0;
	}
}

bool dive_wire_traces(const uint8_t *data, size_t size, DiveTraces *out) {
	if (size < ONLINE_TRACE_HEAD || size > ONLINE_MAX_MESSAGE || data[0] != ONLINE_DIVE_TRACES) return false;
	size_t each = trace_size(data[1]);
	if (!each || size != ONLINE_TRACE_HEAD + data[2] * each || data[2] * each > DIVE_TRACES_ROOM) return false;
	memset(out, 0, sizeof *out);
	out->kind = data[1];
	out->count = data[2];
	for (unsigned k = 0; k < out->count; ++k)
		if (!trace_get(out, k, data + ONLINE_TRACE_HEAD + k * each)) return false;
	return true;
}

bool dive_wire_place(const uint8_t *data, size_t size, DivePlace *out) {
	if (size != ONLINE_DIVE_PLACE_BYTES || data[0] != ONLINE_DIVE_PLACE) return false;
	DivePlace p = { .index = data[1], .party = data[2], .tenths = get16(data + 3), .place = get16(data + 5), .count = get16(data + 7),
		.best = get16(data + 9) };
	if (!name_get(p.best_name, data + 11, false) || !p.party || p.party > ONLINE_PARTY_MOST || !p.tenths || !p.place ||
	    p.place > p.count || !p.best || p.best > p.tenths) return false;
	*out = p;
	return true;
}

bool dive_wire_feed(const uint8_t *data, size_t size, DiveFeed *out) {
	if (size != ONLINE_DIVE_FEED_BYTES || data[0] != ONLINE_DIVE_FEED) return false;
	DiveFeed f = { .kind = data[1], .way = way_get(data + 18), .cause = get16(data + 21), .party = data[23] };
	if (!name_get(f.name, data + 2, false) || f.kind < ONLINE_FEED_DELETED || f.kind > ONLINE_FEED_FLARE || !f.way.depth ||
	    !way_ok(f.way) || f.party > ONLINE_PARTY_MOST || (f.kind == ONLINE_FEED_GUARDIAN && f.party < 2)) return false;
	*out = f;
	return true;
}

bool dive_wire_divers(const uint8_t *data, size_t size, DiveDivers *out) {
	if (size < ONLINE_DIVERS_HEAD || data[0] != ONLINE_DIVERS || data[3] > ONLINE_DIVERS_MOST ||
	    size != ONLINE_DIVERS_HEAD + (size_t)data[3] * ONLINE_DIVERS_ENTRY_BYTES || get16(data + 1) < data[3]) return false;
	memset(out, 0, sizeof *out);
	out->total = get16(data + 1);
	out->count = data[3];
	for (unsigned k = 0; k < out->count; ++k) {
		const uint8_t *e = data + ONLINE_DIVERS_HEAD + k * ONLINE_DIVERS_ENTRY_BYTES;
		DiverEntry *d = &out->e[k];
		d->depth = e[16]; d->act = e[17]; d->flags = e[18];
		if (!name_get(d->name, e, false) || !d->depth || d->depth > ONLINE_WEEK_LAYERS || !d->act || d->act > 4 ||
		    d->flags & ~ONLINE_DIVERS_FIGHTING) return false;
	}
	return true;
}

bool dive_wire_crew_state(const uint8_t *data, size_t size, CrewState *out) {
	if (size != ONLINE_CREW_STATE_BYTES || data[0] != ONLINE_CREW_STATE || data[1] < ONLINE_CREW_FORMED || data[1] > ONLINE_CREW_VITALS)
		return false;
	memset(out, 0, sizeof *out);
	out->stage = data[1];
	out->other = get32(data + 2);
	out->value = data[6];
	for (unsigned k = 0; k < ONLINE_PARTY_MOST; ++k) {
		const uint8_t *e = data + 7 + k * ONLINE_CREW_MEMBER_BYTES;
		CrewMember m = { .session = get32(e), .hp = get16(e + 20), .max_hp = get16(e + 22), .depth = e[24] };
		/* (v24: his HP, max HP and layer; none past the members) */
		if (!name_get(m.name, e + 4, !m.session) || m.hp > m.max_hp || m.max_hp > 9999 || (!m.session && (m.name[0] || m.max_hp || m.depth)))
			return false;
		if (!m.session) continue;
		if (k != out->count) return false;      /* (the members first, zeros after them) */
		out->m[out->count++] = m;
	}
	return true;
}

bool dive_wire_queue_state(const uint8_t *data, size_t size, QueueState *out) {
	if (size != ONLINE_QUEUE_STATE_BYTES || data[0] != ONLINE_QUEUE_STATE || data[1] < ONLINE_QUEUE_WAITING ||
	    data[1] > ONLINE_QUEUE_ALONE || data[6] > ONLINE_PARTY_MOST || data[5] > data[6] + (data[1] == ONLINE_QUEUE_ALONE)) return false;
	memset(out, 0, sizeof *out);
	out->stage = data[1];
	out->door = way_get(data + 2);
	out->place = data[5];
	out->count = data[6];
	if (!out->door.depth || !way_ok(out->door)) return false;
	for (unsigned k = 0; k < ONLINE_PARTY_MOST; ++k) {
		const uint8_t *e = data + 7 + k * ONLINE_QUEUE_ENTRY_BYTES;
		QueueEntry q = { .session = get32(e), .flags = e[20] };
		bool here = k < out->count;
		if (!name_get(q.name, e + 4, !here) || here != (q.session != 0) || q.flags & ~ONLINE_QUEUE_ARENA) return false;
		out->e[k] = q;
	}
	return true;
}

bool dive_wire_damage(const uint8_t *data, size_t size, unsigned *seat, uint32_t *battle, unsigned *damage, unsigned *hp) {
	if (size != 2 + ONLINE_PARTY_BYTES || data[0] != ONLINE_ARENA_SEAT || !data[1] || data[1] >= ONLINE_SEATS ||
	    data[2] != ONLINE_PARTY_DAMAGE || !get32(data + 3)) return false;
	*seat = data[1];
	*battle = get32(data + 3);
	*damage = get16(data + 7);
	*hp = get16(data + 9);
	return true;
}

bool dive_wire_party_hp(const uint8_t *data, size_t size, uint32_t *battle, unsigned *hp, unsigned *max_hp) {
	if (size != ONLINE_PARTY_BYTES || data[0] != ONLINE_PARTY_HP || !get32(data + 1) || get16(data + 5) > get16(data + 7)) return false;
	*battle = get32(data + 1);
	*hp = get16(data + 5);
	*max_hp = get16(data + 7);
	return true;
}

bool dive_wire_splits(const uint8_t *data, size_t size, WeekSplits *out) {
	if (size != ONLINE_WEEK_SPLITS_BYTES || data[0] != ONLINE_WEEK_SPLITS) return false;
	memset(out, 0, sizeof *out);
	out->week = get16(data + 1);
	for (unsigned k = 0; k < 4 * ONLINE_PARTY_MOST; ++k) {
		const uint8_t *e = data + 3 + k * 18;
		WeekSplit *s = &out->best[k / ONLINE_PARTY_MOST][k % ONLINE_PARTY_MOST];
		s->tenths = get16(e);
		if (!name_get(s->name, e + 2, !s->tenths) || (!s->tenths && s->name[0])) return false;
	}
	return true;
}

bool dive_wire_party_state(const uint8_t *data, size_t size, PartyState *out) {
	if (size != ONLINE_ARENA_STATE_BYTES || data[0] != ONLINE_ARENA_STATE) return false;
	PartyState s = { .stage = data[1], .battle = get32(data + 2), .other = get32(data + 6), .seat = data[26], .delay = data[27],
		.form = data[28] };
	for (unsigned k = 0; k < ONLINE_SEATS; ++k) s.seats[k] = get32(data + 29 + 4 * k);
	bool party = s.form == ONLINE_ARENA_PARTY || s.form == ONLINE_ARENA_LINKED;
	if (!s.battle || !party || s.seat >= ONLINE_SEATS) return false;
	if (s.stage == 3) {     /* START: its seats from 0, the recipient's among them */
		unsigned n = 0;
		while (n < ONLINE_SEATS && s.seats[n]) ++n;
		for (unsigned k = n; k < ONLINE_SEATS; ++k) if (s.seats[k]) return false;
		if (n < 2 || s.seat >= n || !s.delay) return false;
	} else if (s.stage != 4 && s.stage != ONLINE_ARENA_SEAT_GONE) return false;
	*out = s;
	return true;
}
