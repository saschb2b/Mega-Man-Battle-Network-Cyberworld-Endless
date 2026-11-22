/* dive_comings.h */
#include "dive_comings.h"

#include <string.h>

#include "bn6_fields.h"
#include "dive_crew.h"
#include "dive_link.h"
#include "dive_words.h"
#include "run.h"

#define GONE_LOOK_MS 600         /* a crewmate who went, looked for in the snapshot so long before he counts as gone */
#define AWAY_MOST 8              /* those whose signal went, kept for their return */

typedef struct {
	char name[ONLINE_NAME_BYTES + 1];
	uint32_t session;
	uint32_t at;                 /* when he went (pending), or when his line was said */
	bool pending;                /* went from the crew, not yet known gone from the layer */
} Away;

static struct {
	Away away[AWAY_MOST];
	int naway;
	struct { char name[ONLINE_NAME_BYTES + 1]; uint32_t at; } said[AWAY_MOST];
	int nsaid;
	CrewState full;              /* the last crew told full, while MegaMan stands near it (count 0 none) */
	bool full_said;
	char last[64];
} K;

void dive_comings_reset(void) { memset(&K, 0, sizeof K); }

/* a line for `name`: his signal gone where his last such was COMINGS_EVERY_MS ago or more (false: held back), his
 * return always (one for each gone said) */
static bool say(const char *name, const char *line, bool gone, uint32_t now) {
	int k = 0;
	while (k < K.nsaid && strcmp(K.said[k].name, name)) ++k;
	if (gone && k < K.nsaid && now - K.said[k].at < COMINGS_EVERY_MS) { fprintf(stderr, "dive: held back: %s\n", line); return false; }
	if (k == K.nsaid) {
		if (K.nsaid == AWAY_MOST) memmove(K.said, K.said + 1, sizeof K.said[0] * (AWAY_MOST - 1)), --K.nsaid;
		k = K.nsaid++;
		snprintf(K.said[k].name, sizeof K.said[k].name, "%s", name);
	}
	if (gone) K.said[k].at = now;
	snprintf(K.last, sizeof K.last, "%s", line);
	fprintf(stderr, "dive: said: %s\n", line);
	dive_link_line(line);
	return true;
}

static bool in_crew(const CrewState *c, uint32_t session) {
	for (int k = 0; k < c->count; ++k)
		if (c->m[k].session == session) return true;
	return false;
}

void dive_comings_crew(const CrewState *before, const CrewState *after, uint32_t me, uint32_t now) {
	for (int k = 0; k < before->count; ++k) {
		const CrewMember *m = &before->m[k];
		/* (gone on to another layer, or home between acts: no signal lost, nothing said) */
		if (in_crew(after, m->session) || !m->session || m->session == me) continue;
		if (m->where == ONLINE_WHERE_HOME || (m->depth && m->depth != (uint8_t)run.depth)) {
			fprintf(stderr, "dive: %s went from the crew %s\n", m->name, m->where == ONLINE_WHERE_HOME ? "home" : "to another layer");
			continue;
		}
		fprintf(stderr, "dive: %s went from the crew; looked for on the layer\n", m->name);
		if (K.naway == AWAY_MOST) memmove(K.away, K.away + 1, sizeof K.away[0] * (AWAY_MOST - 1)), --K.naway;
		Away *a = &K.away[K.naway++];
		snprintf(a->name, sizeof a->name, "%s", m->name);
		a->session = m->session;
		a->at = now;
		a->pending = true;
	}
	/* (in a crew: none full for MegaMan to be told of) */
	if (after->count) memset(&K.full, 0, sizeof K.full);
}

void dive_comings_full(const CrewState *full) {
	bool same = K.full.count == full->count;
	for (int k = 0; same && k < full->count; ++k) same = K.full.m[k].session == full->m[k].session;
	K.full = *full;
	if (same && K.full_said) return;
	K.full_said = true;
	if (full->count) {
		snprintf(K.last, sizeof K.last, "%s", dive_crew_full_words(full->m[0].name));
		fprintf(stderr, "dive: said: %s\n", K.last);
		dive_link_line(K.last);
	}
}

bool dive_comings_full_has(uint32_t session) { return in_crew(&K.full, session); }

static const OnlinePeer *peer_named(const OnlineSnapshot *s, const char *name) {
	for (size_t i = 0; s->connected && i < s->peer_count; ++i)
		if (!strcmp(s->peers[i].name, name)) return &s->peers[i];
	return NULL;
}

void dive_comings_frame(const OnlineSnapshot *s, uint32_t now) {
	for (int k = 0; k < K.naway; ++k) {
		Away *a = &K.away[k];
		const OnlinePeer *p = peer_named(s, a->name);
		if (a->pending) {
			/* (judged once the snapshots after his going have come: still on the layer, he walked off, the plates' and
			 * the ring's to show; gone from it, his signal gone) */
			if (now - a->at < GONE_LOOK_MS) continue;
			if (p) { memmove(a, a + 1, sizeof *a * (size_t)(K.naway - k - 1)); --K.naway; --k; continue; }
			a->pending = false;
			/* (held back: his return unsaid too) */
			if (!say(a->name, dive_crew_gone_words(a->name), true, now)) { memmove(a, a + 1, sizeof *a * (size_t)(K.naway - k - 1)); --K.naway; --k; }
		} else if (p) {
			say(a->name, dive_crew_back_words(a->name), false, now);
			memmove(a, a + 1, sizeof *a * (size_t)(K.naway - k - 1));
			--K.naway;
			--k;
		}
	}
	/* (a full crew forgotten once none of it stands within reach: told again as MegaMan comes near it again) */
	bool near = false;
	for (int k = 0; !near && k < K.full.count; ++k) {
		for (size_t i = 0; s->connected && i < s->peer_count; ++i) {
			const OnlinePeer *p = &s->peers[i];
			long dx = p->x - bn6_player_x(), dy = p->y - bn6_player_y();
			if (p->id == K.full.m[k].session && dx * dx + dy * dy <= (long)DIVE_CREW_FAR * DIVE_CREW_FAR) near = true;
		}
	}
	if (K.full.count && !near) { memset(&K.full, 0, sizeof K.full); K.full_said = false; }
}

void dive_comings_describe(FILE *f) { fprintf(f, "comings %s\n", K.last[0] ? K.last : "none"); }
