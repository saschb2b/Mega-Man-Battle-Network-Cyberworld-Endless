/* What the second screen's panels show (second_read.c), read once a frame
 * after the game's into S2: the folder and the pack, the battle, the
 * NaviCustomizer, the PET's home, MegaMan's status, a shop, a trader. */
#ifndef CW_SECOND_READ_H
#define CW_SECOND_READ_H

void second_read_folder(void);
void second_read_battle(void);
void second_read_navicust(void);
void second_read_home(void);
void second_read_status(void);
void second_read_shop(void);
void second_read_trader(void);

#endif
