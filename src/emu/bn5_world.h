/* BN5 Team Colonel's real world as the run reads it (docs/rom/bn5-world.md,
 * BN5's real world, group by group; docs/world/bn5-town.md): Oran Isle, the
 * town a run may go in from (src/world/town.c's style, src/director/
 * director_stop.c). Read from the player's BN5 ROM at run time; nothing of
 * it is kept in git. */
#ifndef CW_BN5_WORLD_H
#define CW_BN5_WORLD_H

/* Its real world's map lists (BN5 0x0331B4) lie one after another, so the
 * engine reads them as one group, 0x00 (area_src_load_x): ACDC Town's 8
 * maps, SciLab's 5, then Oran Isle's 12, the island first. */
#define BN5_RW_LISTS        0x080331B4u /* its six real-world map lists, a pointer a group */
#define BN5_ORAN_LIST       0x080332BCu /* Oran Isle's (group 0x02): 12 maps of 12 bytes, the island first */
#define BN5_ORAN_COORDS     0x0803404Cu /* ... its coordinate data list (BN5 0x033FAC + 2 x 4), the island first */
#define BN5_ORAN_GROUP      0x00        /* the island as the engine reads BN5's real world */
#define BN5_ORAN_MAP        13          /* ... (8 + 5 maps before it) */

/* The island (128 x 96 tiles, 990 height cells, triggers in section 3):
 * the beach at height 0, the plaza, the seawall's walk and the pier at 32.
 * Its one jack-in point, 0x40, is 15 cells round the signal post on the
 * plaza (x 88..120, y -136..-120, world units, z 32); its checks 0xF0-0xF9;
 * its warp triggers 0x00-0x02 at the pier's end (the boat) and 0x10, 0x11
 * (the paths up to the mine and down to the dock's maps) lead to its other
 * maps, which the run leaves out. The dock: the pier's walk, where the boat
 * lies. */
#define BN5_ORAN_DOCK_X     (-48)       /* where Lan arrives: on the pier (x -56..-40 between its walls, height 32), the boat behind him */
#define BN5_ORAN_DOCK_Y     152
#define BN5_ORAN_DOCK_Z     32
#define BN5_ORAN_PORT_X     104         /* the jack-in point's middle */
#define BN5_ORAN_PORT_Y     (-128)

/* Beach Day, the island's song, started by its first map's own scripts
 * (BN5 0x0808ECAA and 0x0808F0C9; tools/bn5_songs.py) */
#define BN5_SONG_BEACH_DAY  0x06

#endif
