---
type: Overview
title: "Release notes"
description: "The players' notes of every release, newest first, which the site's versions page renders and each GitHub release opens with."
tags: [releases]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-13T23:20:00Z }
---

# Release notes

The players' notes of each release, which the site's versions page renders (`tools/site_versions.py`) and each GitHub release opens with; a release without notes shows its CHANGELOG section instead.


## How a release's notes are written

- They open with the release's trailer where it has one (`../clips/trailer-play.png` linking `../clips/trailer.mp4`), then the release's title and one short paragraph of its news.
- That paragraph says what changed for the player, nothing else: never "the Nth beta" or "this is the Xth beta version" (the owner's direction, 8 November 2026). The beta label is the GitHub release's title, set by `.github/workflows/release.yml`.
- Every CHANGELOG entry of the release is shown, a `###` each with its own picture or GIF, grouped into player-facing sections; where something visible changed, a before/after pair against the last release (`tools/before_after.py`).
- Pictures are whole frames of the game or pages of the site from scripted runs (`build.py screenshots`, `build.py clips`), never a sprite cropped out; paths are relative (`../screenshots/`), which `tools/release_notes.py` rewrites to the tag's raw files.
- The words are the CHANGELOG's voice: short, for players who know Battle Network, never explaining BN6's own mechanics.

## The releases

- [Cyberworld Endless 0.21.1](v0.21.1.md): The net, drawn as its maps draw it.
- [Cyberworld Endless 0.21.0](v0.21.0.md): Three games. One run.
- [Cyberworld Endless 0.20.0](v0.20.0.md): Red Sun joins the run.
- [Cyberworld Endless 0.19.0](v0.19.0.md): Your screen, your sound.
- [Cyberworld Endless 0.18.1](v0.18.1.md): The PS5 app starts.
- [Cyberworld Endless 0.18.0](v0.18.0.md): Four more systems.
- [Cyberworld Endless 0.17.0](v0.17.0.md): Sharper, not bigger.
- [Cyberworld Endless 0.16.0](v0.16.0.md): Your viruses fight for you.
- [Cyberworld Endless 0.15.1](v0.15.1.md): The team joins the dark.
- [Cyberworld Endless 0.15.0](v0.15.0.md): Liberate the net.
- [Cyberworld Endless 0.14.1](v0.14.1.md): Every song, in BN6's own look.
- [Cyberworld Endless 0.14.0](v0.14.0.md): Pick a side.
- [Cyberworld Endless 0.13.0](v0.13.0.md): Cyberworld Endless connects.
- [Cyberworld Endless 0.12.0](v0.12.0.md): One Battle Network.
- [Cyberworld Endless 0.11.4](v0.11.4.md): Any controller, any map.
- [Cyberworld Endless 0.11.3](v0.11.3.md): Locks that hold.
- [Cyberworld Endless 0.11.2](v0.11.2.md): A guardian's fall, done right.
- [Cyberworld Endless 0.11.1](v0.11.1.md): The buttons answer.
- [Cyberworld Endless 0.11.0](v0.11.0.md): Home between dives.
- [Cyberworld Endless 0.10.0](v0.10.0.md): The second screen follows the game.
- [Cyberworld Endless 0.9.0](v0.9.0.md): Battle Network 5 joins the net.
- [Cyberworld Endless 0.8.0](v0.8.0.md): The net's layers are worth exploring now.
- [Cyberworld Endless 0.7.0](v0.7.0.md): Cyberworld Endless comes to iPhone and iPad, and the game runs lighter everywhere.
- [Cyberworld Endless 0.6.0](v0.6.0.md): Battle Network 5 joins in, on a net drawn like the original's.
- [Cyberworld Endless 0.5.3](v0.5.3.md): Two fixes, found by a player.
- [Cyberworld Endless 0.5.2](v0.5.2.md): Smooth on the 3DS.
- [Cyberworld Endless 0.5.1](v0.5.1.md): A fix for the 3DS, and a net that points the way.
- [Cyberworld Endless 0.5.0](v0.5.0.md): The first beta. A map on the 3DS's second screen, and touch controls for every thumb.
- [Cyberworld Endless 0.4.0](v0.4.0.md): A rival who keeps score. And a HOME Menu of its own.
- [Cyberworld Endless 0.3.0](v0.3.0.md): Every run leaves more behind. And it fits in your pocket.
- [Cyberworld Endless 0.2.0](v0.2.0.md): A run with a finish line. A next run with more to choose.
- [Cyberworld Endless 0.1.0](v0.1.0.md): Jack in. Dive deeper. The net is new every run.
