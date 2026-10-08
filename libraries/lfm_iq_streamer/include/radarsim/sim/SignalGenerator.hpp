#pragma once

#include "radarsim/sim/Config.hpp"
#include "radarsim/protocol/LfmWaveform.hpp"

#include <complex>
#include <cstdint>
#include <random>
#include <vector>

namespace radarsim {

/// Produces the received baseband IQ for one PRI at a time: noise + delayed, Doppler-shifted
/// LFM echoes from every configured target. Sample 0 of each PRI is the transmit instant.
class SignalGenerator {
public:
    explicit SignalGenerator(const SimConfig& cfg);

    /// Fills `out` with samplesPerPri() samples for pulse `pulseIndex`.
    void generatePulse(uint64_t pulseIndex, std::vector<std::complex<float>>& out);

    uint32_t           samplesPerPri() const;
    const LfmWaveform& waveform() const;

private:
    void addNoise(std::vector<std::complex<float>>& out);
    void addTargetEcho(const TargetConfig& target, double amplitude, uint64_t pulseIndex,
                       std::vector<std::complex<float>>& out) const;

    SimConfig           cfg_;
    LfmWaveform         waveform_;
    uint32_t            samplesPerPri_;
    double              effectivePri_;
    float               noiseSigma_; // per I/Q component
    std::vector<double> amplitudes_; // per target

    std::mt19937_64                 rng_;
    std::normal_distribution<float> normal_{0.0f, 1.0f};
};

} // namespace radarsim
