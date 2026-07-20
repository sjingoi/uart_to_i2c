if [ $# -eq 0 ]; then
    echo "Usage: flash.sh [picotool path]"
    exit 1
fi

picotool=$1
binary="master"

sudo "${picotool}/build/picotool" load "./build/${binary}.elf" -fx