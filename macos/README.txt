Cyberworld Endless for macOS
============================

A roguelike built on Mega Man Battle Network 6: a new net every run, played
on the real game. One app for Apple silicon and Intel Macs, macOS 11 on.
https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/

Install
-------
Drag Cyberworld Endless into Applications. It is not notarized by Apple, so
the first start is refused ("Apple could not verify..."): open System
Settings > Privacy & Security, scroll down to Cyberworld Endless and choose
Open Anyway, then confirm. (On macOS 14 and older, Control-click the app and
choose Open instead.)

Your ROM
--------
You need Mega Man Battle Network 6: Cybeast Gregar (USA) as an unmodified
.gba file (SHA-1 89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6). Nothing from the
game is included here. The first start looks for it in Downloads and asks
for the file if it is not there; it keeps a copy in
~/Library/Application Support/cyberworld-endless/rom/.

Playing
-------
It opens in a window at the largest whole scale that fits; F11 or Alt+Enter
(Option+Return) switches to fullscreen. Command-Q quits; the run is saved at
the start of each layer and when you quit while MegaMan is free to move.

  Game Boy Advance   Keyboard                Controller
  D-Pad              W A S D or the arrows   D-Pad or left stick
  A                  J or X                  A
  B                  K or Z                  B
  L                  Q                       Left shoulder or trigger
  R                  E                       Right shoulder or trigger
  Start              Return                  Start (Menu)
  Select             R or Delete             Back / Select (View)

Controllers: Xbox, PlayStation and Switch ones and others macOS knows.
Keys are positions; ~/Library/Application Support/cyberworld-endless/keys.ini
changes them. Saves live in the same folder.

Mega Man Battle Network is (c) Capcom. This is an unofficial fan project, not
affiliated with or endorsed by Capcom. Code under the MIT license; it embeds
mGBA (MPL-2.0) and SDL2 (zlib), see the app's Resources/licenses.
