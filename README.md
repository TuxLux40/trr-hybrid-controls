<div align="center">

# TRR Hybrid Controls

**Tank on the d-pad. Modern on the stick. Both at once.**

No menu toggle — traverse with camera-relative movement, platform with the original tank controls, same session.

[![License: MIT](https://img.shields.io/badge/license-MIT-blue)](LICENSE)
![Status](https://img.shields.io/badge/status-idea-lightgrey)
![Games](https://img.shields.io/badge/Tomb%20Raider-I–VI%20Remastered-6B4E31)

</div>

> [!NOTE]
> **Nothing built yet.** This repo holds the design notes and prior-art check until implementation starts.

## Why

| | Modern (menu option) | Tank (classic) | **Hybrid (this mod)** |
|---|---|---|---|
| Getting around rooms | easy | slow turns | left stick |
| Precise jumps / ledge line-ups | soft / wrong moves | intentional | d-pad |
| Menu flip mid-platforming | required today | — | **not needed** |

The remasters already ship **both** schemes as a setting. The gap is using them **at the same time**, by input source.

## Planned behaviour

| Input | Scheme | Use |
|---|---|---|
| **Left stick** | Modern (camera-relative) | Traversal, exploration |
| **D-pad** | Tank (classic) | Platforming, grabs, lining up |
| **Everything else** | Game default | Jump, action, weapons, camera stick, … |

No global toggle. No “hold a modifier to switch.” Stick and d-pad just mean different movement models.

## Targets

| Pack | Steam appid | Dev machine |
|---|---|---|
| Tomb Raider I–III Remastered | *unverified* | not installed yet |
| Tomb Raider IV–VI Remastered | `2525380` | installed |

Goal: **all six titles** across both packs.

## Approach

1. **Reuse in-game code paths** — both modern and tank already exist; prefer selecting the scheme **per input source every frame** over rewriting camera-relative movement from scratch.
2. **Survey tools first** — TRX / Tomb Raider Remastered modding Discord, existing injectors and loaders for the Aspyr builds, before heavy reverse engineering.
3. **Fail safe** — unknown build → stay inactive; never corrupt saves or game files.

## Prior art (checked 2026-10-17)

No published mod found that keeps **tank + modern active simultaneously** on the Aspyr remasters.

| What exists | Relation to this idea |
|---|---|
| Stock remaster setting | Exclusive **or** modern **or** tank — menu toggle only |
| [TRX](https://github.com/LostArtefacts/TRX) (OG TR1–3 reimplementation) | Separate open request for modern controls ([#2868](https://github.com/LostArtefacts/TRX/issues/2868)); not the Aspyr packs; not hybrid stick/d-pad |
| [TRX_Extra 1–6](https://github.com/Nightshades1/TRX_Extra_1-6) | Cheats / QoL for remasters — no control hybrid |
| [Burn’s remaster multiplayer / launcher](https://github.com/burn-sours/tomb-raider-remastered-multiplayer) | MP + runtime mods — no stick/d-pad dual scheme |

If you know one, open an issue — this stub should not reinvent a finished wheel.

## Status

- [x] Problem statement
- [x] Prior-art pass
- [ ] Confirm I–III Steam appid + install both packs on the dev box
- [ ] Map how the remasters store / apply the control-scheme setting
- [ ] Prototype per-input routing (one title first, likely IV–VI)
- [ ] Installer + Linux/Proton path
- [ ] Expand to all six titles

## License

MIT — see [LICENSE](LICENSE).
