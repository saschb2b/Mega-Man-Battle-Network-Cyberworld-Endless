/* Bystander navi lines. */
#ifndef CW_NPC_LINES_H
#define CW_NPC_LINES_H

/* The `i`th line a bystander says at `depth` (ta_talk's boxes): other
 * divers' navis early on, stranger ones deeper, and in a rebuilt net ones
 * that remember. Consecutive `i` do not repeat on a layer. */
const char *npc_line(int depth, int i);

#endif
