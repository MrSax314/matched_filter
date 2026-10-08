#include "radarsim/protocol/LfmWaveform.hpp"

#include "radarsim/protocol/Constants.hpp"

#include <cmath>

namespace radarsim {

LfmWaveform::LfmWaveform(double bandwidthHz, double pulseWidthS)
    : bandwidth_(bandwidthHz), pulseWidth_(pulseWidthS), chirpRate_(bandwidthHz / pulseWidthS) {}

std::complex<double> LfmWaveform::sample(double t) const {
    if (t < 0.0 || t >= pulseWidth_) return {0.0, 0.0};
    const double phase = kTwoPi * (-0.5 * bandwidth_ * t + 0.5 * chirpRate_ * t * t);
    return std::polar(1.0, phase);
}

std::vector<std::complex<float>> LfmWaveform::reference(double sampleRateHz) const {
    const auto n = static_cast<std::size_t>(std::ceil(pulseWidth_ * sampleRateHz - 1e-9));
    std::vector<std::complex<float>> out(n);
    for (std::size_t i = 0; i < n; ++i)
        out[i] = std::complex<float>(sample(static_cast<double>(i) / sampleRateHz));
    return out;
}

double LfmWaveform::bandwidth() const { return bandwidth_; }
double LfmWaveform::pulseWidth() const { return pulseWidth_; }
double LfmWaveform::chirpRate() const { return chirpRate_; }

} // namespace radarsim
