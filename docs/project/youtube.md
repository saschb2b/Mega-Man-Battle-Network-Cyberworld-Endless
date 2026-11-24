---
type: Process
title: "YouTube"
description: "The trailers on YouTube: the channel (@Saschb2b), a video per trailer in one playlist, the 4K masters `tools/trailer_youtube.py` makes, the thumbnails `tools/trailer_thumbnail.py` draws, the narrator, what each video's fields hold, how an upload goes, and each release's step."
tags: [project, youtube, trailers]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-16T18:47:34Z }
---

# YouTube

Every trailer the project has made (`trailers.VERSIONS`, AGENTS.md's
Trailers) is a video on the owner's channel,
<https://www.youtube.com/@Saschb2b> (channel `UCyDGU6G07UPOj6N65wKawIQ`),
in one playlist in version order. The site keeps showing the current
trailer from `docs/clips/`; YouTube is where they are all kept and found,
and the one host [itch.io](itch.md)'s trailer field takes besides Vimeo.

## The channel

The owner's own channel rather than a new one (the owner, 15 November
2026): it had no videos, so no audience to mismatch, and its handle is
the name the project goes by on GitHub and itch.io. A channel of its own
would not have kept a copyright complaint away from the owner's account
either: YouTube's terms bar a terminated channel's owner from any other.
Studio's language is German.

## The masters

`python3 tools/trailer_youtube.py [VERSION ...]` writes into
`.build/youtube/`, for every trailer by default:

- `cyberworld-endless-VERSION-trailer.mp4`: the trailer scaled 3x,
  nearest neighbour, from 1280x720 to 3840x2160, H.264 High at CRF 18
  capped at 20 Mbit/s (uncapped, a trailer's scanlines and flashes ran
  to 50), two B-frames and a closed GOP of 15 (YouTube's
  upload recommendations), its sound as it shipped. A 4K upload is
  streamed with VP9 or AV1 at every size and a higher bitrate; a 720p
  one gets H.264 at a low bitrate that smears the game's pixels.
- `...-trailer.png`: its poster, `docs/clips/trailer.png` (1280x720): the
  thumbnail of a cut that has none of its own (none since 16 November
  2026, when the older cuts got theirs).
- `videos.json`: each one's title, description, tags, date and
  thumbnails.

A trailer's video is the last `docs/clips/trailer.mp4` committed while it
was the current one, read from git: 0.12.0's came after its tag, and
0.3.0's end card was corrected after its tag to name PortMaster. Where
`tools/trailer.py` rendered a trailer on this machine, it also wrote
`.build/trailer-VERSION/youtube.mp4` from its lossless frames and music,
and the tool takes that instead.

## The thumbnail

A trailer's thumbnail is made for YouTube, not borrowed from
the site (the owner, 15 November 2026: a definition of the thumbnail,
following what good thumbnails on YouTube do; 0.22.0's first, and on the
16th the ten older videos reworked to it, "to hold the same standard").
The poster, a logo and
small capitals on navy, is right for a visitor who already chose the
project; at 168 pixels wide between other videos it promises nothing.

`python3 tools/trailer_thumbnail.py VERSION` draws up to three from the
cut's `THUMBNAILS`, each an SVG that Chrome turns into a 1280x720 PNG,
and a check sheet of each at 320, 168 and 120 wide on YouTube's dark and
light pages. The layout is the series' and fixed: a region of one whole
frame of the game filling the picture at a whole scale, a navy scrim
under the words, the PET's cyan frame, the logo with the version and a
green NEW top left, two lines of heavy slanted type bottom left (four
words at most, the hook: `CLOUD SAVES!`), the bottom right clear for
YouTube's length stamp. The frame, its region, the words and an optional
pointer (the PET's orange cursor) are the release's. Never a sprite or a
mugshot cut out: a region of a whole frame, half its width at least
(AGENTS.md, Rules). Three, because Studio's Test & compare takes three
and names the one that held viewers longest. The rules, what was looked
up and where, are the youtube-publish skill's `thumbnail.md`.

## The narrator

From 0.22.0 a trailer is told by Lan's father (the owner, 15 November
2026): after the title card he says who he is, what Cyberworld Endless
is, and hands over to the release's news; before the logo he thanks the
players. BN has no voices, so he speaks as he does in the game, in BN6's
own chat box with his face: the game itself types his lines
([`--talk narrate`](../devtools/headless-shot.md), the boxes in
`CYBERWORLD_NARRATE`), and the cut lays that box over footage that fits
what he says, since a chat stops the game under it (the owner: "having
dad speak while megaman just sits there is boring"). His lines follow
[the voice rules](../voice/rules.md)' Dad. The trailer skill's
`blueprint.md` has the slots, the lines' rules and the rest of the
series' blueprint.

## Each video

| Field | Value |
| --- | --- |
| Title | `HOOK \| Cyberworld Endless VERSION Trailer`, the cut's `YOUTUBE_HOOK` (the news in its first forty letters, which is what a phone shows). Until 16 November 2026 the ten before 0.22.0 read `Cyberworld Endless VERSION Trailer – TAGLINE`, the tagline the release's notes' `description`; `tools/trailer_youtube.py` still falls back to that for a cut with no hook |
| Description | the tagline and the notes' first paragraph (without "the Nth beta"), the release date, the links (play in the browser, download, the release on GitHub, itch.io, the source), the ROM it needs, Capcom's notice, three hashtags |
| Tags | `trailer_youtube.TAGS` |
| Language | English |
| Thumbnail | The cut's thumbnail A, with B and C in Studio's Test & compare where the channel has it (The thumbnail, above). The ten before 0.22.0 have one each, drawn over a picture of their release's notes (`('still', NAME)`), without the green NEW, which is the current trailer's alone |
| Playlist | Cyberworld Endless – Trailers |
| Audience | not made for kids |
| AI used | yes, for the music (below) |
| Category | Gaming, the game Mega Man Battle Network 6 |
| Recording date | the release's tag's date |
| Visibility | public once Studio's copyright check shows no issue; a claim keeps it private, and the owner decides |

The videos went up oldest first, so the channel lists the newest on top.

## Uploading

The steps, in order and with Studio's labels, are the youtube-publish
skill's (`.claude/skills/youtube-publish/SKILL.md`). In short: the agent uploads through the owner's Chrome, signed in to Studio, with
the Claude in Chrome extension. Its file upload takes under 10 MB a call
(a batch of calls counts as one), and a master is 40-90 MB, so the master
goes in 9 MB parts (`split -b 9000000`), one call each, to a file input
of the agent's own on the upload page; the page joins them into one file,
checks its SHA-256 against the master's, and hands it to Studio's own
input (`input[name=Filedata]`) as a change. The poster goes the same way
to the thumbnail's input (`#file-loader`). The rest is Studio's wizard:
the title and description typed, **Details wiederverwenden** from the
last trailer for the playlist, tags, language and category, then the
audience, the AI question, the recording date, the check and **Öffentlich**.

What the channel's verification opens (Studio's Settings, Channel,
Feature eligibility): custom thumbnails need the intermediate features
(a phone number, done 15 November 2026), and after a few in a day the
advanced ones; links in descriptions are clickable only with the
advanced features (a video check, an ID or the channel's history), so
the descriptions' links stay plain text until then. Each step is the
owner's: the agent stops at the prompt and says so.

## Answers the owner gave

- **AI used** ("Verwendung von KI"): **Ja**. Its fourth case is music
  made with AI at the video's centre, and the trailers' music is written
  by an agent as code (the owner, 15 November 2026).
- Not made for kids; the game MMBN6 (2005); Standard licence; comments on.

## Each release

A release with a new trailer, after its tag (the release skill's other
after-the-tag steps, itch.io's among them, in [itch.io](itch.md)): the
youtube-publish skill, `python3 tools/trailer_youtube.py VERSION` to the
published video, and its row below.

## The videos

| Trailer | Video | Up |
| --- | --- | --- |
| 0.3.0 | <https://youtu.be/qV36V80dWQ0> | 2026-11-15 |
| 0.12.0 | <https://youtu.be/EibRs0c9WB0> | 2026-11-15 |
| 0.13.0 | <https://youtu.be/DeWyW9bhroA> | 2026-11-15 |
| 0.14.0 | <https://youtu.be/e29HKE1ajZ8> | 2026-11-15 |
| 0.15.0 | <https://youtu.be/M-dKHuLcFTc> | 2026-11-15 |
| 0.16.0 | <https://youtu.be/LFSkBxv6Nlw> | 2026-11-15 |
| 0.17.0 | <https://youtu.be/KRfFxaC-BII> | 2026-11-15 |
| 0.19.0 | <https://youtu.be/M62DJAEzlPg> | 2026-11-15 |
| 0.20.0 | <https://youtu.be/GOWV6RM4j8s> | 2026-11-15 |
| 0.21.0 | <https://youtu.be/FgRgRRJFtUM> | 2026-11-15 |
| 0.22.0 | <https://youtu.be/XbueF-wBxt4> | 2026-11-16 |
| 0.24.0 | <https://youtu.be/dzqTUNPFKKU> | 2026-11-25 |

0.22.0's master, thumbnails and words were made on 15 November 2026,
ahead of its release (the owner: the video and the release need not go
out together); Studio refused the upload that evening and twice more
after midnight, the day's limit used up by the ten above, and took it on
the evening of the 16th. It went up with thumbnail A alone: Studio shows
the channel no test of thumbnails yet (its edit page holds an
**A/B-Tests** button, hidden), so B and C wait for it.

0.24.0's went up on 25 November 2026, the day after its tag (the owner:
"doesn't need to go live in sync"), its 4K master in eight pieces joined
and hash-checked in the page, the details reused from 0.22.0's with its
own title and description, copyright with no issue. It went up with
thumbnail A alone: the edit page's thumbnail menu now offers **A/B-Tests**,
but it asks first for the channel's one-time identity check
(**Einmalige Überprüfung deiner Identität erforderlich**), the owner's to
make, so B and C wait for it as 0.22.0's do. The same check makes the
description's links clickable.

On 16 November 2026 the ten older videos were reworked to 0.22.0's
standard (the owner): each cut got its `YOUTUBE_HOOK` and one thumbnail in
the series' layout, and its video the new title and that thumbnail from
its edit page (the title box, the thumbnail given to `#file-loader`,
**Speichern**, which takes up to fifteen seconds to settle before the
page lets go). Their descriptions already had the form; 0.22.0's, written
before its tag, got its release date and its release's link then, and the
cut's extra sentence on the dive went once the notes said it themselves.
0.21.0's still, which Studio had picked when the day's custom thumbnails
ran out, went with that.
