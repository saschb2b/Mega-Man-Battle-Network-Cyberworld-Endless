/* The run's economy, measured and planned (docs/ECONOMY.md, issue #154).
 *
 * A plan is five curves, each the share of the flat economy (the rolls as
 * they were before issue #154) a layer hands out on one lever, by how far
 * the run is through its net: the green Mystery Data laid, the chip a
 * green one holds, the good chips' weight in a chip's roll, the zenny a
 * find holds, and the finds leaned to the folder's codes. A curve rises
 * from its floor at the net's first act to 1 at its last on a logarithm
 * (econ_curve): scarce early, opening deeper, never past the flat plan's.
 * The opening plan is in play (econ_plan); the flat one, at 1 everywhere
 * and drawing nothing more, is the economy report's before. The report
 * (src/dev/economy_report.c) measures both and holds each to its bands.
 * ROM-free: tests/test_core.c checks the curves. */
#ifndef CW_ECONOMY_H
#define CW_ECONOMY_H

#include <stdbool.h>
#include <stdint.h>

#define ECON_ACTS 7              /* the report's: six acts and the Nest */
#define ECON_BUST_S 11           /* BN6's busting levels 1-10, and S */
#define ECON_BATTLES_LEAST 1     /* random battles a layer, as playtesters' run logs fight them (docs/ECONOMY.md) */
#define ECON_BATTLES_MOST 3
#define ECON_GOOD_TIER 2         /* a good chip: chip_pool.h's tier 2 or more (BN6's 3 stars up, every Mega and Giga) */
#define ECON_STRONG_POWER 100    /* a strong chip: a straight hit of 100 or more */

/* A curve: its share at the net's first act (1: flat), and how
 * soon it opens (the logarithm's bend; near 0 a straight line) */
typedef struct { double floor, bend; } EconCurve;
/* The levers a plan's curves move: the green Mystery Data a layer lays
 * (net_gen.c), the chip in a green one, kept or zenny in its place
 * (loot_mystery_roll), the good chips' weight in a plain find's roll
 * (roll_chip) and the extra rich data (net_gen.c), the zenny a find
 * holds, the finds leaned to the folder's codes */
enum { ECON_LEVER_MD, ECON_LEVER_CHIPS, ECON_LEVER_GOOD, ECON_LEVER_ZENNY, ECON_LEVER_LEAN, ECON_LEVERS };
typedef struct {
	const char *name;
	EconCurve lever[ECON_LEVERS];
} EconPlan;
extern const EconPlan econ_flat, econ_opening;
/* The plan in play: the opening one (the owner's yes, 21 October 2026);
 * the economy report sets it to measure either, --economy-plan flat to
 * play the flat one. */
extern const EconPlan *econ_plan;

/* Curve `c` at `x`, the run's way through its net (0 its first act, 1 its
 * last before the Nest and on): floor + (1 - floor) ln(1 + bend x) /
 * ln(1 + bend), within [floor, 1] */
double econ_curve(EconCurve c, double x);
/* A layer's way through its net: its act over the net's last before the
 * Nest (the short net's third, the endless net's sixth); its Nest and
 * every later cycle 1 */
double econ_x(int depth);
/* The plan in play's lever `lever` at `depth` */
double econ_factor(int lever, int depth);
/* `n` by the lever, rounded (no draw): n where it is 1 */
int econ_scale(int n, int lever, int depth);
/* Whether a find keeps what the lever scales, by a draw of its share:
 * true where it is 1, drawing nothing (today's rolls stay as they were) */
bool econ_keep(int lever, int depth);
/* ... by a hash in place of a draw (a hook's, which keeps the engine's
 * random numbers as they are: encounter.c's battle rows) */
bool econ_keep_by(int lever, int depth, uint32_t hash);

/* The bands the economy report holds the plan in play to, per run, an
 * act's layers together (the codes: the most chips gained in one letter
 * with the *s, at the act's end), on the endless net or the short one
 * (its acts 0-2 and its Nest, 6) */
enum { ECON_CHIPS, ECON_GOOD, ECON_MD, ECON_ZENNY, ECON_CODES, ECON_METRICS };
typedef struct { double lo, hi; } EconBand;
/* Metric `metric`'s band at act `act` (0-5, 6 the Nest) for `plan` */
EconBand econ_band(const EconPlan *plan, bool short_net, int metric, int act);

/* The report's assumptions: the mix of busting levels a battle is won at
 * (1-10, 11 for S), a level's weight; the random battles a layer (rng);
 * whether a chip of tier `tier` (chip_pool.h) is a good one */
int econ_bust_weight(int bust);
int econ_battles_roll(void);
bool econ_chip_good(int tier);

#endif
