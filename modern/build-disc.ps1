param(
	[ValidateSet("Doom", "Smoke")]
	[string]$Target = "Doom",
	[int]$MaxPasses = 4,
	[switch]$Profile,
	[switch]$Clean
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$GameDir = Join-Path $Root "PSXDOOM\GAME"
$BuildDir = Join-Path $Root "build\modern"
$MkPsxIso = Join-Path $GameDir "mkpsxiso.exe"
$Project = Join-Path $GameDir "PSXDOOMCD.xml"
$MainExe = Join-Path $GameDir "MAIN.EXE"

& (Join-Path $PSScriptRoot "build.ps1") -Profile:$Profile -Clean:$Clean
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if ($Target -eq "Smoke") {
	Copy-Item -Force (Join-Path $BuildDir "psxdoom-smoke.exe") $MainExe
	Push-Location $GameDir
	try {
		& $MkPsxIso -y -q -nolimit $Project
		if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
	} finally {
		Pop-Location
	}
	Write-Host "Smoke disc ready: $(Join-Path $GameDir 'PSXDOOM.cue')"
	exit 0
}

$SourceC = Join-Path $Root "PSXDOOM\PSXCDABS.C"
$SourceH = Join-Path $Root "PSXDOOM\PSXCDABS.H"
$GeneratedC = Join-Path $GameDir "PSXCDABS.C"
$GeneratedH = Join-Path $GameDir "PSXCDABS.H"
$stable = $false

for ($pass = 1; $pass -le $MaxPasses; $pass++) {
	Copy-Item -Force (Join-Path $BuildDir "psxdoom.exe") $MainExe
	Push-Location $GameDir
	try {
		& $MkPsxIso -y -q -nolimit -makeabs $Project
		if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
	} finally {
		Pop-Location
	}

	$cStable = (Get-FileHash $SourceC).Hash -eq (Get-FileHash $GeneratedC).Hash
	$hStable = (Get-FileHash $SourceH).Hash -eq (Get-FileHash $GeneratedH).Hash
	if ($cStable -and $hStable) {
		$stable = $true
		Write-Host "LBA layout stable after pass $pass."
		break
	}

	Write-Host "LBA layout changed on pass $pass; relinking."
	Copy-Item -Force $GeneratedC $SourceC
	Copy-Item -Force $GeneratedH $SourceH
	& (Join-Path $PSScriptRoot "build.ps1") -Profile:$Profile
	if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

if (-not $stable) {
	throw "Disc LBA layout did not stabilize after $MaxPasses passes."
}

Write-Host "Doom disc ready: $(Join-Path $GameDir 'PSXDOOM.cue')"
