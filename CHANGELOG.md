# Changelog

## Unreleased

First playable version.

- The game itself runs from the player's BN6 Cybeast Gregar (USA) ROM on an
  embedded mGBA core: its battles, net, PET, shops, traders and music.
  Nothing from the ROM is shipped.
- Generated net layers in the game's own map formats: floors learned from one
  of the game's maps per area (Central, Seaside, Sky, Green, Graveyard,
  Undernet, Undernet Zero, Underground), with walls the game's collision
  reads. Each area builds its layers in its own layouts, after how BN6's net
  maps are laid out (docs/LEVEL_DESIGN.md): Central's routes, crater fields
  and catwalk mazes, Seaside's fields framed by comb boardwalks, Sky's
  mirrored hubs, Green's plank ladders, Graveyard's holed slabs, the
  Undernet's webs of long bridges and lattices of crosses. Walkways wear the
  area's second floor (blue catwalks, yellow boardwalks, orange planks). Pads
  on spurs and dead ends hide most of the Mystery Data. Tiles made for one place in the original (bridges, cut corners,
  decoration hanging off an edge) are not used on plain walkways. Sky and
  Undernet layers can raise a dead-end room onto a stair taken from the
  area's own map.
- Mystery Data, shops and traders draw from the whole chip library: every
  standard chip, the Megas and, deep in a run, the Gigas, by rarity.
- The story's Robot Control, Aquarium, Judge Tree, Mr. Weather and CopyBot
  comps and the ACDC, Green and Sky homepages join the first four acts, with their own battles, music,
  backgrounds and guardians. Every area now learns its floors from all of
  its maps in the same tiles and at each of their heights, so raised pools,
  fields and platforms have floor to copy; holes that faces hang over are
  told from floor by the walls around them. The originals' free-standing
  scenery (the Aquarium's coral, shells and starfish) stands beside the
  floor, and pads take the look of the original's pads (the Aquarium's
  yellow frames on legs). CopyBot's comp, whose floors no colour tells
  apart, learns them by shape: purple plateaus in stone rims with pods
  beneath, pink and white walkways with teal discs, magenta octagon pads. Runs saved before carry over.
- Three more areas for the first four acts, built from the game's computers
  and homepages: a comp in orange and green, a homepage in pink and teal and
  a comp in blue and pink, each with its own battles, background and
  guardians (CircusMan, Colonel, BlastMan, ElementMan, JudgeMan, DiveMan).
  SpoutMan and TenguMan join Seaside's and Sky's guardians, and most areas
  alternate between two battle backgrounds.
  Every guardian leaves three HPMemory. Runs saved before this version carry
  over.
- Each layer places the game's own exit pads, Mystery Data, Normal Navis,
  Mr. Progs, the Net Dealer, the program vendor, the Chip Trader and the
  BugFrag Trader, all running on the game's NPC and text scripts. Chip
  Traders speak the game's own lines and hand out chips from its own prize
  pools, stronger with depth; deeper layers can hold a Chip Trader Special
  (10 chips for 1).
- Random encounters from the game's own roll and its own formations: each
  area fights the battles of its original maps, 28 of the 29 virus families
  (all but WindBox) where Capcom put them, on their battlefields (grass, ice,
  holes, poison), with versions that grow with depth; viruses the story
  meets late (the dragons, Nightmare) wait for the later acts. Every third
  layer ends in a guardian's arena, staged after Hades' bosses
  (docs/BOSSES.md): a safe room with a heal and the Net Dealer before it;
  the arena seals, the guardian logs in over the boss prelude with a title
  card and lines that remember earlier battles, and the battle starts
  without a question. Deleted, it says a last word and logs out; its
  Guardian Data holds the reward and taking it makes the exit appear. An
  area-clear card and the next area's title card mark each act.
- Exits are the game's own warp pads: MegaMan jacks out and into the next
  layer, built while he jacks out. The Undernet and the Secret Area are
  entered the same way.
- Beaten Navis give their Cross, and the Graveyard's guardian Beast Out,
  through the story's own flags; the game's chat box says so.
- MegaMan stays in the run: R does not jack out, the PET's Save is off, and
  any other map sends him back to the layer.
- Choices on the game's text boxes: a strong virus signal, a dark flame into
  the Undernet, and a gate that three ScrtData open into the Secret Area.
- The original title screen, rebuilt from the ROM, with NEW GAME, CONTINUE and
  a run summary after the game's GAME OVER.
- Checkpoints on arrival at each layer; CONTINUE returns there. A profile
  keeps the best depth. Run saves from before this version are converted.
- PortMaster launcher for ROCKNIX, made for the Retroid Nova and the Retroid
  Pocket Flip 2.
- Dev tools (docs/DEVTOOLS.md): Select+R opens a dev menu for test runs (no
  random battles, can't die, one-hit enemies, up to 8x speed, win the battle,
  heal, zenny, the next layer or guardian, a chosen area). `build.py atlas`
  draws every area's layers with a report on their tiles; `build.py tour` has
  the game show every room of each area.
