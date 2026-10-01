# Strip WSL's Windows PATH entries (/mnt/*). find_library taking too long
LINUX_PATH=$(echo "$PATH" | tr ':' '\n' | grep -v '^/mnt/' | paste -sd:)

# Configure cmake
PATH="$LINUX_PATH" cmake -S . -B build
# PATH="$LINUX_PATH" cmake -S . -B build --profiling-output=trace.json --profiling-format=google-trace

# Build project
cmake --build build