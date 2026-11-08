---
name: release
description: Cut a Cyberworld Endless release - the CHANGELOG and players' notes with every entry pictured, the trailer for a minor version, the version and PS5 contentVersion, dev brought to main and tagged, the release checked, then itch.io and the PS5 catalog in step. Use when asked to publish, release or ship a version, or to prepare one (notes, media, the candidate's checks).
---

# A release

A release is the owner's call to publish (ask for the go-ahead unless he
gave it), but everything before the tag is prepared without waiting on him.
AGENTS.md's Release row lists the checks; this is the order that works.
Releases since 0.5.0 are betas, published as GitHub's latest (the label
comes from `.github/workflows/release.yml` and `src/scenes/scene_title.c`).

## 1. What ships

- `git log --merges --first-parent vPREV..origin/dev`: every PR since the
  last tag. The CHANGELOG's `## Unreleased` has an entry for every
  player-facing change, in its voice: short, for players who know Battle
  Network, never explaining BN6's own mechanics. Developer-only work
  (the differential, symbols, research) stays out.
- Stale pages are a release bug: GUIDE, the site, the README and
  `docs/itch/` say what the build does (a "not in runs yet" left behind
  once a feature shipped is caught here).

## 2. The notes and their pictures

`docs/releases/vX.Y.Z.md`, written as
[docs/releases/overview.md](../../../docs/releases/overview.md), "How a
release's notes are written", says. Above all: open with the trailer, then
the title and one paragraph of the news. **Never "the Nth beta"** (the
owner, 8 November 2026). Every CHANGELOG entry gets a `###` with its own
picture or GIF.

- Pictures come from scripted headless runs added to `build.py`'s
  SCREENSHOTS/CLIPS (a fresh `--data-dir` each; `--input "0:place X Y FACE"`
  stands MegaMan at a set piece; dev routes in docs/devtools/). Two media
  agents in their own worktrees, a 60-minute box each, have done this well.
- Before/after against the last release where something visible changed:
  `python3 tools/before_after.py vPREV --new X.Y.Z` (host build first).
- Look at every picture: capturing has found real bugs before the tag
  (wrong tiles, a stale line, blank cards). Fix those first.
- Clips the notes show go in `README_GIFS` (GitHub plays no video from the
  repo; about 1 MB each at 15 fps).

## 3. The trailer (a minor release)

The trailer skill (`.claude/skills/trailer/`): a new look and an original
piece, its gates reviewed by the owner (the cut, the music). It replaces
the last trailer everywhere; the notes open with it.

## 4. The version

- CHANGELOG `## Unreleased` becomes `## X.Y.Z (date)`; commit "Release X.Y.Z".
- `ps5/sce_sys/param.json`'s contentVersion set to the version (lint holds
  it to the CHANGELOG's).
- `TAG=vX.Y.Z CI=1 python3 build.py`, then `TAG=vX.Y.Z python3 build.py
  screenshots title marks` (screenshots run `build/host/cyberworld` without
  rebuilding it; build first or the title shows the old version). Where
  layers or timing changed, re-shoot all and look at every changed file;
  after `clips`, keep only clips whose `.mp4`/`.png` changed.

## 5. Main and the tag

- Push dev and wait for its CI green (`gh run watch ID --exit-status` in the
  background).
- Bring dev to main by fast-forward (`git push origin SHA:main`, `git branch
  -f main SHA`; no checkout needed), check `git diff dev main` is empty,
  then tag it annotated "Cyberworld Endless X.Y.Z (beta)" and push the tag.
- A flaky job (the iOS Simulator's check, a server timing test) is rerun
  with `gh run rerun ID --failed`, after looking at why.

## 6. After the tag

- Check the release: its title, latest, its assets (16 since 0.7.0: the IPA
  and `altstore-source.json` among them), the site's /download/ links, the
  Linux files in a clean `debian:bookworm` container.
- itch.io (docs/project/itch.md, Each release): `python3 tools/itch.py push`,
  the description where `docs/itch/` changed, the devlog post
  (`tools/itch.py devlog`, posted on itch, then `tools/itch.py record --post
  TAG URL`), and `tools/itch.py check`.
- The PS5 catalog: a PR from the owner's fork updating CWEN00001's record
  (version, artifact_url, sha256, icon_url; ps5/README.md, Submitting to
  Prospero Store).
- Issues fixed on dev get "In vX.Y.Z." once on main.
- Sweep merged branches and worktrees: keep `main`, `dev` and `pr-media`.
- Runs on real devices are the owner's, after the release, and no blocker.
