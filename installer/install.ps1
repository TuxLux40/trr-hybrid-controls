# TRR Hybrid Controls installer for Windows. Start it by double-clicking install.bat.
#   install.bat              install or update
#   install.bat -Uninstall   remove the mod
#   install.bat -GameDir "D:\Games\Steam\steamapps\common\Tomb Raider IV-VI Remastered"
param(
    [switch]$Uninstall,
    [switch]$Defaults,
    [string]$GameDir = ""
)
$ErrorActionPreference = "Stop"

$AppId = 2525380
$Exe = "tomb456.exe"
$SupportedSha256 = "CF0F46623FD0D735CA19D691C7B84877529AB838A3065DFAA7D708C76EC359C0"
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$DllSrc = Join-Path $Here "files\xinput1_3.dll"

function Fail($msg) { Write-Host ""; Write-Host "Error: $msg" -ForegroundColor Red; exit 1 }

# ------------------------------------------------------------------------------------------- find Steam and the game
function Get-SteamLibraries {
    $roots = @()
    foreach ($key in "HKCU:\Software\Valve\Steam", "HKLM:\SOFTWARE\WOW6432Node\Valve\Steam", "HKLM:\SOFTWARE\Valve\Steam") {
        $p = Get-ItemProperty -Path $key -ErrorAction SilentlyContinue
        if ($p -and $p.SteamPath) { $roots += ($p.SteamPath -replace "/", "\") }
        if ($p -and $p.InstallPath) { $roots += $p.InstallPath }
    }
    $libs = @()
    foreach ($r in ($roots | Select-Object -Unique)) {
        $libs += $r
        $vdf = Join-Path $r "steamapps\libraryfolders.vdf"
        if (Test-Path $vdf) {
            foreach ($m in [regex]::Matches((Get-Content $vdf -Raw), '"path"\s+"([^"]+)"')) {
                $libs += ($m.Groups[1].Value -replace "\\\\", "\")
            }
        }
    }
    return $libs | Select-Object -Unique
}

if (-not $GameDir) {
    foreach ($lib in Get-SteamLibraries) {
        $acf = Join-Path $lib "steamapps\appmanifest_$AppId.acf"
        if (-not (Test-Path $acf)) { continue }
        $m = [regex]::Match((Get-Content $acf -Raw), '"installdir"\s+"([^"]+)"')
        if (-not $m.Success) { continue }
        $dir = Join-Path $lib ("steamapps\common\" + $m.Groups[1].Value)
        if (Test-Path (Join-Path $dir $Exe)) { $GameDir = $dir; break }
    }
}
if (-not $GameDir) {
    Fail "Tomb Raider IV-VI Remastered was not found in your Steam libraries.`nRun it again as: install.bat -GameDir `"C:\path\to\Tomb Raider IV-VI Remastered`""
}
if (-not (Test-Path (Join-Path $GameDir $Exe))) { Fail "$GameDir\$Exe not found. Is this the Tomb Raider IV-VI Remastered folder?" }
if (Get-Process -Name "tomb456" -ErrorAction SilentlyContinue) { Fail "Tomb Raider IV-VI Remastered is running. Quit the game first." }

$Dll = Join-Path $GameDir "xinput1_3.dll"
$Backup = Join-Path $GameDir "xinput1_3.dll.trrhc-backup"

function Test-Ours($path) {
    if (-not (Test-Path $path)) { return $false }
    return [System.Text.Encoding]::ASCII.GetString([System.IO.File]::ReadAllBytes($path)).Contains("TRR Hybrid Controls")
}

Write-Host "TRR Hybrid Controls"
Write-Host "Game folder: $GameDir"

# ------------------------------------------------------------------------------------------- uninstall
if ($Uninstall) {
    if (Test-Ours $Dll) { Remove-Item $Dll; Write-Host "Removed the mod." } else { Write-Host "The mod is not installed." }
    if (Test-Path $Backup) { Move-Item $Backup $Dll; Write-Host "Restored your previous xinput1_3.dll." }
    Remove-Item (Join-Path $GameDir "TRRHybridControls.ini"), (Join-Path $GameDir "TRRHybridControls.log") -ErrorAction SilentlyContinue
    Write-Host "Done."
    exit 0
}

# ------------------------------------------------------------------------------------------- install
if (-not (Test-Path $DllSrc)) { Fail "files\xinput1_3.dll is missing next to this script. Extract the whole zip first." }
$sum = (Get-FileHash (Join-Path $GameDir $Exe) -Algorithm SHA256).Hash
if ($sum -ne $SupportedSha256) {
    Write-Host ""
    Write-Host "Warning: this $Exe is not the build the mod supports (Steam, April 2026)." -ForegroundColor Yellow
    Write-Host "The mod will load but stay inactive, and the game plays as usual."
}
if ((Test-Path $Dll) -and -not (Test-Ours $Dll)) {
    Move-Item $Dll $Backup -Force
    Write-Host "Kept your previous xinput1_3.dll as xinput1_3.dll.trrhc-backup."
}
Copy-Item $DllSrc $Dll -Force
Write-Host "Installed the mod."
Write-Host ""
Write-Host "Done. Start the game from Steam as usual." -ForegroundColor Green
