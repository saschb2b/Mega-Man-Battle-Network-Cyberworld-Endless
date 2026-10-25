# itch.io

The game's page on itch.io, <https://saschb2b.itch.io/cyberworld-endless>
(project 5225408; drafted and published 25 October 2026, with 0.15.0's files):
what it holds, why, and how it follows each release. itch is where many
players look for fan games and roguelikes, and the page plays the browser
build in place and offers every system's file, all from the release on
GitHub; the project's website stays the home.

This page is the page's source. A change to the page is made here first,
then on itch: the texts and settings below, the description in
[itch/description.html](itch/description.html), the pictures from
`tools/itch_art.py`, the files from `tools/itch.py`.

## How itch finds a page

Read in itch's creator docs (Quality guidelines, Getting indexed, Designing
your page, HTML5 games; 25 October 2026):

- **Search is by title.** itch's search box matches the project's title
  first; the description and the tags are not search terms. The owner
  chose the brand alone for the title (25 October 2026), clear of Capcom's
  name, so the series is named in the tagline, which shows under the cover
  in every listing, and in the description's first line. Google indexes
  the page as any other, tagline and description both.
- **Tags feed the browse pages.** Up to 10, free-form; itch prefers its
  suggested tags and asks for no synonyms and nothing the Metadata tab
  already says (multiplayer, languages, inputs).
- **Platforms only for what runs there.** Windows, macOS, Linux and
  Android are set per upload, only on files that run on that system;
  the 3DS, PortMaster and iOS files carry no platform, and tags say what
  they are for.
- **Indexed once public**, with a cover image and a file to download or
  play. A page in Draft is seen by its owner alone.
- **Playable in the browser** puts it on itch's web-game browse pages,
  the largest of them.

## The texts

| Field | Value |
| --- | --- |
| Title | Cyberworld Endless |
| Project URL | `saschb2b.itch.io/cyberworld-endless` |
| Short description or tagline | A Mega Man Battle Network 6 roguelike, played by BN6 itself from your ROM. A new net every run, and NetBattles online. |
| Description | [itch/description.html](itch/description.html) |

The title is the site's brand. The tagline, shown under the cover in
listings and search results, names the series and BN6 for the players
who look for it, and says the rest of the site's pitch in fewer words;
itch takes 120 characters at most. The description says first what the game is and what it needs,
then a run, the release's news, BN5, online, where it runs, the browser
player's controls and saves, the links and the legal line, each with a
whole-frame GIF of the game from `docs/clips/`. It keeps the README's
plain voice and claims only what the README and the guide say.

## The settings

| Field | Value | Why |
| --- | --- | --- |
| Classification | Games | |
| Kind of project | HTML | The page plays the browser build; the other files are listed under it |
| Release status | In development | The releases are betas (0.x) |
| Pricing | $0 or donate, suggested donation $2.00 | The owner's choice: free, a tip optional. itch keeps a suggested amount above $0 in this mode; a player can skip it and download for free |
| Genre | Role Playing | BN is an action RPG; roguelike is a tag on itch, not a genre |
| Tags | `roguelike`, `roguelite`, `fangame`, `megaman`, `battle-network`, `deck-building`, `procedural`, `gameboy-advance`, `3ds`, `action-rpg` | Below |
| AI disclosure | Yes: code, text, graphics and sound | The owner's answer: the code and the lines were written with Claude, and so were the scripts that draw the site's art, the cover and the banner and write the trailers' music; the game's own pictures and sound are BN6's |
| Custom noun | (none) | |
| Community | Comments | Questions and reports in one place on the page; GitHub's Discussions and issues stay the project's own |
| Visibility | Public | Published 25 October 2026 |

The tags, with the number of games itch lists under each (25 October 2026;
a capital in itch's name for it marks one of its suggested tags):

| Tag | Games | Who browses it |
| --- | --- | --- |
| Roguelike | 29,293 | Roguelike players |
| Roguelite | 14,168 | The same, for runs with meta progress |
| Fangame | 11,017 | Fan-game players |
| megaman | 178 | Mega Man fans (itch's largest Mega Man tag; `mega-man` has 69) |
| battle-network | 11 | Battle Network fans, where BN fan games gather |
| Deck Building | 3,959 | The folder built from the chips a run finds |
| Procedural Generation | 10,301 | A new net every run (itch stores it as `procedural`) |
| Game Boy Advance | 787 | It plays a GBA game (itch's slug `gameboy-advance`) |
| Nintendo 3DS | 110 | The 3DS build, for homebrew players (itch stores it as `3ds`) |
| Action RPG | 6,082 | What BN is |

Left out: Pixel Art (223,357 games, says little), Multiplayer (the
Metadata tab says it), PortMaster (3 games; the description says it).

## Metadata

| Field | Value |
| --- | --- |
| Languages | English |
| Inputs | Keyboard, Gamepad (any), Touchscreen, Xbox controller, PlayStation controller |
| Accessibility | Configurable controls |
| Multiplayer | Server-based networked multiplayer, 1 to 8 players (a Net Square channel's eight) |
| Average session | About an hour (the short net) |
| Engines & tools | SDL (itch's list has no Emscripten, mGBA or devkitPro) |
| Code license | MIT License |
| Release date | Blank: itch asks for one only when it differs from the page's publishing |
| External links | Homepage: the website; Source code: GitHub; Community: GitHub Discussions; Support: GitHub issues |

## The pictures

`python3 tools/itch_art.py` draws them from `tools/steam_art.py`'s
wordmark and net and the site's PET colours, nothing from the ROM:

| Picture | File | Size |
| --- | --- | --- |
| Cover (listings, search, the page's preview) | [itch/cover.png](itch/cover.png): the infinity mark and the wordmark stacked over the net, as the Steam capsule draws them; the owner's pick of six (25 October 2026) over covers with a frame of the game (`--frame NAME` draws one) | 630x500 |
| Banner (replaces the title on the page) | [itch/banner.png](itch/banner.png) | 960x240 |
| Embed BG (behind the player's Run game button) | [itch/embed.png](itch/embed.png): one whole frame of a Central Area layer at 4x, dimmed a third towards the column's navy; no wordmark, which the banner above carries (the owner, 25 October 2026: the two together read as redundant) | 960x640 |
| Background tile | [itch/background.png](itch/background.png): the site's PET floor | 48x48 |
| Screenshots | `.build/itch/NN-NAME.png`: eight of `docs/screenshots` at 4x, whole frames | 960x640 |

itch's trailer field takes YouTube, Vimeo or Sketchfab only, and the
project has no channel there: the description's GIFs show the game in
motion, and the trailer itself stays on the website.

## The theme

Set in the page's theme editor (Edit theme):

| Setting | Value |
| --- | --- |
| Layout, screenshots | Auto, which hides them on a page with a game in it; the GIFs carry the description |
| BG | `#08bd73`, the PET's green, with the background tile repeated |
| BG2 (the column) | `#0b2532`, the browser player's own background, opaque |
| Text | `#f7fff7` |
| Link | `#6bdeff` |
| Buttons | `#ffd610` |
| Headers | `#ffd610` |
| Header font | Silkscreen |
| Font | Lato, Large |
| Banner | [itch/banner.png](itch/banner.png), centred |
| Background | [itch/background.png](itch/background.png), repeated both ways |
| Embed BG | [itch/embed.png](itch/embed.png) |

The headers are the site's Silkscreen (Google Fonts, which itch offers).
The site's body font, Pixelify Sans, was tried and dropped: Google's copy
draws the "fi" ligature like an A ("file" read "Ale", "five" "Ave"), and
itch's theme cannot turn ligatures off without custom CSS, which itch
enables only on request. Lato, itch's default, reads cleanly.

## The files

`tools/itch.py` takes a release's own files from GitHub and pushes each
to a butler channel under the release's version:

| Channel | The release's file | On the page |
| --- | --- | --- |
| `web` | the website zip's player, made into itch's zip | Played in the browser |
| `windows` | `cyberworld-endless-windows-x64.zip`, unpacked | Windows |
| `mac` | `cyberworld-endless-macos.dmg` | macOS |
| `linux` | `cyberworld-endless-x86_64.AppImage` | Linux |
| `linux-flatpak` | `cyberworld-endless.flatpak` | Linux (Steam Deck) |
| `android` | `cyberworld-endless.apk` | Android |
| `ios` | `cyberworld-endless.ipa` | none (SideStore, AltStore Classic) |
| `portmaster` | `cyberworld-endless-portmaster.zip`, whole | none (PortMaster handhelds) |
| `3ds-cia` | `cyberworld-endless.cia` | none (New 3DS, FBI) |
| `3ds-3dsx` | `cyberworld-endless.3dsx` | none (New 3DS, Homebrew Launcher) |

butler tags the channels named for Windows, macOS, Linux and Android with
their platform. Every file but the Windows one goes up as the release has
it (`--no-auto-unzip`); the Windows zip goes up unpacked, which the itch
app can start. The display names, set once on the edit page:

| Channel | Display name |
| --- | --- |
| `windows` | Windows 10 and 11 (64-bit) |
| `mac` | macOS 11 or newer (.dmg) |
| `linux` | Linux (AppImage) |
| `linux-flatpak` | Linux and Steam Deck (Flatpak) |
| `android` | Android 5 or newer (.apk) |
| `ios` | iPhone and iPad (.ipa, for SideStore or AltStore Classic) |
| `portmaster` | Handhelds with PortMaster (.zip) |
| `3ds-cia` | New 3DS (.cia, for FBI) |
| `3ds-3dsx` | New 3DS (.3dsx, for the Homebrew Launcher) |

### The browser build on itch

itch plays an HTML upload in a frame on its own domain
(`html-classic.itch.zone`), from a zip with `index.html` at its root:
at most 1,000 files, 500 MB in all and 200 MB a file, paths relative.
`tools/itch.py` makes it from the release's website zip: `play/` at the
root with the site's assets it uses, the site's bar hidden (the itch page
is the site there), the manifest left out, `play.css`'s pictures named
from the root (on itch's server `../` leaves the game's folder: the first
upload lost the cartridge slots' frames), and every link to the site, to
GitHub and to the licenses opened in a new tab, since a link in a frame
would open inside it. 60 files, 3.3 MB.

Its embed settings on the edit page:

| Setting | Value |
| --- | --- |
| Embed | In page, 960 x 640 (the game at 3x, the largest whole scale under its band and hints) |
| Mobile friendly | On, any orientation (itch then starts it full screen on a phone, and the player draws its touch controls) |
| Click to play | On (itch's default) |
| Fullscreen button | On |
| Scrollbars | Off |
| SharedArrayBuffer | Off (the browser build has no threads) |

Checked on 25 October 2026 with the 0.15.0 release's files, served as
itch serves them: in a 960x640 frame on another origin than its page, the
ROMs screen at 3x, both ROMs chosen and checked, the game started to the
statistics question at the first start, and after a reload both ROMs
still in the frame's own storage. The ROMs and saves live in that
origin's IndexedDB under `/cyberworld-endless`, apart from the website
player's (another origin) and from every other itch game's (another
name). Online play reaches the server from there too: it checks no
origin.

## Going public

Done on 25 October 2026: butler installed in `.build/butler/` and logged in
as the owner (`butler login`, its key in `~/.config/itch/butler_creds`),
`python3 tools/itch.py push --hidden` for 0.15.0, the `web` upload taken by
itch as the page's game, the display names set, the uploads shown, the
game loaded from itch's servers to its ROMs screen, the page made Public,
and the site's footers, its schema.org `sameAs` and the README linking it.
Not checked there: a phone (itch starts the game full screen on one) and a
ROM chosen on itch's page, which the same zip passed in a cross-origin
frame on this machine.

## Each release

After `build.py release` and the tag (docs: the release recipe):

1. `python3 tools/itch.py push` sends the latest release's files, each
   channel under its version, with butler from `PATH` or `.build/butler/`
   logged in as the page's owner (`butler login` once, in a terminal; the
   key stays in butler's own file). A new channel's first push makes a new upload:
   `--hidden` keeps it off the page until its display name and platform
   are set.
2. A minor release rewrites the description's **New in** section (between
   its `NEW IN` comments) with the release's news and its GIF, and the
   cover when its frame should change (`tools/itch_art.py --frame`).
3. A devlog post on the page with the release's notes, linking the
   version on the website: itch mails it to the page's followers.
