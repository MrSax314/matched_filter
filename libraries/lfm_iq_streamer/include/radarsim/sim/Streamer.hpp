#pragma once

#include "radarsim/sim/Config.hpp"
#include "radarsim/sim/SignalGenerator.hpp"
#include "radarsim/sim/UdpSender.hpp"

#include <functional>

namespace radarsim {

/// Generates pulses, packetizes them and streams them over UDP, paced to the sample rate.
class Streamer {
public:
    explicit Streamer(const SimConfig& cfg);

    /// Blocks until max_pulses is reached or stopRequested() returns true.
    void run(const std::function<bool()>& stopRequested);

private:
    SimConfig       cfg_;
    SignalGenerator generator_;
    UdpSender       sender_;
};

} // namespace radarsim
