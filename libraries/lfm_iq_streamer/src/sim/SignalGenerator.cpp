#include "radarsim/sim/SignalGenerator.hpp"

#include "radarsim/protocol/Constants.hpp"

#include <algorithm>
#include <cmath>

namespace radarsim {

namespace {
uint64_t makeSeed(uint64_t configured) {
    if (configured != 0) return configured;
    std::random_device rd;
    return (static_cast<uint64_t>(rd()) << 32) ^ rd();
}
} // namespace

SignalGenerator::SignalGenerator(const SimConfig& cfg)
    : cfg_(cfg),
      waveform_(cfg.waveform.bandwidth_hz, cfg.waveform.pulse_width_s),
      samplesPerPri_(cfg.waveform.samplesPerPri()),
      effectivePri_(cfg.waveform.effectivePri()),
      noiseSigma_(static_cast<float>(std::sqrt(dbToLinear(cfg.noise.noise_power_db) / 2.0))),
      rng_(makeSeed(cfg.noise.seed)) {
    // Echo amplitude A such that A^2 / noise_power = SNR (per sample, before matched filtering).
    const double noisePower = dbToLinear(cfg.noise.noise_power_db);
    amplitudes_.reserve(cfg.targets.size());
    for (const auto& t : cfg.targets) amplitudes_.push_back(std::sqrt(noisePower * dbToLinear(t.snr_db)));
}

uint32_t SignalGenerator::samplesPerPri() const { return samplesPerPri_; }

const LfmWaveform& SignalGenerator::waveform() const { return waveform_; }

void SignalGenerator::generatePulse(uint64_t pulseIndex, std::vector<std::complex<float>>& out) {
    out.assign(samplesPerPri_, {0.0f, 0.0f});
    if (cfg_.noise.enabled) addNoise(out);
    for (std::size_t i = 0; i < cfg_.targets.size(); ++i)
        addTargetEcho(cfg_.targets[i], amplitudes_[i], pulseIndex, out);
}

void SignalGenerator::addNoise(std::vector<std::complex<float>>& out) {
    // Circular complex Gaussian: each component has variance noise_power / 2.
    for (auto& s : out) s = {noiseSigma_ * normal_(rng_), noiseSigma_ * normal_(rng_)};
}

void SignalGenerator::addTargetEcho(const TargetConfig& target, double amplitude, uint64_t pulseIndex,
                                    std::vector<std::complex<float>>& out) const {
    const auto&  w  = cfg_.waveform;
    const double fs = w.sample_rate_hz;

    // Slow time: range is updated once per pulse ("stop-and-hop" approximation).
    const double slowTime = static_cast<double>(pulseIndex) * effectivePri_;
    const double range    = target.range_m + target.velocity_mps * slowTime;
    if (range <= 0.0) return;

    const double tau = 2.0 * range / kSpeedOfLight;                 // round-trip delay
    const double fd  = -2.0 * target.velocity_mps / w.wavelength(); // Doppler shift

    // Carrier phase -2*pi*fc*tau. Its change from pulse to pulse is what a Doppler FFT measures.
    // Reduce modulo one cycle before scaling to keep full double precision.
    const double carrierPhase = -kTwoPi * std::fmod(w.carrier_freq_hz * tau, 1.0);

    // Only touch samples inside the echo window [tau, tau + T).
    const auto first = static_cast<int64_t>(std::ceil(tau * fs));
    const auto last  = std::min<int64_t>(samplesPerPri_,
                                         static_cast<int64_t>(std::ceil((tau + w.pulse_width_s) * fs)));

    for (int64_t m = first; m < last; ++m) {
        const double tEcho = static_cast<double>(m) / fs - tau; // time within the delayed pulse
        const double phase = carrierPhase + kTwoPi * fd * tEcho;
        const std::complex<double> echo = amplitude * waveform_.sample(tEcho) * std::polar(1.0, phase);
        out[static_cast<std::size_t>(m)] += std::complex<float>(echo);
    }
}

} // namespace radarsim
