# keflings-nx: one Ryujinx run of the NRO with this port's own data folder (docs/banjo-nx/porting-start-here.md).
# Starts only when no Ryujinx runs and the free commit memory is at least 12 GB; minimised; stops only its own process.
# The NRO goes to the SD's switch/keflings-nx/keflings-nx.nro (Ryujinx starts NROs without argv[0]: the SDK takes
# that place). Afterwards the run's logs and captures are copied to logs\emu_<name>\.
#   powershell -File tools\run_emulator.ps1 <seconds> <name> [nro]
param([int]$Seconds = 300, [string]$Name = "emu", [string]$Nro = "")
$root = Split-Path -Parent $PSScriptRoot
$data = "D:\New folder\ryu-data\keflings"
$sd = Join-Path $data "sdcard\switch\keflings-nx"
$ryu = "D:\New folder\Ryubing-src\ryubing\build\Ryujinx.exe"
if (-not $Nro) { $Nro = Join-Path $root "app\out\switch\renut.nro" }
if (Get-Process -Name Ryujinx -ErrorAction SilentlyContinue) { "Ryujinx is running (another port's run): not starting"; exit 2 }
$os = Get-CimInstance Win32_OperatingSystem
$freeCommitGB = [math]::Round($os.FreeVirtualMemory / 1MB, 1)
# 2026-10-30: 8 GB is enough for a Ryujinx run (the user's figure; the porting brief said 11-13).
if ($freeCommitGB -lt 8) { "free commit memory $freeCommitGB GB < 8 GB: not starting"; exit 3 }
Copy-Item $Nro (Join-Path $sd "keflings-nx.nro") -Force
foreach ($d in "logs", "capturas") { $p = Join-Path $sd $d; if (Test-Path $p) { Remove-Item -Recurse -Force $p } }
$env:DOTNET_ROOT = "D:\New folder\dotnet10"
$out = Join-Path $root "logs\emu_$Name"
New-Item -ItemType Directory -Force $out | Out-Null
$p = Start-Process -FilePath $ryu -ArgumentList @("-r", "`"$data`"", "`"$(Join-Path $sd 'keflings-nx.nro')`"") `
    -WindowStyle Minimized -PassThru -RedirectStandardOutput (Join-Path $out "stdout.txt") `
    -RedirectStandardError (Join-Path $out "stderr.txt")
"started Ryujinx pid $($p.Id) (free commit $freeCommitGB GB)"
$exited = $p.WaitForExit($Seconds * 1000)
if (-not $exited) { Stop-Process -Id $p.Id -Force; Start-Sleep -Seconds 2 }
"run ${Name}: " + $(if ($exited) { "exited with $($p.ExitCode)" } else { "stopped after $Seconds s" })
foreach ($d in "logs", "capturas") { $p2 = Join-Path $sd $d; if (Test-Path $p2) { Copy-Item -Recurse -Force $p2 $out } }
Get-ChildItem -Recurse $out | Select-Object FullName, Length | Format-Table -AutoSize | Out-String -Width 220
