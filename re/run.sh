#!/bin/sh
# Headless Ghidra: import each binary, analyze, dump decompiled C + function index to re/out/
cd "$(dirname "$0")"
G="/mnt/ssd1tb/SteamLibrary/steamapps/common/Tomb Raider IV-VI Remastered"
for f in "$G/4/tomb4.dll" "$G/tomb456.exe" "$G/5/tomb5.dll" "$G/6/tomb6.dll"; do
  n=$(basename "$f")
  ghidra-analyzeHeadless proj trr -import "$f" -overwrite -scriptPath scripts -postScript DumpDecomp.java out > out/$n.log 2>&1
done
