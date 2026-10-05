# Voice

The game's words are read in BN6's own chat box, beside BN6's own faces, so
they have to sound like BN6. A player wrote that our dialogue "sometimes
feels like I'm reading a summary of a regular sentence" (October 2026): our
boxes held too much, said it flatly, and explained where BN6 would just
talk. This page is the measure for every line a character says: MegaMan,
Lan, Mr.Prog, the Net Dealer, the bystanders, the guardians, Dad, Chaud,
the townsfolk and the BBS. Menus, the README and the site keep their own
plain style.

## What BN6's script does

Measured over BN6's whole English script (the bn6f disassembly's 916 text
files, read locally, never copied here; docs/SOURCES.md):

| | BN6 | ours before this page |
| --- | --- | --- |
| Words in a box | MegaMan 5.6, Lan 6.0, Mr.Prog 6.6, Navis 6 to 7, Dad 7.3 | 8.8 |
| Boxes ending in "!" | about 2 in 5 (MegaMan and Lan: 3 in 4 have one) | 1 in 3 had one |
| Boxes ending in "..." | about 1 in 6 | rare |
| Colons, semicolons, parentheses in talk | almost none | common |
| A comma | `Lan,let's go!` (no space after it, always) | `Lan, let's go!` |

So: **one beat a box, six words or so, emotion before information.**

## The rules

1. **One thought a box.** A box is a breath: a reaction, a fact or a call to
   act, rarely two. Six words is normal, twelve is the most. More to say
   means another box, and three boxes is a lot.
2. **Talk, don't summarize.** No colons, semicolons, parentheses or dashes
   in a character's mouth. No "which", "whose" or "where" clauses strung on
   a fact. If a sentence needs a comma to hold two facts, make it two
   sentences, or drop one.
3. **Feel first.** Characters react before they inform: "Whoa!", "Huh?",
   "Alright!", "Yikes...", "Phew...". Exclamation marks are normal, "!!"
   for big moments, "?!" for shock, "..." for a pause, a trailing thought
   or worry.
4. **Say only what the player needs now.** A line exists for one fact the
   player acts on: where, how much, what to press. The rest belongs in the
   README. Never repeat what the screen already shows.
5. **Write it as BN6 writes it.** A comma has no space after it (`Lan,look!`),
   `Mr.Prog` has none either, `Zenny` takes a capital, and so do `Pack`,
   `Folder` as a PET screen, `NaviCust` and `BugFrag`. Chip, Navi and
   area names are spelled as BN6 spells them (`CentralArea`, `SkyArea` in
   BN6's own lines; we keep our generated areas' names as the game's map
   names show them).
6. **Each character sounds like themselves.**

| Who | How they talk |
| --- | --- |
| **MegaMan** | Earnest, warm, a little careful. Calls Lan by name, often first: "Lan,look!". Says "we" and "let's". Warns and cheers: "Watch out!", "We can do it!" |
| **Lan** | An excited kid. Short bursts: "Alright!", "Let's go,MegaMan!", "No way!", "Huh?". Jumps to action. |
| **Mr.Prog** | ALL CAPITALS, simple and cheerful, sometimes says a word twice: "WELCOME,WELCOME!" |
| **Net Dealer and shop Navis** | A friendly seller: "Welcome!", "Take a look!", "Come again!" |
| **Bystander Navis** | Chatter, rumors, small complaints, a tip told as gossip: "Did you hear? ..." |
| **Townsfolk** | Everyday people: kids, parents, shopkeepers. Short, a bit of humor. |
| **Dad** | Kind and calm. Explains in short, plain steps. "MegaMan,Lan,listen..." |
| **Chaud** | Terse and cool. "Hmph.", "Don't get cocky,Lan." |
| **Guardians** | Theatrical and proud. A taunt in, a groan out: one or two boxes each. |

## Before and after

| Before | After |
| --- | --- |
| "The Recovery Mr. Prog can patch us up. It's down and to the left: the arrow turns green and leads there first." | "Lan,a Mr.Prog can heal us!" / "Just follow the green arrow!" |
| "Welcome to the Net Dealer! Divers need chips, and I've got 'em, in your folder's codes when I can!" | "Welcome! Chips for divers!" / "I stock your codes when I can!" |
| "Net Dealers set up shop on an area's first layers, and one waits by every guardian. Save some Zenny!" | "There's always a shop near a guardian." / "Save your Zenny!" |
| "We can only carry one Cross down here, Lan, so we keep our SlashCross." | "We can only bring one Cross,Lan." / "We'll keep SlashCross." |

## Checking a line

- Read it aloud as the character. If it sounds like a manual, rewrite it.
- Count: over twelve words in a box, split or cut.
- Look at it in the game (`--talk`, docs/DEVTOOLS.md): BN6's box is three
  lines of about twenty characters; a box that pages on for one word is
  too long.
