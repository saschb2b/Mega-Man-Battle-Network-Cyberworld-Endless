# The Flatpak

`io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless.yml` builds Cyberworld Endless as a
Flatpak on the Freedesktop runtime: mGBA 0.10.5 as a static GBA-only core
(as `docker/mgba.sh` builds it for the other targets), then the game with
`make TARGET=flatpak` against the runtime's SDL2. It is the way SteamOS
installs software (Discover, from Flathub), so it is the Steam Deck's
package.

## Build and install it here

```bash
python3 build.py flatpak
```

needs `flatpak`, and uses `flatpak-builder` or, without it, Flathub's
`org.flatpak.Builder` (`flatpak install --user flathub org.flatpak.Builder`);
the Freedesktop SDK comes on the first build. It writes
`build/release/cyberworld-endless.flatpak`, a bundle that names Flathub as
its runtime's source, so installing it fetches the runtime too:

```bash
flatpak install --user build/release/cyberworld-endless.flatpak
```

```bash
flatpak run io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless
```

On SteamOS the bundle opens in Discover from the file manager. A tag's
release carries it (`.github/workflows/release.yml`, in Flathub's builder
image).

## What it may touch

- Wayland or X11, PulseAudio (PipeWire), and the input devices for
  controllers.
- Read only, where the game looks for the ROM by its contents: Downloads,
  EmuDeck's `~/Emulation/roms/gba`, RetroDECK's `~/retrodeck/roms/gba`,
  `~/ROMs`, `~/roms`, and `/run/media` for the same folders on an SD card.
  A ROM found there is copied into the game's own folder.

Its saves and that copy live in
`~/.var/app/io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless/data/cyberworld-endless/`,
apart from the AppImage's `~/.local/share/cyberworld-endless/` (copy the
folder over to carry a run across).

## Onto Flathub

Flathub builds from a manifest in its own repository, sent as a pull
request to `flathub/flathub` (docs.flathub.org, "Submission"). Before that:

1. **The app ID** is the repository's own name (Flathub checks that
   `io.github.saschb2b.X` names `github.com/saschb2b/X`; Flatpak takes the
   hyphens in its last part). Its linter passes it.
2. **A release.** Tag `vX.Y.Z` and add it to the metainfo's `<releases>`
   (`<release version="X.Y.Z" date="YYYY-MM-DD"/>`); Flathub's linter fails
   without one.
3. **Its source.** In the submitted manifest the last module takes this
   repository at that tag instead of the checkout:

   ```yaml
   sources:
     - type: git
       url: https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless.git
       tag: vX.Y.Z
       commit: <the tag's commit>
   ```

4. **The linter**, as Flathub runs it:

   ```bash
   flatpak run --command=flatpak-builder-lint org.flatpak.Builder manifest linux/flatpak/io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless.yml
   ```

   (A local build's "screenshots not mirrored" errors are Flathub's own
   build's job.)
