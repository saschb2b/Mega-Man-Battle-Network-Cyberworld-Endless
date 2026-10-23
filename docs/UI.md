# The PET's screens: Battle Network's rules

The screens the engine draws where BN6 has none (docs/FIDELITY.md, The
PET's look) follow the rules below, each measured from BN6's own screens.
They were captured from Gregar running headless on 23 October 2026: the
PET menu, ChipFolder and its SELECT pop-up, the folder editor, its Pack
and its SORT pop-up, SubChip, the Library's two pages, STATUS, E-Mail,
KeyItem, the NaviCustomizer and its Yes/No, the Custom screen, a Chip
Trader's Yes/No and the battle's RESULT (`tools/play.py` on a layer;
RESULT from a romlab state). Colours are numbers read from those captures;
nothing of BN6's pictures is kept in the repository.

Not measured in this pass: BN5's screens (its ROM has no state at its PET
here; that it follows the same grammar is assumed), BN1-4's (not studied),
a shop's and the Request BBS's lists, and how a screen opens (its slide)
frame by frame.

To measure again: `python3 tools/play.py start NAME -- --scene emu`, then
`do NAME "mash B 400; press START; ..." --keep DIR` keeps each frame.

## A screen's parts

| Rule | Measured from |
| --- | --- |
| One band across the top, the screen's name in heavy white capitals, two stripes running off to its right | Every PET screen |
| A screen opened inside another takes the band over with its own name; there is no breadcrumb | CHIP FOLDER becomes FOLDER EDIT; STATUS opens NAVICUSTOMIZER |
| The band's right end holds one thing or nothing: a count in a pill, or one key's hint | FOLDER EDIT's `30/30`; E-Mail's `START:SORT`; nothing on the rest |
| Lists sit in windows: a navy field in a cyan frame, a scroll track at the right when the list runs on | Folder editor, Library, E-Mail, KeyItem |
| A window's band names its page; a green arrow at the band's end where L or R turns to the next, none where there is no page | Library: `StdChip ▶`, `◀ MegaChip ▶`; folder editor's `▶` to Pack and Pack's `◀` |
| One information box says what the cursor's item is, in one to three lines, labelled INFORMATION (KeyItem's tag beside it) or NAVIGATOR (MegaMan's face) | KeyItem: "It's your PET--your PErsonal Terminal. MegaMan's in here."; NaviCust: "RUN?", "OK" |
| A small choice opens as a pop-up window, its label in small capitals on a cyan strip | ChipFolder's SELECT (EDIT, EQUIP, CHG NAME); the folder editor's SORT (seven keys) |
| A count stands in a navy pill outlined in light green | FOLDER EDIT `30/30`, Library `12/200` |

## Keys

| Rule | Measured from |
| --- | --- |
| A takes, B goes back a level; neither is ever shown | No PET screen captured names A or B |
| START sorts with no word of it where the list shows its order | The folder editor's SORT pop-up |
| One screen names its key: the one that does something a player would not guess | E-Mail's `START:SORT`, the key white, the action orange |
| A screen that waits for a press says so in words | RESULT's `PRESS (A) BUTTON` |
| Yes and No are asked in the chat box: both words in the chat font, the green triangle before the one chosen, neither coloured | A Chip Trader's "Insert 3 BtlChips?"; "Quit the Navi Customizer?" |

## The cursor and the one action

| Rule | Measured from |
| --- | --- |
| In a list, the orange arrow at the row's left; the row keeps its colour | KeyItem, E-Mail, folder editor, Library |
| The PET menu lights its button instead of an arrow | PET menu |
| A choice, or the one action on a screen, takes the green triangle | Yes/No; NaviCust's `▶ RUN!` |
| The one action is a button: bright green, heavy white capitals | NaviCust's RUN! |

## Letters

| Use | Letters | Measured from |
| --- | --- | --- |
| A screen's name | Heavy capitals | Every band |
| A label | Small capitals, white | INFORMATION, NAVIGATOR, SELECT, SORT, BUSTER; the PET menu's HP, ZENNY, BUGFRAGS (heavy) |
| A list's lines, the information | The chat font, mixed case | KeyItem, E-Mail, the info boxes |
| A chip's name | The bold battle font | Folder editor, Library, Custom screen |

## Colours

| Role | RGB | Where |
| --- | --- | --- |
| Band | 0, 123, 49 | Every band |
| Body | 8, 189, 115 | The screen under its windows |
| Window frame | 107, 222, 255 / 66, 198, 231 / 16, 140, 239 | Light edge, band, blue edge |
| Window field | 16, 82, 107 | Lists |
| List letters | 247, 255, 247 on a 33, 115, 140 shade | Lists |
| Information | 74, 231, 115 box, 247, 247, 247 page, 66, 57, 57 ink | KeyItem, E-Mail |
| Cursor | Orange arrow (255, 173, 16 body) | Lists |
| Choice | 57, 255, 0 body, 115, 255, 74 light, 33, 165, 33 outline | Yes/No, RUN! |
| Page arrow | 74, 198, 16 body, 189, 231, 132 light, 8, 165, 0 dark | Library, folder editor |
| Count | 16, 82, 107 pill, 173, 255, 189 outline and digits | FOLDER EDIT |
| The one action | 16, 231, 41 | RUN! |
| A key's action | 247, 173, 33 | E-Mail's SORT, nowhere else |

Orange is the cursor's and E-Mail's SORT; green triangles are choices.
A colour carries a role, not decoration: E-Mail's sender in cyan, NEW on a
new mail, a chip's code in yellow.

## Ours

What the engine's screens take from these rules (pet_look.c draws the
parts: `look_band_hint`, `look_arrow`, `look_choice`, `look_cursor`):

- No A or B hints: the Jukebox, Options, the statistics question, Trade
  Central's two screens and the emote strip show none.
- One key's hint on the band where the key does something unguessed:
  the Jukebox's `START:REPEAT` (the mode), the reconnect wait's `B:GO
  HOME` (leaving the Square). None at the foot.
- Pages turn by L and R under green arrows: the Jukebox's sections, the
  emote strip's two pages.
- A section takes the band: Options' Display and Audio.
- A list's line keeps its colour under the cursor (Options).
- Yes and No as BN6 asks them: the statistics question.
- The second screen's HP, ZENNY and BUGFRAGS named as the PET menu
  names them.
- Words that explained the screen are gone ("Pick a song and press A.",
  "Spares: the copies your folders don't hold"); what the cursor's item
  is stays in the information box.

Kept where BN6 has no screen like it: the controls screen's keys line (a
remapping screen: what Enter, Backspace and Tab do there is its content),
and the ROMs screen's and SAVES' words (before a ROM is read, in the
engine's pixel font). On phones the rows and buttons are the tap targets;
no hint says so.
