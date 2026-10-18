# RE notes — TR IV-VI Remastered (Steam build, tomb456.exe 2026-04-01)

Reproduce: `./run.sh` (headless Ghidra -> `out/*.c`, gitignored). `strtab.py` finds the name-string tables.

## Architecture
- `tomb456.exe` (x64 host, Aspyr/Saber "TRX2") loads `4|5|6/tomb?.dll`, which export one function `run`.
  The DLL talks to the host through a function/data table (`DAT_180664e48` in tomb4.dll; offsets like +0x4b0, +0x500, +0x9e4).
- Host polls pads itself: `LoadLibraryA("xinput1_3.dll")` (fallback `xinput9_1_0.dll`) + `GetProcAddress("XInputGetState")`
  (exe `FUN_140006…`, called for pads 0..3, then deadzone, stored in `DAT_140586130/138` sticks, `DAT_140586118` buttons).
  => an `xinput1_3.dll` proxy next to `tomb456.exe` is loaded first (app dir wins), same trick as re4og-modern-controls.
- Host also links SteamInput006 (used to pick button-prompt style, not obviously for movement) and raw HID (DualSense etc.).
- Options word `DAT_140585f64` is bit-packed (bit1, bits6-9 sensitivity, bits11-14, bit15, bit16).

## Option names
- tomb4.dll string table at VA 0x1801b8230 (340 entries): idx 84 PM_ROLL, 252 OPT_CONTROL, 272 OPT_CONTROLTYPE, 282 OPT_TANK, 283 OPT_MODERN.
  These are UI text ids only; no direct code ref to the scheme variable found yet.

## Plan
1. Find the scheme variable (set by OPT_CONTROLTYPE) and where movement consumes it, per game DLL.
2. Proxy flips it per frame: stick active -> modern, d-pad active -> tank.

## CONFIRMED (2026-10-18, live memory diff under Proton, /proc/<pid>/mem)
- **Control scheme = bit 1 of the byte at host VA `0x140585f64`** (`DAT_140585f64`): 1 = Modern (0x7b), 0 = Tank (0x79).
  Watched flipping 0x79 -> 0x7b -> 0x79 with the menu toggle. A copy lives at `0x1401a8b78` (probably what gets saved).
- `FUN_140001e90` (host) branches on `(DAT_140585f64 >> 1) & 1`: the Modern path adds the camera free-look deltas
  (`DAT_140585f04/f08`), the Tank path doesn't. Host image base is the fixed 0x140000000 (not relocated).
- Tools: `snap.py` / `diff.py` / `watch.py` (read the running game's memory; works as the same user, ptrace_scope=1 is no obstacle for /proc/pid/mem reads here).
