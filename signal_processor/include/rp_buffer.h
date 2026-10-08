#pragma once
#include <stdint.h>

#include <cstddef>
#include <cstdint>
#include <cuda/std/complex>
#include <map>
#include <optional>
#include <queue>
#include <vector>

#include "message.h"

namespace matched_filter {

using CCFloat = cuda::std::complex<float>;

class ReceiveWindow {
  public:
	CCFloat* GetData() { return h_complex_data_; }

	// TODO(as3): Verify we are setting these at the right time
	void SetData(CCFloat* data) { h_complex_data_ = data; }
	void SetSize(const size_t& size) { size_ = size; }

	void AddFilledBytes(const size_t& bytes_filled) { filled_bytes_ += bytes_filled; }
	bool IsFull() { return filled_bytes_ >= size_; }

  private:
	CCFloat* h_complex_data_ = nullptr;
	size_t size_ = 0;

	size_t filled_bytes_ = 0;
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

	void StoreSamples(const IqPacketHeader* header, const uint8_t* iq_bytes);

  private:
	/**
	 * @brief Allocate pinned memory for receive window buffer
	 *
	 * @param rw_size Number of samples per receive window
	 */
	void InitializePinnedMemory(size_t rw_sample_ct);

	ReceiveWindow* GetNextAvailable(const uint64_t& pulse_index);
	ReceiveWindow* GetReceiveWindow(const int& vector_index) { return &data_.at(vector_index); }

	std::optional<size_t> IsCached(uint64_t pulse_index) {
		auto value = pulse_id_to_index_.find(pulse_index);
		if (value == pulse_id_to_index_.end()) {
			return std::nullopt;
		}
		return value->second;
	}

	std::vector<ReceiveWindow> data_;

	// TODO(as3): Verify push and pop at correct time for each of these
	//! Add available when processing complete
	//! Remove pulse id when data processing complete
	std::map<uint64_t, size_t> pulse_id_to_index_;
	std::queue<uint32_t> available_windows_;
};

}  // namespace matched_filter