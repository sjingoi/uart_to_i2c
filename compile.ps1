# AI Generated script for windows, untested.

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Push-Location (Join-Path $repoRoot "build")
try {
    make
}
finally {
    Pop-Location
}
