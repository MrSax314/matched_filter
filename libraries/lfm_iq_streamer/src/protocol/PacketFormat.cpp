#include "radarsim/protocol/PacketFormat.hpp"

#include <cstring>

namespace radarsim::packet {

namespace {

void putU16(uint8_t* p, uint16_t v) {
    p[0] = static_cast<uint8_t>(v);
    p[1] = static_cast<uint8_t>(v >> 8);
}

void putU32(uint8_t* p, uint32_t v) {
    for (int i = 0; i < 4; ++i) p[i] = static_cast<uint8_t>(v >> (8 * i));
}

void putU64(uint8_t* p, uint64_t v) {
    for (int i = 0; i < 8; ++i) p[i] = static_cast<uint8_t>(v >> (8 * i));
}

void putF32(uint8_t* p, float f) {
    uint32_t u;
    std::memcpy(&u, &f, sizeof u);
    putU32(p, u);
}

uint16_t getU16(const uint8_t* p) { return static_cast<uint16_t>(p[0] | (p[1] << 8)); }

uint32_t getU32(const uint8_t* p) {
    uint32_t v = 0;
    for (int i = 0; i < 4; ++i) v |= static_cast<uint32_t>(p[i]) << (8 * i);
    return v;
}

uint64_t getU64(const uint8_t* p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v |= static_cast<uint64_t>(p[i]) << (8 * i);
    return v;
}

float getF32(const uint8_t* p) {
    const uint32_t u = getU32(p);
    float          f;
    std::memcpy(&f, &u, sizeof f);
    return f;
}

} // namespace

std::size_t serialize(const Header& h, const std::complex<float>* samples, std::vector<uint8_t>& buffer) {
    const std::size_t total = kHeaderSize + static_cast<std::size_t>(h.num_samples) * kBytesPerSample;
    buffer.resize(total);

    uint8_t* p = buffer.data();
    putU32(p + 0, kMagic);
    putU16(p + 4, kVersion);
    putU16(p + 6, static_cast<uint16_t>(kHeaderSize));
    putU64(p + 8, h.sequence);
    putU64(p + 16, h.pulse_index);
    putU32(p + 24, h.sample_offset);
    putU32(p + 28, h.num_samples);
    putU32(p + 32, h.samples_per_pri);
    putU32(p + 36, h.flags);

    uint8_t* d = p + kHeaderSize;
    for (uint32_t i = 0; i < h.num_samples; ++i, d += kBytesPerSample) {
        putF32(d, samples[i].real());
        putF32(d + 4, samples[i].imag());
    }
    return total;
}

bool parse(const uint8_t* data, std::size_t size, Header& h, std::vector<std::complex<float>>& samples) {
    if (size < kHeaderSize || getU32(data) != kMagic || getU16(data + 4) != kVersion) return false;

    const std::size_t headerSize = getU16(data + 6);
    h.sequence        = getU64(data + 8);
    h.pulse_index     = getU64(data + 16);
    h.sample_offset   = getU32(data + 24);
    h.num_samples     = getU32(data + 28);
    h.samples_per_pri = getU32(data + 32);
    h.flags           = getU32(data + 36);

    if (headerSize < kHeaderSize ||
        size != headerSize + static_cast<std::size_t>(h.num_samples) * kBytesPerSample)
        return false;

    samples.resize(h.num_samples);
    const uint8_t* d = data + headerSize;
    for (uint32_t i = 0; i < h.num_samples; ++i, d += kBytesPerSample)
        samples[i] = {getF32(d), getF32(d + 4)};
    return true;
}

} // namespace radarsim::packet
