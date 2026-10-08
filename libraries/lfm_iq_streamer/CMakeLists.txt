# =============================================================================
# LFM IQ Streamer
#
# Builds three targets:
#   radarsim::protocol  - wire format + LFM chirp. Link this from your listener.
#   radarsim::sim       - config, signal generation, UDP streaming.
#   lfm_iq_streamer     - the streamer executable (optional, see RADARSIM_BUILD_STREAMER).
#
# Works standalone (cmake -S . -B build) or from a parent project via add_subdirectory().
# =============================================================================
cmake_minimum_required(VERSION 3.16)
project(lfm_iq_streamer VERSION 1.1 LANGUAGES CXX)

# True when this is the project being built directly, false under add_subdirectory().
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(RADARSIM_IS_TOP_LEVEL ON)
else()
    set(RADARSIM_IS_TOP_LEVEL OFF)
endif()

option(RADARSIM_BUILD_STREAMER "Build the lfm_iq_streamer executable" ON)

# Only set global defaults when standalone; never override a parent project's choices.
if(RADARSIM_IS_TOP_LEVEL AND NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(Threads REQUIRED)

function(radarsim_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# -----------------------------------------------------------------------------
# Protocol library: no sockets, no config. Safe to share with any receiver.
# -----------------------------------------------------------------------------
add_library(radarsim_protocol STATIC
    src/protocol/Constants.cpp
    src/protocol/LfmWaveform.cpp
    src/protocol/PacketFormat.cpp
)
add_library(radarsim::protocol ALIAS radarsim_protocol)
target_include_directories(radarsim_protocol PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(radarsim_protocol PUBLIC cxx_std_17)
radarsim_enable_warnings(radarsim_protocol)

# -----------------------------------------------------------------------------
# Simulator library
# -----------------------------------------------------------------------------
add_library(radarsim_sim STATIC
    src/sim/Config.cpp
    src/sim/SignalGenerator.cpp
    src/sim/Streamer.cpp
    src/sim/UdpSender.cpp
)
add_library(radarsim::sim ALIAS radarsim_sim)
target_link_libraries(radarsim_sim PUBLIC radarsim::protocol PRIVATE Threads::Threads)
if(WIN32)
    target_link_libraries(radarsim_sim PRIVATE ws2_32)
endif()
radarsim_enable_warnings(radarsim_sim)

# -----------------------------------------------------------------------------
# Streamer executable
# -----------------------------------------------------------------------------
if(RADARSIM_BUILD_STREAMER)
    add_executable(lfm_iq_streamer app/main.cpp)
    target_link_libraries(lfm_iq_streamer PRIVATE radarsim::sim)
    radarsim_enable_warnings(lfm_iq_streamer)

    # Copy the default config next to the executable (wherever the generator puts it).
    add_custom_command(TARGET lfm_iq_streamer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:lfm_iq_streamer>/config
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/config/radar_sim.ini
                $<TARGET_FILE_DIR:lfm_iq_streamer>/config/radar_sim.ini
        COMMENT "Copying radar_sim.ini next to lfm_iq_streamer"
    )
endif()
