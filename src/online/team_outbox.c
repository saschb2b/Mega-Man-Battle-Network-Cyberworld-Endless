/* Two MegaMen a side: a partner's outbox (team_outbox.h). */
#include "team_outbox.h"

static uint16_t get16(const TeamMem *m, uint32_t at) {
	return (uint16_t)(m->read8(m->context, at) | m->read8(m->context, at + 1) << 8);
}

static void put16(const TeamMem *m, uint32_t at, uint16_t v) {
	m->write8(m->context, at, (uint8_t)v);
	m->write8(m->context, at + 1, (uint8_t)(v >> 8));
}

unsigned team_outbox_count(const TeamMem *m, uint32_t room) { return m->read8(m->context, room + TEAM_OUTBOX_COUNT); }

bool team_outbox_push(const TeamMem *m, uint32_t room, uint16_t value) {
	unsigned head = m->read8(m->context, room + TEAM_OUTBOX_HEAD) % TEAM_OUTBOX, n = team_outbox_count(m, room);
	if (n >= TEAM_OUTBOX) return false;
	put16(m, room + 2u * ((head + n) % TEAM_OUTBOX), value);
	m->write8(m->context, room + TEAM_OUTBOX_COUNT, (uint8_t)(n + 1));
	return true;
}

bool team_outbox_pop(const TeamMem *m, uint32_t room, uint16_t *value) {
	unsigned head = m->read8(m->context, room + TEAM_OUTBOX_HEAD) % TEAM_OUTBOX, n = team_outbox_count(m, room);
	if (!n) return false;
	*value = get16(m, room + 2u * head);
	m->write8(m->context, room + TEAM_OUTBOX_HEAD, (uint8_t)((head + 1) % TEAM_OUTBOX));
	m->write8(m->context, room + TEAM_OUTBOX_COUNT, (uint8_t)(n - 1));
	return true;
}
