# Changelog

## Unreleased

First playable version.

- The game itself runs from the player's BN6 Cybeast Gregar (USA) ROM on an
  embedded mGBA core: its battles, net, PET, shops, traders and music.
  Nothing from the ROM is shipped.
- Generated net layers in the game's own map formats: floors learned from one
  of the game's maps per area (Central, Seaside, Sky, Green, Graveyard,
  Undernet, Undernet Zero, Underground), with walls the game's collision
  reads.
- Each layer places the game's own exit pads, Mystery Data, Normal Navis,
  Mr. Progs, the Net Dealer, the program vendor, the Chip Trader and the
  BugFrag Trader, all running on the game's NPC and text scripts.
- Random encounters from the game's own roll, with viruses and versions that
  grow with depth; a Navi guards every third layer's exit.
- Choices on the game's text boxes: a strong virus signal, a dark flame into
  the Undernet, and a gate that three ScrtData open into the Secret Area.
- The original title screen, rebuilt from the ROM, with NEW GAME, CONTINUE and
  a run summary after the game's GAME OVER.
- Checkpoints on arrival at each layer; CONTINUE returns there. A profile
  keeps the best depth.
- PortMaster launcher for ROCKNIX; tested on the Retroid Nova and the
  Retroid Pocket Flip 2.
