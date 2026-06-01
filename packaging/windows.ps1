<#
.SYNOPSIS
    Deploy CAMM3.exe with its Qt dependencies and zip the result.
.DESCRIPTION
    Runs windeployqt against build\app\CAMM3.exe, then compresses build\app into
    build\CAMM3-windows.zip.
.PARAMETER QtBinDir
    Optional path to the Qt bin dir (containing windeployqt.exe). If omitted,
    windeployqt is taken from PATH.
.EXAMPLE
    packaging\windows.ps1 -QtBinDir C:\Qt\6.8.2\msvc2022_64\bin
#>
param(
    [string]$QtBinDir = ""
)

$ErrorActionPreference = "Stop"

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$Exe      = Join-Path $RepoRoot "build\app\CAMM3.exe"
$AppDir   = Join-Path $RepoRoot "build\app"
$Zip      = Join-Path $RepoRoot "build\CAMM3-windows.zip"

if ($QtBinDir) {
    $WinDeployQt = Join-Path $QtBinDir "windeployqt.exe"
} else {
    $cmd = Get-Command windeployqt.exe -ErrorAction SilentlyContinue
    $WinDeployQt = if ($cmd) { $cmd.Source } else { "" }
}

if (-not $WinDeployQt -or -not (Test-Path $WinDeployQt)) {
    Write-Error "windeployqt.exe not found. Put the Qt bin dir on PATH or pass -QtBinDir."
}

if (-not (Test-Path $Exe)) {
    Write-Error "$Exe not found. Build the project first (see packaging/README.md)."
}

Write-Host "Deploying $Exe with $WinDeployQt"
& $WinDeployQt --release $Exe

if (Test-Path $Zip) { Remove-Item $Zip }
Write-Host "Zipping $AppDir -> $Zip"
Compress-Archive -Path (Join-Path $AppDir "*") -DestinationPath $Zip

Write-Host "Done: $Zip"
