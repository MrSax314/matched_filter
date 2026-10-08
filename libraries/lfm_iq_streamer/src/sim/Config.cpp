#include "radarsim/sim/Config.hpp"

#include "radarsim/protocol/Constants.hpp"
#include "radarsim/protocol/PacketFormat.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <ostream>
#include <stdexcept>

namespace radarsim {

// ---------------------------------------------------------------------------
// Derived waveform quantities
// ---------------------------------------------------------------------------
double WaveformConfig::chirpRate() const { return bandwidth_hz / pulse_width_s; }

double WaveformConfig::wavelength() const { return kSpeedOfLight / carrier_freq_hz; }

uint32_t WaveformConfig::samplesPerPri() const {
    return static_cast<uint32_t>(std::llround(pri_s * sample_rate_hz));
}

uint32_t WaveformConfig::samplesPerPulse() const {
    return static_cast<uint32_t>(std::ceil(pulse_width_s * sample_rate_hz - 1e-9));
}

double WaveformConfig::effectivePri() const { return samplesPerPri() / sample_rate_hz; }

double WaveformConfig::rangeResolution() const { return kSpeedOfLight / (2.0 * bandwidth_hz); }

double WaveformConfig::minimumRange() const { return kSpeedOfLight * pulse_width_s / 2.0; }

double WaveformConfig::unambiguousRange() const {
    return kSpeedOfLight * (effectivePri() - pulse_width_s) / 2.0;
}

double WaveformConfig::unambiguousVelocity() const {
    return wavelength() / (4.0 * effectivePri());
}

// ---------------------------------------------------------------------------
// INI parsing helpers
// ---------------------------------------------------------------------------
namespace {

std::string trim(const std::string& s) {
    const auto b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return {};
    const auto e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

[[noreturn]] void fail(const std::string& where, const std::string& msg) {
    throw std::runtime_error(where + ": " + msg);
}

double parseDouble(const std::string& v, const std::string& where) {
    try {
        std::size_t pos = 0;
        const double d = std::stod(v, &pos);
        if (pos == v.size() && std::isfinite(d)) return d;
    } catch (const std::exception&) {}
    fail(where, "expected a number, got '" + v + "'");
}

uint64_t parseUInt(const std::string& v, const std::string& where) {
    try {
        std::size_t pos = 0;
        if (!v.empty() && v.front() != '-') {
            const uint64_t u = std::stoull(v, &pos);
            if (pos == v.size()) return u;
        }
    } catch (const std::exception&) {}
    fail(where, "expected a non-negative integer, got '" + v + "'");
}

bool parseBool(const std::string& v, const std::string& where) {
    const std::string s = toLower(v);
    if (s == "true" || s == "1" || s == "yes" || s == "on") return true;
    if (s == "false" || s == "0" || s == "no" || s == "off") return false;
    fail(where, "expected true/false, got '" + v + "'");
}

void applyKey(SimConfig& cfg, const std::string& section, const std::string& key,
              const std::string& value, const std::string& where) {
    auto unknown = [&] { fail(where, "unknown key '" + key + "' in [" + section + "]"); };

    if (section == "network") {
        auto& n = cfg.network;
        if (key == "host") n.host = value;
        else if (key == "port") {
            const uint64_t p = parseUInt(value, where);
            if (p == 0 || p > 65535) fail(where, "port must be 1..65535");
            n.port = static_cast<uint16_t>(p);
        }
        else if (key == "samples_per_packet") {
            const uint64_t spp = parseUInt(value, where);
            if (spp > std::numeric_limits<uint32_t>::max()) fail(where, "samples_per_packet too large");
            n.samples_per_packet = static_cast<uint32_t>(spp);
        }
        else unknown();
    } else if (section == "waveform") {
        auto& w = cfg.waveform;
        if (key == "sample_rate_hz") w.sample_rate_hz = parseDouble(value, where);
        else if (key == "carrier_freq_hz") w.carrier_freq_hz = parseDouble(value, where);
        else if (key == "bandwidth_hz") w.bandwidth_hz = parseDouble(value, where);
        else if (key == "pulse_width_s") w.pulse_width_s = parseDouble(value, where);
        else if (key == "pri_s") w.pri_s = parseDouble(value, where);
        else unknown();
    } else if (section == "noise") {
        auto& n = cfg.noise;
        if (key == "enabled") n.enabled = parseBool(value, where);
        else if (key == "noise_power_db") n.noise_power_db = parseDouble(value, where);
        else if (key == "seed") n.seed = parseUInt(value, where);
        else unknown();
    } else if (section == "stream") {
        auto& s = cfg.stream;
        if (key == "realtime") s.realtime = parseBool(value, where);
        else if (key == "max_pulses") s.max_pulses = parseUInt(value, where);
        else if (key == "stats_interval_s") s.stats_interval_s = parseDouble(value, where);
        else unknown();
    } else if (section == "target") {
        auto& t = cfg.targets.back();
        if (key == "name") t.name = value;
        else if (key == "range_m") t.range_m = parseDouble(value, where);
        else if (key == "velocity_mps") t.velocity_mps = parseDouble(value, where);
        else if (key == "snr_db") t.snr_db = parseDouble(value, where);
        else unknown();
    } else {
        fail(where, "key '" + key + "' appears outside of any [section]");
    }
}

} // namespace

// ---------------------------------------------------------------------------
// SimConfig
// ---------------------------------------------------------------------------
SimConfig SimConfig::loadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open config file '" + path + "'");

    SimConfig   cfg;
    std::string section;
    std::string line;
    int         lineNo = 0;

    while (std::getline(in, line)) {
        ++lineNo;
        const auto comment = line.find_first_of("#;");
        if (comment != std::string::npos) line.erase(comment);
        line = trim(line);
        if (line.empty()) continue;

        const std::string where = path + ":" + std::to_string(lineNo);

        if (line.front() == '[') {
            if (line.back() != ']') fail(where, "malformed section header");
            section = toLower(trim(line.substr(1, line.size() - 2)));
            if (section == "target") {
                // Each [target] section starts a new target.
                cfg.targets.emplace_back();
                cfg.targets.back().name = "target" + std::to_string(cfg.targets.size());
            } else if (section != "network" && section != "waveform" && section != "noise" &&
                       section != "stream") {
                fail(where, "unknown section [" + section + "]");
            }
            continue;
        }

        const auto eq = line.find('=');
        if (eq == std::string::npos) fail(where, "expected 'key = value'");
        applyKey(cfg, section, toLower(trim(line.substr(0, eq))), trim(line.substr(eq + 1)), where);
    }

    cfg.validate();
    return cfg;
}

void SimConfig::validate() const {
    const auto& w = waveform;
    auto require = [](bool ok, const std::string& msg) {
        if (!ok) throw std::runtime_error("invalid config: " + msg);
    };

    require(w.sample_rate_hz > 0.0, "sample_rate_hz must be > 0");
    require(w.carrier_freq_hz > 0.0, "carrier_freq_hz must be > 0");
    require(w.bandwidth_hz > 0.0, "bandwidth_hz must be > 0");
    require(w.pulse_width_s > 0.0, "pulse_width_s must be > 0");
    require(w.pri_s > 0.0, "pri_s must be > 0");
    require(w.bandwidth_hz <= w.sample_rate_hz,
            "bandwidth_hz must be <= sample_rate_hz (complex Nyquist), otherwise the chirp aliases");
    require(w.pri_s * w.sample_rate_hz < std::numeric_limits<uint32_t>::max(),
            "pri_s * sample_rate_hz is too large");
    require(w.samplesPerPulse() >= 2, "pulse must span at least 2 samples");
    require(w.pulse_width_s < w.effectivePri(), "pulse_width_s must be shorter than pri_s");

    const std::size_t maxSamples =
        (packet::kMaxUdpPayload - packet::kHeaderSize) / packet::kBytesPerSample;
    require(network.samples_per_packet >= 1 && network.samples_per_packet <= maxSamples,
            "samples_per_packet must be 1.." + std::to_string(maxSamples));
    require(!network.host.empty(), "host must not be empty");

    require(stream.stats_interval_s >= 0.0, "stats_interval_s must be >= 0");

    for (const auto& t : targets) {
        require(t.range_m > 0.0, "target '" + t.name + "': range_m must be > 0");
    }
}

std::vector<std::string> SimConfig::warnings() const {
    std::vector<std::string> out;
    const auto& w = waveform;

    const std::size_t datagram =
        packet::kHeaderSize + network.samples_per_packet * packet::kBytesPerSample;
    if (datagram > 1472) {
        out.push_back("datagrams are " + std::to_string(datagram) +
                      " bytes; they will be IP-fragmented on a standard 1500-byte MTU "
                      "(fine on localhost, risky over a real network)");
    }
    if (targets.empty()) out.push_back("no [target] sections: stream will contain noise only");
    if (!noise.enabled) out.push_back("noise disabled: snr_db still sets echo amplitude relative to noise_power_db");

    for (const auto& t : targets) {
        const std::string tag = "target '" + t.name + "': ";
        if (t.range_m < w.minimumRange())
            out.push_back(tag + "range is inside the minimum range c*T/2 = " +
                          std::to_string(w.minimumRange()) + " m (echo overlaps the transmit pulse)");
        if (t.range_m > w.unambiguousRange())
            out.push_back(tag + "range beyond c*(PRI-T)/2 = " + std::to_string(w.unambiguousRange()) +
                          " m; echo will be truncated or missing");
        if (std::abs(t.velocity_mps) > w.unambiguousVelocity())
            out.push_back(tag + "|velocity| exceeds lambda*PRF/4 = " +
                          std::to_string(w.unambiguousVelocity()) + " m/s; Doppler will alias");
    }
    return out;
}

void SimConfig::print(std::ostream& os) const {
    const auto&  w       = waveform;
    const double tbp     = w.bandwidth_hz * w.pulse_width_s;
    const double mfGain  = linearToDb(static_cast<double>(w.samplesPerPulse()));
    const auto   flags   = os.flags();

    os << std::fixed << std::setprecision(3);
    os << "=== LFM IQ Streamer configuration ===\n"
       << "Destination        : " << network.host << ':' << network.port << "  ("
       << network.samples_per_packet << " samples/packet, "
       << packet::kHeaderSize + network.samples_per_packet * packet::kBytesPerSample << " bytes)\n"
       << "Sample rate        : " << w.sample_rate_hz / 1e6 << " MHz\n"
       << "Carrier            : " << w.carrier_freq_hz / 1e9 << " GHz  (lambda = " << w.wavelength()
       << " m)\n"
       << "Bandwidth / width  : " << w.bandwidth_hz / 1e6 << " MHz / " << w.pulse_width_s * 1e6
       << " us  (TBP = " << tbp << ", k = " << w.chirpRate() << " Hz/s)\n"
       << "PRI (effective)    : " << w.effectivePri() * 1e6 << " us  (" << w.samplesPerPri()
       << " samples, PRF = " << 1.0 / w.effectivePri() << " Hz)\n"
       << "Samples per pulse  : " << w.samplesPerPulse() << "  (MF gain ~ " << mfGain << " dB)\n"
       << "Range resolution   : " << w.rangeResolution() << " m\n"
       << "Range window       : " << w.minimumRange() << " .. " << w.unambiguousRange() << " m\n"
       << "Unambiguous vel.   : +/- " << w.unambiguousVelocity() << " m/s\n"
       << "Noise              : " << (noise.enabled ? "on" : "off") << ", " << noise.noise_power_db
       << " dB/sample\n"
       << "Stream             : " << (stream.realtime ? "real-time" : "as fast as possible") << ", "
       << (stream.max_pulses ? std::to_string(stream.max_pulses) + " pulses" : std::string("endless"))
       << "\n";

    for (const auto& t : targets) {
        const double tau = 2.0 * t.range_m / kSpeedOfLight;
        const double fd  = -2.0 * t.velocity_mps / w.wavelength();
        os << "Target '" << t.name << "'  R0 = " << t.range_m << " m, v = " << t.velocity_mps
           << " m/s, SNR = " << t.snr_db << " dB\n"
           << "    -> delay " << tau * 1e6 << " us (sample " << tau * w.sample_rate_hz
           << "), Doppler " << fd << " Hz, expected post-MF SNR ~ " << t.snr_db + mfGain << " dB\n";
    }
    os << "=====================================\n";
    os.flags(flags);
}

} // namespace radarsim
