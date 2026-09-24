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
- Each layer places the game's own exit pads, Mystery Data, Normal Navis,
  Mr. Progs, the Net Dealer, the program vendor, the Chip Trader and the
  BugFrag Trader, all running on the game's NPC and text scripts. Chip
  Traders speak the game's own lines and hand out chips from its own prize
  pools, stronger with depth; deeper layers can hold a Chip Trader Special
  (10 chips for 1).
- Random encounters from the game's own roll and its own formations: each
  area fights the battles of its original maps, all 29 virus families where
  Capcom put them, on their battlefields (grass, ice, holes, poison), with
  versions that grow with depth. A guardian Navi stands before every third layer's exit
  pad, which stays shut until he is beaten.
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
