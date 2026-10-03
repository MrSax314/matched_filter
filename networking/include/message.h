#include <cstdint>

#pragma pack(push, 1)
struct IqPacketHeader {		   // 40 bytes, little-endian
	uint32_t magic;			   // 0x464C5149 ('I','Q','L','F' on the wire)
	uint16_t version;		   // 1
	uint16_t header_size;	   // 40
	uint64_t sequence;		   // +1 per datagram (detect drops)
	uint64_t pulse_index;	   // pulse this packet belongs to
	uint32_t sample_offset;	   // first sample's index within the PRI (0 = TX instant)
	uint32_t num_samples;	   // complex samples in this packet
	uint32_t samples_per_pri;  // total samples in one PRI
	uint32_t flags;			   // bit0 = first in pulse, bit1 = last in pulse
};
#pragma pack(pop)

static_assert(sizeof(IqPacketHeader) == 40, "header must be 40 bytes");

struct IqSample {
	float i, q;
};	// payload: num_samples * 8 bytes, I0 Q0 I1 Q1 ...

constexpr uint32_t kIqMagic = 0x464C5149u;
constexpr uint16_t kIqVersion = 1;
constexpr uint32_t kFirstInPulse = 1u << 0;
constexpr uint32_t kLastInPulse = 1u << 1;
