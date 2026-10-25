---
type: Code Map
title: "The site, screenshots and clips"
description: "What `web/` (the GitHub Pages site in BN6's PET look, its Japanese pages, the player and downloads), `docs/screenshots/` and `docs/clips/` (the trailer included) hold."
tags: [code]
generated: { by: claude-code/unrecorded, at: 2026-10-22T20:21:04Z }
---

# The site, screenshots and clips

| Path | Contents |
| --- | --- |
| `web/` | The project site on GitHub Pages, laid out like BN6's PET screens: the home page (`index.html`, `assets/`), the downloads in `download/` (the newest release's files, one system at a time), the player in `play/` (ROM check and storage: BN6's, and BN5's beside it, each told by its game code and checked by its SHA-1; scaling), the FAQ in `faq/` as the PET's E-Mail, its answers taken from the docs, every release in `versions/` (the newest first with its trailer, the rest in a list, each at `#v0-13-0`; the notes of docs/releases/, or the CHANGELOG's section without them, rendered at build time by `tools/site_versions.py`, which also writes `versions/feed.xml`; the template holds the frame and the words the releases need), and `404.html`. Each page has its Japanese one in `ja/`, written for Japan's players, who know the game as ロックマンエグゼ6 (the ROM they need said first; BN6's names as Capcom's Japanese releases write them): the same page in all but its words, with the same ids, pictures, clips and scripts, which take their words from the page's `lang`. `tests/test_site.py` holds the two in step, with their hreflang links, canonical addresses and every link between the pages; a change to a page's words changes its Japanese one too. `assets/qr.js` draws the 3DS title's link as a QR code for FBI; `assets/site.js` sends the analytics' events (Umami: what was done, never by whom; links name theirs with `data-umami-event`). Each page has its canonical address, Open Graph tags and schema.org data; `build.py site` adds the sitemap, the FAQ's FAQPage data, the versions page's releases and feed, and the content hash on each page's own files. Its frames and icons (`assets/ui/*.png`) are drawn by `tools/site_art.py`, not taken from the ROM |
| `docs/screenshots/` | Screenshots of the game for the README and the site (`build.py screenshots`) |
| `docs/clips/` | Short videos of the game for the site: WebM, MP4 and a poster each, and a GIF of those the README shows (`build.py clips`, ffmpeg in a pinned image); the current trailer (`trailer.*`: the site's hero and every page's preview picture) and `trailer-play.png`, its poster with a play button, which the README links to the MP4 (`tools/trailer.py`, Trailers below) |
