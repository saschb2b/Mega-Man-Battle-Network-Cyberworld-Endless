/* The lining up a layer's floor asks of MegaMan on a walk (lineups.c,
 * docs/devtools/atlas.md): the atlas's measure of a layer's walk. */
#ifndef CW_LINEUPS_H
#define CW_LINEUPS_H

/* On the shortest walk over the layer's floor (layer.cell) from (sx, sy)
 * to (gx, gy): the times MegaMan must stop on one exact panel to go on,
 * as BN6 walks him (a one-wide walkway entered from wider floor, a turn
 * at a junction where the floor goes on ahead; not a corner, whose wall
 * stops him there), in *len the panels walked and in *junctions the
 * walkways' junctions passed (a panel in no 2 x 2 block of floor with
 * floor on three sides or four: where the way can be lost and the arrow
 * flips past it); -1 where there is no walk. */
int layer_lineups(int sx, int sy, int gx, int gy, int *len, int *junctions);

#endif
