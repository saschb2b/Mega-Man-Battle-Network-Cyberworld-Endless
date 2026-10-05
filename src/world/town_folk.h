/* The townsfolk at a visit home (town_folk.c, docs/HOME.md: crowds that
 * move), for town.c. */
#ifndef CW_TOWN_FOLK_H
#define CW_TOWN_FOLK_H

#include <stdbool.h>
#include <stdint.h>

#include "town_lines.h"

/* Who stands at each of a town's places this visit (`who[i]` the folk of
 * place i: town_lines.c's own, or at home another's), and who is out. */
typedef struct {
	int who[MAX_FOLK];
	bool out[MAX_FOLK];
} FolkVisit;

/* This visit's townsfolk, from the town's `seed` and the run's depth: a
 * few out, and at `home` its standing people shuffled among their places
 * (a walker keeps his walk; the Mr.Prog and the robot dog keep theirs). */
void town_folk_visit(const TownLines *lines, uint32_t seed, bool home, FolkVisit *v);

#endif
