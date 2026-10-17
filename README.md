<div align="center">

# TRR Hybrid Controls

**Tank on the d-pad. Modern on the stick. Both at once.**

Traverse with camera-relative movement. Platform with classic tank controls. No menu toggle mid-jump.

[![License: MIT](https://img.shields.io/badge/license-MIT-blue)](LICENSE)
![Status](https://img.shields.io/badge/status-coming%20soon-yellow)
![Games](https://img.shields.io/badge/Tomb%20Raider-I%E2%80%93VI%20Remastered-6B4E31)
![Windows](https://img.shields.io/badge/Windows-planned-0078D6?logo=windows)
![Linux / Steam Deck](https://img.shields.io/badge/Linux%20%2F%20Steam%20Deck-planned-FCC624?logo=linux&logoColor=black)

</div>

> [!NOTE]
> **Not released yet.** This page describes what the mod will do. Downloads and an installer will appear in
> [Releases](https://github.com/TuxLux40/trr-hybrid-controls/releases) when the first build is ready.

## What you get

The remasters let you pick **modern** or **tank** in the menu — one or the other. This mod keeps **both** live:

| | |
|---|---|
| 🕹️ **Left stick → modern** | Move where you push, relative to the camera. Good for getting around. |
| ⬆️ **D-pad → tank** | Classic turn-and-walk. Good for precise jumps, grabs, and lining up with ledges. |
| 🎯 **No toggle** | Stick and d-pad simply mean different movement. Everything else stays on the game’s own buttons. |

Modern controls stay the base so button mappings never change. The D-pad is only used for precise tank-style positioning (walk, backwards, tight turns).

## Controls

| Input | Effect |
|---|---|
| **Left stick** | Modern, camera-relative movement |
| **D-pad** | Original tank controls (precise positioning) |
| **Right stick, face buttons, triggers, …** | Unchanged — jump, action, weapons, look, inventory as usual |

## Supported games

| Pack | Titles |
|---|---|
| **Tomb Raider I–III Remastered** | I, II, III |
| **Tomb Raider IV–VI Remastered** | IV, V, VI (Angel of Darkness) |

Steam on **Windows** and **Linux / Steam Deck** (Proton) are the planned targets.

## Install

Installer and step-by-step instructions will land with the first release. Expected flow (same idea as other Steam mods here):

1. Download the release zip from [Releases](https://github.com/TuxLux40/trr-hybrid-controls/releases)
2. Run the installer (Windows or Linux / Deck Desktop Mode)
3. Start the game from Steam as usual

Nothing to paste into launch options by hand if the installer can set it up.

## Status

- [ ] Research input handling in both collections
- [ ] Prototype D-pad → tank movement while keeping modern scheme
- [ ] Camera interaction tests
- [ ] Windows + Proton builds
- [ ] Installer

## Why this exists

Modern controls are better for general traversal, but Lara always runs and cannot walk backwards. Tank controls are better for precise platforming, jumps, and lining up with ledges. This mod lets you use both without switching in the menu.

## Technical approach

Keep the game’s control scheme permanently on modern so button mappings stay consistent. Intercept D-pad input and apply tank-style movement interpretation (relative to Lara’s facing, with walk/backwards) only for that source. Left stick continues to use the normal modern path.

## Open questions

- Does forcing tank-style movement on the D-pad still allow walking (not running) and proper backwards movement?
- How does the camera behave when movement comes from the D-pad vs stick?
- Priority when both stick and D-pad are active.
- Any side effects on animation, footstep sounds, or interaction prompts.

## License

MIT — see [LICENSE](LICENSE). Free to use, share, and mod further.
