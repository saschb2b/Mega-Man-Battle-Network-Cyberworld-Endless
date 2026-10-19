# Voice sweep: the net and the run

Group: L's briefing, MegaMan's lessons and his NaviCust words, the
services, rewards, locks, the dealer and vendor, bystanders and rumors,
DarkChips, Crosses and where the nets meet (`briefing_words.c`,
`lesson_words.c`, `board_words.c`, `navicust_words.c`, `service_words.c`,
`reward_words.c`, `layer_words.c`, `lock_words.c`, `npc_lines.c`,
`rumor_lines.c`, `dark_words.c`, `powers_words.c`, `nets_words.c`).
19 October 2026, on the owner's direction of that day. The rules are
docs/VOICE.md's What nobody says, by number: 1 nobody knows the game is a
game, 2 a button is named once, 3 nobody explains BN6, 4 what the run adds
is taught where BN6 teaches, 5 MegaMan never coaches, 6 the goal line
explains nothing, 7 bystanders never tutor. Ids are the catalog's as of
93568943 (positional ids after a cut have moved since).

A **CUT** is gone, with the code that said it; where only some boxes of a
line went, the rule column says which. A **NOTE** stays for now: the
player needs what it says, but it says it the wrong way or in the wrong
mouth; it is listed for a later rewrite or move.

## Counts

| File | Read | Kept | Cut | Noted |
| --- | --- | --- | --- | --- |
| `briefing_words` | 126 | 80 | 41 | 5 |
| `lesson_words` | 12 | 6 | 6 | 0 |
| `board_words` | 11 | 2 | 7 | 2 |
| `navicust_words` | 63 | 43 | 20 | 0 |
| `service_words` | 43 | 36 | 6 | 1 |
| `reward_words` | 46 | 35 | 6 | 5 |
| `layer_words` | 52 | 42 | 8 | 2 |
| `lock_words` | 18 | 18 | 0 | 0 |
| `npc_lines` | 31 | 20 | 11 | 0 |
| `rumor_lines` | 8 | 6 | 2 | 0 |
| `dark_words` | 6 | 5 | 1 | 0 |
| `powers_words` | 11 | 10 | 1 | 0 |
| `nets_words` | 7 | 6 | 0 | 1 |
| All | 434 | 309 | 109 | 16 |

A line both cut in part and noted counts once, as cut.

## Systems removed

- **Rush's bone panels' lesson** (`rush_hint`): MegaMan naming A, RushFood
  and the count on every layer with a gap; its flag `LAYER_RUSH_TOLD_FLAG`
  (0x1464) is free. The dealer still names the gap and its width, and L
  still senses "bone panels".
- **The map's lessons and the set pieces' lessons** (`mark_lessons`,
  `piece_lessons`, the violet-mark boxes): every mark and piece is now
  named in L's "I sense ..." from the start. `profile.marks_taught` and
  `profile.pieces_taught` stay in the profile's layout, unread.
- **The virus families' warnings** (`family_words`) and what fed them:
  `loot_families_here`, `profile_family_fought`/`_note` (the
  `families_fought` field stays, unused).
- **The NaviCust's how-to talk**: the Spin and L/R words
  (`navicust_turn_words`, `navicust_color_turns`), the no-room and
  cramped words (and `program_watch`, `in_draft`), the compression code
  recited, the bug's effects, rules and "fix it in the PET". A bug is
  still warned of and named; why it came stays on the second screen.
- **The bystanders' beginners' tips** (`tips[]`, and the newcomer's basics
  filter `early_basic`/`VETERAN_RUNS`).
- **L's ScrtData count** (`scrt_note`, `D.fragments_told`): the pickup
  says it.

## LAYER_MAKE

Not bumped. The bystanders' and rumors' tables lost entries, which changes
the line a bystander or whisper picks from its roll, but not the roll, the
layer's objects or its loot: `npc_line` takes `obj.npc_line` (rolled
0-255 whatever the table) modulo the table, and the rumor picks by the
layer seed. `build.py test` passes `LAYER_MAKE_HASH` unchanged.

## CUT

| Id | The line as it was | Rule |
| --- | --- | --- |
| `board_words:no_room_words:0` | Hmm... {name} won't fit beside the others yet. / It'll keep in the PET till the board grows,Lan.{code_words} | 5 (the board's fit) and 3 (ExpMemry) |
| `board_words:no_room_words:1` | Hmm... {name} won't fit beside the others,Lan. / To use it,we'd have to take one off the board.{code_words} | 5, 3 |
| `board_words:off_board_words:1` | There's no room for it as the board stands. / We'll have to move a program or two. | 5: how to make room on the board |
| `board_words:code_words:0` | Or we compress it! / In the NaviCust,hold RIGHT on it, / then press {code} | 2 (RIGHT and the code), 3; the code is in Dad's Compression mail |
| `board_words:cramped_words:0` | Hmm... {off} won't fit in our free space,Lan. / We'll have to move a program or two to make room.{code_words} | 5; the draft says "We'll move one to make room" at the pick |
| `board_words:reg_words:0` | A RegUp,Lan! Our Reg memory's {reg} MB now! / A Regular Chip starts every battle in our hand! / SELECT picks one in the Folder's EDIT. Up to {reg} MB! / A + chip like Atk+10 powers up the chip picked before it. / Each dive starts at 4 MB,so watch for RegUps! | 3 (Regular Chip, + chips), 2 (SELECT), 1 ("Each dive starts at 4 MB"): boxes 2-5 cut |
| `board_words:reg_words:2` | Lan! Chaud's clearance comes with the TagChip system! / Press SELECT in the Folder's EDIT, / then pick Choose TagChip. / Two tagged chips come to our hand together. / As long as they total under 60 MB! | 2, 3: boxes 2-5 (TagChip's how-to) cut; box 1 kept (NOTE) |
| `briefing_words:guardian_words:2` | Watch the yellow panels! Attacks land there! | 3: the yellow panels |
| `briefing_words:guardian_words:5` | Word is,{name} waits at the end! / We've got no battle data on him,Lan. | 5: box 2 cut ("no battle data" only explains the tip system); box 1 kept |
| `briefing_words:guardian_words:6` | A strong Navi's signal waits at the end... / We've got no battle data on it,Lan. | 5: box 2 cut, as above; box 1 kept |
| `briefing_words:guardian_words:7` | Watch the yellow panels! Attacks land there! | 3: the yellow panels |
| `briefing_words:guardian_words:9` | In EraseCross,watch his HP for a 4! / Then a plain chip bugs him,Lan. / His HP drains for the rest of the fight! | 3: EraseCross's bug rule; 5 |
| `briefing_words:scrt_note:0` | We've got {counts} ScrtData,Lan! / {fragments} | 4: said at each pickup already (fragment_words), repeated on L |
| `briefing_words:scrt_note:1` | Three open the Secret Area's golden gate. / It's in the Undernet! | 4: said at each pickup already (fragment_words), repeated on L |
| `briefing_words:scrt_note:2` | The Secret Area's golden gate will open for us! / It's in the Undernet,through a dark warp. | 4: said at each pickup already (fragment_words), repeated on L |
| `briefing_words:heal_note:1` | Let's visit the Recovery Mr.Prog! / He's {hw},{near_far}{wind}. / The arrow turns green and leads there! | 5: box 3 cut ("The arrow turns green", the screen's); boxes 1-2 kept |
| `briefing_words:family_words.warn:0.words` | We might meet ScarCrows again! / Their lightning heals them. So do Elec chips! / Hit them hard with anything but Elec! | 4 (strategy, the BBS's) and 5; system removed |
| `briefing_words:family_words.warn:1.words` | We might meet DarkMechs again! / They warp right beside us to slash. / Keep moving,and strike as they appear! | 4 (strategy, the BBS's) and 5; system removed |
| `briefing_words:family_words.warn:2.words` | We might meet StarFish again! / Their bubbles soak up our shots. / Touch one,and it traps us! / A chip that drops from above gets past them! | 4 (strategy, the BBS's) and 5; system removed |
| `briefing_words:off_board_note:0` | That program | 5: its no-room word (board_words:no_room_words) went |
| `briefing_words:off_board_note:2` | Lan,{off} isn't on our NaviCust board yet! / {turns}{turns}{turns} | 2, 3: the Spin turn words cut; the reminder kept (NOTE) |
| `briefing_words:mark_lessons:0` | The crystals up top count our Mystery Data! / What we've taken,out of what we've seen or sensed. / There could be more out there! | 5: the map's marks and counters explained; each piece names itself at A. Its names now go straight into "I sense..." |
| `briefing_words:mark_lessons:1` | A strong virus signal,marked violet on the map! / Its Server offers a hard battle for a good chip! | 5: the map's marks and counters explained; each piece names itself at A. Its names now go straight into "I sense..." |
| `briefing_words:mark_lessons:2` | A dark warp,marked violet on the map! / It leads to the Undernet. / Tougher viruses in there,but richer data! | 5: the map's marks and counters explained; each piece names itself at A. Its names now go straight into "I sense..." |
| `briefing_words:mark_lessons:3` | The Secret Area's golden gate,marked violet on the map! | 5: the map's marks and counters explained; each piece names itself at A. Its names now go straight into "I sense..." |
| `briefing_words:mark_lessons:4` | A gate sealed with {name}'s code! / It's marked violet on the map. / His code opens it for good. His SP waits inside! | 5: the map's marks and counters explained; each piece names itself at A. Its names now go straight into "I sense..." |
| `briefing_words:mark_lessons:5` | A collector's vault,marked violet on the map! / A big enough Library opens it. Rare chips inside! | 5: the map's marks and counters explained; each piece names itself at A. Its names now go straight into "I sense..." |
| `briefing_words:piece_talk:0.lesson` | Purple Mystery Data,locked tight! / An Unlocker opens it. A Net Dealer might sell one! | 3 (purple data, Rush, pads, arrows, skull doors) or 4 (the lock says its own key at A) |
| `briefing_words:piece_talk:1.lesson` | Bone panels by a gap! / Rush can bridge it,if we've got RushFood! | 3 (purple data, Rush, pads, arrows, skull doors) or 4 (the lock says its own key at A) |
| `briefing_words:piece_talk:2.lesson` | Teleport pads! Step on one,and we beam to the other! | 3 (purple data, Rush, pads, arrows, skull doors) or 4 (the lock says its own key at A) |
| `briefing_words:piece_talk:3.lesson` | Arrow panels! They only carry us the way they point. | 3 (purple data, Rush, pads, arrows, skull doors) or 4 (the lock says its own key at A) |
| `briefing_words:piece_talk:4.lesson` | Something's blocking a walkway! / A Link Navi could clear it. Or the right Cross! | 3 (purple data, Rush, pads, arrows, skull doors) or 4 (the lock says its own key at A) |
| `briefing_words:piece_talk:5.lesson` | A security cube! / It wants a P-Code someone here knows. Or a toll! | 3 (purple data, Rush, pads, arrows, skull doors) or 4 (the lock says its own key at A) |
| `briefing_words:piece_talk:6.lesson` | A skull door! Only a WWW-ID gets us past. | 3 (purple data, Rush, pads, arrows, skull doors) or 4 (the lock says its own key at A) |
| `briefing_words:piece_talk:7.lesson` | A number door! / We'll find its answer by counting something here. | 3 (purple data, Rush, pads, arrows, skull doors) or 4 (the lock says its own key at A) |
| `briefing_words:violet_mark:0` | Another violet mark on the map. | 5: the map's colour, the screen's |
| `briefing_words:violet_mark:1` | It's the violet mark on the map. | 5: the map's colour, the screen's |
| `briefing_words:more_words:0` | Brr... The battlefields here are icy,Lan. / An Aqua hit on ice freezes us! / Keep off the ice when viruses shoot water. | 3 + 5: boxes 2-3 (Aqua on ice) cut; "Brr... icy" kept |
| `briefing_words:more_words:1` | The battlefields here have conveyors and ice! / The arrows carry us along. / And an Aqua hit on ice freezes us. Careful! | 3 + 5: boxes 2-3 cut; box 1 kept |
| `briefing_words:more_words:2` | ProtoMan's {rw}! / He's the pink mark on the map. | 5: box 2 ("the pink mark on the map") cut |
| `briefing_words:status_words:0` | The port's {way},by the {landmark}! / Stand next to it,press R,and jack me in! | 2, 3: box 2 ("press R,and jack me in") cut |
| `briefing_words:status_words:4` | There's more here,Lan. Press L again! | 2: "Press L again!" cut; "There's more here,Lan." kept |
| `briefing_words:last_stop:0` | Want to get ready first? / The Net Dealer's {dealer}. / A Recovery Mr.Prog's {heal}. / {NO_RUNNING} | 3: no running from a story boss is BN6's; the {NO_RUNNING} box went from all three |
| `briefing_words:last_stop:1` | Once we're in,there's no running! | 3: no running from a story boss is BN6's; the {NO_RUNNING} box went from all three |
| `briefing_words:last_stop:2` | Want to get ready first? / The Net Dealer's {dealer}. / {NO_RUNNING} | 3: no running from a story boss is BN6's; the {NO_RUNNING} box went from all three |
| `briefing_words:last_stop:3` | Once we're in,there's no running! | 3: no running from a story boss is BN6's; the {NO_RUNNING} box went from all three |
| `briefing_words:last_stop:4` | Want to heal up first? / A Recovery Mr.Prog's {heal}. / {NO_RUNNING} | 3: no running from a story boss is BN6's; the {NO_RUNNING} box went from all three |
| `briefing_words:last_stop:5` | Once we're in,there's no running! | 3: no running from a story boss is BN6's; the {NO_RUNNING} box went from all three |
| `dark_words:dark_price_words:0` | That DarkChip burned a BugFrag each time,Lan. / Its darkness bugged me till the battle ended. / That bug's gone now. But something stays... / My max HP fell by {lost},for this whole dive. | 4: boxes 1-2 repeat what the flame said; kept from "That bug's gone now" |
| `lesson_words:rush_hint:0` | Bone panels,Lan! Rush can bridge this gap! / Press A at the edge to toss him RushFood! | 3 (Rush, RushFood), 2 (A, on every layer), 5; the dealer names the gap. System removed |
| `lesson_words:rush_hint:1` | Bone panels,Lan! Rush could bridge this gap! / He needs one RushFood. / A Net Dealer might have some. | 3 (Rush, RushFood), 2 (A, on every layer), 5; the dealer names the gap. System removed |
| `lesson_words:rush_hint:2` | Bone panels,Lan! Rush could bridge this gap! / He comes when we hold a RushFood a panel,{len} here. / He eats just one. A Net Dealer might have some. | 3 (Rush, RushFood), 2 (A, on every layer), 5; the dealer names the gap. System removed |
| `lesson_words:port_words:0` | The PC's {way},Lan! / Step up to it and press R! | 2, 3: box 2 ("Step up to it and press R!") cut |
| `lesson_words:port_words:6` | Almost,Lan! The {landmark}'s {way}. / Step up to it and press R! | 2, 3: box 2 ("Step up to it and press R!") cut |
| `lesson_words:port_words:7` | Almost,Lan! The {landmark}'s {way}. / Step right up to it and press R! | 2, 3: box 2 ("Step up to it and press R!") cut |
| `powers_words:powers_reward_words:3` | Next dive,we can start with his Cross! | 1: "Next dive" unlocks are the setup's to show |
| `layer_words:dealer_word:4` | has no weak element. / His armor blocks all,but not when he swings or leaps | 4: a second tip (KnightMan's armor), the BBS's |
| `layer_words:dealer_rush:2` | And there's a gap {where},{hold} panels wide! / Rush comes when you hold a RushFood / for each panel. He eats just one! / It's on my list,too! | 3: boxes 2-3 (as many RushFood as panels, he eats one) cut |
| `layer_words:spin_words:0` | Lan,look! A Spin for {c} programs! / Now we can turn {c} programs on the NaviCust! / Hold one,then press L or R. / And it stays with us,in every dive! / Each dive hides one more,deeper in. / A color we don't have yet! | 3, 2: boxes 2-3 cut; the rest kept (NOTE) |
| `layer_words:spin_words:1` | A Spin for {c} programs,Lan! / That's all six! Every program turns now! / In every dive! | 3: "Every program turns now!" cut |
| `layer_words:spin_words:2` | A Spin for {c} programs,Lan! / {c}{c} programs turn with L and R now! / In every dive from here on! | 2, 3: box 2 cut |
| `layer_words:dealer_words:2` | Its needles drop a moment late. / So fire when he stops! | 4: a second tip, strategy (how AquaNdl2 lands), the BBS's |
| `layer_words:vendor_words:2` | NaviCust programs! / Fresh from my workbench!{again} / Install them in the PET. / Pick MegaMan,then NaviCust! | 3: boxes 3-4 (PET, MegaMan, NaviCust) cut |
| `layer_words:duel_terms:0` | {met}I busted {count} here in {sec}:{sec}.{cs}. / Beat that{rung}. / Real viruses,Lan! / If they delete us,the dive's over. / Let's heal up first if we're hurt. | 5: "Let's heal up first if we're hurt." cut; the stake kept (NOTE) |
| `navicust_words:navicust_color_turns:0` | Our {navicust_color_name} Spin lets L and R turn it! | 2, 3 (Spins and L/R) |
| `navicust_words:navicust_turn_words:0` | L and R turn any program as we place it! | 2, 3 |
| `navicust_words:navicust_turn_words:1` | L and R turn | 2, 3 |
| `navicust_words:navicust_turn_words:2` | programs. {held} we hold! | 2, 3 |
| `navicust_words:navicust_turn_words:3` | That's the Spin | 2, 3 |
| `navicust_words:navicust_turn_words:4` | Those are the Spins | 2, 3 |
| `navicust_words:navicust_bug_words.effect:1` | Every step slides me as far as I can go | 3: what BN6's bug does |
| `navicust_words:navicust_bug_words.effect:2` | My mood will swing in battle | 3: what BN6's bug does |
| `navicust_words:navicust_bug_words.effect:3` | Panels may crack under me as I move | 3: what BN6's bug does |
| `navicust_words:navicust_bug_words.effect:4` | I'll get fewer chips as a battle goes on | 3: what BN6's bug does |
| `navicust_words:navicust_bug_words.effect:5` | More viruses will find us | 3: what BN6's bug does |
| `navicust_words:navicust_bug_words.effect:6` | Battles will pay Zenny,not chips | 3: what BN6's bug does |
| `navicust_words:navicust_bug_words.effect:7` | My Buster may misfire | 3: what BN6's bug does |
| `navicust_words:navicust_bug_words.effect:9` | I'll lose HP in battle,faster with every hit | 3: what BN6's bug does |
| `navicust_words:navicust_bug_words.effect:11` | five colors! Each battle will start with something odd | 3: what BN6's bug does |
| `navicust_words:navicust_bug_words.effect:12` | six colors! Each battle will start with something odd,for longer | 3: what BN6's bug does |
| `navicust_words:navicust_bug_words:1` | the RUN says OK... / But our NaviCust | 5: "the RUN says OK" reads the screen |
| `navicust_words:navicust_bug_words:6` | {article} {name} bug! / {effect}. | 3: the effect box after the bug's name cut |
| `navicust_words:navicust_bug_words:7` | Bugs come from going off the board,or off the command line. / Or a plus part on it,or same colors side by side. | 3, 5: BN6's board rules |
| `navicust_words:navicust_bug_words:8` | We can fix it in the PET,or live with it. / {navicust_turn_words} | 5, 2 |
| `npc_lines:early:4` | Green Mystery Data has chips and Zenny inside! / The blue ones? Even rarer stuff! | 3, 7 |
| `npc_lines:early:6` | My operator holds B to charge my Buster! / A charged shot makes most viruses flinch! | 2, 3, 7 |
| `npc_lines:early:7` | Careful! Mystery Data in battle breaks at any hit! / Even your own! Win with it whole,and it's yours! | 3 (the field's Mystery Data), 7 |
| `npc_lines:mid:5` | My new program did nothing until I installed it! / In the PET,pick MegaMan,then NaviCust! | 3 (the PET's menus), 7 |
| `npc_lines:deep:3` | The BugFrag Trader wants BugFrags,not chips! / And the Undernet's full of 'em! | 3, 7 |
| `npc_lines:tips:0` | Did you know? / Chips of one code can be sent together! / Stack 'em in your Folder! | 3, 7; the beginners' tips removed |
| `npc_lines:tips:1` | Got junk chips? Try a Chip Trader! / Three chips in,one chip out! | 3, 7; the beginners' tips removed |
| `npc_lines:tips:2` | Custom Gauge full? My operator hits L or R! / New chips right away! Neat,huh? | 3, 7, 2; the beginners' tips removed |
| `npc_lines:tips:3` | AreaGrab steals the enemy's front column! / More room to move,less room to hide! | 3, 7; the beginners' tips removed |
| `npc_lines:tips:4` | My operator pulls me out of bad fights! / L on the Custom Screen! It doesn't always work! | 3, 7, 2; the beginners' tips removed |
| `npc_lines:tips:5` | Something coming at you while picking chips? / Press SELECT! The Custom Screen hides for a peek! | 3, 7, 2; the beginners' tips removed |
| `reward_words:draft_take:0` | Let's install it!{turns}{turns} | 2, 3: its Spin's "L and R turn it" cut; "Let's install it!" kept |
| `reward_words:ta_gate_reward:1` | It's in our Pack! Let's add it to our Folder,Lan! | 3 |
| `reward_words:pick_three:1` | It's in our Pack! Let's add it to our Folder,Lan! | 3 |
| `reward_words:ta_gift:1` | IT'S IN YOUR PACK! ADD IT TO YOUR FOLDER IN THE PET! | 3 (the Pack, the Folder) by a Mr.Prog |
| `reward_words:ta_gift:3` | TO INSTALL IT,OPEN MEGAMAN IN THE PET, THEN NAVICUST! | 3 (the PET's menus) |
| `reward_words:ta_gift:6` | HELLO,MEGAMAN! / I'M DR.HIKARI'S DIVE SUPPORT PROGRAM! / EVERY DIVE STARTS FROM SCRATCH... / SO HERE'S A GIFT! | 1: box 3 "EVERY DIVE STARTS FROM SCRATCH..." cut |
| `rumor_lines:secret_line:0` | Did you hear? NaviCust programs can shrink! / Hold RIGHT on one and press a secret pattern! / Nobody I know has it,though... | 2, 3 (compression), 7 |
| `rumor_lines:secret_line:4` | Every layer before a guardian hides a RegUp,off the path! / More Reg memory,a bigger Regular Chip! | 3: box 2 ("More Reg memory,a bigger Regular Chip!") cut |
| `service_words:ta_dark_flame:1` | L-Lan... That flame is dark data! / There's a DarkChip in it. {chip}!{bar}{does} / It only comes when I'm worried. / At a quarter HP,or hit again and again! / It's real power... But it eats at me. / Use one,and I go dark for that battle. / And each battle I use one in... / ...takes {DARK_PRICE} max HP,for this whole dive! | 3 (BN5's own worried rule and dark battle): "At a quarter HP,or hit again and again!" and "Use one,and I go dark for that battle." cut |
| `service_words:ta_dark_flame:2` | A flame of darkness,Lan! / A DarkChip's inside. {chip}!{bar}{does} / It comes when I'm worried,at a quarter HP. / Each battle I use one in takes {DARK_PRICE} max HP,for the dive. | 4 (the rule once): "It comes when I'm worried,at a quarter HP." cut; the price kept |
| `service_words:ta_dark_flame:5` | It's in our Pack! To use it,pick PET, Folder,then EDIT. It'll come when I'm worried. At a price. | 3: the Pack, PET, Folder, EDIT |
| `service_words:ta_dark_flame6:2` | A flame of darkness,Lan! / A DarkChip's inside. {chip}! / Each use burns a BugFrag... / ...for {does}. / Then it bugs me for the battle. / No BugFrags? Then it's {an} {base}. / Each battle it runs in takes {DARK_PRICE} max HP,for the dive. | 4: the rule's four boxes, said at the first flame, cut from every later one; the price kept |
| `service_words:ta_dark_flame6:5` | It's in our Pack! To use it,pick PET, Folder,then EDIT. Three DarkChips max in a Folder,though. | 3: the Pack, PET, Folder, EDIT |
| `service_words:ta_challenge_reward:1` | It's in our Pack! Let's add it to our Folder,Lan! | 3: the Pack and the Folder |

## NOTE

| Id | The line | What the player needs, and where BN6 would put it |
| --- | --- | --- |
| `board_words:off_board_words:0` | Lan,{off} isn't on our board!{stands}{turns}{turns}{code_words} | The bare reminder after the PET: see off_board_note. |
| `board_words:reg_words:2` | Lan! Chaud's clearance comes with the TagChip system! / Press SELECT in the Folder's EDIT, / then pick Choose TagChip. / Two tagged chips come to our hand together. / As long as they total under 60 MB! | Box 1 kept: the system the run gates behind Chaud's clearance belongs in his mail (as the Aquarium Director's TagChip mail in BN6), not MegaMan's mouth. |
| `board_words:compressed_words:0` | {about}'s compressed,Lan! / Dad's lab keeps its code for every dive now. / It's in his Compression mail! | "Dad's lab keeps its code for every dive now" is a rule across runs (1). The Compression mail is the place; a new-mail notice would do. |
| `briefing_words:guardian_words:1` | We've got battle data on him! / {tip} | Strategy (BN6 puts it on the BBS). Kept: Dad's battle-data mail covers BN6's guardians only, so this is the one place BN5's guardians' and the super bosses' tips are spoken since the story sweep cut MegaMan's tip before each guardian. Move them to BBS posts. |
| `briefing_words:off_board_note:1` | Lan,{off} is still off our board! | BN6 never reminds; a program off the board is the player's to see. If the reminder is wanted, the NaviCust's mail, once. |
| `briefing_words:off_board_note:2` | Lan,{off} isn't on our NaviCust board yet! / {turns}{turns}{turns} | As above. |
| `briefing_words:off_board_note:3` | Lan,a program isn't on our NaviCust board yet! | As above. |
| `briefing_words:first_words:1` | Layer {depth},Lan. We're in {area}! | "Layer N" repeats the arrival card's label, a level counter (rule 1, 6). "We're in {area}!" alone is the goal line. |
| `briefing_words:more_words:3` | Hold SELECT for a map of where we've been! | Names SELECT for the run's map on layers 1-2 until held (per session: every session hears it again). Rule 2 says once: a profile flag, or the second screen's legend. |
| `nets_words:old_net_words:0` | Huh? Some of our chips came from the older Net... / Lan: And its cartridge isn't here! / Right. They'll sit out until it's back! | "And its cartridge isn't here!" is Lan knowing the ROM (1). The player needs it after a CONTINUE without BN5: a screen's notice (the title or the ROMs screen), not Lan. |
| `layer_words:spin_words:0` | Lan,look! A Spin for {c} programs! / Now we can turn {c} programs on the NaviCust! / Hold one,then press L or R. / And it stays with us,in every dive! / Each dive hides one more,deeper in. / A color we don't have yet! | "And it stays with us,in every dive!|Each dive hides one more,deeper in." is the run's meta (1). The Spin's mail, or the BBS. |
| `layer_words:duel_terms:0` | {met}I busted {count} here in {sec}:{sec}.{cs}. / Beat that{rung}. / Real viruses,Lan! / If they delete us,the dive's over. / Let's heal up first if we're hurt. | "If they delete us,the dive's over." knows the run ends (1); kept and noted as the story sweep did in duel_words. |
| `layer_words:fragment_words.found:0` | Lan,look! A ScrtData! / Three open the golden gate to the Secret Area. / It stands in the Undernet's copy. / A dark warp leads there. / Let's find two more! | The run's ScrtData system (three open the gate, where it stands) told by MegaMan at the pickup: a mail, or a Mr.Prog at the gate (rule 4). The third's goal line can stay. |
| `layer_words:fragment_words.found:1` | Our second ScrtData! / One more,and the Secret Area's gate opens! / It's in the Undernet's copy! | As above. |
| `reward_words:draft_menu:0` | Program data,too! Pick one! Or B for {skip_frags} BugFrags,Lan! | B named before the menu ("Or B for 30 BugFrags"); its choice, so kept. A legend in the menu itself would do it (rule 2). |
| `reward_words:ta_navi_gate:0` | It's sealed with {navi}'s code,Lan. / If we delete {navi} {needed} as a guardian... / ...in any dive,it'd crack! {beaten} | "...in any dive,it'd crack!" is a rule across runs (1). The gate's rule, by a Mr.Prog posted at it (rule 4), or a mail. |
| `reward_words:ta_vault:0` | A vault,Lan! With a collector's lock. / It opens for a Library of {need} chips. / Ours has {have}. / Every chip we get counts,from any dive! | "Every chip we get counts,from any dive!" (1); the lock's rule, as above. |
| `reward_words:ta_official:7` | An official gate,Lan! / It needs Chaud's full clearance. / The official vault's behind it. Three Mega chips! / That takes three duel wins against ProtoMan. / The last is a NetBattle with him. We have {won}. | Chaud's clearance system recited at the gate (three duel wins, the last a NetBattle, "We have 2"): Chaud's mail's, or his call's (rule 4). |
| `reward_words:ta_official:8` | An official gate,Lan! / It needs Chaud's first clearance. / A Chip Order's behind it. Three chips we've held! / We get to order one. / One duel win against ProtoMan opens it. / Not yet,though! | As above. |
| `service_words:ta_dark_flame:1` | L-Lan... That flame is dark data! / There's a DarkChip in it. {chip}!{bar}{does} / It only comes when I'm worried. / At a quarter HP,or hit again and again! / It's real power... But it eats at me. / Use one,and I go dark for that battle. / And each battle I use one in... / ...takes {DARK_PRICE} max HP,for this whole dive! | The price must come before the Yes, but six boxes are a lecture on a system the run adds. A Mr.Prog posted by the first flame, or Dad's mail on DarkChips, then one box at the flame. |
| `service_words:ta_dark_flame6:1` | L-Lan... That flame is dark data! / There's a DarkChip in it. {chip}! / Its darkness feeds on bugs! / Each use burns one of our BugFrags... / ...for {does}! / Then it bugs me,like a NaviCust bug. / For the rest of that battle! / No BugFrags? Then it's only {an} {base}. / And each battle it runs in... / ...takes {DARK_PRICE} max HP,for this whole dive! | As above: ten boxes on the run's BN6 DarkChip rule (BugFrags, bug, base chip, price). |

## Kept, and why some stay

- The goal lines (where, how far, "Follow the arrow!", as VOICE's own
  table has it), the super bosses sensed, the Net's clock, the Recovery
  Mr.Prog's "ONE PATCH A LAYER!" (a place's own rule by its posted
  Mr.Prog, rule 4), every question with its choice and the boxes before it
  that make the choice (a Server's strength and prize, the Undernet's warp,
  a Navi gate's fight, the vow), the locks' own words at A, the dealer's
  one tip and his keys, ProtoMan, the rumors' Program Advances (a friend's
  recipe, rule 7's one tip), the nets' story.
- `navicust_words`' program descriptions (the shops' lists, the PET's mail
  and the second screen read them, as BN6's own program text names B+Left)
  and its bug causes (the second screen's NaviCust panel; no longer
  spoken).
- `dark_bn5_does` (what DarkPlus and DarkInvs do): the owner's call of 13
  October 2026.

