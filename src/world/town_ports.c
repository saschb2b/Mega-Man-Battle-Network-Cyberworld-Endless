/* town_ports.h, and the ports of town.h (docs/HOME.md, issue #86). After an
 * act the next act's ways are the town's ports: the landmark's the first,
 * then one of the original's own jack-in points or a check made a port for
 * each other way. Their cells, middles and names, and which one Lan stands
 * on. */
#include "town_ports.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "town.h"

#define PORT_CELLS 64
#define REGION_CELLS 256

/* a check made a port: the original's cells of it that are the port's */
static struct {
	int n;
	int x[REGION_CELLS], y[REGION_CELLS];
} region[TOWN_PORTS];

static struct {
	int n;
	int x[PORT_CELLS], y[PORT_CELLS], value[PORT_CELLS];
} P[TOWN_PORTS];
static const char *names[TOWN_PORTS];
static int open_ways = 1;

void ports_begin(const char *landmark, const char *second, const char *third) {
	memset(P, 0, sizeof P);
	memset(region, 0, sizeof region);
	names[0] = landmark;
	names[1] = second;
	names[2] = third;
}

bool ports_cell(int port, int x, int y, int value) {
	if (port < 1 || port >= TOWN_PORTS || P[port].n >= PORT_CELLS) return false;
	int i = P[port].n++;
	if (getenv("CYBERWORLD_TOWN_DEBUG")) fprintf(stderr, "town: port %d cell at %d,%d\n", port, x, y);
	P[port].x[i] = x;
	P[port].y[i] = y;
	P[port].value[i] = value;
	return true;
}

/* (cells as town.c counts them, rounded down) */
static int cell(int u) { return u >= 0 ? u / 8 : -((-u + 7) / 8); }

static bool in_region(int port, int cx, int cy) {
	for (int i = 0; i < region[port].n; ++i)
		if (region[port].x[i] == cx && region[port].y[i] == cy) return true;
	return false;
}

static bool beside_region(int port, int cx, int cy) {
	for (int i = 0; i < region[port].n; ++i)
		if (abs(region[port].x[i] - cx) <= 1 && abs(region[port].y[i] - cy) <= 1) return true;
	return false;
}

void ports_region(const CoordCell *c, int n, int port, int value, int sx, int sy) {
	if (port < 1 || port >= TOWN_PORTS) return;
	int best = -1;
	for (int i = 0; i < n; ++i) {
		int d = abs(cell(c[i].x) - sx) + abs(cell(c[i].y) - sy);
		if (c[i].value == value && (best < 0 || d < abs(cell(c[best].x) - sx) + abs(cell(c[best].y) - sy))) best = i;
	}
	if (best < 0) return;
	region[port].x[0] = cell(c[best].x);
	region[port].y[0] = cell(c[best].y);
	region[port].n = 1;
	/* (and every one joined to it, corner to corner) */
	for (bool grew = true; grew;) {
		grew = false;
		for (int i = 0; i < n && region[port].n < REGION_CELLS; ++i) {
			int cx = cell(c[i].x), cy = cell(c[i].y);
			if (c[i].value != value || in_region(port, cx, cy) || !beside_region(port, cx, cy)) continue;
			region[port].x[region[port].n] = cx;
			region[port].y[region[port].n++] = cy;
			grew = true;
		}
	}
}

bool ports_in_region(int port, int cx, int cy) { return port >= 1 && port < TOWN_PORTS && in_region(port, cx, cy); }

/* the port (1, 2) whose cell holds world (x, y), and the cell; -1 none */
static int cell_at(int x, int y, int *cell) {
	for (int k = 1; k < TOWN_PORTS; ++k)
		for (int i = 0; i < P[k].n; ++i)
			if (x >= P[k].x[i] && y >= P[k].y[i] && x < P[k].x[i] + 8 && y < P[k].y[i] + 8) {
				if (cell) *cell = i;
				return k;
			}
	return -1;
}

int ports_value(int x, int y, int value) {
	int i = 0, k = cell_at(x, y, &i);
	if (k < 0 || k < open_ways) return value;
	/* (closed: a check's ground no trigger at all, the original's own
	 * jack-in points the first way's as before) */
	return P[k].value[i] ? value : 0;
}

void town_ways(int open) { open_ways = open < 1 ? 1 : open > TOWN_PORTS ? TOWN_PORTS : open; }

int town_port_at(int x, int y) {
	if (town_on_port(x, y) && cell_at(x, y, NULL) < 0) return 0;
	return cell_at(x, y, NULL);
}

const char *town_port_name(int port) { return port >= 0 && port < TOWN_PORTS ? names[port] : NULL; }
