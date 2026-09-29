# The rival: Chaud and ProtoMan

A run's guardians are the Nest's copies. One Navi on the net is not: the
real ProtoMan, with Chaud behind him, who hears Lan is diving the
Cyberworld and wants to see how good he is. He does not fight MegaMan at
first. He races him, on BN6's own verb: busting viruses, timed by BN6's own
DeleteTime. Reasoned with the game-design skill; the owner chose a rival
netbattler (over license exams and the Nest's trials) and access as what
his respect opens.

## The experience

"Someone out there is faster than me, and he is keeping score." A duel is
not a fight dressed up with a prize: it is a time to beat, set by a rival
who remembers every duel, gets faster when he loses, and opens doors when
he is beaten. A player seeks ProtoMan out because the record is theirs and
visible, the target is concrete before they commit, and every win moves
the rivalry toward something they can see coming: the next rung, the next
door, and at the top ProtoMan himself.

Aesthetics (the eight kinds of fun): Challenge first, then Narrative and
Fantasy (BN's own rival, Chaud, keeping Lan honest) and Achievement.
Motivation: competence (a time certified by BN6's own results screen),
autonomy (take the duel or not, with the folder you built), relatedness
(Chaud reacts to your record, not to a script).

Not: a Server with a prize, another battle to farm, or a rival who is
just a harder guardian.

## The dialectic

The run's, restated: **prepare or press on**. A duel costs HP and a hand
of chips in the middle of a layer and gives nothing that makes MegaMan
stronger. It pays in standing (the record) and access (what Chaud's
clearance opens), which only matter to a player who plays on.

## The loop

| Loop | What the rival adds |
| --- | --- |
| Moment | A busting duel: one squad, ProtoMan's time, one term |
| Session | At most one duel an act, announced as its layer begins (Chaud's call), ProtoMan waiting on the layer |
| Meta | The head-to-head record, ProtoMan's times (faster each time he loses), Chaud's clearance, and at the top a netbattle with ProtoMan |

## Patterns

- **The rival as a relationship** (Hades' Thanatos, who turns up in a
  chamber and competes for the same kills, his record and his words
  growing across runs): the rival competes on the core verb, not a new
  one, and the relationship is the reward the player feels first.
- **The staircase, one lesson a rung** (Ascension, Heat): each rung of the
  rivalry adds one term, so each duel teaches one thing.
- **Telegraph honestly**: the squad, ProtoMan's time and the term are said
  before the player accepts; the verdict is BN6's own DeleteTime.
- **Fair challenge, cheap retry**: a lost duel costs only the HP spent,
  the time to beat stays known, and ProtoMan comes back.
- **Meta as variety, not power**: Chaud's clearance opens places, not
  stats (phase two).
- **Name failure in the fiction's voice**: "Too slow, Lan." "Log out,
  ProtoMan."

## The duel

1. **Chaud's call.** On a duel's layer, after the arrival words, Lan's
   PET rings: "Lan. It's Chaud. ProtoMan's on this layer. He busted a
   squad here in 0:14.20. Think MegaMan can do better?" L's briefing
   names ProtoMan's way, and the map marks him.
2. **The ring.** ProtoMan stands on the layer, off the way on. His words
   give the terms, the squad and his time; the choice starts on No.
3. **The squad.** A formation of the act, a little above its band: the
   same viruses ProtoMan busted, fixed by the layer's seed.
4. **ProtoMan's time.** Set from the squad's HP so that a good hand beats
   it and a slow one does not, then eight percent faster for each duel
   he has lost at that rung, never under a floor. Tuned by playtests.
5. **The terms, by rung** (the rivalry's wins, in any run):
   - rung 0: beat his time;
   - rung 1: beat his time without taking a hit;
   - rung 2: the netbattle: ProtoMan faces MegaMan himself (phase two);
   - after it, the rungs again, tighter.
6. **The verdict.** BN6's results screen shows the DeleteTime; then
   Chaud: a win ("...0:12.80. Not bad, Lan. ProtoMan, we train
   harder."), a loss ("0:15.10. Too slow."), a hit taken on rung 1 ("You
   took a hit. That doesn't count.").
7. **The record.** Wins and losses in the profile; Dad's Records mail
   lists "Chaud and ProtoMan 2-1", and the run's summary names a duel won.

## What BN6 gives it

- The DeleteTime: a u32 at `0x020348C0`, frames the battle has run (it
  holds on the Custom screen and in the pause), which the results screen
  shows as seconds and hundredths, cut (616 frames: "0:10:26").
  docs/ROM_DATA.md.
- A hit: MegaMan's HP in his battle object, lower at any frame than as the
  battle began.
- Chaud's face (the chat's `@C`), ProtoMan's overworld sprite (`0x3B`)
  and his navi (`11`, for the netbattle).

## Phases

1. The duel: the call, ProtoMan on the layer, the terms of rungs 0 and 1,
   the battle, the verdict, the record. LAYER_MAKE (a new object).
2. The netbattle rung, and Chaud's clearance: official gates on later
   layers, sealed for a Netbattler without it, each holding what only an
   official's pass reaches.
3. Tuning with the playtest loop: ProtoMan's times against real hands.

## What could go wrong

- **Times that are not fair**: a squad whose time depends on a lucky
  draw. The squad is fixed and its time set from its HP; the first duels
  stay loose until playtests set them.
- **A duel that reads as a chore**: one an act at most, never on a
  guardian's layer, and never forced.
- **The copy and the real ProtoMan**: the Nest copies ProtoMan as a
  guardian too. Chaud says so ("A copy. It'll never match the real
  thing.") the first time the copy is met.
