#!/usr/bin/env bash
# Builds the x64 proxy xinput1_3.dll with zig (clang + lld, mingw target): build/xinput1_3.dll
#   bash build.sh                       development build (version "dev")
#   VERSION=1.0.0 bash build.sh package release build + dist/TRRHybridControls-v1.0.0.zip
# zig comes from PATH, or set ZIG, e.g. ZIG="uv tool run --from ziglang python -m ziglang".
# FOLLOW_MENU=1 builds the beta variant: the buttons follow the Control Type of the game menu.
set -euo pipefail
cd "$(dirname "$0")"
ZIG="${ZIG:-zig}"
VERSION="${VERSION:-dev}"
mkdir -p build
$ZIG c++ -target x86_64-windows-gnu -O2 -std=c++17 -shared -fno-exceptions -fno-rtti -g0 -s \
  "-DTRRHC_VERSION_STR=\"$VERSION\"" ${FOLLOW_MENU:+-DTRRHC_FOLLOW_MENU} src/proxy.cpp src/xinput1_3.def \
  -o build/xinput1_3.dll -luser32 -lkernel32
ls -la build/xinput1_3.dll

if [ "${1:-}" = package ]; then
  mkdir -p dist
  python3 - "$VERSION" <<'PY'
import sys, zipfile
v = sys.argv[1]
root = f"TRRHybridControls-v{v}/"
out = f"dist/TRRHybridControls-v{v}.zip"

def add(z, src, dst, executable=False):
    info = zipfile.ZipInfo.from_file(src, root + dst)
    info.compress_type = zipfile.ZIP_DEFLATED
    info.external_attr = (0o100755 if executable else 0o100644) << 16
    with open(src, "rb") as f:
        z.writestr(info, f.read())

with zipfile.ZipFile(out, "w") as z:
    add(z, "installer/install.bat", "install.bat")
    add(z, "installer/install.ps1", "install.ps1")
    add(z, "installer/install.sh", "install.sh", executable=True)
    add(z, "docs/INSTALL.txt", "README.txt")
    add(z, "LICENSE", "LICENSE.txt")
    add(z, "build/xinput1_3.dll", "files/xinput1_3.dll")
print(out)
PY
fi
