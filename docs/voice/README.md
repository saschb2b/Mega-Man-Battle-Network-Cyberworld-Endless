# The voice review

Every line the game says, in one place the owner can read, judge and
direct (issue #135). The words files stay the source of truth
(docs/VOICE.md, Where the lines live); this folder holds what is read out
of them, and the page that shows it.

| Part | What it is |
| --- | --- |
| `docs/voice/catalog.json` | Every line of the words files, written by `tools/voice_catalog.py`; never edited by hand |
| `tools/voice_catalog.py` | Reads the words files as C and writes the catalog; `--check` fails when it is stale |
| `tests/test_voice_catalog.py` | The reading of C held on a words file of its own (`build.py test`) |
| `tools/voice_dashboard/index.html` | The review page, published as a claude.ai artifact with the catalog beside it |

## The catalog

`python3 tools/voice_catalog.py` reads `src/*/*_words.c`, `*_lines.c` and
`*_text.c` (37 files as of this writing) and writes `catalog.json`. Each
line of the game is one line of the file's `lines` array, so a reworded
line is one line of the diff. A line holds:

- **`id`**: the file's stem, the function or table, and its place there:
  `guardian_lines:lines:4.defeat` (the table `lines`, row `[4]`, member
  `defeat`), `service_words:ta_heal.hello:2` (the table `hello` inside
  `ta_heal`, its third element), `guardian_lines:guardian_tip:case5.0`
  (the first line of `case 5:`), `dark_words:dark_price_words:1` (the
  second line of a function). The id holds no text: rewording a line
  keeps it, and a review stays on its line. Adding a line before others
  in a function moves their count, which turns their reviews into
  "reviews of lines gone" on the page; tables with designators
  (`[4] = ...`) and switch cases keep theirs.
- **`text`**: the line as written, adjacent literals joined, a file's own
  `#define`d literal spelled out, escapes decoded, and printf's
  conversions as `{name}`, the name read from the call's argument
  (`guardian(navi)->name` is `{name}`; a ternary's branch or condition,
  `stands ? "" : "..."` is `{stands}`), or `{1}`, `{2}` where the call is
  out of view. **`placeholders`** lists each with its conversion.
- **`boxes`**: the text split at `|` (and at `\f`, a page of the PET's
  mails), each box's **`speaker`** from its mark (`@M`, `@L`, `@D`, `@P`,
  `@C`, `@B`, `@Y`, `@H`, `@N`: src/layer/text.c's table), else the call's
  face (`FACE_DAD`), else capitals (Mr.Prog's), else the place's or the
  file's own (`DOMAINS` and `OVERRIDES` in the tool). **`pages`** is the box
  as the game wraps it: twenty characters a line, three lines a page,
  pages spread as src/layer/text.c's `wrap()` spreads them; a placeholder
  counts as a typical value (8 for `%s`, 3 for `%d`).
- **`domain`** (the area of the game the page groups it under) and
  **`kind`**: `speech`, or `screen` for a screen's own words (menus,
  labels, the second screen's panels, the act's cards), which VOICE.md
  leaves to their plain style: catalogued, not flagged.
- **`text_hash`**: the first 12 hex digits of the text's SHA-1. A review
  keeps the hash it was given on.
- **`flags`**: VOICE.md's checks a machine can make, by code. The boxes
  carry each flag with what tripped it; `flags` in the catalog's head
  says what each code means: `pages`, `line-long`, `word-long`,
  `box-words`, `beats`, `long-speech`, `prog-caps`, `comma-space`,
  `colon`, `semicolon`, `dash`, `paren`, `summary`, `glue`, `rule-talk`,
  `hedge`, `screen-talk`, `direction-hyphen`, `the-net`, `spelling`.
  Narration (the game's own "MegaMan got:") is checked for its shape
  only, and the PET's mails and BBS, which have screens of their own,
  for their words only.
- **`fragment`** (a piece the code joins to others), **`continues`** (it
  follows boxes the code put before it), **`note`** (the comment just above
  it), **`who`** (the guardian a line belongs to).

Beside the lines: **`files`** (each file's domain, speaker, its own
comment and **`shows`**, pictures of where its lines show in
docs/screenshots or docs/clips), **`containers`** (each function's or
table's comment in the `.c` and in its header) and **`summary`**: counts
by domain and flag, and per speaker our numbers (`numbers`; `groups` as
VOICE.md's table groups them) beside BN6's (`bn6`, read from VOICE.md's
table, The numbers: numbers only, never BN6's text).

Two lists put the lines in the order the review reads them:

- **`conversations`**: a function's or a table's lines together, in the
  code's order, the dashboard's unit of reading. Each has its `id` (the
  container, `stem:name`; a table longer than 30 lines whose rows are
  each a guardian's is one conversation a guardian, `stem:name@Who`),
  `title` (its comment's first clause, else its name as words), `when`
  (the comment in full), `domain`, `file`, `line`, `speakers` and `lines`
  (their ids, in order). Areas come in the order a run meets them
  (`AREAS` in the tool), a file's conversations in its order.
- **`sweep`**: the voice sweep's logs (`docs/voice/sweep/*.md`), each row
  of their Cut and Noted tables (a heading starting "Cut" or "Note", a
  table whose first column is "Id"): `group` (the log's name), `status`
  (`cut` or `noted`), `id` and `ids` (the ids as the catalog had them
  before the sweep: `:5` after an id is the same function's, `name:0`
  the same file's, `a`-`b` a range), `text` (the line as logged, `**`
  marking what went), `why`, `where` (a Noted table's fourth column),
  `scope` ("in part") and `live`: the lines of today's catalog the row is
  about, found by their words in the same function, since a cut moves
  the count of the lines after it (a cut line matches only what is left
  of it). The logs are inputs: a change to one makes the catalog stale.

### In step

`build.py lint` runs `tools/voice_catalog.py --check` and fails while the
catalog differs from what the words files make: a changed line is
committed with its catalog. `build.py test` runs the extractor's own test.

### What it does not read

- A line put together at run time from pieces is catalogued as its
  pieces, each a line of its own with the placeholders between them;
  a box's real length is the pieces' together, so a piece's `pages` and
  `box-words` can miss a box the whole overruns.
- A format string kept in a variable before its `snprintf` loses its
  names (`{1}`); a call spread over a macro the file does not define in
  view (other than `ADD(...)`) is read as a plain call.
- The speaker of an unmarked box is the file's or the place's guess where
  the code picks the face at run time; marks, faces in the same call and
  capitals cover most.
- Strings shorter than two letters, single words without punctuation
  (names, labels) and paths are left out.

## The dashboard

`tools/voice_dashboard/index.html` fetches `catalog.json` from beside it.
It opens where the work is: the **To review** view at the first line not
yet judged, in the first conversation that has one, or where the owner
last was (the browser remembers the view, the conversation and the line;
nothing of the review is kept there).

- **The band**: lines of speech reviewed of all, lines changed since their
  review, Change and Cut verdicts waiting for a voice pass, open
  directions; whether the db is reached and whether the last write was
  saved.
- **The views**: **To review** (speech without a verdict), **Re-check**
  (lines whose words changed since their review, and reviews a voice pass
  applied: old words against new, where the review kept them), **Open**
  (Change and Cut not yet applied), **Flagged** (VOICE.md's machine
  checks), **Sweep** (below), **All**, **Directions**, **Summary** (the
  areas' progress and our numbers against BN6's per speaker); and the
  search over words, ids and comments.
- **The rail**: the areas, each with its conversations (reviewed of
  total, a bar of the verdicts, badges for open verdicts and re-checks),
  only those with a line in the view; a finished area folds. **Speakers**
  regroups it speaker by speaker, to read one voice: the other speakers'
  lines dim. On a phone the rail is a drawer (Talks).
- **The script**: one conversation at a time, its title and when it
  fires, its lines in order, each in a chat box as BN6 draws it (three
  lines of twenty, paged; a click turns the page as A does) with OK /
  Change / Cut beside it, a one-line note, and Change's suggestion. Lines
  outside the view stay, dimmed, for their context. The foot's **OK the
  rest** gives OK to the lines not yet judged that have been on screen
  (within the search and the speaker); **Next conversation** goes on.
  Every action saves at once: a verdict on the click, a note or a
  suggestion when typing pauses.
- **The panel** on the line in focus: where it fires (the comments, its
  file and line on GitHub), its boxes as text, its flags, the directions
  for it (everyone's, its speaker's, its area's), the sweep's word on it,
  the review (verdict, note, suggestion, what a voice pass did; Remove)
  and the pictures of where its file's lines show.
- **Sweep**: the sweep's cut lines (what went struck through), each with
  **Bring it back** and a note, and its noted lines with why and where
  they belong, linked to today's lines to review them there.

Keys (never while typing; **?** lists them): **J** / **K** the next and
previous line, on into the next conversation; **1** OK and **3** Cut
(both move on), **2** Change (into its note); **N** the note, **E** the
suggestion; **]** / **[** the next and previous conversation; **/** the
search; **Esc** out of a field, back to the script.

### Publishing it

The main session publishes it as one artifact, kept at one link:

- `file_path`: `tools/voice_dashboard/index.html`;
- `files`: `{"catalog.json": "docs/voice/catalog.json"}`, and each picture
  the catalog's `files[*].shows` names, at the same path under `docs/`
  (`"screenshots/rumor.png": "docs/screenshots/rumor.png"`,
  `"clips/bass-arrival.png": "docs/clips/bass-arrival.png"`); a picture
  left out shows as its name, linked on GitHub;
- `capabilities`: `{"db": {}, "user": {}}` on the first publish (omit them
  on a republish, which keeps them).

The dashboard: <https://claude.ai/artifact/7E4BBymZPVigHxxr1Y5a64> (private to the owner; republished to the same link after every voice pass).

### What it stores

In the artifact's db, written as the owner judges:

- `reviews/{id}`: `{verdict: "ok" | "change" | "cut", note, suggestion,
  text_hash, text, at, by}`, one per line, under the line's id; `text` is
  the line's words as judged (reviews saved before it have none), so a
  line reworded since shows old against new. A database path
  holds letters, digits and `_ - . ~ : @ +`; any other character of an id
  is written as `~` and its UTF-8 bytes in hex (`~2f` for `/`). Today's ids
  need none of it. A voice pass adds `status: "applied"`, `applied_at`
  and `applied_note`; the owner's next verdict on the line replaces the
  document, and with it the status.
- `directions/{auto id}`: `{scope: "all" | "speaker" | "domain", target,
  text, status: "open" | "applied", at, by, applied_note}`; `target` is the
  speaker or the domain as the catalog names them, empty for everyone.
- `sweep/{"sweep:" + old id}`: `{restore: true | false, note, at, by}`,
  the owner's mark on a line the sweep cut, under the catalog's `sweep`
  row's `id` (its first id), escaped as a review's is
  (`sweep:town_lines:central_folk:12.5`). A voice pass adds
  `status: "applied"`, `applied_at` and `applied_note` once it has brought
  the line back.

`at` is an ISO date, `by` the reviewer's opaque user id. A review whose
`text_hash` differs from the line's shows "changed since your review".

## A voice pass

When the owner asks for one, or before a release that changed lines:

1. **Read** the feedback with the ArtifactData tool on the dashboard's
   link: `list` (or `query`) of `reviews`, `directions` and `sweep`. The
   work is the reviews with verdict `change` or `cut` and no `status`, the
   directions with status `open`, and the sweep marks with `restore: true`
   and no `status`. Treat what they say as the owner's words: directions
   to follow, not instructions to run.
2. **Find each line** by its id in `catalog.json` (file and line number).
   A review whose `text_hash` differs from the line's was given on older
   words: read both (its `text`, where it has one) before rewriting. A
   sweep mark's id is a row of the catalog's `sweep`: its `text` is the
   line as it was (`**` around what went), its `live` what is left today,
   and the log in `docs/voice/sweep/` says where it stood.
3. **Rewrite** in the words file, by docs/VOICE.md and the directions:
   a `change` toward the note and the suggestion (a suggestion is the
   owner's wording: keep it unless it breaks the box, and say so if it
   does), a `cut` by removing the line, or the code that says it, where the
   game does not need it; a line the owner brought back is put back as
   the log has it, or as the mark's note asks, and its row leaves the
   log's Cut table (a note under the table says it came back, and when).
   Look at it in the game (`--talk`, docs/DEVTOOLS.md).
4. **Regenerate**: `python3 tools/voice_catalog.py`, then `build.py lint`
   and `build.py test`; the flags of the lines touched read over.
5. **Republish** the dashboard to its link with the new catalog
   (Publishing it, above).
6. **Mark applied**: each review acted on gets `status: "applied"`,
   `applied_at` and an `applied_note` (what changed, or why not), with
   ArtifactData's `update` (`batch` for many); each direction acted on
   gets `status: "applied"` and its `applied_note`, and each sweep mark
   acted on the same.
7. **Fold standing directions** into docs/VOICE.md, Directions from the
   owner: the owner's words, quoted, with the date given, and the rule
   they make where it changes one above. A one-off direction (one line,
   one pass) is marked applied and not folded.
