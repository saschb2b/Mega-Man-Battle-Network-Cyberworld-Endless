/* The net's rumors (docs/META.md, Rumors): what a bystander navi whispers,
 * where a layer has one to pass on. Every one is true of this game. */
#ifndef CW_RUMORS_H
#define CW_RUMORS_H

/* The line for a layer's whisperer (from the run's state, the profile and
 * the folder as the layer was made), NULL for none. */
const char *rumors_line(void);

#endif
