# AI Generated script for windows, untested.

param(
    [Parameter(Position = 0, Mandatory = $true)]
    [string]$PicoSdkPath
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Push-Location $repoRoot
try {
    Copy-Item (Join-Path $PicoSdkPath "external/pico_sdk_import.cmake") -Destination ".\pico_sdk_import.cmake" -Force

    New-Item -ItemType Directory -Force -Path ".\build" | Out-Null
    Push-Location ".\build"
    try {
        $env:PICO_SDK_PATH = $PicoSdkPath
        cmake .. --fresh -DPICO_BOARD=pico2
    }
    finally {
        Pop-Location
    }

    New-Item -ItemType Directory -Force -Path ".\.vscode" | Out-Null
    if (-not (Test-Path ".\.vscode\settings.json")) {
        @'
{
  "C_Cpp.default.compileCommands": [
    "${workspaceFolder}/build/compile_commands.json"
  ]
}
'@ | Set-Content ".\.vscode\settings.json" -Encoding utf8
    }
}
finally {
    Pop-Location
}
