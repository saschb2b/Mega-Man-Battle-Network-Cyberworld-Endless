---
name: youtube-publish
description: Publish Cyberworld Endless's trailers on YouTube (the owner's channel, @Saschb2b) - a 4K master per trailer from tools/trailer_youtube.py and its thumbnails from tools/trailer_thumbnail.py (the series' layout, a hook of two lines over a frame of the game, up to three for Studio's Test & compare; thumbnail.md), uploaded through the owner's signed-in Chrome in 9 MB pieces the Studio page joins and hash-checks, then each video's title (the news first), description, thumbnail, playlist, audience, AI answer, date and copyright check filled in Studio's wizard and published oldest first, and its id written into docs/project/youtube.md. Use when a release's new trailer is to go on YouTube, when a trailer is to be uploaded again or its details, thumbnail or playlist changed, when a thumbnail or a title is to be designed or judged, or when asked to publish, upload or manage the project's videos.
---

# Publishing the trailers on YouTube

The channel, what each video's fields hold and the table of videos are
[docs/project/youtube.md](../../../docs/project/youtube.md); read it
first. This skill is how an upload goes, learned on the first ten (15
November 2026). A new trailer joins after the release's tag, from the
release skill's after-the-tag steps.

What a video looks like on YouTube before anyone plays it, its thumbnail
and its title, is [thumbnail.md](thumbnail.md): what good ones do, the
series' layout, the release's half (`THUMBNAILS` and `YOUTUBE_HOOK` in
the cut) and its rules. The trailer itself, with Dad as its narrator, is
the trailer skill's (`.claude/skills/trailer/blueprint.md`).

## The owner's standing answers

- The channel is the owner's own, @Saschb2b (Studio in German).
- **Verwendung von KI** (was AI used): **Ja**. Its last case is music made
  with AI at the video's centre, and the trailers' music is an agent's.
- Not made for kids. Public once Studio's copyright check says
  **Keine Probleme gefunden**; a claim keeps the video private and goes
  to the owner.
- Publishing these is authorised by the owner's request to publish the
  trailers; anything else on the account (its name, banner, settings) is
  the owner's call, asked first.

## 1. The masters

```sh
python3 tools/trailer_youtube.py VERSION     # or none: every trailer
```

It writes `.build/youtube/cyberworld-endless-VERSION-trailer.mp4` (4K,
nearest neighbour, at most 20 Mbit/s), the `.png` poster beside it and
`videos.json` (titles, descriptions, tags, dates, thumbnails). One
trailer takes a few minutes; it runs two at once. Pull a frame (`ffmpeg
-ss 20 -frames:v 1`) and look: hard pixel edges, 3840x2160.

A cut on the blueprint has thumbnails of its own. Make them first, so
`videos.json` names them:

```sh
python3 tools/trailer_thumbnail.py VERSION
```

and read `...-thumbnail-check.png` by thumbnail.md's rule 4 before any
goes up. `videos.json`'s `thumbnail` is then thumbnail A, `thumbnails`
all of them; an older cut's is its poster.

Split each master for the upload, and note its hash and size:

```sh
Y=$PWD/.build/youtube; mkdir -p $Y/p
split -b 9000000 -d -a 2 $Y/cyberworld-endless-VERSION-trailer.mp4 $Y/p/VERSION.part
sha256sum $Y/cyberworld-endless-VERSION-trailer.mp4; stat -c %s $Y/cyberworld-endless-VERSION-trailer.mp4
```

(Use absolute paths and no `cd ... && rm`: the shell's guard refuses a
removal it cannot resolve. Put parts in a fresh folder instead.)

## 2. The browser

Claude in Chrome, the owner's Chrome, where they are signed in to
YouTube: load its tools in one ToolSearch, open a tab of the session's
own group, and close it at the end. The built-in browser is not signed
in. A consent banner: **Alle ablehnen**.

The extension's `file_upload` takes under 10 MB **per message**: a
`browser_batch` of several uploads is one message and is refused. So one
`file_upload` call per 9 MB part (several calls in one turn are fine).

## 3. One video, in order

Upload the oldest first: the channel lists by publish time.

1. Navigate to
   `https://studio.youtube.com/channel/UCyDGU6G07UPOj6N65wKawIQ/videos/upload?d=ud`
   (the upload dialog opens), wait 4 s, paste
   [scripts/studio_helpers.js](scripts/studio_helpers.js) whole into one
   `javascript_tool` call, then `find` "cw parts file input" for its ref.
2. `file_upload` each part, then the thumbnail (`videos.json`'s
   `thumbnail`: thumbnail A, or an older cut's poster), to that ref, one
   call each.
3. `await cwJoin('cyberworld-endless-VERSION-trailer.mp4', SHA, SIZE)`:
   both `sizeOk` and `shaOk` true, or start again from 1.
4. `cwGive()`, wait 5 s: the wizard opens on **Details**, saving as
   private, the upload running on.
5. **Details wiederverwenden**: pick the newest trailer (the first tile),
   untick **Titel** and **Beschreibung**, **Übernehmen**. That brings the
   playlist, tags, video language, remix setting and category with the
   game (MMBN6, 2005). The first video of a channel sets these by hand
   instead (the edit page's **Erweiterte Einstellungen anzeigen**).
6. Title: click the title box, `ctrl+a`, type it. Description: click its
   box, type it whole (newlines are fine). Both from `videos.json`.
   Dismiss the hashtag suggestions by clicking the **Details** heading,
   **never Escape**: Escape closes the whole dialog.
7. `cwThumb()`, wait 3 s, `cwState()`: `thumbTook` true, else
   `cwThumb()` again (it missed once in ten).
8. `find` the radio **Nein, es ist nicht speziell für Kinder** and the
   button **Erweiterte Einstellungen anzeigen**; click the button, then
   the radio (in that order: the radio once failed to stick when clicked
   first). Then `find` **Ja, es wurde KI verwendet** and
   **Aufnahmedatum**: click Ja; open the date, select the picker's text
   field at its top, `ctrl+a`, type `DD.MM.YYYY` (the release's tag
   date), Return.
9. `cwState()`: radios include "Nein, es ist nicht speziell für Kinder"
   and "Ja"; the title and playlist right.
10. Click the **Vorabprüfung** step: **Urheberrecht: Keine Probleme
    gefunden** with a tick. Still checking: wait. A claim: stop, leave it
    private, tell the owner.
11. **Weiter** to **Sichtbarkeit**, **Öffentlich**, **Veröffentlichen**.
    The dialog shows the link: its id goes into the table in
    `docs/project/youtube.md`.

A video left as a draft (the dialog closed early, a prompt in the way)
finishes from its edit page, `https://studio.youtube.com/video/ID/edit`:
the same fields (the thumbnail input is there too), **Speichern**, then
**Entwurf bearbeiten** for the wizard's checks and visibility.

## Test & compare

Where the cut has three thumbnails and Studio offers it (the video's
edit page, **Thumbnail**, **Testen und vergleichen**; a computer only,
and not while the video is private): upload B and C beside A there after
publishing. The test runs up to two weeks and names the one with the
largest watch time share; note the winner in docs/project/youtube.md's
table, and what it had that the others lacked in thumbnail.md, so the
next release's three start from it. Where Studio does not offer it yet,
publish with A and say so in the table.

## Verification and its prompts

Studio's Settings, Kanal, **Verfügbarkeit der Funktionen** shows what is
open. Custom thumbnails need the intermediate features (a phone number,
on since 15 November 2026), and after a few in a day the advanced ones;
links in descriptions are clickable only with the advanced features (a
video check, an ID or the channel's history), and become clickable on
the published videos when they open.

**Tägliches Upload-Limit erreicht** in a bar under the wizard, right
after `cwGive()` (the wizard opens, "Link wird erstellt…" never becomes a
link): an unverified channel takes about ten uploads a day, and the first
ten trailers used them on 15 November 2026, so 0.22.0's was refused that
evening. Its **Überprüfung starten** is the owner's (the identity check
below); otherwise the limit lifts 24 hours after the day's first upload.
Close the dialog (no draft is left), say so, and upload again then: the
parts must be given to the page anew.

A prompt **Einmalige Überprüfung deiner Identität erforderlich** is the
owner's to answer: stop, tell them where it is, never enter a phone
number, ID or code. After they verify, **reload** the Studio page: a page
loaded before keeps prompting. Where the day's thumbnails are used up,
publish with the still Studio picked (**Aus Video auswählen** offers
three) and set the poster the next day from the edit page.

## After

- The table in `docs/project/youtube.md` gets each video's row, and a
  log entry (`docs/log.d/`) names the page.
- Check the channel's video tab: the count and the newest on top.
- Close the session's Chrome tabs.
