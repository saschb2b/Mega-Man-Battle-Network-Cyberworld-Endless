---
type: Research
title: "Settings screens elsewhere: what Options can take"
description: "How well-regarded games, the accessibility guidelines, emulator frontends and retro collections show a setting's effect while it is chosen (live preview, a still frame behind the menu, a picture per setting, a one-press look), describe, confirm, reset and lay out their settings, researched on 2 November 2026 for Options' Display, and what of it fits a PET panel at 240x160."
tags: [engine, ui, options, research]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-02T12:00:00Z }
sources:
  - id: xag
    resource: "https://learn.microsoft.com/en-us/gaming/accessibility/guidelines"
    title: "Xbox Accessibility Guidelines 101-120 (v3.2), read on 2026-11-02"
  - id: gag
    resource: "https://gameaccessibilityguidelines.com/full-list/"
    title: "Game Accessibility Guidelines, the full list and 21 of its pages, read on 2026-11-02"
  - id: apple-hig-settings
    resource: "https://developer.apple.com/design/human-interface-guidelines/settings"
    title: "Apple's Human Interface Guidelines: Settings, Game controls, Designing for games"
  - id: retroarch
    resource: "https://github.com/libretro/RetroArch"
    title: "RetroArch's source (runloop.c, config.def.h, settings_def_menu_*.h, msg_hash_us.h) and docs.libretro.com's Quick Menu and Shaders guides"
  - id: mgba-qt
    resource: "https://github.com/mgba-emu/mgba/tree/master/src/platform/qt"
    title: "mGBA's desktop frontend: SettingsView.ui, ShaderSelector.ui and ShaderSelector.cpp"
  - id: analogue-pocket
    resource: "https://assets.analogue.co/pdf/f48dbccc2a7c529e3ef677db3b4ab9d8/Analogue+Pocket+User+Manual+v1.2.pdf"
    title: "Analogue Pocket user manual v1.2: Original Display Modes"
  - id: owner-valhalla
    resource: "human:saschb2b"
    title: "Assassin's Creed Valhalla's Graphics settings (PC), a screenshot the owner shared on 2026-11-02"
---

# Settings screens elsewhere: what Options can take

Options' Display ([The GBA's screen](lcd.md) has its lines) covers the
whole 240x160 picture while the player chooses, so Colour correction,
Frame blending and LCD 3x change a picture nobody can see until Options
closes. The owner asked on 2 November 2026 how well-regarded games handle
this before Options was redesigned. The research below was done that day
in three time-boxed passes (the accessibility guidelines, emulators and
retro collections, big-budget games); each claim names its source and is
**verified** (read on that page or in that source file) or **assumed**
(from memory or a search engine's excerpt, the page itself not read). A
negative claim names the search that came up empty. What the PET's
screens may look like is [The PET's screens](ui.md); this page is the
evidence the redesign of Options rests on.

## Seeing the setting while it is chosen

- **The guidelines ask for it.** XAG 114 (UI context): show a visual
  simulation of how a setting will change what the player sees, in a
  realistic game context where possible (Fenyx Immortals Rising's in-game
  preview as the cursor moves). XAG 104 (Subtitles) says the same for
  subtitles, with Forza Horizon 4 showing its preview on one button, the
  View button. Verified ([XAG 114](https://learn.microsoft.com/en-us/gaming/accessibility/xbox-accessibility-guidelines/114),
  [XAG 104](https://learn.microsoft.com/en-us/gaming/accessibility/xbox-accessibility-guidelines/104)).
  The Game Accessibility Guidelines have no such rule: the site's search
  for "preview" returns nothing, and neither its 108 titles nor the 21
  settings pages read mention a preview or a sample (verified, negative).
- **Apple puts a setting where its effect is.** The HIG's Settings page:
  task-specific options belong in the screens they affect, as a separate
  settings area "often hides the results" until the task resumes.
  Verified ([Settings](https://developer.apple.com/design/human-interface-guidelines/settings)).
- **RetroArch shows the frozen game under its menu.** "Pause Content When
  Menu Is Active" is on by default; the core runs no frame while the Quick
  Menu is up, and RetroArch presents the last frame it made with the menu
  over it, the menu's background at 0.9 opacity by default and RGUI's
  "Transparency" (the content shown behind the menu) on by default.
  Verified in its source (`runloop.c`, `config.def.h`,
  `settings_def_menu_appearance.h`, `settings_def_menu_rgui_transparency.h`)
  and the [Quick Menu guide](https://docs.libretro.com/guides/quick-menu/).
  A shader's parameters are "previewed live" as they change; Apply Changes
  is needed only to rebuild the chain (verified, the
  [Shaders guide](https://docs.libretro.com/guides/shaders/)). That the
  frozen frame is drawn again through a changed shader is assumed from the
  code path.
- **mGBA's desktop app** applies Interframe blending and integer scaling
  on Apply or OK in its Settings dialog; its Shaders window writes each
  parameter into the shader as it changes, with Reset and Restore Defaults
  and no Apply (verified, `SettingsView.ui`, `ShaderSelector.ui`,
  `ShaderSelector.cpp`). That the game's window stays visible beside the
  dialog is assumed; it holds only on a desktop.
- **The Analogue Pocket switches the picture with no menu at all.**
  Holding the Analogue button and pressing Left or Right cycles the
  Original Display Modes on the running game; the same modes, Frame
  Blending among them, sit in Settings > Systems > GBA > Video. Verified
  ([manual v1.2](https://assets.analogue.co/pdf/f48dbccc2a7c529e3ef677db3b4ab9d8/Analogue+Pocket+User+Manual+v1.2.pdf)).
  It is the one pattern found where frame blending can be judged as it is
  chosen, since the game keeps running.
- **Nintendo's own choose blind.** The NES Classic's display mode is
  picked from its HOME menu with no game running, and its support page
  says there is no on-screen confirmation of the change
  ([a_id 17357](https://en-americas-support.nintendo.com/app/answers/detail/a_id/17357),
  verified). The Switch Online GBA app's two toggles ("Reproduce classic
  feel", "Display with small screen") sit in its Settings
  ([a_id 41232](https://en-americas-support.nintendo.com/app/answers/detail/a_id/41232),
  verified); that Settings opens only from the game list is assumed.
- **Battle Network Legacy Collection** has a filter on by default (plain
  or smoothed pixels), four screen sizes and, on PC, a colour space,
  set from the PET hub of its main menu (verified,
  [Steam Deck HQ](https://steamdeckhq.com/game-reviews/mega-man-battle-network-legacy-collection/),
  [TMMN](https://themmnetwork.com/2023/04/12/tmmn-reviews-mega-man-battle-network-legacy-collection/)).
  Whether it can be reached during play or shows the game while chosen
  was not found; Capcom's web manual has no display section.
- **A picture per setting.** Assassin's Creed Valhalla's Graphics page
  (the owner's screenshot) puts the list on the left and, for the line
  under the cursor, its name, a sentence on what it does and a picture of
  a scene it changes on the right; Q and E turn between the tabs (Screen,
  Graphics, Controls, Gameplay, Interface, Sound), sections inside a tab
  are headed (Environment, Textures, Postprocessing), and the foot names
  Back, Default (R) and Apply. Verified from the screenshot.
- **Motion needs motion.** A still frame shows a colour change and a
  grid; it cannot show frame blending or a refresh-rate fix, which change
  how one frame follows the next (assumed by construction: mGBA's Simple,
  Smart and LCD ghosting give a still frame back unchanged,
  [The GBA's screen](lcd.md)). Of the frontends read, only the Pocket's
  shortcut shows blending as it is chosen.

## Saying what a setting does

- XAG 114: help text for the item under focus where needed (Forza
  Horizon 4's under its list), settings words readable at 7 to 9 years of
  school, the game's own terms defined. Verified.
- GAG "Use simple clear language" (Cognitive, Basic): plain, not dumbed
  down. Verified.
- Never carry a value on colour alone (XAG 103, GAG Vision Basic):
  verified. Options' values are words, so this already holds.

## Applying, confirming, resetting

- XAG 115 (Error messages and destructive actions): let players review or
  reverse a change before it is committed (Grounded warns when leaving
  settings without Apply), offer a reset to defaults (Grounded's "Factory
  Default"), and never require a held button to confirm. Verified.
- No guideline covers reverting a display change that leaves the screen
  unusable (searched XAG 101-120 and the 21 GAG pages for revert, keep
  changes, countdown, resolution, brightness, gamma: nothing). XAG 116
  (Time limits) asks any UI time limit to be essential, warned of in more
  than one way, and extendable with one press for at least 20 seconds.
  Windows' own "Keep these display settings?" reverting after about 15
  seconds is assumed from memory.
- Apple: good defaults so most players change nothing; as few settings as
  possible; settings opened where players expect them. Verified.

## Laying out and moving through them

- XAG 112 (UI navigation): a linear list wraps from its last line to its
  first and back; Back always in the same place and always possible; every
  screen reachable by digital input alone. Verified.
- XAG 113: a focus mark always visible, never on a hidden element; focus
  moves into a dialog and stays there. Verified.
- XAG 114 and Apple's Game controls page: the shoulder buttons move
  between tabs or sections the same way everywhere (Forza Horizon 4's LB
  and RB); A takes, B goes back. Verified. Valhalla's Q and E are the same
  on a keyboard.
- XAG 107 (Input): menus work with single presses, never two buttons at
  once; a slider needs no held button; a touch acts on release, sliding
  off cancels. GAG asks a hold to have a toggle alternative. Verified.
- Touch targets: XAG 107 asks 15 mm on a phone, GAG 0.96 cm at least,
  Apple 28 by 28 points in menus, Android 48 by 48 dp. Verified.

## Filters themselves

- XAG 117: moving or blinking menu content can be turned off; blur can be
  turned off. XAG 118 and GAG "Avoid flickering images and repetitive
  patterns": flash and pattern limits; GAG counts more than eight static
  high-contrast stripes over 40% of the screen as a trigger, so a
  full-screen grid filter could count (inferred), and asks a setting be
  named for what it changes, never "epilepsy safe". Verified.
  Options' LCD 3x is off by default, which this supports.

## What fits a PET panel

Options is a PET screen of the engine's own at the game's 240x160, on
every platform from a 3DS's top screen to a phone held upright. What the
research offers, held against it:

| Pattern | Seen in | At 240x160 in the PET's look |
| --- | --- | --- |
| The game frozen behind the menu, the setting drawn on it | RetroArch | The game already waits under Options with its last frame; only the panel hides it |
| A preview picture beside the list | Valhalla, Fenyx (XAG 114) | A slot beside a six-line list leaves about 90 by 46 of the game's pixels: too little to judge a filter by, and a scaled-down picture loses the pixels the filters work on |
| One press shows the preview | Forza Horizon 4 (XAG 104) | A key's hint on the band ([The PET's screens](ui.md), Keys); a single press, not a hold (XAG 107) |
| The menu gives way to the picture | RetroArch's transparency, the Pocket's shortcut | The panel stepping aside to a strip at the foot, as BN6's PET menu lets the field show through its middle |
| Help text for the line under the cursor | Forza Horizon 4, Valhalla | Already Options' INFORMATION box |
| Sections on the shoulder buttons | Forza Horizon 4, Valhalla, Apple | L and R under green arrows, as the Library turns its pages |
| Default on one key | Valhalla, Grounded (XAG 115) | Six lines whose defaults are Off and Auto: little to reset |
| Apply with a countdown | Windows (assumed), mGBA's Apply | Not needed: every display change here is borderless and instant, none can leave the screen unusable |
| The filter cycled while playing | Analogue Pocket | The only way to judge Frame blending as it is chosen; a new key in play, the owner's to decide |

The layouts mocked from this and the owner's choice are in [The PET's
screens](ui.md), Ours.
