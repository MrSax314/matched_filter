#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

namespace radarsim {

/// Where and how the IQ stream is sent.
struct NetworkConfig {
    std::string host               = "127.0.0.1";
    uint16_t    port               = 50000;
    uint32_t    samples_per_packet = 128;   ///< complex samples per UDP datagram
};

/// LFM pulse train parameters.
struct WaveformConfig {
    double sample_rate_hz  = 2.0e6;   ///< complex (IQ) sample rate, fs
    double carrier_freq_hz = 3.0e9;   ///< RF carrier fc (drives Doppler and carrier phase)
    double bandwidth_hz    = 1.0e6;   ///< chirp sweep B (-B/2 .. +B/2)
    double pulse_width_s   = 50.0e-6; ///< pulse duration T
    double pri_s           = 500.0e-6;///< pulse repetition interval (rounded to whole samples)

    double   chirpRate() const;           ///< k = B / T  [Hz/s]
    double   wavelength() const;          ///< lambda = c / fc
    uint32_t samplesPerPri() const;       ///< round(PRI * fs)
    uint32_t samplesPerPulse() const;     ///< ceil(T * fs)
    double   effectivePri() const;        ///< samplesPerPri / fs (the PRI actually used)
    double   rangeResolution() const;     ///< c / (2B)
    double   minimumRange() const;        ///< c * T / 2  (echo overlaps the transmit pulse)
    double   unambiguousRange() const;    ///< c * (PRI - T) / 2  (whole echo fits in the PRI)
    double   unambiguousVelocity() const; ///< lambda * PRF / 4  (Doppler spans +/- this)
};

/// Receiver thermal noise model (complex white Gaussian).
struct NoiseConfig {
    bool     enabled        = true;
    double   noise_power_db = 0.0;   ///< complex noise power per sample, dB relative to 1.0
    uint64_t seed           = 12345; ///< 0 = seed from std::random_device
};

/// Pacing and diagnostics for the output stream.
struct StreamConfig {
    bool     realtime         = true; ///< pace packets to the sample rate
    uint64_t max_pulses       = 0;    ///< 0 = stream forever
    double   stats_interval_s = 1.0;  ///< 0 = no periodic stats
};

/// One point target (constant radial velocity).
struct TargetConfig {
    std::string name;
    double range_m      = 10000.0; ///< range at stream start (pulse 0)
    double velocity_mps = 0.0;     ///< radial velocity; positive = receding, negative = approaching
    double snr_db       = 0.0;     ///< per-sample SNR of the echo, before matched filtering
};

/// Full simulator configuration, loaded from an INI-style file.
struct SimConfig {
    NetworkConfig             network;
    WaveformConfig            waveform;
    NoiseConfig               noise;
    StreamConfig              stream;
    std::vector<TargetConfig> targets;

    /// Parses and validates a config file. Throws std::runtime_error on any problem.
    static SimConfig loadFromFile(const std::string& path);

    /// Throws std::runtime_error if the configuration cannot produce a valid stream.
    void validate() const;

    /// Non-fatal issues (target outside unambiguous range, IP fragmentation, ...).
    std::vector<std::string> warnings() const;

    /// Human-readable summary, including derived values useful for checking a receiver.
    void print(std::ostream& os) const;
};

} // namespace radarsim
