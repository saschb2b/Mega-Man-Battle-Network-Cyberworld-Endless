/* The run's Glitch layer in the generator (net_glitch.c), for net_gen.c. */
#ifndef CW_NET_GLITCH_H
#define CW_NET_GLITCH_H

/* The run's Glitch warp on its layer, in view of the way, away from the
 * arrival; `order` the layer's rooms, n of them (net_gen.c's room_order) */
void ng_place_glitch(const int *order, int n);
/* A Glitch layer's copy at its heart: the farthest end of its detours, or
 * the next room of `order` (*next on) where it has none so long */
void ng_place_copy(const int *order, int n, int *next);
/* A Glitch layer's data pulled from deeper in the Net, by how far off the
 * way each lies (NetObj.deep), once all stands */
void ng_glitch_deep(void);

#endif
