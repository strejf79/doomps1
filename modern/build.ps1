param(
	[ValidateSet("Release", "Debug")]
	[string]$Configuration = "Release",
	[switch]$Clean
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Sdk = Join-Path $Root "toolchain\psn00bsdk\PSn00bSDK-0.24-win32"
$Toolchain = Join-Path $Sdk "lib\libpsn00b\cmake\sdk.cmake"
$BuildDir = Join-Path $Root "build\modern"
$Ninja = Join-Path $Sdk "bin\ninja.exe"

function Convert-ToMsysPath([string]$Path) {
	$full = [System.IO.Path]::GetFullPath($Path)
	if ($full -match '^([A-Za-z]):\\(.*)$') {
		return "/$($Matches[1].ToLower())/$($Matches[2] -replace '\\','/')"
	}
	return $full -replace '\\','/'
}

if (-not (Test-Path $Toolchain)) {
	throw "PSn00bSDK toolchain not found: $Toolchain"
}

if ($Clean -and (Test-Path $BuildDir)) {
	Remove-Item -Recurse -Force $BuildDir
}

$WingetCmake = Get-ChildItem `
	(Join-Path $env:LOCALAPPDATA "Microsoft\WinGet\Packages\Kitware.CMake_*") `
	-Recurse -Filter cmake.exe -ErrorAction SilentlyContinue |
	Select-Object -First 1 -ExpandProperty FullName
$Cmake = if ($WingetCmake) {
	$WingetCmake
} else {
	(Get-Command cmake -ErrorAction Stop).Source
}
$UseMsysPaths = $Cmake -match '[\\/]msys2[\\/]'

$SourceArg = if ($UseMsysPaths) { Convert-ToMsysPath $PSScriptRoot } else { $PSScriptRoot }
$BuildArg = if ($UseMsysPaths) { Convert-ToMsysPath $BuildDir } else { $BuildDir }
$ToolchainArg = if ($UseMsysPaths) { Convert-ToMsysPath $Toolchain } else { $Toolchain }
$NinjaArg = if ($UseMsysPaths) { Convert-ToMsysPath $Ninja } else { $Ninja }

& $Cmake -S $SourceArg -B $BuildArg `
	-G Ninja `
	"-DCMAKE_MAKE_PROGRAM=$NinjaArg" `
	"-DCMAKE_TOOLCHAIN_FILE=$ToolchainArg" `
	"-DCMAKE_BUILD_TYPE=$Configuration"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $Cmake --build $BuildArg
exit $LASTEXITCODE
