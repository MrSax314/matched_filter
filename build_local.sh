# Strip WSL's Windows PATH entries (/mnt/*). find_library taking too long
LINUX_PATH=$(echo "$PATH" | tr ':' '\n' | grep -v '^/mnt/' | paste -sd:)

# Parse flags: -d for debug build
BUILD_TYPE=""
while getopts "d" opt; do
    case $opt in
        d) BUILD_TYPE="-DCMAKE_BUILD_TYPE=Debug" ;;
        *) echo "Usage: $0 [-d]" >&2; exit 1 ;;
    esac
done

# Configure cmake
PATH="$LINUX_PATH" cmake -S . -B build $BUILD_TYPE
# PATH="$LINUX_PATH" cmake -S . -B build --profiling-output=trace.json --profiling-format=google-trace

# Build project
cmake --build build