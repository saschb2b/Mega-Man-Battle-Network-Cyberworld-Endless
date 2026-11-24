/* The shelf of starting folders (meta.h; docs/game/meta/shelf.md, issue
 * #864): every folder shown on the setup's FOLDER row, each in one of
 * three states. Locked: its hint, which points at the kind of deed. Heard:
 * its milestone said plainly, once a bystander's rumor told it, or the
 * floor's post on the Endless Net BBS three finished runs on. Open: as
 * meta_folder_open says, NEW on the setup as before. Which folders there
 * are, what they hold and how they open are meta.c's, unchanged. */
#include "meta.h"

#include "save.h"

/* finished runs from the shelf's first showing, or the floor's last post,
 * to the floor's next */
#define SHELF_FLOOR_RUNS 3

static bool heard(int folder) { return folder > FOLDER_STANDARD && meta_folder_exists(folder) && (profile.shelf.heard >> folder & 1); }

int meta_folder_state(int folder) {
	if (meta_folder_open(folder)) return SHELF_OPEN;
	return heard(folder) ? SHELF_HEARD : SHELF_LOCKED;
}

int meta_folders_open(void) {
	int n = 0;
	for (int f = 0; f < FOLDER_COUNT; ++f) n += meta_folder_open(f);
	return n;
}

int meta_folders(void) {
	int n = 0;
	for (int f = 0; f < FOLDER_COUNT; ++f) n += meta_folder_exists(f);
	return n;
}

void meta_folder_hear(int folder) {
	if (folder <= FOLDER_STANDARD || !meta_folder_exists(folder) || heard(folder)) return;
	profile.shelf.heard |= (uint16_t)(1u << folder);
	profile_save();
}

/* (said only while the profile has yet to open the folder, as every
 * secret's rumor is, and once the setup has shown the shelf it speaks of:
 * a first run's bystanders say nothing of folders it has never seen) */
int meta_folder_rumor(uint32_t pick) {
	if (!profile.shelf.floor_at) return 0;
	int locked[FOLDER_COUNT], n = 0;
	for (int f = FOLDER_STANDARD + 1; f < FOLDER_COUNT; ++f)
		if (meta_folder_exists(f) && meta_folder_state(f) == SHELF_LOCKED) locked[n++] = f;
	return n ? locked[pick % (uint32_t)n] : 0;
}

bool meta_folder_floored(int folder) { return folder > FOLDER_STANDARD && meta_folder_exists(folder) && (profile.shelf.floored >> folder & 1); }

void meta_shelf_seen(void) {
	if (profile.shelf.floor_at) return;
	profile.shelf.floor_at = (uint16_t)(profile.runs < 0xFFFE ? profile.runs + 1 : 0xFFFF);
	profile_save();
}

/* (called before profile_record_run counts the run: profile.runs + 1 runs
 * are finished with this one; the guidance comes late, never first, and a
 * folder at a time, in the shelf's order: the summary's closest unlock) */
void meta_shelf_run_over(void) {
	FolderShelf *s = &profile.shelf;
	int finished = profile.runs + 1;
	if (!s->floor_at || finished - (s->floor_at - 1) < SHELF_FLOOR_RUNS) return;
	for (int f = FOLDER_STANDARD + 1; f < FOLDER_COUNT; ++f) {
		if (!meta_folder_exists(f) || meta_folder_state(f) != SHELF_LOCKED) continue;
		s->heard |= (uint16_t)(1u << f);
		s->floored |= (uint16_t)(1u << f);
		s->floor_at = (uint16_t)(finished < 0xFFFE ? finished + 1 : 0xFFFF);
		return;
	}
}
