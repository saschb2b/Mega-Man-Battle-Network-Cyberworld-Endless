# Voice sweep: home and online

Group: the town, home, the requests, AsterLand and the Academy, the Net
Square's gate, the Square, Trade Central and the Arena (`town_lines.c`,
`town_words.c`, `gate_words.c`, `home_words.c`, `job_words.c`,
`place_lines.c`, `square_words.c`, `square_life_words.c`,
`online_text.c`, `trade_words.c`, `arena_words.c`). 19 October 2026,
after the owner's direction of that day: sweep every line that does not
need to be said, rewrite nothing. The rules are docs/VOICE.md's "What
nobody says", numbered as there (1 nobody knows the game is a game,
2 a button named once, 3 nobody explains BN6, 4 what the run adds taught
where BN6 teaches, 5 MegaMan never coaches, 6 the goal line explains
nothing, 7 bystanders never tutor, 8 words of the world).

416 lines of speech read (the catalog's `speech` entries, fragments
included): 12 cut, 20 noted, 384 kept. Ids are the catalog's before the
sweep. No table an RNG picks from lost an entry (every townsperson
stands where he stood, only boxes went), so neither `LAYER_MAKE` nor the
town's tests change.

## Cut

| Id | The line as it was (what went in bold) | Why |
| --- | --- | --- |
| `town_lines:central_folk:12.5` | Ack! I'm late for the Academy's NetBattle club!\|Everyone's hunting for Program Advances!\|**Three chips in the right order make a new one!** | 3: a bystander explains Program Advances; the gossip stays |
| `town_lines:acdc_folk:3.5` | WELCOME! I'M THE PARK'S PORT GUIDE!\|**PRESS R BY THE SQUIRREL TO JACK IN!** | 2, 3: a bystander names R and teaches jacking in; the guide still greets |
| `town_lines:seaside_folk:0.5` | The mermaid fountain has a port,you know.\|**Press R beside it to jack in!** | 2, 3: as above; where the port is stays |
| `town_lines:green_folk:0.5` | The knight statue has a port,you know.\|**Press R beside it to jack in!** | 2, 3: as above |
| `town_lines:beast_words:0` | **And the CybeastButton's unlocked!\|Our trump card. Alright!** (every run with BeastOut after the first) | 4: a power is told once, by whoever gave it (Dad's call, kept); the next ids shift down by one |
| `gate_words:gate_keeper_privacy:0` | NAVIS FROM OTHER PETS MEET THERE!\|ITS SERVER SEES YOUR ADDRESS AND NAME,\|AND MEGAMAN,TOO! **YOUR RUN STAYS SAFE!** | 1: names the run. What the server learns, and the Yes/No after it, stay whole |
| `gate_words:gate_save_failed:0` | **Huh? The save didn't go through...**\|Let's stay home for now,Lan. | 1: no character names a save. The failure is now the game's own note, "Run not saved", where "Run saved" shows (`director_gate.c`), and MegaMan keeps his second box |
| `home_words:home_hp_status:0` | The pink pad's {way},Lan!\|**Or press R,and I'll jack out.** | 2, 3: R named at every visit home, and jacking out is BN6's; `home_hp_status` loses its `first` |
| `job_words:job_due_words:2` | We didn't finish {who}'s request...\|Let's tell them. **Then we can take another!** | 5, 6: the goal line explains the one-at-a-time rule, which the askers say themselves when it matters |
| `place_lines:place_folk:4.2.5` | Lan! Welcome to the club!\|**Tip of the day! Match your codes!\|Same code,and you can send more chips!** | 3, 7: a classmate explains chip codes on the Custom Screen |
| `square_life_words:girl_lines:4` | **The clerks swap chips at Trade Central!** | 7: the GirlNavi's third tip; Trade Central's clerk and the Board's tip tell it. She keeps one (where Navis cheer); `girl_lines:5` becomes `:4` |
| `square_life_words:board:12` (and `:13`) | **>Tip: face a Navi and press A to wave or battle.**\|>Tip: post a chip at Trade Central. | 2, 3: A to talk is BN6's, and the referee tells the Arena; the Trade Central tip stays |

## Noted (kept now, for a later rewrite or move)

| Id | Line | What the player needs from it | Where BN6 would put it |
| --- | --- | --- | --- |
| `gate_words:gate_end_words:caseVISIT_END_VERSION.0` | OH NO! THAT SQUARE IS ANOTHER VERSION!\|BOTH SIDES NEED THE SAME ONE! | The server runs another build; update one side | The keeper still, in the Net's words, no "version" (1) |
| `gate_words:gate_end_words:caseVISIT_END_ROM.0` | THAT SQUARE IS SET FOR OTHER GAME DATA! | The server is for another ROM | The keeper, without "game data" (1) |
| `home_words:home_back_portal_words:0` | This link leads back to {area}!\|We could stock up there,Lan.\|But the Net's clock keeps running...\|Each trip back,the guardians ahead get tougher! | A trip back costs a notch on the Net's clock | A system the run adds: Dad's mail, once (4); MegaMan keeps a worried word before his question (5) |
| `home_words:home_port_words:4` | A dark way leads down here...\|It's sealed. Clearing the Secret Area would open it. | How the sealed way opens | Its first box is MegaMan's; the condition is a rule (5): Dad's mail or the BBS |
| `home_words:home_errands_words:1` | AsterLand can order us a chip,too! | The Order Service is open | The clerk says it at his counter; the courier Mr.Prog in Lan's HP, who already relays AsterLand's stock (4), rather than MegaMan's goal line each visit |
| `square_words:square_peer_words:1` | HP {hp} of {max_hp}. Act {act},layer {depth}! | How far another player has come | Stats in MegaMan's mouth (1, 8): the player's card on a screen of its own, or the second screen |
| `square_words:square_peer_words:4`, `:5` | His PET holds BN5 chips too! / His PET holds BN6 chips only. | What a trade or NetBattle with him can carry | Game titles in a line (1): "the older Net's chips", the run's own name for BN5's |
| `square_life_words:square_referee_words:1` | THIS IS THE ARENA!\|FACE A NAVI AND PRESS A. PICK BATTLE! | How to ask for a NetBattle | The right speaker (4); without the button (2): talk to a Navi, pick Battle |
| `trade_words:trade_pick:0`, `trade_bn6_only:0`, `trade_bn5_spares:0` | ...THAT NAVI TRADES BN6 CHIPS! / ...TRADES BN6 CHIPS ONLY!\|AND YOUR SPARES ARE ALL BN5 CHIPS! / ...YOUR SPARES ARE ALL BN5 CHIPS!\|NO NAVI HERE TRADES THOSE! | Which chips a partner can take | The clerk still, in the Net's words for the two nets, no game titles (1) |
| `arena_words:rules_words:1`, `arena_sent_words:1`, `arena_challenged_words:1`, `:2`, `arena_no_folder_words:0` | "BN6 rules", "Our Souls stay home: BN6 rules!", "Souls and BN5 chips too!", "no Souls,and BN6 chips only.", "...every folder of ours has BN5 chips...\|BN6 rules won't take them..." | Which rules a challenge goes by, before the Yes/No | Game titles as the rules' names (1): the rules named in the Net's words, or on the challenge's own screen |
| `arena_words:arena_standin_words:0`, `:1` | {folder}'s {gone} sits out,Lan.\|Our {in} stands in for it! / {n} BN5 chips sit out...\|Chips from our Pack stand in for them... | The folder he carries differs from the one he built | MegaMan explains a system (5); the folder's own screen, or the referee |
| `arena_words:arena_installs_apart_words:0` | Huh? Our PETs don't match on BN5...\|Lan,let's reboot the PET and try again! | Restart the game to battle | "BN5" (1); the reboot is a fair fiction for the restart |

## Kept, in short

The townsfolk's gossip and checks, Dad's first call (the premise: copied
data, the backup, the Nest), the Net News and the town that remembers;
the coming home, MegaMan's sensing of each portal and the Net's clock as
a feeling; the askers' offers (the terms of a deal are the asker's to
say), their waits and pay; the gate keeper's privacy words and question;
the Square's chatter, the channel keeper, the greeter and the Board's
posts; Trade Central's clerk, his "How it works" (asked for), and
BN6's order screen's legends ("B BUTTON:BACK", rule 2's menu legend).
