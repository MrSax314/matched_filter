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

	/**
	 * @brief Construct a new RPBuffer object and allocate pinned memory
	 *
	 * @param rw_count Number of receive windows to allocate in buffer
	 * @param rw_sample_ct Number of samples per receive window
	 */
	explicit RPBuffer(int rw_count, size_t rw_sample_ct);
	~RPBuffer();
	ResourcePeriod GetNextAvailable(const int& buffer_size);

  private:
	/**
	 * @brief Allocate pinned memory for receive window buffer
	 *
	 * @param rw_size Number of samples per receive window
	 */
	void InitializePinnedMemory(size_t rw_sample_ct);

	std::vector<ResourcePeriod> data_;
	std::vector<uint8_t> rw_buffer_;
	int next_idx_;
};

}  // namespace matched_filter