# UART to I2C Converter Project

This project builds and flashes a Raspberry Pi Pico 2 firmware example that talks to an I2C camera device.

## Scripts

### init_workspace.sh / init_workspace.ps1
Initializes the workspace for a fresh Pico build.

What it does:
- copies the Pico SDK import file into the project root
- creates or refreshes the build directory
- runs CMake with the Pico 2 board target
- writes a VS Code settings file so IntelliSense can find the generated compile commands

Usage:
- Linux/macOS:
  ```bash
  ./init_workspace.sh /absolute/path/to/pico-sdk
  ```
- Windows PowerShell:
  ```powershell
  powershell -ExecutionPolicy Bypass -File .\init_workspace.ps1 "C:\path\to\pico-sdk"
  ```

### compile.sh / compile.ps1
Builds the firmware.

What it does:
- changes into the build directory
- runs make to compile the project

Usage:
- Linux/macOS:
  ```bash
  ./compile.sh
  ```
- Windows PowerShell:
  ```powershell
  powershell -File .\compile.ps1
  ```

### flash.sh / flash.ps1
Flashes the built firmware to the Pico.

What it does:
- uses picotool to load the generated ELF binary to the device

Usage:
- Linux/macOS:
  ```bash
  ./flash.sh /path/to/picotool
  ```
- Windows PowerShell:
  ```powershell
  powershell -File .\flash.ps1 "C:\path\to\picotool"
  ```

## Typical workflow

1. Run the workspace initializer with the path to your Pico SDK.
2. Build the project with the compile script.
3. Flash the resulting firmware with the flash script.

## Notes

- You must have the Raspberry Pi Pico SDK and toolchain installed.
- On Linux/macOS, the flash script may require sudo depending on your USB permissions.
- On Windows, PowerShell execution policy may block scripts; use the provided `-ExecutionPolicy Bypass` flag if needed.
