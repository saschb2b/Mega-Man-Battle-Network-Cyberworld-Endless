---
type: Feature
title: "Its life"
description: "The Square's life (`square_life.c`, words in `square_life_words.c`): two walkers on wall clock routes, a parade, BN6's log-in beam, check text, players answered together, the chat box talk, and Look around to find the parts."
tags: [online]
generated: { by: claude-code/unrecorded, at: 2026-10-23T14:25:12Z }
---

# Its life

`square_life.c`, its words in `square_life_words.c` ([Voice](../../voice/rules.md)):

- **Two walkers**, a GreenNavi and a GirlNavi, puppets as the peers are,
  walk the plan's routes (the GreenNavi round the landmark at the
  plaza's corners, four seconds at each; the GirlNavi to and fro on the
  stage's floor before it, six seconds at either end), a world unit a
  frame, each leg along a grid axis (the
  unit tests hold them on ground floor, through no hole). Their place is
  a function of the wall clock (UTC seconds and the frames since), so
  every client whose clock agrees shows them in the same place: two
  clients' logs (with `CYBERWORLD_EMU_DEBUG`, their "walkers at clock"
  lines) name the same positions at the same second. The server sends no
  clock of its own and the wire is unchanged. A faces one: the walker
  stops, turns to MegaMan and says a line built from what the client
  knows: the Navis here by name, how many, the hour, the area, the last
  Navi to log in, the last trade and NetBattle this game made; the
  GirlNavi tells one of the Square's tips as gossip (where Navis cheer).
- **A parade** every three minutes of the wall clock, thirty seconds
  long (after the Pokemon Wi-Fi Plaza's spectacle and the fan servers'
  parades): the walkers log out where they are and in on a loop three
  panels round the landmark, BN6's beam where the first does and his
  "Parade time!" over his head, and walk round it two panels apart
  without stopping; then they log back in on their routes. Their talk
  meanwhile is the parade's. Every client whose clock agrees sees the
  same parade, and players who walk with it stand round the landmark,
  where saying something together makes the Square cheer.
- **BN6's log-in beam** (list 7's sprite 0, sound 0x76) where a Navi
  logs in or out, one NPC for all, and the **greeter**'s word over his
  head ("ALPHA LOGGED IN!"). His own words, and the **referee**'s (how
  to ask for a NetBattle until this game has fought one; then who beat
  whom in the last bout and, from a rematch on, the count with that
  player; how many are watching), are rewritten as they change, as the
  keeper's are. The referee calls each bout over his head as MegaMan
  comes back ("YOU WIN!", "ALPHA WINS!").
- **Check text**: A facing a prop says what it is (the Board also where
  its art shows, over the floor before the rim it stands past: its piece
  is drawn 24 units lower, and a player facing it there read nothing,
  session 74); the Board's posts are
  the channel, who is here, the last log-in, the last trade and battle,
  the trade board's offers and a tip (Trade Central); the sign names the
  channel.
- **Together**: when two players say something or emote within four
  seconds of each other round the landmark (three panels) or on the
  stage, the walker nearer the landmark shows BN6's "!!" (list 5's sprite
  7), the other says "!!" and BN6's sparkle sounds; once in ten seconds,
  each client from the events it sees. The more join, the bigger it
  answers (issue #176): a third MegaMan inside the four seconds answers
  again within the rest, the walker's word "Yeah!! Go,go!" and BN6's pops
  (0x19A) in the sparkle's place; four or more and the greeter calls over
  his head ("ALL TOGETHER NOW!", from five "WHOA! EVERYBODY!!"). Each
  size answers once in a rest, and nothing of it moves a MegaMan.

None of it takes an event flag: the walkers, beam and mark are puppets,
their talk BN6's talk box from the host (`talk_start`), the Mr.Progs'
words their own archives in the layer's scratch.

**Talk.** Every exchange is BN6's chat box with its options, three to a
box (`online_chat.c`, words in `square_words.c`): L for Look around and
the quiet ([Emotes](emotes.md)), SELECT for the emote strip and its phrases, L
held with a direction for a favourite, A facing another MegaMan for his card (HP, act and layer,
Crosses, Souls) and Battle (where both passes hold the Arena stamp),
Wave, Mute, Block; R or the pad for home, stepped on as drawn or A on
it (its zone its middle stretched towards the screen's front, as Lan's
HP's portals are: a player stood on its front 21 units from its middle
and nothing asked, session 74). A talk's column of options takes up
and down (`cinema_keys` lets the pad through while MegaMan is held).
The Square's flags are `0x1480`-`0x14D0`, its own while it lasts.

**Finding each other.** A player ran round an empty Square for ten calls
and found the greeter, the keeper, a walker and the props, but neither
Trade Central's clerks nor the referee (session 74); friends in the old
Square lost each other (the owner, above). Each part is now a warp off
the plaza, the same three ways in every Square, and the world says where
they go and who is there (the game-design skill: signposts over
waypoints, words that agree with the map): the gold label over each gem
(`online_labels.c`), the props by each warp checked (Trade Central's BBS
posts who is there and the offers; MegaMan, at the Arena's braziers,
"Lan,look! The Arena's over there!" and "Mira's over there!"), and L's
third option, **Look around** (`square_life_look`, words in
`square_life_words.c`): in the plaza, "Lan,Trade Central's warp is to the
upper left!", "The Arena's is to the upper right." and "And the stage's
is to the lower left!"; in a part, "The warp we came in on goes back to
the plaza,Lan!"; then who is in each other part ("Mira's in the Arena!",
"Dex and 2 more are at Trade Central!"). The greeter's welcome ends "THE
WARPS GO TO TRADE CENTRAL,THE ARENA,THE STAGE!". No map: SELECT in the
Square is the emote strip's ([Emotes](emotes.md)), not a map of the plaza.
Phrases and emotes are fixed lists, never free text; mute and block are
the player's, kept by the server's public identity of the other.
