# Cyberworld Endless for Linux

A roguelike built on Mega Man Battle Network 6. This is the desktop build of
the ROCKNIX handheld port: the same game in a window on a Linux PC.

## Your ROM

You need **Mega Man Battle Network 6: Cybeast Gregar (USA)** as an
unmodified `.gba` file (SHA-1 `89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6`).
Copy it into either:

- `~/.local/share/cyberworld-endless/rom/` (created on the first start), or
- the `rom/` folder next to `cyberworld-endless`.

Any file name works; the game checks the contents. Nothing from the game is
included here.

## Start

```sh
./cyberworld-endless
```

`./install.sh` adds it to your desktop's application menu. The first start
records the game's boot once, which takes a few seconds.

It runs on x86-64 distributions with glibc 2.34 or newer (Ubuntu 22.04,
Debian 12, Fedora 35 and later) and brings its own SDL2 in `lib/`, which
uses X11 or Wayland and PipeWire, PulseAudio or ALSA, whichever the system
has.

## Controls

A keyboard or any game controller SDL knows.

| Game Boy Advance | Keyboard | Controller |
| --- | --- | --- |
| D-Pad | Arrow keys | D-Pad or left stick |
| A | X or Space | A |
| B | Z or Backspace | B |
| L | A or Q | Left shoulder or trigger |
| R | S or W | Right shoulder or trigger |
| Start | Enter | Start |
| Select | Tab or Right Shift | Back / Select |

F11 or Alt+Enter switches between the window and fullscreen, Escape quits.
`--fullscreen` starts in fullscreen.

## Saves

Saves, the boot recording and `runlog.txt` live in
`~/.local/share/cyberworld-endless/` (or `$XDG_DATA_HOME/cyberworld-endless`).
`--data-dir DIR` keeps them elsewhere; the handheld port's `savedata/` can be
copied in to carry a run over.

## Licenses

The code is MIT-licensed (`LICENSE`). The embedded GBA core is mGBA
(MPL-2.0) and the bundled SDL2 is zlib-licensed; both licenses are in
`licenses/`. Mega Man Battle Network is © Capcom; this project is not
affiliated with or endorsed by Capcom.
