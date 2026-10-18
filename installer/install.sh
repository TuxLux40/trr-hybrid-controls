#!/usr/bin/env bash
# TRR Hybrid Controls installer for Linux and Steam Deck (Proton).
#   ./install.sh               install or update
#   ./install.sh --uninstall   remove the mod
#   ./install.sh --defaults    same as install (no questions are asked)
#   ./install.sh --game-dir "/path/to/Tomb Raider IV-VI Remastered"   use this game folder instead of searching Steam
set -euo pipefail

APPID=2525380
EXE=tomb456.exe
SUPPORTED_SHA256=cf0f46623fd0d735ca19d691c7b84877529ab838a3065dfaa7d708c76ec359c0
HERE="$(cd "$(dirname "$0")" && pwd)"
DLL_SRC="$HERE/files/xinput1_3.dll"
REG_SECTION='[Software\\Wine\\AppDefaults\\tomb456.exe\\DllOverrides]'  # as written in user.reg
REG_MATCH='AppDefaults\\tomb456.exe\\DllOverrides]'

mode=install
game_dir=""
while [ $# -gt 0 ]; do
  case "$1" in
    --uninstall) mode=uninstall ;;
    --defaults) ;;
    --game-dir) game_dir="${2:-}"; shift ;;
    -h|--help) sed -n '2,6p' "$0"; exit 0 ;;
    *) echo "Unknown option: $1"; exit 1 ;;
  esac
  shift
done

say() { printf '%s\n' "$*"; }
die() { printf '\nError: %s\n' "$*" >&2; exit 1; }

# ------------------------------------------------------------------------------------------- find Steam and the game
steam_roots=()
for r in "$HOME/.local/share/Steam" "$HOME/.steam/steam" "$HOME/.steam/root" \
         "$HOME/.var/app/com.valvesoftware.Steam/.local/share/Steam" "$HOME/snap/steam/common/.local/share/Steam"; do
  [ -f "$r/steamapps/libraryfolders.vdf" ] && steam_roots+=("$(cd "$r" && pwd -P)")
done

libraries=()
for r in "${steam_roots[@]}"; do
  libraries+=("$r")
  while IFS= read -r p; do libraries+=("$p"); done < <(
    sed -n 's/^[[:space:]]*"path"[[:space:]]*"\(.*\)"[[:space:]]*$/\1/p' "$r/steamapps/libraryfolders.vdf")
done

if [ -z "$game_dir" ]; then
  for lib in "${libraries[@]}"; do
    acf="$lib/steamapps/appmanifest_$APPID.acf"
    [ -f "$acf" ] || continue
    name=$(sed -n 's/^[[:space:]]*"installdir"[[:space:]]*"\(.*\)"[[:space:]]*$/\1/p' "$acf")
    [ -f "$lib/steamapps/common/$name/$EXE" ] && { game_dir="$lib/steamapps/common/$name"; break; }
  done
fi
[ -n "$game_dir" ] || die "Tomb Raider IV-VI Remastered was not found in your Steam libraries.
Run the installer again with: ./install.sh --game-dir \"/path/to/Tomb Raider IV-VI Remastered\""
[ -f "$game_dir/$EXE" ] || die "$game_dir/$EXE not found. Is this the Tomb Raider IV-VI Remastered folder?"

prefix_reg=""
for lib in "${libraries[@]}"; do
  [ -f "$lib/steamapps/compatdata/$APPID/pfx/user.reg" ] && { prefix_reg="$lib/steamapps/compatdata/$APPID/pfx/user.reg"; break; }
done

if ps -eo args= | grep -qE "[\\]tomb456[.]exe$"; then die "Tomb Raider IV-VI Remastered is running. Quit the game first."; fi

say "TRR Hybrid Controls"
say "Game folder: $game_dir"

# ------------------------------------------------------------------------------------------- Proton DLL override
# Proton prefers its own xinput1_3; this tells it to load the mod's copy for tomb456.exe only (no launch option needed).
reg_add() {
  [ -n "$prefix_reg" ] || return 1
  grep -qF "$REG_MATCH" "$prefix_reg" && return 0
  cp "$prefix_reg" "$prefix_reg.trrhc-backup"
  printf '\n%s %s\n"xinput1_3"="native,builtin"\n' "$REG_SECTION" "$(date +%s)" >> "$prefix_reg"
}
reg_remove() {
  [ -n "$prefix_reg" ] || return 0
  grep -qF "$REG_MATCH" "$prefix_reg" || return 0
  # Drop the section header and its entries up to the next blank line.
  REG_MATCH="$REG_MATCH" awk 'BEGIN {m = ENVIRON["REG_MATCH"]} substr($0, 1, 1) == "[" && index($0, m) {skip=1; next}
       skip && /^$/ {skip=0; next}
       !skip' "$prefix_reg" > "$prefix_reg.tmp" && mv "$prefix_reg.tmp" "$prefix_reg"
}

is_ours() { [ -f "$1" ] && grep -qa "TRR Hybrid Controls" "$1"; }

# ------------------------------------------------------------------------------------------- uninstall
if [ "$mode" = uninstall ]; then
  if is_ours "$game_dir/xinput1_3.dll"; then rm -f "$game_dir/xinput1_3.dll"; say "Removed the mod."; else say "The mod is not installed."; fi
  [ -f "$game_dir/xinput1_3.dll.trrhc-backup" ] && mv "$game_dir/xinput1_3.dll.trrhc-backup" "$game_dir/xinput1_3.dll" && say "Restored your previous xinput1_3.dll."
  rm -f "$game_dir/TRRHybridControls.ini" "$game_dir/TRRHybridControls.log"
  reg_remove
  say "Done. If you added a WINEDLLOVERRIDES launch option for this mod yourself, you can remove it now."
  exit 0
fi

# ------------------------------------------------------------------------------------------- install
[ -f "$DLL_SRC" ] || die "files/xinput1_3.dll is missing next to this script. Extract the whole zip first."
if command -v sha256sum >/dev/null; then
  sum=$(sha256sum "$game_dir/$EXE" | cut -d' ' -f1)
  if [ "$sum" != "$SUPPORTED_SHA256" ]; then
    say ""
    say "Warning: this $EXE is not the build the mod supports (Steam, April 2026)."
    say "The mod will load but stay inactive, and the game plays as usual."
  fi
fi

if [ -f "$game_dir/xinput1_3.dll" ] && ! is_ours "$game_dir/xinput1_3.dll"; then
  mv "$game_dir/xinput1_3.dll" "$game_dir/xinput1_3.dll.trrhc-backup"
  say "Kept your previous xinput1_3.dll as xinput1_3.dll.trrhc-backup."
fi
cp "$DLL_SRC" "$game_dir/xinput1_3.dll.new" && mv "$game_dir/xinput1_3.dll.new" "$game_dir/xinput1_3.dll"
say "Installed the mod."

say ""
if reg_add; then
  say "Proton is set up to load the mod. Start the game from Steam as usual."
else
  say "One more step, needed once: the game has not been started on this PC yet."
  say "Either start it once from Steam, quit, and run this installer again,"
  say "or open Steam > Tomb Raider IV-VI Remastered > Properties > Launch Options and paste:"
  say ""
  say '    WINEDLLOVERRIDES="xinput1_3=n,b" %command%'
fi
