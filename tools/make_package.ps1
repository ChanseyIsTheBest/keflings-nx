# keflings-nx: the console package (D:\kingdomkeflings\packages\<name>\), from a Switch build folder.
#   powershell -File tools\make_package.ps1 <name> [build folder, default app\out\switch]
# (2026-10-30: the everyday build until the user asks for the PGO + LTO one, app\out\switch-pgo-use.)
# Layout: COPY_TO_SD\switch\keflings-nx\ (NRO, tomls, configs, notes), and beside it for console profiles: the ELF,
# the link map, the build's CMakeCache and a source snapshot (git archive of HEAD plus the working tree's changes).
# No game data: game_root\ is the player's own (README), the shaders in the NRO are made from the game's files on this
# PC and the NRO is for the user's own console only.
param([Parameter(Mandatory = $true)][string]$Name, [string]$Build = "")
$root = Split-Path -Parent $PSScriptRoot
if (-not $Build) { $Build = Join-Path $root "app\out\switch" }
$pkg = Join-Path "D:\kingdomkeflings\packages" $Name
if (Test-Path $pkg) { "package folder $pkg exists: not overwriting"; exit 1 }
$free = (Get-CimInstance Win32_LogicalDisk -Filter "DeviceID='D:'").FreeSpace / 1GB
if ($free -lt 3) { "D: has only {0:N1} GB free: not packaging" -f $free; exit 2 }
$sd = Join-Path $pkg "COPY_TO_SD\switch\keflings-nx"
New-Item -ItemType Directory -Force $sd, (Join-Path $sd "game_root") | Out-Null
Copy-Item (Join-Path $Build "renut.nro") (Join-Path $sd "keflings-nx.nro")
$dist = Join-Path $root "dist\switch\keflings-nx"
# 2026-10-31 (the user's request): only the player's renut.toml (language, input, MSAA, player icon); every other
# setting is built into the NRO (app/keflings_switch_defaults.toml).
foreach ($f in "renut.toml", "NOTES-test.txt") {
  Copy-Item (Join-Path $dist $f) (Join-Path $sd $f)
}
Set-Content -Path (Join-Path $sd "game_root\PUT_YOUR_GAME_HERE.txt") -Encoding ascii -Value @(
  "Copy your own A Kingdom for Keflings files here: default.xex and the fsxb2 folder (from the game package,",
  "content type 000D0000), AND title update 2's default.xexp (keflings-nx is title update 2, 0.0.3.2).")
# 2026-10-30 (evening): title update 2's own files and the DLC packages.
New-Item -ItemType Directory -Force (Join-Path $sd "update"), (Join-Path $sd "dlc") | Out-Null
Set-Content -Path (Join-Path $sd "update\PUT_THE_UPDATE_HERE.txt") -Encoding ascii -Value @(
  "Copy title update 2's fsxb2 folder here (fsxb2\blockfiles\Title_Update.blk.bz2 and the three font blocks),",
  "so this folder holds update\fsxb2\blockfiles\... . Its default.xexp goes in game_root.")
Set-Content -Path (Join-Path $sd "dlc\PUT_DLC_PACKAGES_HERE.txt") -Encoding ascii -Value @(
  "Optional: the Kingdom Pack 1 / 2 package files as they come from the console (584108DB\00000002\<40-character",
  "name>, about 60 KB each). keflings-nx installs them at start; the kingdoms appear in Choose your Kingdom.")
# 2026-10-31: no four-core forwarder NPDM any more (the user: three cores are enough).
$sym = Join-Path $pkg "for_console_profiles"
New-Item -ItemType Directory -Force $sym | Out-Null
foreach ($f in "renut.elf", "renut", "renut.map", "CMakeCache.txt") {
  $p = Join-Path $Build $f
  if (Test-Path $p) { Copy-Item $p (Join-Path $sym $(if ($f -eq "renut") { "renut.elf" } else { $f })) }
}
Push-Location $root
git archive --format=zip -o (Join-Path $sym "source-snapshot.zip") HEAD
git diff HEAD > (Join-Path $sym "source-snapshot-uncommitted.diff")
git rev-parse HEAD > (Join-Path $sym "source-snapshot-commit.txt")
Pop-Location
Get-ChildItem -Recurse $pkg -File | Select-Object @{n = 'File'; e = { $_.FullName.Substring($pkg.Length + 1) } }, Length |
  Format-Table -AutoSize | Out-String -Width 200
