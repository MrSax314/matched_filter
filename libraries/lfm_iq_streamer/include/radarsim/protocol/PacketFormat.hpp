#pragma once
//
// UDP wire format shared by the streamer and any receiver.
//
// Every datagram = 40-byte header + num_samples * (float32 I, float32 Q).
// All fields are LITTLE-ENDIAN regardless of host byte order.
//
//  offset  size  type     field
//  ------  ----  -------  -----------------------------------------------------------
//     0     4    u32      magic            0x464C5149 (bytes on the wire: 'I' 'Q' 'L' 'F')
//     4     2    u16      version          1
//     6     2    u16      header_size      40
//     8     8    u64      sequence         packet counter, +1 per datagram (detect drops)
//    16     8    u64      pulse_index      n, the pulse this packet belongs to
//    24     4    u32      sample_offset    index of first sample within the PRI (0 = TX instant)
//    28     4    u32      num_samples      complex samples in this packet
//    32     4    u32      samples_per_pri  total samples in one PRI
//    36     4    u32      flags            bit0 = first packet of pulse, bit1 = last packet of pulse
//    40   8*N    f32[2N]  payload          I0 Q0 I1 Q1 ...
//
#include <complex>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace radarsim::packet {

inline constexpr uint32_t    kMagic          = 0x464C5149u;
inline constexpr uint16_t    kVersion        = 1;
inline constexpr std::size_t kHeaderSize     = 40;
inline constexpr std::size_t kBytesPerSample = 8;     // float32 I + float32 Q
inline constexpr std::size_t kMaxUdpPayload  = 65507; // IPv4 UDP limit

enum Flags : uint32_t {
    kFirstInPulse = 1u << 0,
    kLastInPulse  = 1u << 1,
};

struct Header {
    uint64_t sequence        = 0;
    uint64_t pulse_index     = 0;
    uint32_t sample_offset   = 0;
    uint32_t num_samples     = 0;
    uint32_t samples_per_pri = 0;
    uint32_t flags           = 0;
};

/// Writes header + samples into `buffer` (resized as needed). Returns the datagram size.
std::size_t serialize(const Header& header, const std::complex<float>* samples,
                      std::vector<uint8_t>& buffer);

/// Parses a received datagram. Returns false if it is malformed or not from this protocol.
bool parse(const uint8_t* data, std::size_t size, Header& header,
           std::vector<std::complex<float>>& samples);

} // namespace radarsim::packet
