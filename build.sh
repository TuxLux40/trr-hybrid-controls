#!/usr/bin/env bash
# Builds the x64 proxy xinput1_3.dll with zig (clang + lld, mingw target): build/xinput1_3.dll
# zig comes from PATH, or set ZIG, e.g. ZIG="uv tool run --from ziglang python -m ziglang".
set -euo pipefail
cd "$(dirname "$0")"
ZIG="${ZIG:-zig}"
VERSION="${VERSION:-dev}"
mkdir -p build
$ZIG c++ -target x86_64-windows-gnu -O2 -std=c++17 -shared -fno-exceptions -fno-rtti -g0 -s \
  "-DTRRHC_VERSION_STR=\"$VERSION\"" ${FOLLOW_MENU:+-DTRRHC_FOLLOW_MENU} src/proxy.cpp src/xinput1_3.def \
  -o build/xinput1_3.dll -luser32 -lkernel32
ls -la build/xinput1_3.dll
