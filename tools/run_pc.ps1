# keflings-nx: one PC test run of app\out\pc-nx\renut.exe, minimised and muted, for <seconds>; the log is
# logs\<name>.log, the captures land next to the exe. Only this run's own process is stopped.
#   powershell -File tools\run_pc.ps1 <seconds> <name> [extra --cvar=value ...]
param([int]$Seconds = 60, [string]$Name = "pc_run", [Parameter(ValueFromRemainingArguments = $true)][string[]]$Extra)
$root = Split-Path -Parent $PSScriptRoot
$dir = Join-Path $root "app\out\pc-nx"
$log = Join-Path $root "logs\$Name.log"
if (Test-Path $log) { Remove-Item $log }
$cvarArgs = @("--log_file=$log", "--audio_mute=true", "--window_start_minimized=true")
if ($Extra) { $cvarArgs += @($Extra | Where-Object { $_ }) }
$p = Start-Process -FilePath (Join-Path $dir "renut.exe") -ArgumentList $cvarArgs -WorkingDirectory $dir -PassThru
$exited = $p.WaitForExit($Seconds * 1000)
if (-not $exited) { Stop-Process -Id $p.Id -Force }
"run $Name pid $($p.Id): " + $(if ($exited) { "exited with $($p.ExitCode)" } else { "stopped after $Seconds s" })
