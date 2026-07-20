# AI Generated script for windows, untested.

param(
    [Parameter(Position = 0, Mandatory = $true)]
    [string]$PicotoolPath
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$binary = "master"
$picotoolExe = Join-Path $PicotoolPath "build/picotool"

if (-not (Test-Path $picotoolExe)) {
    throw "Could not find picotool at $picotoolExe"
}

Push-Location $repoRoot
try {
    & $picotoolExe load ".\build\$binary.elf" -fx
}
finally {
    Pop-Location
}
