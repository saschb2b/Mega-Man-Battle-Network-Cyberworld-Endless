/* The on-disk native records have the same byte layout on every target.
 * A change needs a save-format decision before these numbers are updated. */
#ifndef CW_SAVE_LAYOUT_H
#define CW_SAVE_LAYOUT_H

#include <limits.h>
#include <stddef.h>

#include "save.h"
#include "rivals.h"
#include "loot.h"

_Static_assert(CHAR_BIT == 8 && sizeof(int) == 4 && sizeof(bool) == 1, "Save scalar widths differ");


_Static_assert(sizeof(Profile) == 184, "Profile save size changed");
_Static_assert(offsetof(Profile, runs) == 0, "Profile.runs save offset changed");
_Static_assert(offsetof(Profile, best_depth) == 4, "Profile.best_depth save offset changed");
_Static_assert(offsetof(Profile, last_depth) == 8, "Profile.last_depth save offset changed");
_Static_assert(offsetof(Profile, bosses) == 12, "Profile.bosses save offset changed");
_Static_assert(offsetof(Profile, viruses) == 16, "Profile.viruses save offset changed");
_Static_assert(offsetof(Profile, secret_clears) == 20, "Profile.secret_clears save offset changed");
_Static_assert(offsetof(Profile, nest_clears) == 24, "Profile.nest_clears save offset changed");
_Static_assert(offsetof(Profile, music_volume) == 28, "Profile.music_volume save offset changed");
_Static_assert(offsetof(Profile, sfx_volume) == 32, "Profile.sfx_volume save offset changed");
_Static_assert(offsetof(Profile, seen_intro) == 36, "Profile.seen_intro save offset changed");
_Static_assert(offsetof(Profile, first_guardian) == 37, "Profile.first_guardian save offset changed");
_Static_assert(offsetof(Profile, navicust_taught) == 38, "Profile.navicust_taught save offset changed");
_Static_assert(offsetof(Profile, marks_taught) == 39, "Profile.marks_taught save offset changed");
_Static_assert(offsetof(Profile, threat_open) == 40, "Profile.threat_open save offset changed");
_Static_assert(offsetof(Profile, folders_open) == 42, "Profile.folders_open save offset changed");
_Static_assert(offsetof(Profile, short_wins) == 44, "Profile.short_wins save offset changed");
_Static_assert(offsetof(Profile, last_net) == 46, "Profile.last_net save offset changed");
_Static_assert(offsetof(Profile, last_folder) == 47, "Profile.last_folder save offset changed");
_Static_assert(offsetof(Profile, last_threat) == 48, "Profile.last_threat save offset changed");
_Static_assert(offsetof(Profile, last_helpers) == 49, "Profile.last_helpers save offset changed");
_Static_assert(offsetof(Profile, marks) == 50, "Profile.marks save offset changed");
_Static_assert(offsetof(Profile, first_area) == 52, "Profile.first_area save offset changed");
_Static_assert(offsetof(Profile, crosses_open) == 53, "Profile.crosses_open save offset changed");
_Static_assert(offsetof(Profile, last_cross) == 54, "Profile.last_cross save offset changed");
_Static_assert(offsetof(Profile, library) == 55, "Profile.library save offset changed");
_Static_assert(offsetof(Profile, library_start) == 96, "Profile.library_start save offset changed");
_Static_assert(offsetof(Profile, last_town) == 98, "Profile.last_town save offset changed");
_Static_assert(offsetof(Profile, programs_found) == 99, "Profile.programs_found save offset changed");
_Static_assert(offsetof(Profile, library_run) == 108, "Profile.library_run save offset changed");
_Static_assert(offsetof(Profile, setup_new) == 112, "Profile.setup_new save offset changed");
_Static_assert(offsetof(Profile, area_before) == 113, "Profile.area_before save offset changed");
_Static_assert(offsetof(Profile, guardian_before) == 114, "Profile.guardian_before save offset changed");
_Static_assert(offsetof(Profile, families_fought) == 116, "Profile.families_fought save offset changed");
_Static_assert(offsetof(Profile, gem_taught) == 124, "Profile.gem_taught save offset changed");
_Static_assert(offsetof(Profile, spins) == 125, "Profile.spins save offset changed");
_Static_assert(offsetof(Profile, spin_colour) == 126, "Profile.spin_colour save offset changed");
_Static_assert(offsetof(Profile, spin_run) == 128, "Profile.spin_run save offset changed");
_Static_assert(offsetof(Profile, records_mark) == 132, "Profile.records_mark save offset changed");
_Static_assert(offsetof(Profile, duel_won) == 136, "Profile.duel_won save offset changed");
_Static_assert(offsetof(Profile, duel_lost) == 138, "Profile.duel_lost save offset changed");
_Static_assert(offsetof(Profile, duel_run) == 140, "Profile.duel_run save offset changed");
_Static_assert(offsetof(Profile, duel_depth) == 144, "Profile.duel_depth save offset changed");
_Static_assert(offsetof(Profile, duel_beat) == 146, "Profile.duel_beat save offset changed");
_Static_assert(offsetof(Profile, pieces_taught) == 147, "Profile.pieces_taught save offset changed");
_Static_assert(offsetof(Profile, codes_entered) == 148, "Profile.codes_entered save offset changed");
_Static_assert(offsetof(Profile, codes_mailed) == 156, "Profile.codes_mailed save offset changed");
_Static_assert(offsetof(Profile, reg_taught) == 157, "Profile.reg_taught save offset changed");
_Static_assert(offsetof(Profile, tag_taught) == 158, "Profile.tag_taught save offset changed");
_Static_assert(offsetof(Profile, bbs_seen) == 159, "Profile.bbs_seen save offset changed");
_Static_assert(offsetof(Profile, pack_taught) == 160, "Profile.pack_taught save offset changed");
_Static_assert(offsetof(Profile, guest_taught) == 161, "Profile.guest_taught save offset changed");
_Static_assert(offsetof(Profile, dark_taught) == 162, "Profile.dark_taught save offset changed");
_Static_assert(offsetof(Profile, recode_taught) == 163, "Profile.recode_taught save offset changed");
_Static_assert(offsetof(Profile, cross_old_told) == 164, "Profile.cross_old_told save offset changed");
_Static_assert(offsetof(Profile, soul_taught) == 165, "Profile.soul_taught save offset changed");
_Static_assert(offsetof(Profile, dark6_taught) == 166, "Profile.dark6_taught save offset changed");
_Static_assert(offsetof(Profile, back_taught) == 167, "Profile.back_taught save offset changed");
_Static_assert(offsetof(Profile, last_lost_to) == 168, "Profile.last_lost_to save offset changed");
_Static_assert(offsetof(Profile, last_won) == 169, "Profile.last_won save offset changed");
_Static_assert(offsetof(Profile, hp_taught) == 170, "Profile.hp_taught save offset changed");
_Static_assert(offsetof(Profile, last_job) == 171, "Profile.last_job save offset changed");
_Static_assert(offsetof(Profile, beast) == 172, "Profile.beast save offset changed");
_Static_assert(offsetof(Profile, played_run) == 176, "Profile.played_run save offset changed");
_Static_assert(offsetof(Profile, played_frames) == 180, "Profile.played_frames save offset changed");

_Static_assert(sizeof(Run) == 112, "Run save size changed");
_Static_assert(offsetof(Run, active) == 0, "Run.active save offset changed");
_Static_assert(offsetof(Run, seed) == 4, "Run.seed save offset changed");
_Static_assert(offsetof(Run, depth) == 8, "Run.depth save offset changed");
_Static_assert(offsetof(Run, biome) == 12, "Run.biome save offset changed");
_Static_assert(offsetof(Run, side_kind) == 16, "Run.side_kind save offset changed");
_Static_assert(offsetof(Run, layer_seed) == 20, "Run.layer_seed save offset changed");
_Static_assert(offsetof(Run, biome_order) == 24, "Run.biome_order save offset changed");
_Static_assert(offsetof(Run, boss_order) == 30, "Run.boss_order save offset changed");
_Static_assert(offsetof(Run, bosses_beaten) == 64, "Run.bosses_beaten save offset changed");
_Static_assert(offsetof(Run, viruses_deleted) == 68, "Run.viruses_deleted save offset changed");
_Static_assert(offsetof(Run, fragments) == 72, "Run.fragments save offset changed");
_Static_assert(offsetof(Run, secret_cleared) == 76, "Run.secret_cleared save offset changed");
_Static_assert(offsetof(Run, mode) == 77, "Run.mode save offset changed");
_Static_assert(offsetof(Run, folder) == 78, "Run.folder save offset changed");
_Static_assert(offsetof(Run, threat) == 79, "Run.threat save offset changed");
_Static_assert(offsetof(Run, helpers) == 80, "Run.helpers save offset changed");
_Static_assert(offsetof(Run, cross) == 81, "Run.cross save offset changed");
_Static_assert(offsetof(Run, codes) == 82, "Run.codes save offset changed");
_Static_assert(offsetof(Run, programs) == 85, "Run.programs save offset changed");
_Static_assert(offsetof(Run, clock) == 93, "Run.clock save offset changed");
_Static_assert(offsetof(Run, back_spent) == 94, "Run.back_spent save offset changed");
_Static_assert(offsetof(Run, home_depth) == 96, "Run.home_depth save offset changed");
_Static_assert(offsetof(Run, job) == 98, "Run.job save offset changed");

_Static_assert(sizeof(Rival) == 16, "Rival save size changed");
_Static_assert(offsetof(Rival, met) == 0, "Rival.met save offset changed");
_Static_assert(offsetof(Rival, megaman_won) == 4, "Rival.megaman_won save offset changed");
_Static_assert(offsetof(Rival, navi_won) == 8, "Rival.navi_won save offset changed");
_Static_assert(offsetof(Rival, last) == 12, "Rival.last save offset changed");

_Static_assert(sizeof(LootMemory) == 20, "LootMemory save size changed");
_Static_assert(offsetof(LootMemory, biome) == 0, "LootMemory.biome save offset changed");
_Static_assert(offsetof(LootMemory, pick) == 4, "LootMemory.pick save offset changed");
_Static_assert(offsetof(LootMemory, families) == 8, "LootMemory.families save offset changed");
_Static_assert(offsetof(LootMemory, viruses) == 12, "LootMemory.viruses save offset changed");
_Static_assert(offsetof(LootMemory, families_before) == 16, "LootMemory.families_before save offset changed");

_Static_assert(sizeof(Job) == 12, "Job save size changed");
_Static_assert(offsetof(Job, kind) == 0, "Job.kind save offset changed");
_Static_assert(offsetof(Job, asker) == 1, "Job.asker save offset changed");
_Static_assert(offsetof(Job, need) == 2, "Job.need save offset changed");
_Static_assert(offsetof(Job, got) == 3, "Job.got save offset changed");
_Static_assert(offsetof(Job, state) == 4, "Job.state save offset changed");
_Static_assert(offsetof(Job, act) == 5, "Job.act save offset changed");
_Static_assert(offsetof(Job, pay_kind) == 6, "Job.pay_kind save offset changed");
_Static_assert(offsetof(Job, code) == 7, "Job.code save offset changed");
_Static_assert(offsetof(Job, pay) == 8, "Job.pay save offset changed");
_Static_assert(offsetof(Job, depth) == 10, "Job.depth save offset changed");

#endif
