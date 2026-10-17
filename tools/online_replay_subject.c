/* Subject driver. Only the independent Python comparator can accept a run. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "online_frame_pair_internal.h"
#include "online_link_script.h"
#include "online_replay_execute.h"
#include "online_replay_observe.h"
#include "online_replay_snapshot.h"

#define POINT_COUNT 6
typedef struct {
	char name[32];
	unsigned frame, round;
	OnlineReplaySnapshot *snapshot;
	uint64_t take_ns;
} Point;
typedef struct {
	Point points[POINT_COUNT];
	const char *directory;
	unsigned restores;
	uint64_t restore_ns, replay_ns;
	bool failed, bounded_omission;
} Subject;

static uint64_t nanoseconds(void) {
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return (uint64_t)now.tv_sec * UINT64_C(1000000000) + (uint64_t)now.tv_nsec;
}

static bool read_points(Subject *s, const char *path) {
	static const char *const names[] = {"custom", "held-charge", "before-release", "after-damage", "nonidle-cable", "flash-change"};
	FILE *file = fopen(path, "r");
	if (!file) return false;
	bool ok = true;
	for (unsigned i = 0; ok && i < POINT_COUNT; ++i) {
		Point *p = &s->points[i];
		ok = fscanf(file, "%31s %u %u", p->name, &p->frame, &p->round) == 3 &&
			!strcmp(p->name, names[i]) && p->frame + 8 < online_link_full_frames() && p->round <= 4000000;
	}
	char extra[2];
	ok = ok && fscanf(file, "%1s", extra) == EOF;
	fclose(file);
	return ok;
}

static void take_point(OnlineReplayExecutor *e, Point *p) {
	uint64_t start = nanoseconds();
	p->snapshot = online_replay_snapshot_take(e);
	p->take_ns = nanoseconds() - start;
}

static bool take_boundary(OnlineReplayExecutor *e, void *context) {
	Subject *s = context;
	for (unsigned i = 0; i < POINT_COUNT; ++i) {
		Point *p = &s->points[i];
		if (p->round && !p->snapshot && p->frame == e->pair->frame && p->round == e->round) {
			take_point(e, p);
			if (!p->snapshot) s->failed = true;
		}
	}
	return false;
}

static bool record_noop(OnlineReplayExecutor *e, Subject *s) {
	char path[1024];
	if (snprintf(path, sizeof path, "%s/noop.jsonl", s->directory) >= (int)sizeof path) return false;
	FILE *out = fopen(path, "w");
	if (!out) return false;
	bool ok = true;
	for (;;) {
		for (unsigned i = 0; i < POINT_COUNT; ++i) {
			Point *p = &s->points[i];
			if (!p->round && p->frame == e->pair->frame) { take_point(e, p); if (!p->snapshot) ok = false; }
		}
		ok = ok && online_replay_observe(e->pair, out);
		if (!ok || e->pair->frame == online_link_full_frames()) break;
		ok = online_replay_execute_frame(e, take_boundary, s) && !s->failed;
	}
	if (fclose(out)) ok = false;
	for (unsigned i = 0; i < POINT_COUNT; ++i) ok = ok && s->points[i].snapshot != NULL;
	return ok;
}

static bool restore(OnlineReplayExecutor *e, Subject *s, const Point *p, OnlineReplayFault fault) {
	uint64_t start = nanoseconds();
	bool ok = online_replay_snapshot_restore(e, p->snapshot, fault);
	s->restore_ns += nanoseconds() - start;
	++s->restores;
	return ok;
}

static bool write_case(OnlineReplayExecutor *e, Subject *s, const Point *p, const char *label, unsigned horizon) {
	char path[1024];
	if (p->round) {
		if (snprintf(path, sizeof path, "%s/%s-immediate.json", s->directory, label) >= (int)sizeof path) return false;
		FILE *immediate = fopen(path, "w");
		if (!immediate) return false;
		bool ok = online_replay_write_boundary(e, immediate);
		if (fclose(immediate)) ok = false;
		if (!ok) return false;
	}
	if (snprintf(path, sizeof path, "%s/%s.jsonl", s->directory, label) >= (int)sizeof path) return false;
	FILE *out = fopen(path, "w");
	if (!out) return false;
	bool ok = p->round || online_replay_observe(e->pair, out);
	while (ok && e->pair->frame < p->frame + horizon) {
		uint64_t start = nanoseconds();
		ok = online_replay_execute_frame(e, NULL, NULL);
		s->replay_ns += nanoseconds() - start;
		ok = ok && online_replay_observe(e->pair, out);
	}
	if (fclose(out)) ok = false;
	return ok;
}

static bool run_cases(OnlineReplayExecutor *e, Subject *s) {
	static const unsigned horizons[] = {1, 4, 8};
	for (unsigned i = 0; i < POINT_COUNT; ++i)
		for (unsigned h = 0; h < sizeof horizons / sizeof *horizons; ++h)
			for (unsigned repeat = 1; repeat <= 2; ++repeat) {
				char label[64];
				snprintf(label, sizeof label, "%s-%u-%u", s->points[i].name, horizons[h], repeat);
				if (!restore(e, s, &s->points[i], REPLAY_RESTORE_ALL) ||
					!write_case(e, s, &s->points[i], label, horizons[h])) return false;
			}
	return true;
}

static bool record_omission_stop(OnlineReplayExecutor *e, Subject *s) {
	OnlineFramePair *p = e->pair;
	unsigned side = (unsigned)p->running;
	if (!e->in_frame || e->at_check || !e->battle || side >= 2 ||
		p->core[side]->frameCounter(p->core[side]) - p->start_frame[side] - e->relative != ONLINE_FRAME_LOOKAHEAD) return false;
	char path[1024];
	if (snprintf(path, sizeof path, "%s/omit-cable-stop.json", s->directory) >= (int)sizeof path) return false;
	FILE *out = fopen(path, "w");
	if (!out) return false;
	/* Observe the actual executor rejection. No frame or trace record is
	 * advanced, repeated or filled in after the failed execution step. */
	fprintf(out, "{\"reason\":\"frame-lead\",\"cursor\":{\"stage\":\"before-step\",\"frame\":%u,\"round\":%u,"
		"\"targets\":[%u,%u],\"relative\":%u,\"battle\":true,\"next_side\":%u,\"keys\":[",
		p->frame, e->round, e->target[0], e->target[1], e->relative, side);
	for (unsigned ahead = 0; ahead < ONLINE_FRAME_LOOKAHEAD; ++ahead)
		fprintf(out, "%s[%u,%u]", ahead ? "," : "", online_frame_script_key(p->frame + ahead, 0),
			online_frame_script_key(p->frame + ahead, 1));
	fputs("]},\"state\":", out);
	bool ok = online_replay_observe(p, out);
	fputs("}\n", out);
	if (fclose(out)) ok = false;
	return ok;
}

static bool run_faults(OnlineReplayExecutor *e, Subject *s) {
	static const char *const names[] = {"omit-cable", "omit-flash", "credit", "receive-word", "event-delay"};
	bool complete = true;
	for (unsigned i = 0; i < sizeof names / sizeof *names; ++i) {
		Point *p = &s->points[i == 1 ? 5 : 4];
		if (!restore(e, s, p, REPLAY_RESTORE_ALL)) return false;
		/* Cross the native flash/cable changes before omitting or corrupting
		 * restoration. This executes the mutation; it never edits trace files. */
		unsigned advance = i == 0 ? 1 : 8;
		while (e->pair->frame < p->frame + advance)
			if (!online_replay_execute_frame(e, NULL, NULL)) return false;
		if (!restore(e, s, p, (OnlineReplayFault)(i + 1))) return false;
		if (!write_case(e, s, p, names[i], 8)) {
			if (!i && record_omission_stop(e, s)) s->bounded_omission = true;
			else complete = false;
		}
	}
	return complete;
}

static bool write_statistics(Subject *s) {
	char path[1024];
	if (snprintf(path, sizeof path, "%s/statistics.json", s->directory) >= (int)sizeof path) return false;
	FILE *file = fopen(path, "w");
	if (!file) return false;
	fputs("{\"snapshots\":[", file);
	for (unsigned i = 0; i < POINT_COUNT; ++i)
		fprintf(file, "%s{\"point\":\"%s\",\"bytes\":%zu,\"take_ns\":%llu}", i ? "," : "", s->points[i].name,
			online_replay_snapshot_bytes(s->points[i].snapshot), (unsigned long long)s->points[i].take_ns);
	fprintf(file, "],\"restores\":%u,\"restore_ns\":%llu,\"replay_ns\":%llu}\n", s->restores,
		(unsigned long long)s->restore_ns, (unsigned long long)s->replay_ns);
	return fclose(file) == 0;
}

int main(int argc, char **argv) {
	if (argc != 5) { fprintf(stderr, "usage: %s ROM BOOT_STATE SELECTION OUTPUT_DIRECTORY\n", argv[0]); return 2; }
	Subject subject = {.directory = argv[4]};
	if (!read_points(&subject, argv[3])) return 2;
	char prefix[1024];
	if (snprintf(prefix, sizeof prefix, "%s/noop", argv[4]) >= (int)sizeof prefix) return 2;
	OnlineFramePair *pair = online_frame_pair_open(argv[1], argv[2], prefix);
	if (!pair) return 1;
	OnlineReplayExecutor executor;
	online_replay_executor_init(&executor, pair);
	bool ok = record_noop(&executor, &subject);
	if (fclose(pair->trace)) ok = false;
	pair->trace = fopen("/dev/null", "w");
	ok = ok && pair->trace && run_cases(&executor, &subject) && run_faults(&executor, &subject);
	if (!write_statistics(&subject)) ok = false;
	online_frame_pair_close(pair);
	for (unsigned i = 0; i < POINT_COUNT; ++i) online_replay_snapshot_free(subject.points[i].snapshot);
	return ok ? (subject.bounded_omission ? 3 : 0) : 1;
}
