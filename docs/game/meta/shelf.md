---
type: Design
title: "The shelf of starting folders"
description: "The setup's FOLDER row shows all six starting folders (issue #864): open, locked with a hint at the kind of deed, or heard (its milestone said plainly) once a bystander's rumor or the BBS's floor post told it; the summary's closest unlock follows the same rule."
tags: [game, meta, setup]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-24T17:28:00Z }
sources:
  - id: issue-864
    resource: "https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/864"
    title: "The setup's FOLDER row: every starting folder shown, the locked ones with a hint, the answer heard in the net"
    author: human:saschb2b
---

# The shelf of starting folders (24 November 2026)

A player asked on Discord (Baron Solomon, 23 November 2026) where the
unlockables and their conditions are shown: he wanted to try more
starting folders. The setup's FOLDER row turned through the open folders
alone, so a profile with one saw no sign of the five others, and the
information box named every shut folder's deed outright ("Blade: delete
any guardian"). It showed too little (how many there are, what each
plays like) and told too much (the exact deed).

The owner's direction (24 November 2026): show every starting folder;
they are no spoiler. The ones not yet opened are marked locked with a
slight hint, no dead giveaway, no list of "X is missing and you get it by
Y". The summary's closest-unlock line follows the same rule (his call).
Which folders exist, what they hold and how they open are unchanged
([Decisions and phase one](decisions.md), The designed three).

## Three states

The FOLDER row turns through all six, the open ones counted beside its
name ("Folder 3/6"). The setup is still skipped on a profile's first run
([Decisions and phase one](decisions.md)).

| State | The row | The information box |
| --- | --- | --- |
| Open | its name, taken by the run | what it plays like (its `about`) |
| Locked | its name in a list's dim letters, the PET's tag before it (LOCKED, the information box's pill in the field's blue: BN6's own pieces, no new art) | what it plays like, and under it, dim, one line on the kind of deed: "Opens when something big falls" |
| Heard | as locked | what it plays like, and the deed plainly: "Delete any guardian to open it" |

A locked folder can be looked at, not taken: its 30 chips stay hidden, A
on it gives BN6's refusal (`SFX_ERROR`), and the run starts with the last
open folder the row showed; off the row, the row shows that one again. A
remembered `last_folder` that is not open starts on Standard, as before.
Opened, a folder is marked NEW on the row as before (`setup_new`).

The hints point at the kind of deed in the folder's own spirit, each
milestone having been chosen to teach its style (`meta_text.c`'s `hint`):

| Folder | Opens | The hint |
| --- | --- | --- |
| Blade | Any guardian deleted | Opens when something big falls |
| Storm | An Aqua guardian deleted | Opens where lightning strikes hardest |
| Z-Canon | A Program Advance used | Opens when chips work as one, in order |
| Busting | An act cleared with no Recovery chip | Opens for an act with no safety net |
| Lean | Layer 10 reached | Opens deep down in the Net |

## Heard in the net

The middle state is earned in play, the answer found by playing rather
than read from a list:

- **A rumor.** The layer's second bystander, who passes on a secret one
  layer in three while one is left ([Rumors](rumors.md)), may tell how a
  locked, unheard folder opens, as a friend's story ("My buddy deleted a
  guardian! / Next dive,his whole folder was swords!"). It is said only
  while the folder is locked and unheard, and only once the setup has
  shown the shelf, so a first run's bystanders say nothing of folders it
  has never seen. Its saying sets a choice's flag (`CHOICE_WHISPER`), and
  the director keeps the folder heard in the profile. The rumor is picked
  as the layer's text is made, from the profile, as every secret's is: the
  layer's layout and rolls are untouched, so `LAYER_MAKE` stands.
- **The floor.** Guidance arrives late, never first: once three runs
  have finished since the setup first showed the shelf, the first folder
  still locked and unheard, in the shelf's order (the summary's closest
  unlock), is heard through a post on the Endless Net BBS, the mail every
  profile holds, marked NEW with it; the count begins again from there,
  so a stuck player hears one folder every three runs.

What MegaMan has heard is kept as hearsay is ([What MegaMan
knows](what-megaman-knows.md)): in the profile, `Profile.shelf`
(`save.h`), in the eight bytes that held the virus families battled
(unread since 19 October 2026). A profile of before holds other data
there; `shelf.kept` reads `SHELF_KEPT` once they are the shelf's, and
`profile_load` clears them once (nothing heard, the floor's count not
begun). The profile's size and every other field stand. Its two bit sets
are sixteen bits each, a bit per folder (`1 << FOLDER_*`), so the shelf
holds up to sixteen; the first shelf's byte-wide sets, never released,
held eight, and a profile that kept them starts its shelf afresh
(issue #865).

## The summary

The run's summary names the closest unlock in KeyItem's information box
([The meta layer's design](design.md), Teaching it). For a folder, it
gives the hint while the folder is locked and unheard ("The Blade folder
opens when something big falls.") and the plain sentence once heard
("Delete any guardian to open the Blade folder.").

## Through the lens

- **The dialectic** is the rumors' own, discovery against guidance: point
  at the secret, leave the finding to the player.
- **Patterns.** Meta-progression unlocks variety, and a shelf with visible
  gaps gives variety its pull. Staged help that keeps the achievement's
  signal (the skill's onboarding page: InvisiClues from subtle to
  explicit; Super Guide's stuck-detection), here three stages: the hint,
  the heard deed, and the floor's post for whoever is stuck.
- **Smells checked.** No checklist, no progress bars, no counts toward a
  condition (the overjustification guardrail); a hint is a whole
  sentence, since a clipped line once read as broken; no new screen.

## Where the issue was departed from

- **The floor's mail is a post on the Endless Net BBS, not a mail from
  Dad or Higsby.** The PET's lab mails take BN6's mail slots one each, and
  the next free one was taken by MegaMan's looks (issue #854) in the same
  days; and the voice rules put strategy on the BBS, in its posters'
  voices, as every other rumor's late answer is
  ([Voice](../../voice/rules.md), What nobody says, 4).
- **One folder at a time.** The floor tells the closest folder still
  locked and unheard every three finished runs, rather than all of them at
  once.
