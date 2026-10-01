param(
	[ValidateSet("Smoke", "Title", "Regression")]
	[string]$Profile = "Regression",
	[int]$RunSeconds = 0
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Cue = Join-Path $Root "PSXDOOM\GAME\PSXDOOM.cue"
$OutputDir = Join-Path $Root "build"
$Settings = Join-Path $env:LOCALAPPDATA "DuckStation\settings.ini"
$DuckLog = Join-Path $env:LOCALAPPDATA "DuckStation\duckstation.log"
$SavedLog = Join-Path $OutputDir "duckstation-$($Profile.ToLower()).log"
$PathFile = Join-Path $PSScriptRoot ".duckstation-path"

function Set-IniValue {
	param([string]$Path, [string]$Section, [string]$Key, [string]$Value)

	$lines = if (Test-Path $Path) {
		[Collections.Generic.List[string]](Get-Content $Path)
	} else {
		[Collections.Generic.List[string]]::new()
	}
	$sectionLine = "[$Section]"
	$sectionIndex = $lines.IndexOf($sectionLine)
	if ($sectionIndex -lt 0) {
		$lines.Add("")
		$lines.Add($sectionLine)
		$lines.Add("$Key = $Value")
	} else {
		$nextSection = $lines.Count
		for ($i = $sectionIndex + 1; $i -lt $lines.Count; $i++) {
			if ($lines[$i] -match '^\[.+\]$') {
				$nextSection = $i
				break
			}
		}
		$keyIndex = -1
		for ($i = $sectionIndex + 1; $i -lt $nextSection; $i++) {
			if ($lines[$i] -match "^\s*$([regex]::Escape($Key))\s*=") {
				$keyIndex = $i
				break
			}
		}
		if ($keyIndex -ge 0) {
			$lines[$keyIndex] = "$Key = $Value"
		} else {
			$lines.Insert($nextSection, "$Key = $Value")
		}
	}
	[IO.File]::WriteAllLines($Path, $lines)
}

$configuredPath = if (Test-Path $PathFile) {
	(Get-Content $PathFile -Raw).Trim()
} else {
	$null
}
$candidates = @(@(
	$configuredPath,
	(Join-Path $env:LOCALAPPDATA "Programs\DuckStation\duckstation-qt-x64-ReleaseLTCG.exe"),
	(Join-Path $env:LOCALAPPDATA "DuckStation\duckstation-qt-x64-ReleaseLTCG.exe")
) | Where-Object { $_ -and (Test-Path $_) })
if (-not $candidates) {
	throw "DuckStation was not found. Put its full path in $PathFile."
}
$DuckStation = $candidates[0]

$profiles = @{
	Smoke = @{
		Seconds = 25
		Markers = @("SMOKE READY", "SMOKE PASS sustained-frames")
	}
	Title = @{
		Seconds = 60
		Markers = @("PORT PASS title-ready")
	}
	Regression = @{
		Seconds = 125
		Markers = @(
			"PORT PASS title-ready",
			"PORT PASS level-loaded",
			"PORT PASS playable-map",
			"PORT PASS demo-complete"
		)
	}
}
$selected = $profiles[$Profile]
if ($RunSeconds -le 0) { $RunSeconds = $selected.Seconds }

New-Item -ItemType Directory -Force $OutputDir | Out-Null
New-Item -ItemType Directory -Force (Split-Path -Parent $Settings) | Out-Null
Set-IniValue $Settings "BIOS" "TTYLogging" "true"
Set-IniValue $Settings "Logging" "LogToFile" "true"
Set-IniValue $Settings "Logging" "LogLevel" "Debug"
Set-IniValue $Settings "Logging" "LogFileTimestamps" "true"

Get-Process "duckstation-*" -ErrorAction SilentlyContinue | Stop-Process -Force
if (Test-Path $DuckLog) { Remove-Item -Force $DuckLog }

$process = Start-Process -FilePath $DuckStation `
	-ArgumentList @("-batch", "-fastboot", $Cue) -PassThru
$deadline = [DateTime]::UtcNow.AddSeconds($RunSeconds)
while (-not $process.HasExited -and [DateTime]::UtcNow -lt $deadline) {
	[Threading.Thread]::Sleep(1000)
	$process.Refresh()
}
if (-not $process.HasExited) {
	Stop-Process -Id $process.Id -Force
	$process.WaitForExit()
}

if (-not (Test-Path $DuckLog)) {
	throw "DuckStation did not produce a log."
}
Copy-Item -Force $DuckLog $SavedLog
$text = Get-Content $DuckLog -Raw
$missing = @($selected.Markers | Where-Object { $text -notmatch [regex]::Escape($_) })
$errors = [regex]::Matches(
	$text,
	'(?im)^.*(SMOKE FAIL|HandleUnknownGP0|CopyVRAM|Setting BFRD without|Unhandled exception|Fatal error).*$'
)

if ($missing.Count) {
	Write-Error "Missing milestones: $($missing -join ', '). Log: $SavedLog"
	exit 1
}
if ($errors.Count) {
	$errors | ForEach-Object { Write-Error $_.Value }
	exit 1
}

Write-Host "$Profile profile passed for $RunSeconds seconds. Log: $SavedLog"
