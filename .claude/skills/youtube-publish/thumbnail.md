# The thumbnail

The owner (15 November 2026): the trailers need "a definition of the
thumbnail used for youtube. we need to follow more 'clickbait'-ish
youtube trends and see what good tuhmbnails on youtube means."

Until 0.21.0 a video's thumbnail was the trailer's poster: its title
card, a logo and a line of small capitals on navy. That is a fine poster
for the site, where the visitor already chose the project. On YouTube it
sits at 168 pixels wide between other videos, and says nothing a stranger
would click.

## What a good one does (looked up, 15 November 2026)

- **It is read small.** Most views see it at phone size. Whatever does
  not read at 168 wide is not there.
- **One subject, large, in high contrast and saturated colour**, and
  **few words in large heavy type**: three or four at most, the hook
  itself, not the video's title again (the title already sits beside it).
- **It promises one concrete thing** the video then shows. A thumbnail
  that wins the click and loses the viewer costs reach: YouTube's own
  test picks its winner by watch time share, not by clicks.
- **It is tested.** Studio's **Test & compare** takes up to three
  thumbnails for one video and names the one that held viewers best
  (computer only; not for a private video or one made for kids).
- **Faces draw clicks.** We have none of our own to show, and a Navi's
  mugshot cut out as an asset is not ours to make (below): the subject is
  the game's own moment instead, a blast, a boss mid-strike, a new screen.
- Size: 16:9, 1280x720 at least; under 2 MB always uploads (a 50 MB limit
  for 4K thumbnails on computers was reported in 2025, not confirmed on
  the help page: 1280x720 stays the size here).

Sources: Google's help page "Test & compare thumbnails"
(support.google.com/youtube/answer/13861714); vidIQ's and thumbnailtest.com's
2026 guides for the practice, both sellers of thumbnail tools, so their
numbers are leads, their agreement on the points above the finding.

## The layout (fixed: the blueprint's half)

`tools/trailer_thumbnail.py` draws it as an SVG and Chrome turns that
into the PNG, so the layout is one template and no thumbnail drifts from
the series:

- **The picture**: a region of one whole frame of the trailer's footage,
  filling the thumbnail at a whole scale (240, 160 or 120 game pixels
  wide: 5.33x, 8x, 10.67x of nearest-neighbour pixels, drawn at 3840 and
  shown at 1280). Its subject sits in the right two thirds.
- **A navy scrim** from the left and from below, under the words.
- **The PET's cyan frame** round the edge: the series' mark at any size.
- **Top left**: the logo, under it the version on BN's plate in gold and,
  on the current trailer's alone, a green **NEW** (an older release's
  thumbnail is not news; `'new': True` or `False` in a spec overrides).
- **Bottom left**: the words, two lines of Lato Black Italic as large
  as the longer line allows (up to 190 px), white and gold, a thick navy
  edge and a hard shadow. Not the site's Silkscreen: its bold S, W and 4
  close up at 168 wide ("NOW" read as "NO" and a heart on the first
  sheet), and the heavy slanted sans is what a thumbnail is read in.
- **The pointer** (optional): the PET's orange cursor arrow, its point on
  the subject.
- **Bottom right stays clear**: YouTube lays the video's length there.
- **`'side': 'right'`** mirrors the scrim, the logo and the words (raised
  clear of the length stamp), for a subject on the left: in a battle
  MegaMan always stands there.

## The release's half: `THUMBNAILS` in the cut

Up to three, different in what they promise (one headline each), not
three crops of one idea:

```python
THUMBNAILS = [
    {'hero': ('cold-a', 80),         # scene, sample (or ('still', NAME), a docs/screenshots picture)
     'region': (80, 40, 160),        # x, y, width in game pixels; 16:9
     'words': ('CLOUD', 'SAVES!'),   # two lines, four words in all, nine letters a line at most
     'lit': 1,                       # the gold line
     'side': 'right',                # optional: the words and logo on the right (a subject on the left)
     'point': (176, 88)},            # optional: the cursor's point, in the frame's pixels
]
```

Rules for it:

1. **The words are the hook, and true.** What a player would shout to a
   friend, and what the trailer shows within its first segment or names
   on a caption: `CLOUD SAVES!`, `NOW ON PSP!`, `3 GAMES, 1 RUN`. Never
   the title's words again, never a claim the CHANGELOG does not hold,
   never "you won't believe". An exclamation mark or a number, not both
   lines shouting.
2. **The hero is the release's most striking frame**, bright and busy at
   its subject, calm where the words go: a hit landing, a Navi
   mid-attack, a screen that did not exist before. It need not be the
   picture of the words: a blast under `CLOUD SAVES!` is honest (it is
   the game), a menu's page under them is not clickable.
3. **Never a sprite cut out.** The hero is a region of a whole frame,
   half the frame's width at least, with everything round its subject
   kept (AGENTS.md's rule on Capcom's graphics: whole frames, no sprite
   or mugshot isolated as an asset). The subject is lifted by where the
   region is set, by the scrim and by the pointer, which are ours.
4. **Read the check sheet** (`...-thumbnail-check.png`): at 168 wide, on
   the dark page and the light one, both lines read at a glance and the
   subject is a shape, not noise. If not: fewer letters, a tighter
   region, another frame.
5. An older cut, whose footage is not on this machine, takes its hero
   from a picture of its release's notes: `('still', NAME)`, a whole
   240x160 frame in `docs/screenshots/`. The ten before 0.22.0 were given
   one thumbnail each this way on 16 November 2026.
6. Thumbnail A is the one a video is published with; all three go into
   Test & compare where the channel has it, and the winner is noted in
   docs/project/youtube.md's table.

## The title, with it

`Cyberworld Endless VERSION Trailer – TAGLINE` put the project's name
first, which only someone who knows it searches for. A cut's
`YOUTUBE_HOOK` leads instead: `Cloud Saves, the PSP and 3D on the New
3DS | Cyberworld Endless 0.22.0 Trailer`, the news in the first forty
letters (what a phone shows of a title), the name and version after it.
Under 100 letters; `tools/trailer_youtube.py` falls back to the old form
for a cut with no hook.

## Making them

```sh
python3 tools/trailer_thumbnail.py VERSION      # after the trailer's footage is there (tools/trailer.py --footage)
```

writes `.build/youtube/cyberworld-endless-VERSION-thumbnail-a.png` (`-b`,
`-c`), each one's `.svg`, and the check sheet. `tools/trailer_youtube.py`
then lists A as the video's thumbnail and all of them under `thumbnails`
in `videos.json`. Nothing of it is committed: like the masters, they are
made from the tree when needed.
