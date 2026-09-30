// LFM IQ Streamer
// Simulates the baseband IQ output of a pulsed radar receiver (noise + LFM target echoes)
// and streams it over UDP.
//
// Usage:  lfm_iq_streamer [path/to/config.ini]
// With no argument, looks for config/radar_sim.ini in the working directory, then next to
// the executable (the build copies it there).

#include "radarsim/sim/Config.hpp"
#include "radarsim/sim/Streamer.hpp"

#include <csignal>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

volatile std::sig_atomic_t g_stopRequested = 0;

extern "C" void onSignal(int) { g_stopRequested = 1; }

std::string resolveConfigPath(int argc, char* argv[]) {
    namespace fs = std::filesystem;
    if (argc > 1) return argv[1];

    const fs::path relative = fs::path("config") / "radar_sim.ini";
    if (fs::exists(relative)) return relative.string();

    if (argc > 0) {
        std::error_code ec;
        const fs::path  besideExe = fs::absolute(argv[0], ec).parent_path() / relative;
        if (!ec && fs::exists(besideExe)) return besideExe.string();
    }
    return relative.string(); // not found: the loader reports a clear error
}

void printUsage(const char* exe) {
    std::cout << "Usage: " << exe << " [config.ini]\n"
              << "Streams simulated LFM radar IQ data over UDP. Press Ctrl+C to stop.\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc > 1 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
        printUsage(argv[0]);
        return 0;
    }

    try {
        const std::string configPath = resolveConfigPath(argc, argv);
        const auto        cfg        = radarsim::SimConfig::loadFromFile(configPath);

        std::cout << "Config: " << configPath << '\n';
        cfg.print(std::cout);
        for (const auto& w : cfg.warnings()) std::cerr << "WARNING: " << w << '\n';

        std::signal(SIGINT, onSignal);
        std::signal(SIGTERM, onSignal);

        radarsim::Streamer streamer(cfg);
        std::cout << "Streaming... press Ctrl+C to stop.\n";
        streamer.run([] { return g_stopRequested != 0; });
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
