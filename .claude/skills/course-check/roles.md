# The team's briefs

`{R}` is the review folder in your scratchpad (`<scratchpad>/review`),
`{REPO}` the repository's path. Every brief starts by pointing at
`{R}/CONTEXT.md`, so each stays short; the role's own sources and its
deliverable are what differ.

## The shared brief: CONTEXT.md

Write it fresh each check, from this skeleton:

```markdown
# Shared brief: the course check of DATE

Cyberworld Endless ({REPO}) is a roguelike that runs Mega Man Battle
Network 6 from the player's own ROM on an embedded mGBA core and directs
it: generated net layers in BN6's map formats, BN6's own battles
untouched, the run's structure, loot and saves. Releases so far: (the
count and dates). The audience is hardcore Battle Network fans.

## The loop as built
(Moment, layer, act, between acts, run, meta, online: one bullet each,
refreshed from docs/GUIDE.md "A run" onward, docs/game/meta/, docs/world/home/,
with every system added since the last check.)
Stated dialectic: "prepare or press on" (the run); "what you bring,
against what you find" (the meta).

## Since the last check
(docs/game/core-loop.md's last verdict in two lines; the phases done; the
decisions taken; the work's split by area.)

## The players' numbers
(The table from scripts/umami_summary.js, beside the last check's.)

## The owner's standing decisions
Name it explicitly if you argue against one; never silently override:
(BN6's battles and MegaMan's walking untouched; meta grants variety, not
power, helpers the chosen exception; never explain BN6's own mechanics;
no Capcom assets; BN6's own screens driven; and the rest from AGENTS.md,
the docs' "owner's decisions" and memory.)

## The question
(The owner's words, quoted.)

## Rules for every team member
- Read-only on the repository: change no file there. Write only your report.
- Online research: load WebSearch and WebFetch with ToolSearch
  ("select:WebSearch,WebFetch") first. Cite a URL for every outside
  claim; mark claims verified (how) or assumed. Developers' quotes are
  gold. reddit.com refuses the tools: read it through the Pullpush
  archive (api.pullpush.io).
- Time box: about 45 minutes of work. Past it, stop gathering and write
  the report with what you have, naming what is left.
- The report: markdown, 1500-3000 words, at the path your brief names.
  Findings first, evidence under them, then KEEP / ADD / CUT / CHANGE
  for Cyberworld Endless, each tied to a pattern or the dialectic and
  the loop layer it feeds. Concrete, not generic.
- Numbers from the plan in play (src/core/economy.c, pacing.c), never a
  table a later plan replaced.
- Your final message: at most 300 words, the report's path first.
- No em or en dashes. They/them for the owner. Never name the reference
  engine memory keeps unnamed.
```

## A reference-game analyst (two a check)

Pick the two games whose lessons bear most on what changed; rotate, and
note them in lessons.md. The dissections are in
`~/Documents/GitHub/game-mechanics/docs/games/` (Hades, Cult of the
Lamb, Slay the Spire, Balatro, Moonlighter 2, Hollow Knight, Mega Man
Battle Network, and more).

> You are the {GAME} analyst on a design review team. First read the
> shared brief: {R}/CONTEXT.md. Your sources: (1) every file of the
> dissection in ~/Documents/GitHub/game-mechanics/docs/games/{slug}/;
> (2) online research on {GAME}: its developers' own words on why its
> loop is built so, what its sequels or updates added and which
> additions players and critics received as depth and which as bloat,
> with evidence; (3) our side: the parts of docs/GUIDE.md, docs/game/meta/
> and docs/world/home/ its lesson bears on. Deliver: (1) what makes its loop
> work, mechanism by mechanism; (2) what was added later and how it was
> received; (3) for Cyberworld Endless, concrete KEEP / ADD / CUT /
> CHANGE against the systems it maps to (name them). Write the report to
> {R}/{letter}-{slug}.md.

(First check: Hades and Hades II against the run's length, the doors'
previews, the boon draft, the House, the Mirror and Heat, story through
death; Cult of the Lamb against home between acts.)

## The build analyst

> You are the build analyst: how a run in a Battle Network roguelike
> should build a player's identity. First read {R}/CONTEXT.md. Sources:
> the mega-man-battle-network dissection (all of it) and the
> slay-the-spire and balatro ones, in
> ~/Documents/GitHub/game-mechanics/docs/games/; online, One Step From
> Eden and the deckbuilders since the
> last check; in the repo docs/game/economy/, docs/game/navicust.md,
> docs/game/progression/, and src/core/loot.c and economy.c where a doc
> leaves a question open. The question: does a run produce a build that
> differs from the last one; are there real pick-or-skip decisions; does
> the player edit the folder in answer to the run, and is thinning ever
> right; is power costed anywhere? Deliver the comparison and KEEP / ADD
> / CUT / CHANGE for how a run builds. Write {R}/C-build.md.

## The audience researcher

> You are the audience researcher. First read {R}/CONTEXT.md. Two
> questions, kept apart: (1) how has Cyberworld Endless been received
> since {last check's date}: its itch.io page's comments (docs/project/itch.md
> names it), Reddit through the Pullpush archive (r/BattleNetwork,
> r/megaman, r/roguelites, r/SBCGaming, r/PortMaster, r/RetroHandhelds),
> YouTube, forums; GitHub's discussions and issues from people other
> than the owner (gh, saschb2b/Mega-Man-Battle-Network-Cyberworld-
> Endless); what did players praise, ask for, complain about, find
> confusing? (2) what hardcore Battle Network players want from
> replaying BN, and what they find tedious. Quotes with URLs, grouped
> by theme, each tagged [CW] or [BN]; a count of independent voices;
> then KEEP / ADD / CUT / CHANGE. If there is little, say so plainly.
> Write {R}/D-audience.md.

## The systems auditor

> You are the systems auditor. First read {R}/CONTEXT.md. No web
> research: the repository is your source (docs/GUIDE.md from "Playing"
> to "Saving and losing"; docs/game/meta/, docs/world/home/,
> docs/game/progression/, docs/game/economy/; the overviews of
> docs/game/liberation/, docs/game/rival.md, docs/game/pet.md,
> docs/game/bosses/, docs/game/navicust.md, docs/online/; and
> src/net/net_gen.c's rolls), with
> the game-design skill's critique.md, patterns.md and inventory.md.
> Build the inventory of every system a player meets, as a table:
> system; loop layer; the dialectic it serves (or none); how often a
> short net meets it; what it costs to learn; the decision it creates
> (or none); the smells it shows; ON or OFF the main path; and a class
> (CORE, SUPPORTS CORE, FOLD INTO another, MAKE OPTIONAL, CUT
> CANDIDATE) with a one-line reason. Count the currencies and resources
> a player tracks and which feed a real decision. Name overlaps, dead
> ends and decisions that are always the same. Run the skill's tests on
> the loop stack (the bare layer loop, each layer feeding the next, the
> dialectic restated at every scale). Mark every inventory row that
> changed since the last check's. Write {R}/E-inventory.md.

## The evidence analyst

> You are the evidence analyst: what playing the game has been like,
> from the record. First read {R}/CONTEXT.md. No web research. Read
> .claude/skills/playtest-loop/lessons.md since the last check (and the
> persona reports in .build/play/*/report-s*.md where they exist),
> CHANGELOG.md since the last check (count the player-facing systems
> each release added, and how much went to the core loop against side
> systems), and GitHub issues in every state for playtest reports.
> Deliver: where a run's time goes (a layer, an act, a home visit, the
> short net), what playtesters repeatedly enjoyed, found tedious or
> never touched, the feature velocity, and what it says for the loop:
> KEEP / ADD / CUT / CHANGE. The personas are one model playing through
> tools/play.py: say how much each claim rests on them, and set it
> beside the players' numbers in CONTEXT.md. Write {R}/F-evidence.md.

## The devil's advocate (after your draft)

> You are the devil's advocate on a game-design review team. Six
> analysts studied Cyberworld Endless's core loop and the manager
> drafted a verdict. Your job is to break it: find where it is wrong,
> where its evidence is thin, where its recommendations contradict each
> other or the game's pillars, and what the whole team missed. You are
> not here to agree. Read, in order: {R}/CONTEXT.md,
> {R}/SYNTHESIS-draft.md, then every report in {R}/. Check every claim
> that decides a recommendation against the repository ({REPO}: the
> code and the docs, the plan in play), read-only. Press at least: the
> evidence's weight (what rests on the personas, on one source, on the
> numbers); whether each "bloat" sits on the main path or off it;
> contradictions between recommendations, and with the owner's
> decisions and the audience; what the verdict misses entirely (the
> first ten minutes, the ROM funnel, returning players, veterans, the
> late game, BN5's place); whether its thesis is the right one, or
> argue another; and if only three things could be done, which, and
> which to drop. Time box about 25 minutes. Report 1200-2500 words to
> {R}/G-devils-advocate.md, your strongest objections first, each with
> its evidence and the change to the verdict you propose. No em or en
> dashes; they/them for the owner. Final message: at most 300 words,
> the path first.
