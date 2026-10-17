# trr-hybrid-controls

**Status: idea — nothing built yet.**

Makes both the original **tank controls** (d-pad) and **modern, camera-relative movement** (left stick) usable at the same time, with no menu toggle. Works for all six titles across the two remaster packs.

Normal traversal is easier with modern controls; precise jumps, grabs, and lining up with ledges are better with tank controls. Same split that worked in [re4og-modern-controls](https://github.com/TuxLux40/re4og-modern-controls): stick = modern, d-pad = tank.

## Targets

| Pack | Steam appid | Notes |
|------|-------------|--------|
| Tomb Raider I–III Remastered | *unverified* | Not installed on the dev machine yet |
| Tomb Raider IV–VI Remastered | `2525380` | Installed |

## Approach (starting notes)

- Both remasters already expose **modern** and **tank** schemes as a menu option, so both code paths exist in-game.
- Likely path: pick the scheme **per input source every frame** (stick → modern, d-pad → tank) instead of from the global setting — not a full camera-relative movement rewrite like the RE4 mod.
- Before reverse-engineering: check the existing modding scene (TRX / Tomb Raider Remastered modding Discord) for tools and prior art.

## Inspiration

[re4og-modern-controls](https://github.com/TuxLux40/re4og-modern-controls) — camera-relative stick movement while keeping RE4 tank on the d-pad.

## License

MIT — see [LICENSE](LICENSE).
