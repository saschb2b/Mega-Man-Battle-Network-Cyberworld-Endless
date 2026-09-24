# Guardians

How a guardian Navi is met, fought and left behind, from the approach to the
next area. The structure follows how Hades stages its bosses; everything on
screen is the game's own (NPC and text commands, mugshots, songs, sounds)
or drawn over its frame by `src/director/cinema.c`.

## What Hades does, and what was taken

| Hades | Here |
| --- | --- |
| The room before a boss is safe: Charon's shop, often a fountain | The antechamber, the room the arena's bridge leaves from, always holds a heal and the Net Dealer |
| The boss chamber is an arena of its own, locked until the fight is over | A square arena at the end of one bridge, drawn in the area's second floor; the exit pad inside it stays hidden and shut |
| The music drops to ambience on approach, the boss theme starts with the fight | Entering the arena silences the area's theme; the boss prelude (song 0x1C) plays through the intro, the game's boss theme in the battle |
| Short intro dialogue with portraits, chosen by history (first meeting, who won last) | Lines with the guardian's mugshot for a first meeting, a rematch, revenge after beating MegaMan, a stronger version, and grudging respect after many losses; MegaMan answers now and then |
| The boss's name and epithet on its health bar | A title card: the area it guards, its name large (with EX or SP for stronger versions), its epithet; the game's own battle shows its name too |
| No question before the fight | The battle starts after the last line |
| A defeat line, a flash, the boss leaves | A last word, a white flash, and the guardian logs out, fading away |
| A reward to walk up to; the exit opens | Its Guardian Data materializes where it stood (its Cross or Beast Out, five HPMemory, its own Navi chip at the version fought, a full heal); taking it makes the exit pad appear |
| Stairs into the next region, its name on screen | An area-clear card (guardian, viruses, time) over the jack-out, then the next area's title card |
| Bosses remember runs | `rivals.sav` counts meetings and who won each battle, per Navi |

Not taken: arenas that change during the fight and extra phases, which
would mean changing the game's battles themselves.

## The sequence (`src/director/boss.c`)

| State | What happens |
| --- | --- |
| Wait | The guardian's NPC waits hidden in the arena's middle |
| Enter | MegaMan steps into the arena: bars slide in, the music stops, MegaMan walks up; the boss prelude starts, the guardian logs in (its animation 25 and sound 0x77) and the screen shakes |
| Title | The title card, while Gregar's own Navis strike their signature pose (animation 24) |
| Talk | The intro lines; input is A and B only |
| Fight | The game's navi battle, forced at once (the random battle record is not re-rolled over it) |
| After | Back on the map: bars, silence |
| Last word | Its defeat line |
| Log out | A flash; the guardian plays the log-out sound (0x76) and fades by its alpha before it is freed |
| Reward | The Guardian Data shows (sound 0x94); the area's theme returns and MegaMan walks to it |
| Open | Taken: the exit pad appears and its warp works |

Event flags `0x1448`-`0x144C` drive the NPCs: the guardian leaves, logs in,
the Guardian Data shows, it was taken, the exit shows. While the exit is
shut, flag `0x16F1` keeps its warp from starting.

## Guardians

Mugshots share their index with the Navi's overworld sprite in list 6:
HeatMan 0x47, ElecMan 0x49, SlashMan 0x4B, ChargeMan 0x4F, EraseMan 0x50,
BlastMan 0x51, DiveMan 0x52, Colonel 0x53, CircusMan 0x54, JudgeMan 0x55,
ElementMan 0x56, ProtoMan 0x3B, MegaMan 0x37. Gregar has neither for
Falzar's Navis (SpoutMan, TomahawkMan, TenguMan, GroundMan, DustMan), who
speak without one and stand as a HeelNavi on the net, as do BlastMan and
ElementMan (no overworld sprite).

A guardian's navi index is its ai in the enemy table: HeatMan 1 .. ElementMan
16, and Colonel 18. Index 17 (and 0, 22) is an unnamed navi with 4000 HP at
V1, which the Graveyard's SP battle uses; runs saved before Colonel moved
to 18 have their 17 changed on loading, and his rival record follows.

Which guardian an area gets depends on its act (docs/PROGRESSION.md): from
the area's pool the Navis whose HP at the act's version lies in the act's
band (400-600 in act 1 up to 1200-2000 in act 6), else another Navi in the
band, else the pool's nearest. Acts 1-3 fight V1, acts 4-6 EX where it
fits, the Nest, the Secret Area and later cycles SP.
