cmake_minimum_required(VERSION 3.24)

# Check for CUDA compiler
if(NOT DEFINED CMAKE_CUDA_COMPILER AND EXISTS "/usr/local/cuda/bin/nvcc")
    set(CMAKE_CUDA_COMPILER "/usr/local/cuda/bin/nvcc")
endif()

project(matched_filter LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Put CUDA include paths inline in compile_commands.json instead of in an
# nvcc --options-file, which clangd can't read.
set(CMAKE_CUDA_USE_RESPONSE_FILE_FOR_INCLUDES OFF)

# RTX 2070 == compute capability 7.5. Widen this list if you build on other
# GPUs later (e.g. "75;86;89").
if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES 75)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

find_package(CUDAToolkit REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable(matched_filter
    src/main.cpp
    signal_processor/src/app_config.cpp
    signal_processor/src/rp_buffer.cpp
    signal_processor/src/signal_processor.cpp
    networking/src/socket.cpp
    networking/src/udp_socket.cpp
)

target_include_directories(matched_filter PRIVATE
    ${CMAKE_SOURCE_DIR}/signal_processor/include
    ${CMAKE_SOURCE_DIR}/networking/include
    ${CMAKE_SOURCE_DIR}/cuda/include
)

target_link_libraries(matched_filter PRIVATE CUDA::cudart CUDA::cufft spdlog::spdlog)

set_target_properties(matched_filter PROPERTIES
    CUDA_SEPARABLE_COMPILATION OFF
)

add_subdirectory(libraries/lfm_iq_streamer)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()