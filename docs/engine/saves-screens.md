---
type: Research
title: "Save screens elsewhere: what SAVES can take"
description: "How consoles, launchers, emulator frontends and games lay out save management with many actions (what they group by, how many actions a screen shows, where status, automatic switches and destructive actions go), researched on 15 November 2026 when SAVES had grown to eight buttons, three layouts drawn for a 240x160 PET panel, and the one recommended: CLOUD, CODE and FILE as Options' tabs."
tags: [engine, ui, saves, research]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-15T01:30:00Z }
sources:
  - id: ps5
    resource: "https://www.playstation.com/en-us/support/subscriptions/ps5-ps-plus-cloud-storage/"
    title: "PS5 cloud storage for saved data (Saved Data and Game/App Settings), read on 2026-11-15"
  - id: switch-auto
    resource: "https://nintendo.com/au/support/articles/how-to-enabledisable-automatic-save-data-backups-and-downloads"
    title: "Nintendo Switch: automatic save data backups and downloads, read on 2026-11-15"
  - id: switch-transfer
    resource: "https://www.nintendo.co.uk/Support/Nintendo-Switch/How-to-Transfer-Your-Save-Data-To-a-Nearby-Nintendo-Switch-Console-1549113.html"
    title: "Nintendo Switch: Transfer Your Save Data (send, receive), read on 2026-11-15"
  - id: steam-cloud
    resource: "https://partner.steamgames.com/doc/features/cloud"
    title: "Steamworks: Steam Cloud, read on 2026-11-15"
  - id: steam-conflict
    resource: "https://help.steampowered.com/faqs/view/68D2-35AB-09A9-7678"
    title: "Steam Support: Steam Cloud conflicts, read on 2026-11-15"
  - id: xbox-dialogs
    resource: "https://devdocs.xbox.com/build/core-features/common/game-save/game-saves-dialogues"
    title: "Xbox game saves: the system's dialogs, read on 2026-11-15"
  - id: playfab-conflicts
    resource: "https://devdocs.xbox.com/services/playfab/player-progression/game-saves/conflicts"
    title: "PlayFab game saves: conflicts, read on 2026-11-15"
  - id: retroarch
    resource: "https://docs.libretro.com/guides/retroarch-cloud-sync/"
    title: "RetroArch: Cloud Sync, read on 2026-11-15"
  - id: delta
    resource: "https://faq.deltaemulator.com/using-delta/delta-sync"
    title: "Delta: Delta Sync, read on 2026-11-15"
  - id: hick
    resource: "https://lawsofux.com/hicks-law/"
    title: "Laws of UX: Hick's law, read on 2026-11-15"
  - id: nng-disclosure
    resource: "https://www.nngroup.com/articles/progressive-disclosure/"
    title: "Nielsen Norman Group: progressive disclosure, read on 2026-11-15"
  - id: xag112
    resource: "https://devdocs.xbox.com/gaming/accessibility/xbox-accessibility-guidelines/112"
    title: "Xbox Accessibility Guidelines 112: UI navigation, read on 2026-11-15"
  - id: xag114
    resource: "https://learn.microsoft.com/en-us/gaming/accessibility/xbox-accessibility-guidelines/114"
    title: "Xbox Accessibility Guidelines 114: UI context, read on 2026-11-15"
---

# Save screens elsewhere: what SAVES can take

[Portable saves](saves.md) grew from three buttons to eight in two
releases (EXPORT, IMPORT, SEND, RECEIVE, CLOUD, UNDO IMPORT, AUTO-COPY,
FOLDER), each a pill of the same colour in a two-column grid, and the
owner found they "blur into each other" (15 November 2026). This page is
what the screens of others do with that many actions, read on that day,
and what SAVES can take from them, as [Settings screens
elsewhere](settings-screens.md) was for Options.

## What they group by

None shows its save actions as peers. Each splits them three ways:

- **A status line**, where the automatic part says how it stands: Steam's
  per-game Cloud switch and the library's sync state [steam-cloud],
  PS5's View Sync Status [ps5], Delta's Sync Status once a service is
  chosen [delta], RetroArch's progress on its status line [retroarch].
  Once set up, the line replaces the buttons.
- **Automatic switches in a settings place**: Switch's Save Data Cloud
  settings hold only "Automatically Back Up Save Data" and
  "Automatically Download Save Data" [switch-auto]; PS5's Auto-Sync
  Saved Data stands apart from its upload and download screens [ps5];
  Steam's is one switch [steam-cloud].
- **One-off actions from a place**, in direction pairs: PS5's console
  storage and cloud storage screens, each with its upload or download
  and Delete [ps5]; Switch's Transfer Your Save Data as "Send Save Data
  to Another Console" and "Receive Save Data" [switch-transfer].

No screen shows more than about three actions at once. Destructive ones
come last and apart, their scope in their label, behind a confirmation.
A conflict is never a menu item: Steam asks at launch [steam-conflict],
Xbox asks "which one do you want to use?" with the console's or the
cloud's [xbox-dialogs], one decision that says what each choice drops and
how it comes back [playfab-conflicts]. SAVES' comparison is that already.

## What the principles say

Hick's law: fewer choices, the task split, the recommended one
highlighted [hick]. Progressive disclosure: the frequent first, the rest
on request under a label that says what is behind it, and no more than
two levels [nng-disclosure]. On a pad, focus follows the layout, a
linear menu wraps and a grid should not [xag112]; help for the item under
the cursor, groups uniquely labelled, and LB and RB meaning the same on
every screen [xag114]. The [game-design skill's](../../.claude/skills/game-design/SKILL.md)
menus-and-screens reference says the same in its words: chunk by
frequency and depth, a loud focus, set-once things fast to reach.

Applied to SAVES: AUTO-COPY and FOLDER are settings, not actions; SEND
and RECEIVE, EXPORT and IMPORT are direction pairs; UNLINK and DELETE
are the destructive pair; CLOUD once on is a status line; UNDO IMPORT
belongs to no one method (it undoes a file's, a code's and the cloud's
imports alike).

## Three layouts drawn

Drawn at 240x160 in the PET's colours and the engine's own letters
(`pixfont.c`), beside today's grid:

- **A. Tabs by method: CLOUD, CODE, FILE**, Options' tabs turned with L
  and R, THIS DEVICE's run and record in a strip under them, the page's
  lines in the cyan window (two to four), the line under the cursor told
  in the information box. CLOUD: Turn on and Link to another system, or
  once on Add a system, Unlink this system and Delete the cloud copy,
  last; the status in the information box. CODE: Send these saves,
  Receive a code. FILE: Export, Import, Auto-copy with its value, Folder
  with its place. UNDO IMPORT as a band hint, START:UNDO IMPORT, where
  an import can be undone, as Options' START:DEFAULT.
- **B. One list with the information box**: seven lines (Cloud On, Send
  by code, Receive a code, Export to a file, Import a file, Auto-copy
  Off, Folder Downloads), THIS DEVICE and the cloud's status in the
  information box. It fits without scrolling only by giving up the
  panel, and Hick's cost stays.
- **C. A hub of three ways**: KEEP IN STEP, MOVE ONCE BY CODE, MOVE BY
  FILE, each a window with its status or what it holds, each opening a
  page of its own. The fewest choices, but a level more for every
  errand, and a second kind of screen beside Options.

## What is recommended

A: the three methods are the three players SAVES serves (one who moves
between systems for good, one who moves once, one who keeps files), so
a tab is the player's intent, not a mechanism; each page holds two to
four lines, so nothing blurs; it is the grammar Options and the JACK-IN
SETUP already taught (the owner's calls of 2 and 5 November 2026,
[the PET's rules](ui.md)); and the destructive pair stands last on one
page. Its conditions: the pill grid becomes a vertical list (a grid
should not wrap, and a list puts the information box beside the
cursor); Auto-copy a value line, not an action; the comparison stays the
cloud's and the file's one question; UNDO IMPORT one place on every page.
Open questions for the owner: which tab opens first (CLOUD, the way
recommended to switchers, or the last one used), and START or SELECT
for UNDO IMPORT.

## What was built

The owner chose A (15 November 2026). Before it was built, three
reviewers read it apart (a console and accessibility review, a review of
its words against BN6's PET and [the voice rules](../voice/rules.md), a
walk-through of eight tasks as three players counting presses) and agreed
on what follows, built with it ([Portable saves](saves.md), The views):

- **Nothing that changes saves on one press.** START is UNDO IMPORT
  alone, asked first in the information box with Yes and No, No chosen,
  as Options' START asks (the walk-through found Enter, a keyboard's
  START, swapping saves from any line); Turn on, Unlink and Delete ask
  too, Delete a line apart, the "DELETE: SURE?" label that flipped on a
  first press gone.
- **One word for each thing:** device (never "system", which the
  comparison's row uses for the platform and a phone reads as its OS),
  sync and synced (never "in step"), cloud saves, the code, the file, the
  copy. The handshake one name a side: Add a device shows a code, Link
  with a code takes it; a code's view says the route in full ("On your
  other device: SAVES, CLOUD, Link with a code.").
- **The box:** the line's job first, then on CLOUD and FILE how the page
  stands, never joined into one sentence; THIS DEVICE named on its strip,
  as on the CLOUD page an unnamed run read as the cloud's.
- **The pad and the thumb:** the list wraps round as Options' does, each
  page keeps its line, Auto-copy turns by Left and Right as a value; on a
  canvas held upright the lines are 18 of its pixels, a thumb's, and the
  tabs' taps taller than their pills; the band's left, its back arrow and
  name, is the way back by touch (an iPhone has no Back key), its right
  end left to the hint.
- **Where it opens:** on the page that needs the player, else the last.
- **The comparison:** THE FILE, THE CODE or THE CLOUD beside THIS DEVICE,
  KEEP THESE and USE THOSE whatever brought it; one that opened by itself
  waits half a second, and only A answers it.

Left for later: a mark on a page's tab that needs the player (as E-Mail
marks NEW), the page kept across starts, dates with the month's name,
and a faster code entry on a pad.
