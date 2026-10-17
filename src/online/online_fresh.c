#include "online_fresh.h"

#include "director_folder.h"
#include "meta.h"
#include "powers.h"
#include "run.h"

void online_fresh_prepare(void) {
	MetaLoadout choices = meta_last_loadout();
	/* online_visit owns this temporary, inactive Run and restores it on
	 * return. No seed roll, progress, starting gift or save is produced. */
	run.folder = choices.folder;
	run.cross = choices.cross;
	run.helpers = choices.helpers;
	set_start_folder();
	library_to_game();
	powers_bring();
	star_folder_pack();
	star_records(run_all_star());
}
