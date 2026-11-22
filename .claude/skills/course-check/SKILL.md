---
name: course-check
description: Step back and check Cyberworld Endless's course - whether the core loop is still the direction, what it lacks, what has bloated - the way the October 2026 review did it. You manage a time-boxed team (two reference-game analysts, a build analyst, an audience researcher, a systems auditor, an evidence analyst), read the players' own statistics first, draft a verdict, have a devil's advocate break it, ask the owner what reverses their calls, and write the result into docs/game/core-loop.md. Use when asked to step back, reevaluate the core loop or the game's direction, look for bloat or feature creep, run a course check or a whole-game design review, steer the project back on course, or every few minor releases. For one mechanic or screen, use the game-design skill alone.
---

# The course check

Fifteen releases in a month taught the project that it can ship faster
than it is played, and grow sideways while its core waits. The course
check is the step back: a small team looks at the whole loop from
outside (reference games, the audience, the record, the players'
numbers), you draw a verdict, an adversary tries to break it, and the
owner decides what reverses their own calls. Its output is
docs/game/core-loop.md, kept as the living answer to "are we on course?".

Read first:

- `lessons.md`: the checks run so far, what each overturned, and what
  the last one said to measure again;
- docs/game/core-loop.md: the last verdict, its phases and where they stand,
  the owner's decisions;
- the game-design skill (`.claude/skills/game-design/SKILL.md`, its
  `references/design/critique.md` and `patterns.md`): the lens every
  role uses.

## When, and what it costs

Every three to five minor releases, or about a month; when releases
outrun playtests; when the statistics or the players' words sag; before
choosing a minor release's theme; whenever the owner asks.

The full check is seven agents (six and the devil's advocate). The
first ran about an hour of wall clock and 1.7 million tokens of
subagents (about 200,000 to 300,000 each; each finished in 7 to 12
minutes inside a 45-minute box). When the owner asked only "are we on
course?", offer the **light check** first: steps 1, 2 and 6 alone (the
numbers, what changed since the last check, your verdict against
docs/game/core-loop.md's), no team.

## The check

1. **What changed since the last check.** Pull `dev` first: the first
   check read a tree a release behind and called a shipped system
   unbuilt. Then CHANGELOG.md and `git log`
   since the date of docs/game/core-loop.md's last review: the systems added (on
   the main path or off it), the decisions taken, how far each phase got.
   Count where the work went (core loop, BN5, online, PET, platforms), as
   the first check did.
2. **The players' numbers, before anyone argues.** The opt-in statistics
   on the owner's Umami (docs/GUIDE.md, Anonymous statistics): ask the
   owner to sign in in Chrome, then run `scripts/umami_summary.js` in
   the page (`javascript_tool`) for the period since the last check.
   Read it against the last table in docs/game/core-loop.md. Give the table to
   every role: in the first check the numbers came last and overturned
   two findings the team had agreed on (people skip half the Mystery
   Data; guardians are a formality). Close the tab after.
3. **The shared brief.** Write `CONTEXT.md` in the scratchpad from
   `roles.md`'s template: the loop as built (refreshed from docs/GUIDE.md
   "A run" onward, docs/game/meta/, docs/world/home/), the owner's standing
   decisions (from AGENTS.md, the docs and memory), the question, the
   statistics, the rules.
4. **The team**, in one message, every agent in the background with its
   brief from `roles.md`: two reference-game analysts (rotate the games,
   lessons.md says which were used), the build analyst, the audience
   researcher, the systems auditor, the evidence analyst. Each writes
   `review/<letter>-<role>.md` in the scratchpad and returns a summary.
   Start a background timer for the 45-minute box; past it, a wrap-up
   message (memory: time-box subagents). Read each report as it lands.
5. **The draft verdict**, yours, one page in `review/SYNTHESIS-draft.md`:
   the verdict in a sentence; what works and must not move; what it
   lacks; what has grown (on the main path, where the player meets it,
   apart from off it); the next evolution as phases, measured one at a
   time; deferred until measured; not proposed. Where two or more roles
   reached a finding apart, say so: that is the strongest signal. A
   finding one role made alone is marked so.
6. **The devil's advocate**: a fresh agent with the brief, the draft,
   every report and the repository, its brief in `roles.md`. In the
   first check it changed the verdict: a score trend that was really two
   bugs, numbers from a superseded plan, a cadence target that
   contradicted the cadence complaint, four proposals dropped.
7. **Revise in writing.** Each objection accepted or answered, with why.
   Then the owner: every recommendation that reverses a decision of
   theirs (the docs and memory name them) is asked, not decided
   (AskUserQuestion, your recommendation first, two or three options);
   the rest is written as proposed.
8. **Write it down.** docs/game/core-loop.md: the dated review (its verdict,
   the statistics' table beside the last one, what works, lacks and has
   grown, the phases, deferred, not proposed), the owner's decisions
   with their date. docs/ is an Open Knowledge Format bundle: keep the
   page's frontmatter and docs/game/index.md's entry for it in step (the
   okf skill where its shape changes). The reports go to `.build/research/course-check-DATE/`
   (never committed: players' handles and quotes). An entry in
   `lessons.md`. Then tell the owner briefly: the verdict, what works,
   what it lacks, what has grown, the order of work, and the decisions
   that reverse theirs.
9. **Act on the first item.** Offer to do Phase 1's first item in the
   same session; the first check's was ProtoMan's netbattle, which the
   numbers showed ending a third of lost runs.

## Rules

- **The lens is the game-design skill's**: the dialectic restated at
  every scale, the loop stack, the bare core test, the critique smells.
  Bloat is counted where the player meets it: the main path's words,
  rules and walks, not the number of systems.
- **Weigh the evidence.** The players' statistics, then players' own
  words, then the persona playtesters. The personas are one model: they
  clear every layer, stay at threat 1 and score generously; their trend
  across sessions is confounded by fatigue and bugs.
- **Numbers from the plan in play**, checked in the code (`economy.c`,
  `pacing.c`), never a table a later plan replaced.
- **The owner's standing decisions** are named whenever a role argues
  against one; never overridden in a doc or code without their call.
- **Never name** what memory keeps unnamed (the reference engine) in
  anything committed or published, nor players' handles in committed
  docs.
- **Writing**: no em or en dashes; they/them for the owner; plain
  prose, the project's.
