<div align="center">

# TRR Hybrid Controls

**Tank on the d-pad. Modern on the stick. Both at once.**

Traverse with camera-relative movement. Platform with classic tank controls. No menu toggle mid-jump.

[![Build](https://github.com/TuxLux40/trr-hybrid-controls/actions/workflows/release.yml/badge.svg)](https://github.com/TuxLux40/trr-hybrid-controls/actions/workflows/release.yml)
[![Latest release](https://img.shields.io/github/v/release/TuxLux40/trr-hybrid-controls?include_prereleases&label=download)](https://github.com/TuxLux40/trr-hybrid-controls/releases)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue)](LICENSE)
![Windows](https://img.shields.io/badge/Windows-untested-lightgrey?logo=windows)
![Linux / Steam Deck](https://img.shields.io/badge/Linux%20%2F%20Steam%20Deck-tested-FCC624?logo=linux&logoColor=black)

</div>

## What you get

The remasters let you pick **modern** or **tank** in the menu — one or the other. This mod keeps **both** live:

| | |
|---|---|
| 🕹️ **Left stick → modern** | Move where you push, relative to the camera. Good for getting around. |
| ⬆️ **D-pad → tank** | Classic turn-and-walk. Good for precise jumps, grabs, and lining up with ledges. Gives you the backward jump that Modern controls can't do. |
| 🎯 **No toggle** | Stick and d-pad simply mean different movement. Buttons keep the game's Modern layout. |
| 🔫 **Combat stays modern** | While you hold the left trigger the game uses Modern, so drawing, aiming and firing work as usual. |

## Supported games

| Game | Status |
|---|---|
| Tomb Raider IV: The Last Revelation | ✅ **Works** — tested on Linux (Proton) |
| Tomb Raider V: Chronicles | ⏳ Should work (same engine host), not tested yet |
| Tomb Raider VI: The Angel of Darkness | ⏳ Should work (same engine host), not tested yet |
| Tomb Raider I | 🛠️ Planned (separate game pack and executable) |
| Tomb Raider II | 🛠️ Planned |
| Tomb Raider III | 🛠️ Planned |

IV–VI are all in **Tomb Raider IV–VI Remastered** and share one executable, so one install covers all three.
I–III are a separate pack (**Tomb Raider I–III Remastered**) and need their own work.

## Install

1. **Download** the latest zip from [**Releases**](https://github.com/TuxLux40/trr-hybrid-controls/releases) and extract it anywhere.
2. **Run the installer**
   - **Linux / Steam Deck:** run `install.sh` (Desktop Mode: right-click → *Run in Konsole*)
   - **Windows:** double-click `install.bat`
3. Start the game from Steam as usual and set **Control Type** to **Modern** in the options menu.

The installer finds the game in your Steam libraries and, on Linux, tells Proton to load the mod
(no launch option needed). To remove it: `install.sh --uninstall` / `install.bat -Uninstall`.

Manual install: copy `files/xinput1_3.dll` next to `tomb456.exe`. On Linux also set the launch option
`WINEDLLOVERRIDES="xinput1_3=n,b" %command%`.

## Settings

`TRRHybridControls.ini` is created next to `tomb456.exe` the first time you start the game:

| Key | Default | |
|---|---|---|
| `Stick` | `Modern` | Movement scheme of the left stick (`Modern` or `Tank`) |
| `DPad` | `Tank` | Movement scheme of the d-pad (`Modern` or `Tank`) |
| `AimKeepsStick` | `1` | `1`: while the left trigger is held the game uses the stick's scheme. `0`: the d-pad keeps Tank while aiming |

Changes apply the next time the game starts. `TRRHybridControls.log` shows what the mod did.

## How it works, and its limit

The game keeps one control scheme at a time in a single setting and checks it in hundreds of places (movement,
weapons, aiming, button layout). The mod sits in front of the game's controller input and flips that setting from
frame to frame: stick → Modern, d-pad → Tank. So it **switches quickly between the two schemes**, it does not run both
at the same moment. For platforming and exploring this feels like both at once; Lara's current animation always
finishes in the scheme it started in.

Because of this:

- The d-pad is meant for **platforming**, not combat. That is why holding the left trigger returns to Modern.
- The button layout is the game's Modern layout whatever the menu says. A variant that follows the menu's choice for
  tank purists lives on the [`beta`](https://github.com/TuxLux40/trr-hybrid-controls/tree/beta) branch.
- Everything beyond that (a true simultaneous scheme, remapping individual actions, other control tweaks) is out of scope.

The mod only activates on the exact `tomb456.exe` build it was made for (Steam, April 2026). After a game update it
stays inactive and the game plays as usual until the mod is updated.

## Build

Needs [zig](https://ziglang.org/) 0.17, bash and python3:

```sh
bash build.sh                 # build/xinput1_3.dll
VERSION=1.0.0 bash build.sh package   # + dist/TRRHybridControls-v1.0.0.zip
```

How the control setting was found is in [re/NOTES.md](re/NOTES.md).

## License

MIT — see [LICENSE](LICENSE). Free to use, share, and mod further.
