#pragma once
#include <cuComplex.h>
#include <stdint.h>

#include <cstdint>
#include <vector>

namespace matched_filter {

class ResourcePeriod {
  public:
	cuComplex* GetData() { return h_complex_data_; }
	uint64_t GetPulseID() { return pulse_idx_; }

  private:
	cuComplex* h_complex_data_ = nullptr;
	uint64_t pulse_idx_ = 0;
	uint32_t next_free_idx_ = 0;
};

class RPBuffer {
  public:
	RPBuffer() = delete;
	RPBuffer(const RPBuffer& buff) = delete;
	RPBuffer(RPBuffer&& buff) = delete;

	explicit RPBuffer(int rp_count, size_t rp_size);
	~RPBuffer();
	ResourcePeriod GetNextAvailable(const int& buffer_size);

  private:
	void InitializePinnedMemory(size_t rp_size);

	std::vector<ResourcePeriod> data_;
	std::vector<uint8_t> buffer_;
	int next_idx_;
};

}  // namespace matched_filter