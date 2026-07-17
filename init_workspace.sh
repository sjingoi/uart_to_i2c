if [ $# -eq 0 ]; then
    echo "Usage: init.sh [pico-sdk absolute path]"
    exit 1
fi

# Copy pico sdk import cmake file to current directory
cp "${1}/external/pico_sdk_import.cmake" pico_sdk_import.cmake

# Run cmake
mkdir -p build
cd build
PICO_SDK_PATH=$1 cmake .. --fresh -DPICO_BOARD=pico2
cd ..

# Setup vscode to see the build commands
mkdir -p .vscode
cat > ./.vscode/settings.json <<'EOF'
{
    "C_Cpp.default.compileCommands": [
        "${workspaceFolder}/build/compile_commands.json"
    ]
}
EOF
