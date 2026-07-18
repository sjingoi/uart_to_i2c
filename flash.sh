if [ $# -eq 0 ]; then
    echo "Usage: flash.sh [picotool path] [slave|master]"
    exit 1
fi

picotool=$1
binary=$2

sudo "${picotool}/build/picotool" load "./build/${binary}.elf" -fx