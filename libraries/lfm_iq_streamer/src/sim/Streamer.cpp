#include "radarsim/sim/Streamer.hpp"

#include "radarsim/protocol/PacketFormat.hpp"

#include <algorithm>
#include <chrono>
#include <complex>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

namespace radarsim {

namespace {
using Clock = std::chrono::steady_clock;

struct Counters {
    uint64_t packets    = 0;
    uint64_t bytes      = 0;
    uint64_t sendErrors = 0;
};
} // namespace

Streamer::Streamer(const SimConfig& cfg)
    : cfg_(cfg), generator_(cfg), sender_(cfg.network.host, cfg.network.port) {}

void Streamer::run(const std::function<bool()>& stopRequested) {
    const uint32_t samplesPerPri    = generator_.samplesPerPri();
    const uint32_t samplesPerPacket = cfg_.network.samples_per_packet;
    const double   fs               = cfg_.waveform.sample_rate_hz;
    const auto     statsInterval    = std::chrono::duration<double>(cfg_.stream.stats_interval_s);

    std::vector<std::complex<float>> pulse;
    std::vector<uint8_t>             datagram;
    packet::Header                   header;
    header.samples_per_pri = samplesPerPri;

    Counters   total;
    Counters   atLastReport;
    uint64_t   samplesStreamed = 0; // stream time = samplesStreamed / fs
    const auto start           = Clock::now();
    auto       lastReport      = start;

    auto streamDeadline = [&](uint64_t samples) {
        return start + std::chrono::duration_cast<Clock::duration>(
                           std::chrono::duration<double>(static_cast<double>(samples) / fs));
    };

    for (uint64_t n = 0; !stopRequested() && (cfg_.stream.max_pulses == 0 || n < cfg_.stream.max_pulses);
         ++n) {
        generator_.generatePulse(n, pulse);

        for (uint32_t offset = 0; offset < samplesPerPri && !stopRequested(); offset += samplesPerPacket) {
            const uint32_t count = std::min(samplesPerPacket, samplesPerPri - offset);

            header.sequence      = total.packets + total.sendErrors;
            header.pulse_index   = n;
            header.sample_offset = offset;
            header.num_samples   = count;
            header.flags         = (offset == 0 ? packet::kFirstInPulse : 0u) |
                                   (offset + count == samplesPerPri ? packet::kLastInPulse : 0u);

            const std::size_t size = packet::serialize(header, pulse.data() + offset, datagram);

            // Like a real digitizer: a packet leaves once its last sample has been "acquired".
            samplesStreamed += count;
            if (cfg_.stream.realtime) std::this_thread::sleep_until(streamDeadline(samplesStreamed));

            if (sender_.send(datagram.data(), size)) {
                ++total.packets;
                total.bytes += size;
            } else {
                ++total.sendErrors;
            }
        }

        const auto now = Clock::now();
        if (statsInterval.count() > 0.0 && now - lastReport >= statsInterval) {
            const double dt = std::chrono::duration<double>(now - lastReport).count();
            std::cout << std::fixed << std::setprecision(1) << "[stats] pulse " << n + 1 << " | "
                      << (total.packets - atLastReport.packets) / dt << " pkt/s | "
                      << (total.bytes - atLastReport.bytes) / dt / 1e6 << " MB/s | send errors "
                      << total.sendErrors;
            if (cfg_.stream.realtime) {
                const double lagMs =
                    std::chrono::duration<double, std::milli>(now - streamDeadline(samplesStreamed)).count();
                std::cout << " | lag " << std::setprecision(2) << lagMs << " ms";
            }
            std::cout << '\n';
            atLastReport = total;
            lastReport   = now;
        }
    }

    std::cout << "Sent " << total.packets << " packets (" << total.bytes / 1e6 << " MB), "
              << total.sendErrors << " send errors.\n";
}

} // namespace radarsim
