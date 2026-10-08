#pragma once
//
// Baseband LFM (linear frequency modulated) chirp, swept from -B/2 to +B/2:
//
//     s(t) = exp( j * 2*pi * ( -B/2 * t + (k/2) * t^2 ) ),   0 <= t < T,   k = B / T
//     s(t) = 0 otherwise
//
// Instantaneous frequency: f(t) = -B/2 + k*t.
// Matched filter: correlate received samples with reference(fs), i.e. h[n] = conj(s[-n]).
//
#include <complex>
#include <vector>

namespace radarsim {

class LfmWaveform {
public:
    LfmWaveform(double bandwidthHz, double pulseWidthS);

    /// Continuous-time complex envelope, t in seconds from the start of the pulse.
    std::complex<double> sample(double t) const;

    /// Sampled pulse s[n] = s(n / fs), n = 0 .. ceil(T*fs)-1. Use as the matched-filter reference.
    std::vector<std::complex<float>> reference(double sampleRateHz) const;

    double bandwidth() const;
    double pulseWidth() const;
    double chirpRate() const;

private:
    double bandwidth_;
    double pulseWidth_;
    double chirpRate_;
};

} // namespace radarsim
