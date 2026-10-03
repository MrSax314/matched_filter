#pragma once
#include <cuComplex.h>
#include <stdint.h>

#include <cstdint>
#include <vector>

namespace matched_filter {

class ReceiveWindow {
  public:
	cuComplex* GetData() { return h_complex_data_; }
	uint64_t GetPulseID() { return pulse_idx_; }

  private:
	cuComplex* h_complex_data_ = nullptr;
	uint64_t pulse_idx_ = 0;
	uint32_t next_free_idx_ = 0;
};

class RWBuffer {
  public:
	RWBuffer() = delete;
	RWBuffer(const RWBuffer& buff) = delete;
	RWBuffer(RWBuffer&& buff) = delete;

	/**
	 * @brief Construct a new RWBuffer object and allocate pinned memory
	 *
	 * @param rw_count Number of receive windows to allocate in buffer
	 * @param rw_sample_ct Number of samples per receive window
	 */
	explicit RWBuffer(int rw_count, size_t rw_sample_ct);
	~RWBuffer();
	ReceiveWindow GetNextAvailable(const int& buffer_size);

  private:
	/**
	 * @brief Allocate pinned memory for receive window buffer
	 *
	 * @param rw_size Number of samples per receive window
	 */
	void InitializePinnedMemory(size_t rw_sample_ct);

	std::vector<ReceiveWindow> data_;
	std::vector<uint8_t> rw_buffer_;
	int next_idx_;
};

}  // namespace matched_filter